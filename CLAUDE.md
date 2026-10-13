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
  committing it. Work that agents do in a run is the exception: see "Work by
  several agents".
- Commit and push together. A commit is never left unpushed.

## History

- The history is meant to be read: one deliberate, self-contained commit per
  step, described accurately. Get a commit right before it is pushed rather
  than fixing it afterwards.

## The three trees

The source is three trees, and they are a story in that order.

- `src/asm/` is Volkov Commander 4.05 as its author published it, in
  assembly, changed only as far as building it with free tools took. Its
  files are CP866 text with CRLF line endings: edit them as bytes, and do
  not let an editor or a tool re-encode them or change line endings. The
  first commit holds them exactly as published, in `src/` itself, so
  `git diff $(git rev-list --max-parents=0 HEAD):src HEAD:src/asm` shows
  every change made since. The test `asm-bytes` builds this tree and holds
  it against the binaries its author shipped.
- `src/c/` is that program translated to C for DOS, the C translation: it
  behaves as the original does, bugs included, and every scenario holds
  it against the original. It is finished and frozen. It changes only to
  mend a place where it is found not to behave as the original, and then
  toward the original.
- `src/portable_c/` is the portable C: the program on its way from DOS to
  modern operating systems. It began as a copy of `src/c/`. This is where
  work goes on.

In prose the first is the assembly or the original source, the second the
C translation, the third the portable C. No tree is called "the port".
The tests are named by the tree they judge: `asm-bytes`, `c-data`,
`c-arith`, `c-scenarios`, and those of the portable C with `portable_c-`
in front.

## The C translation

These held while `src/c/` was made, and they say what it is.

- Fidelity means the same behaviour, bugs included. A bug of the original
  was ported as it is and reported.
- Its routines and data keep the names they have in the assembly.
- It has no assembly file: its code and its data are C. Where C could be
  written in more than one way, portable C came before C that is tied to
  this compiler or this machine, that before the compiler's intrinsics,
  and those before assembly, which is written inline in a C file and only
  where nothing else works. That a thing would be smaller or faster was no
  reason to go down that list. The README lists what rests on Open Watcom
  and every piece of inline assembly.
- The mouse is tested like the keys: the bench has one, and what the
  program does with it is held against the original in scenarios. An
  instruction that no scenario can reach is listed as out of reach with
  its reason.
- It is larger than the original and uses more memory, the part that
  stays in memory while a command runs included. No effort went into
  making it small.

## The portable C

- The translation of the program is complete and verified, on the user's
  word of 12 October 2026, so work for other operating systems may go in.
  It goes into `src/portable_c/` and nowhere else.
- What judges the portable C is what judged the C translation: the
  scenarios, with goldens recorded from the original. Where the portable C
  is to behave otherwise than the original, that is a deviation.
- Every deviation is declared, in `src/portable_c/DEVIATIONS.md`: what
  the original does, what the portable C does, why, and which scenarios
  show it. A scenario that a deviation changes gets a golden of its own,
  recorded from the portable C and read by whoever records it; such a
  golden is kept apart from those of the original and says where it is
  from, since it shows that nothing has changed, not that anything is
  right.
- A feature that only made sense on DOS is removed, not kept as a setting
  that does nothing. The first is Memory allocation in the Options menu:
  the portable C stays in memory whole while a command runs. A removal of
  that kind is declared and made without asking. Any other deviation, a
  fix of a bug of the original included, is asked first.
- A bug of the original stays as it is until its fix has been decided.
- DOS stays a target of the portable C for as long as DOS is where it is
  judged: every step builds for DOS and passes its scenarios there before
  it lands.
- The order holds here as it did for the translation, and it is now the
  point of the work: portable C first, then C tied to a compiler or a
  machine, then intrinsics, then assembly inline in a C file. What the
  README lists as resting on Open Watcom is to shrink, not grow.
- What the program asks of the system it is on, it asks in one place; the
  rest of it does not know which system that is. `dos.c` is that place
  today.
- Names may change here once a thing no longer is what its name in the
  assembly says. The C translation keeps the old names, and a comment at
  the new name says what it was.

## How the translation was made

The C translation was made a batch of routines at a time, each with
scenarios that ran every instruction of it in the original, and the
scenarios' test alone decided whether a batch was right. The history
tells it commit by commit, and the three sections below are what the
batches wrote down for each other: how the data went to C, what the
assembly left behind in the C, and what Open Watcom does with it. They
are notes, not rules, and they are kept because the portable C starts
from the same code.

When a scenarios' test fails where the change at hand cannot explain it,
derive the goldens again before anything else. If `git diff` then shows
them changed, it is the machine that differs, QEMU or DOS, and not the
program.

### Moving the data of a file to C

This is how the texts, boxes, dialogs, buttons and colours of the
assembly went to C, a file at a time. It is done, and `src/c/asm/` is
gone. What is here of the assembly, its
`PUBLIC` and `EXTERN` lines and its stubs, is of then; what is here of
the files of constants and of the test `c-data` holds for a constant that
is written or changed now. `VCHELP.INC` went first, into
`src/c/help_t.c`: read that file and `src/c/tables.h` before anything
else.

For one file, in this order:

1. Build, and run `python3 harness/datacheck.py build/shareware-layout
   build/c`. Under `assembly` it names every object of the file that
   has a name of the original, with its length. That is the list of what
   moves, and each is as the original now. Keep the output, in `build/`:
   step 5 holds the new one against it. To read the assembly, make a
   copy that an editor shows, `iconv -f cp866 src/c/asm/VCFIND.INC >
   build/VCFIND.txt`; change the file itself by a script that reads and
   writes bytes.
2. Write the file of constants, `src/c/NAME_t.c`, under the name that
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
     names something elsewhere is `NUMBER(n)`. What `src/c/tables.h`
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
   Then `c-scenarios` has to pass, two minutes.
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

What the test `c-data` goes by is the name: an object is held against the
label of the original that has its name, and a word that holds an
address there against where the translation has what that label names. What it
says when it fails, and what that means as a rule:

- "Name+N: the translation has XXh, the original YYh". Byte N of the object,
  counted from 0, is another. With 00h in the translation at the last byte, the
  string's zero is one that the assembly has not: give the array its
  length. With 00h behind a single byte, a structure is not packed. With
  everything from N on shifted, a byte is missing or one too many just
  before: an escape that ran on, a `BOX` that should be `BOX1`, a count
  of lines that is not the number of lines.
- "the translation has XXXXh, which is not where it has Y, ...: the original
  names Y there". The word names the wrong thing: two lines exchanged, a
  name of another box. A number where a name belongs shows like this
  too.
- "the original names Y there, which the translation has not by that name".
  Nothing in the translation is called Y: a name written otherwise in C, or Y
  left in neither place.
- "the original has no data called X". A constant of C whose name the
  original has not: a name mistyped, as a rule.
- "X is listed in datacheck.py, and the C code has no constant of that
  name". An entry of a table there that has outlived its constant.

Where the translation is meant to be otherwise, `harness/datacheck.py` has four
tables, each entry with its reason. A file of the assembly that moves as
it is needs no entry in any of them, and wanting one is a sign to look
again:

- `ELSEWHERE`, for a constant that the original has under another name
  or inside another object. It is there for the constants that code
  files had before the test was. A constant that moves keeps its name.
- `OWN`, for a constant that the original has not as data, which is
  then not compared. Right for a table that only the translation has, as
  `Fnc_T` is. The tables by which the stubs of the
  commands went on, `CmdNx_T`, `MnuNx_T` and `F35_T`, became `switch`
  statements and are no data.
  Wrong for anything that was in the published source: an entry here to
  quiet "no data called X" takes the object out of the test.
- `NAMED_OTHERWISE`, for a word that names something the translation has in
  another way: a routine that has become a number in a table of
  functions. Texts, boxes and buttons are never named otherwise.
- `DIFFERENT`, for an object that is meant to differ from the original.
  It is empty. An entry is behaviour other than the original's: stop and
  ask, as "A run of batches" has it.

A difference that the test reports is mended in the C. If the C is as
the assembly of the translation and the test still fails, the translation's assembly
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
  const CODE Fnc_T[]` in `src/c/dirfunc.c`, and what called through the
  word calls `Fnc_T[md.FncAddr](attributes)`. The type has the registers,
  `#pragma aux FncFn parm [ax] value [ax] modify exact [ax]`; a function
  that is far only by its own `#pragma aux` is cast to it, `(FncFn *)`,
  or it is "Pointer type mismatch". The stubs of the routines go, with
  the entry that called through the word, their lines in the `EXTERN`
  list and their functions in `harness/ported.py`. Routines that take
  different things, as the commands do, are called from a `switch`
  instead. The routine that a dialog names for Enter and F10 went that
  second way: the word in its header is a number, `SUB_` of
  `src/c/tables.h`, and `Dialog` has a `switch` that calls `F38_10`,
  `M30_60` or `F35_61`, each of which takes and hands back something
  else. The test `c-data` is told of each such word in `NAMED_OTHERWISE`,
  with "is" and the number.
- The linker's map has the static objects too, `option static`, marked
  with an `s`, and that is how the test finds a constant that is
  `static`.
- No code may come within 4K of the stack: `tools/mkexe.c` stops the
  build. The constants stand between, `CONST2`, which the linker's
  command has in front of the code.

What the agents found who moved the other files, each in ten to fifteen
minutes by the steps above, and what those steps did not say:

- `Var_S`, `Title_S` and `Tree_S` are declared far in `src/c/vc.h`, as
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
read too, and the translation has to have it there: the object is written as
long as what is read, with the original's bytes, and the test holds
them. The copy may be in another file than the text: `User_S` of
`src/c/keys_t.c` is copied as 80 bytes by Menu file edit, in
`src/c/commands.c`, and was found last for that.

- `python3 harness/reads.py build/c` lists every place where a file
  of code names a constant, with the bytes that are read of it there. A
  place that it knows by its form, a loop to a count, `MvzLine`,
  `MvsBw`, `MainFil`, a text read as far as its zero, a box handed on
  by its offset, needs nothing. Any other is in `REVIEWED` there, with
  the count and how it is known, by reading the code from where the
  constant is named to where the read stops, through the functions that
  it is handed to.
- The test `c-data` fails for an object that is shorter than what is read
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
  those is addresses of routines, or is not data of the translation at all,
  and no layout of the constants can make it the original's.

### What the assembly left in the C

The program was translated a few routines at a time, and what was still
assembly called what was C through stubs, a few instructions for each
routine that handed the registers over. There is no assembly code any
more, and no stub. What they have left is how the C is written: a
function has the registers that the assembly called it with, in its
`#pragma aux`, and takes of them what the routine reads. These are the
notes of the batches on what a reader of the C meets for that reason,
and on the part of the program that stays in the memory, the compiler
and the tests.

- ES is not always on the main data in the original: `ReDir` calls
  `FnFile` and `NearFl` with ES on the old list of files. So such
  functions take what they read as far pointers. A function that touches
  nothing of the main data, only far pointers and constants, needs no
  DS. A panel is its segment as a number, and `PANEL` of `src/c/vc.h`
  makes the pointer.
- A routine that hands back two words returns them as a `u32` in
  `value [dx ax]`; one that hands back more gets room on the stack and a
  `STACK` pointer to it: `AdrNum`, with `struct Place`. A routine that
  answers in a flag returns a number, and where it hands back a register
  too, the flag is in the high word.
- Code of the original that stands between two procedures belongs to
  none: the commands, and `PutTre` and `PutInf` behind `PutFls`.
  `harness/reach.py` and `harness/ported.py` take such code by its first
  label, as far as the next procedure, or by a later label of it for
  what stands from there on: such a stretch may hold the end of one file
  and the start of the next, as `M60_00` of `VCMENU.INC` runs on into
  `F37_00` of `VCF1.INC`.
- Where every jump inside such code ends up back at one place, the
  wait for the next key, each label that is jumped to is a function, with
  the registers that the jump carries as its arguments: a jump is a call
  and a return. `src/c/edit.c` has the editor's keys like that.
- A text or a table that is known by its offset is read at that
  offset in the segment that `AsmSeg` of `src/c/misc.c` returns:
  `AT(AsmSeg(), OFFSET(text))` costs no relocation, where naming the
  text as a far object does, four bytes of the file each time.
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
- What the viewer keeps of a file, `ViewDat`, lies in `TmpBuf` from
  its byte 200 on, and what the editor keeps, `EdData`, lies over it:
  `EdBase` is `ViBase`, `EdSize` is `ViSize`, `EdTxHex` is `ViTxHex`.
  `src/c/view.c` has the viewer's as a structure there. The text is in a
  segment of its own, which the original has in DS and the C has as a
  number, like a panel; the table of its lines is at the top of the same
  segment.
- A new scenario may reach what an earlier batch listed as out of
  reach, and the `c-scenarios` test then fails on the stale entry, "listed as
  out of reach, but executed": take it out of that batch's list.
  `harness/reach.py` shows it beforehand, as a routine with nothing
  left.
- A routine may read a panel through DS that no comment names: `PutPrm`
  takes the size of the panel in DS for where the prompt goes, and the
  viewer hands it the right panel's whichever is active.
- A register that a routine of the original stores without having
  set it, as `V32_00` does DX where nothing found is on the screen, is an
  argument of the function. That is less than showing that nothing reads
  what was stored, which was done for `V32_00`: see there.
- A caller may go on with a register as the routine happens to leave
  it, not only with what it returns: CX is 0 after `T70_00` and `T75_00`
  in the original, because a `REP` ended there, and `T21_00` and `T00_53`
  compare with CL as if they had cleared it. With `T75_00` the test
  showed it only by chance, in a status line drawn where the original
  leaves it: what went wrong was the count of free bytes, which is not
  compared.
- `DialBox` and `Dialog` take their box as an offset, where the
  original has it at CS:BP. C code has the offset of a constant as
  `(u16)(u32)(const void __far *)object`, `OFFSET` in most files, which
  compiles to the offset alone and costs no relocation.
- Nothing in the program tells the tests where it is. The driver of the
  tests (`harness/DRIVER.ASM`) watches it from inside the machine: it
  takes a call of `INT 28h` for the program's when it comes from the
  memory of the program that DOS has as the current one, and that is
  named as the file under test is. `LeaveTm` of `src/c/blank.c` makes that
  call, through `Int28` of `src/c/dos.c`, every time `Input0` has gone
  round, and `Blank` while it waits; whatever makes it has to do so with
  DS on the main data and on the program's own stack, as C code does. And the
  driver reads `AutoTim` and `Clock` in the main data, which
  `harness/driver.py` finds in the listing of the original,
  `build/shareware-layout/VC.LST`, for the translation too: its main data is
  laid out as the original's. The translation's stack it finds in the linker's
  map, `stack` of `harness/coverage.py`: it begins at `DataSav` of
  `src/c/resident.c` and is all of the segment `R_BSS`.
- A routine that jumps on by a table of keys, with `Case`, has the table
  as a `switch` in C. `Case` takes no key without a scan code, and an
  entry of the table without one takes the character with any.
- `Case` jumps with SI behind the entry of its table that it found,
  and what it jumps to may go on with that: `ChTr55` calls `DelTr0`,
  which takes SI for the address of a path in the main data. With the
  table a `switch`, the C hands on the address that the table has in the
  original. The listing has a table of `.DATA` by its offset in that
  segment only; where it is in the program shows in
  `build/shareware-layout/VC.COM`, by looking for its bytes.
- A command is no routine in the original: the main loop jumps to it
  by its table of keys and the menu bar by the tables of the menus, with
  the current panel in DS, and it jumps back to one of several places of
  the loop, or on into another command. In C it is a function that
  returns where the program goes on, and the main loop calls it from a
  `switch`: `Command` of `src/c/main.c`, with `CmdEnd`, `MnuEnd`, `F1Next`
  and `F35_90` there, which make of what a function returns what stood
  at the end of each family of commands in the original. "The commands"
  in `src/c/vc.h` declares them, with the places that each family
  returns. Where the place that a command goes on at takes registers, a
  panel in DS and a byte in CL or CH for `M01_01`, `M03_00` and `M06_00`,
  the function returns them too, in the high word and the high byte of
  the low one.
- A label inside a command may be an entry of its own, as inside a
  procedure: the menu comes in at `F17_01` and `F76_01`, the mouse at
  `F00_05`, `F01_05` and `F00_60`, with other registers than the key's
  entry has. Each is a function, and has a number of its own among the
  commands, a `CMD_` of `src/c/vc.h`.
- A routine may be called with DS somewhere that only happens to do:
  `F16_00` asks `BegBX` where a panel begins with DS on the main data,
  and takes the row, which is that of either panel.
- A label of the original names code between two procedures from
  there to the next procedure, not to the next command: `F22_00` runs to
  `F42_10` and holds `F80_00`, `F30_00` and `F32_00` as well. That is
  what a name in `routines` of `harness/ported.py` stands for.
- `Input` calls `Help` for F1, so every routine that takes keys can have
  the help on the stack above it, and the help takes its own keys with
  `MinMnu`, which calls `Input`: F1 in the help enters `Help` again,
  which returns at once because `HlpPage` is 0FFh then.
- A jump on a flag behind two calls may never be taken. `FndBack`
  has `JC` behind `MnMn40` to go back for the next key, and `MnMn40` ends
  in `Window`, which always leaves the carry clear. The C does what the
  assembly did then, not what it meant: it goes on to `FndMove` with
  what `MnMn40` left in AX, which the function returns.
- A routine that a command puts into `FncAddr`, as `F38_40` deletes
  and `M30_50` sets attributes, is called by `FncSel`, by `FncMas` and by
  `DirFunc`, with the attributes of a file, and answers 1, 0, or 100h to
  stop. `FncAddr` holds the number of the function, and `FncCall` of
  `src/c/vc.h` calls it: see "Moving the data of a file to C".
- A dialog may name a routine that `Dialog` calls when it is
  answered: `F38_10`, `M30_60` and `F35_61`. The word in its header is a
  number, a `SUB_` of `src/c/tables.h`, and `Dialog` calls the function
  from a `switch`.
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
  `INT 21h` itself is `Int21d` of `src/c/dos.c` in C, which has the flag
  clear when DOS is called: `Fcb29` of `src/c/dirfunc.c`.
- A box and a dialog are constants of a file of constants, and
  `DialBox`, `Dialog`, `MinMnu`, `MnMn10` and `MnMn40` take them as
  offsets in the segment of the constants, as the original has them at
  offsets in its one segment. The C reads them through far pointers made
  of the offset and of that segment: `AsmSeg`, and `AT` in
  `src/c/dialog.c`.
- Where the original has DS elsewhere than on the main data and the
  routine reads there, the function takes that segment as a number: the
  filter of a panel is a dialog whose fields are in the panel's segment.
  C code that means the main data passes `MAIN_SEG` of `src/c/vc.h`,
  which compiles to a copy of DS.
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
- Copy and Rename end where the commands that go through files do:
  at `FncSel`, `FncMas` or `FncExt`, or back in the loop. `F35_90` of
  `src/c/main.c` goes on from what they return, with the title for
  `FncMas` in the high word. The commands of `src/c/dirfunc.c` and the
  file search return a `TO_` of `src/c/vc.h`, and `F1Next` goes on from
  that; `F67_00` returns the panel for `F24_02` in the high word.
- A routine that reads through ES what its caller has at DS takes the
  wrong bytes where the two differ, and the C has to hand it the same
  wrong address: `ChgDisk` reads the path at ES:DX, and Go to of the
  file search calls it with DS on the list. Read what a routine takes
  its argument through, not only which register.
- The header of a dialog has its bytes in the order of `struct BOXHD`
  of `src/c/tables.h`, the key bar first: `DB 1, 0FFh, 32, 10h` of the
  original is key bar 1, no menu item, help page 32, flags.
- DOS called with `INT 21h` itself, where the original did not go
  through `Intr21`, is `Int21d` of `src/c/dos.c`, which takes `struct
  DosRegs` as `Intr21` does. `Int2F` is next to it.
- A routine that hands back the cursor of a line and the carry flag
  returns both as a number of 32 bits: `EdLin` has the column in the low
  word and 1 in the high one for the flag.
- A routine that reads DS, as `ClsLine` reads `CrntCol`, and that
  is once called with DS left elsewhere by a jump out of a loop, takes
  DS as a number: `ClsLine` has the segment as an argument, MAIN_SEG for
  the main data. `Run_00` leaves DS on the panel when the commands have
  no room for a name, and passes that.
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
  original leaves into `struct Regs` of `src/c/vc.h` now.
- The menu bar says which item was chosen and which table has its
  command, and `F39_01` of `src/c/main.c` makes of that the registers as
  the original has them at its jump to the command, all but BP, which no
  command reads, and finds the command in the tables of
  `src/c/commands_t.c`. Reading the commands for what they take was not
  enough while another batch had some in hand: Rename goes by BX and DX
  as they come, which showed only when its batch arrived.
- The program is in three parts: the part that goes while a command
  runs, which is the code and the constants; the main data, which the
  program asks DOS for; and the part that stays in the memory while a
  command runs, where the stack is, with `Counter`, `PrgList`, `RunPar`
  and the interrupt handlers. The original reaches that part through SS;
  C code gets its segment from `ResSeg` of `src/c/misc.c`, which is SS
  too, and the offset of an object there like that of any constant. What that part keeps is one
  structure, `struct Stays` of `src/c/vc.h`, and a member has no name to
  the linker: `#define Counter STAYS(Counter)` at the top of a file
  makes `OFFSET(Counter)` the member's place.
- The part that stays is the first five segments of the file, in the
  order that the linker's command has them in (`order` in
  `CMakeLists.txt`): `RTEXT`, its compiled code, which begins with the
  room of the PSP, `Psp` of `src/c/resident.c`; and the four that the
  compiler makes for the data of that file, `RCONST`, `RCONST2`,
  `R_DATA` and `R_BSS`, the last of which is the stack. The five are
  one group, `R_GROUP`, which begins at the PSP: every offset in that
  part counts from there. The part that goes begins behind them, on a
  paragraph, behind the paragraph of its block's header: both are in
  `TrnBeg` of `src/c/transit_t.c`, 32 bytes that the linker puts where
  the stack ends, and `tools/mkexe.c` works out where from them. It
  begins with the constants of the C code, `CONST2`, some 8,900 bytes
  of them: QEMU is many times slower at a write to a page that has code
  which it has run, the stack ends just in front, and with code there
  that ran at every key, a scenario of the cursor's keys took 19.7
  seconds where it takes 5.6. The constants are a group of their own,
  `DGROUP`, which begins where they do, not at the PSP: a word of a box
  that names a text is an offset in that group, and `AsmSeg` of
  `src/c/misc.c` is its segment. Where the two parts begin and end is
  not known before the program is linked: `TrnPrW` of `src/c/resident.c`
  is a word that `tools/mkexe.c` fills in.
- The compiled code of the part that stays is `src/c/resident.c`, a file
  of `port_c_resident` in `CMakeLists.txt`: compiled into `RTEXT` with
  `RESIDENT` defined. DS is the PSP's segment in all of it, not the main
  data, and `md` means nothing there. Its data is its own, static data
  of C, which `tools/mkexe.c` lets that file have; a place in the PSP
  is a near pointer made of its offset. Nothing of the
  part that goes can be called from it, since that is not in the memory
  while a command runs: `src/c/dos.c` is compiled a second time into
  `RTEXT`, as `dos_r.obj`, and `src/c/vc.h` names the functions of that
  copy with `_r` where the first has `_c`, by a macro in their `#pragma
  aux`, which the compiler expands. `Int21e` there calls DOS with ES as
  the structure has it, for a block of memory in ES.
- An `__interrupt` function of the compiler saves AX, CX, DX, BX, SP,
  BP, SI, DI, DS, ES and two words more, clears the direction flag and
  loads DS with its group, by a number that has to be relocated: in
  `RTEXT` the group is that of the part that stays, the PSP. Its parameters
  are what lies on the stack: a structure of those twelve words, and
  behind it one of three words for each return address that an
  interrupt has left, `struct Saved` and `struct Frame` of
  `src/c/resident.c`. Writing to them changes the registers that come
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
  `Left` of `src/c/resident.c`, with `Added`, `Taken` and `Doubled` for
  what `ADD`, `SUB` or `CMP`, and `SHL` leave of two numbers. A routine
  of that file that the assembly left with flags that go on to DOS
  returns them: `AddProg`.
- The `c-scenarios` test knows code that ran by its bytes in the file, and a
  stretch with a relocated word in it does not count as run, but for a
  far call or jump: an `__interrupt` function begins with one. List in
  `functions` of `harness/ported.py` the naked entry in front of it,
  `New21`, not the function.
- The part that goes is called from the part that stays as a far
  function: `TrnMain` of `src/c/main.c`, which `Stay` of
  `src/c/resident.c` calls through a pointer. A word of that part that
  holds a segment is set once, when the program is loaded, and the part
  that goes is somewhere else after every command. So a word of the
  part that stays which names a segment of the part that goes is left
  as the linker has it, in paragraphs from the PSP: `tools/mkexe.c`
  keeps it out of the table of relocations, and the code adds where
  that part is, `RunPar`: `TRN_SEG` of `src/c/resident.c`, which is the
  high word of a far pointer to the thing. `tools/mkexe.c` knows such a
  word by its number, which is not 0, the segment of the part that
  stays.
  `TrnMain` puts DS on the main data when there is some, and back before
  it returns. While that part came back through the assembly, it did so
  with every register changed, BP too, which the compiler takes to be
  kept: `Stay` keeps the pointer in memory, `TrnAdr`, and has no
  variables, since then.
- The program is entered at a C function, `Alloc0` of
  `src/c/resident.c`: the linker is told `option start=Alloc0`, and
  `tools/mkexe.c` takes the address from the linker's header. DOS enters
  it with DS on the PSP, which is what that file takes DS to be, and on
  the stack that the header names. The direction flag is as DOS leaves
  it: the first thing called is `IntOn`, which clears it. Until the
  relocations of the part that stays have been applied there, nothing
  may be called that needs one: an `__interrupt` or `__loadds` function,
  or a far function of another segment. A far call within `RTEXT` needs
  none, the linker makes `push cs` and a near call of it.
- The numbers that `tools/mkexe.c` fills in are seven words of
  `src/c/resident.c`, each of which it finds by its name in the linker's
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
  begins with the code. That is how the part that stays is one segment
  to the program, the PSP's.
- A layout at the number 0, `(*(struct MainDat __near *)0)`, is not the
  main data. Read and written through it, it is; but an address in it
  is a number to the compiler, and of a near pointer that is a number
  it makes a far one with the segment 0, not with DS, without a
  warning: `MvzLine(to, md.DosPth, ...)` would copy from the bottom of
  the memory. `__based(__segname("_DATA"))` in its place makes every
  address of a field far, "Pointer truncated" wherever one is kept in a
  near pointer. `md` is an `extern` object for that reason, and its
  place is the linker's to say: `#pragma aux md "Psp"` names it as the
  object that the program has at offset 0, the room of the PSP, and the
  linker takes the offset of that in its own group.
- The linker's command says the order of the segments: `order clname
  RCODE clname DATA segment CONST segment CONST2 clname CCODE`, classes
  and the segments within one, whatever the order of the object files.
  Its help does not list the directive for DOS; it works. Without it
  the linker places segments by their class, in the order in which it
  first meets each, and every object file of C names its code segment
  first. What a segment holds is in the order of the object files:
  `resident.c` and `transit_t.c` are linked first for that.
- A group may have segments of other groups, or of none, between its
  own: a group is where its first segment is, and a segment without
  contents counts for that. The linker has no word of its own for a
  group or for a class: both come from object files, so from the
  compiler's options.
- A word that holds a segment holds that of a group, where the thing
  named is of one, and the offset counts from there. So the test
  `data` holds a word that names something against its offset in its
  group, and `tools/mkexe.c` knows by the number of a segment which
  part is named: 0 is the part that stays, and nothing of the part
  that goes may name that.
- An `__interrupt` function of the part that goes has DS on the
  compiler's group when its first statement runs, that of the
  constants: its segment, where `SecDat` is, a variable of
  `src/c/main_t.c`. `FatErr` of `src/c/main.c`
  reads `SecDat` there as a near word and puts DS on the main data by
  `DataSeg`, a `#pragma aux` of one instruction, `mov ds,ax`, with
  nothing in its `modify` list: DS in a `parm` or a `modify` list is
  "Illegal register modified". Everything behind it is C as anywhere,
  `md` and the functions of other files.
- A function that does not return is `#pragma aux name aborts`; the
  compiler then ends the caller there.
- A far function of `src/c/resident.c` that the part that goes
  calls, through a pointer made of that part's segment and the
  function's offset, with DS anywhere, is `__far __loadds`: `SetBrk`, and
  `Release` for the map of the memory. Its first instructions load DS by
  a relocated number.
- Where the program has no stack, behind INT 2Eh, the instructions that
  set one are a function `__declspec(naked)` with `_asm`, which ends in
  a jump to a C function that does not return: `Int2E`. `offset
  DataSav + LenStk` there is the end of an array of the file, the
  stack. A parameter of a naked function that is only in
  its register wants `(void)name;`, or it is "not referenced".
- `_disable` and `_enable` are known only where a `#pragma intrinsic`
  names them, which is in `src/c/dos.c`: "No prototype found" elsewhere.
  Call `IntOff` and `IntOn`.
- A command may come to a place with DS on the main data where the
  place reads a panel: `M60_02` of the menus writes the prompt and tests
  `Visible`, and the configuration and EGA lines jump there after a
  `PUSH ES`, `POP DS`. The C takes the segment as a number, MAIN_SEG for
  those, and reads there what the assembly read.
- The BIOS called more than once with registers that go from one
  call to the next, as EGA lines calls it for the font and the mode,
  takes AX, BX, CX and DX from four words on the stack and puts them
  back: `Int10` of `src/c/dos.c`, with `struct BiosRegs` of `src/c/vc.h`.
  `Int10m` is another thing: it takes the mouse pointer off the screen
  around the call.
- Code that is entered by jumps at several labels, falls from one
  into the next, and leaves by jumps, never by a return, as the main loop
  does, is one function that takes the number of the label and returns
  where the program goes on: `Loop` of `src/c/main.c`, a `switch` whose
  cases fall into one another, in a loop for what jumps back. What goes
  round in registers from one command to the next is `struct Regs` of
  `src/c/vc.h`, which `Loop` and `Command` keep as the original has the
  registers wherever a command reads one: the top of `src/c/main.c` says
  which are read.
- The shared routines answer in numbers where the original answers
  in flags: `GetMous`, `InWind`, `DoublMo`, `TstBrk`, `TstDrv`, `SetPth`,
  `Fantom`, `ChgDisk` and `OpenFil` have 1 for a flag that is set, or the
  flag in the high word; `Int10m` returns DX in the low word and CX in
  the high one; `OncZoom` always takes the include line; `InvVar` takes
  a far pointer, since the original hands it a byte of a panel too.
- Registers that a routine leaves as the BIOS left them, and that
  a caller goes on with, come back in a structure on the stack: the code
  that starts the program takes the mode in AL, the columns in AH and
  the page in BH after `ChkMode`, which changes only the high byte of
  what the structure has for BX.
- The places by which a command says where the program goes on are
  in `src/c/vc.h`, a family each: `NEXT_`, `MNU_`, `TO_` and `GO_`. A name
  there is seen by every C file, so it must not be that of a routine:
  `NEXT_M01_01` is a place, `M01_01` the function. The four families are
  the original's, by the file that a command is in; nothing but that
  keeps them apart.
- A command that does not come back, as Enter does not when it runs
  something, returns all the same, `NEXT_EXEC` or `NEXT_INIT11`, and the
  main loop returns for it, to `TrnMain`, which returns to the part of
  the program that stays in the memory, or starts the program over. So
  the stack is as deep after a command as before it.
- A label in the middle of a command may be where the start of the
  program comes in: `Init45`, of the main loop, goes to `F21_31`, in
  Enter, to save the setup it has read where a command leaves it, and
  start over from there. That is a command of its own for the loop,
  `CMD_F21_31`.
- C code goes on after the main data has been given back to DOS, as
  the assembly did: `F21_31` frees it and then reads the copy in the
  part that stays, through far pointers made with `ResSeg`. DS stays
  where it was, and nothing has taken the memory yet.
- A register may reach the BIOS with what it was last used for: Enter
  has the BIOS write the end of a line with BX on the saved setup,
  where the BIOS takes a page and a colour. The C hands it the address
  that the original has there, from the listing of
  `build/shareware-layout/`, as for `DelTr0`.
- The start of the program runs before there is main data:
  `Init00` and `Init11` of `src/c/start.c` get the segment of the part
  that stays in the memory as a number, and read and write through far
  pointers only. A function that touches nothing of `md` can be entered
  with DS anywhere, and so can what it calls if that reads none either:
  `ChkMode`, `Cursor`, `Int10`, `Int21d`, `CutPath`, `MvzLine`, `AddFile`,
  `AsmSeg`, `ResSeg` and `MemRun` do not, `CaseMap`, `PathLin`, `ScrOffs`
  and `HookVct` do. `Init11` returns the segment of the main data that it
  has asked DOS for, `TrnMain` of `src/c/main.c` puts DS there, and the
  rest is a third function, `IniData`.
- A routine of the part that stays is called from the part that
  goes through a pointer to a far function, made of that part's segment
  and the routine's offset, with a `#pragma aux` on the type of the
  function that says which registers it changes: `StaysFn` in
  `src/c/start.c`, for `SetBrk`, and `ReleaseFn` in `src/c/memory.c`.
- A routine may know a thing by its offset alone. `MainPth` took the
  text of `NcPath` where it was to write at the offset of `TempDir`,
  whatever the segment, and its first call writes into another segment,
  at an offset 12h short of that in the original and 0Bh short in the
  translation. The C has an argument for it. Where the assembly compares an
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
  seemingly taken along. Take the bytes of a table from the listing of
  the original, `build/shareware-layout/VC.LST`, or hold the C against
  the assembly by a script. The `c-scenarios` test passed with
  the three bytes wrong: no screen shows the stars of the blanker. What
  showed it was the setup file that a program started on the defaults
  saves, held against the original's; `start-setup` ends with that
  file on its disk for this reason.

### C for the translation

`src/c/vc.h` says what the program asks of C code. What Open Watcom does
with it is less obvious:

- Where the program is right only because of what this compiler or its
  linker does, and the language promises nothing, is one list in the
  README, "What rests on Open Watcom", and every piece of inline
  assembly is another there, "Inline assembly". A place that is added
  to either gets its comment where it is done and its line in the list.
- A byte register in the `parm` list of `#pragma aux` is ignored without a
  warning, and the argument goes on the stack. Pass bytes in 16-bit
  registers and mask them in C.
- A 32-bit argument in a pair of registers does not arrive with its halves
  in the order written. Pass the halves as two 16-bit arguments.
- DS cannot carry an argument: a far pointer in `[ds si]` is refused.
  The segment comes in another register.
- A far pointer in a pair of general registers has its segment in DX
  whichever is written first: `[bx dx]` and `[dx bx]` both put the
  offset in BX. Read the call in the listing.
- A function that returns nothing may change AX all the same, whatever
  `modify exact` says. List AX there.
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
  `src/c/menu.c`. In the listing every access to a constant has `cs:`.
- Texts that follow one another in one array can be written as a macro
  of string literals, each ended by `\0`, with octal escapes for bytes
  that are no characters: `sizeof` of the macro, less one, is where the
  next one begins, and costs nothing in the program. See `MenuLn` there.
- The option `-nd` makes the compiler load a segment for every access to
  the main data. Do not use it for a file of the part that goes.
- The translation's stack is 4,096 bytes where the original's is 256, so stack
  use needs no care. The `c-scenarios` test reports how much each scenario
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
- The structures of `src/c/vc.h` are the assembly's, but a field that the
  assembly declares with `0 DUP`, a name for a place, is not in them:
  `TreList` is where `TreSum2` is, two bytes before the end of `DIRTREE`.
  Read the `STRUC` in `src/VC.ASM` before taking an offset from a
  C structure.
- There is no C library. String literals and static variables are refused
  by `tools/mkexe.c`, but for the data of `src/c/resident.c`, which has
  DS on it, and for a variable with a value in a file that has no code:
  `SecDat` of `src/c/main_t.c`, the one. A text may be written as a
  string where it gives an array declared `CODE` its contents: `static const u8 CODE Name_S[] =
  "text";` is in the code segment, with its zero byte. Constants that
  are named by their offsets, boxes and dialogs with their texts, are in
  files of their own, without `CODE`: see "Moving the data of a file to
  C".
- For 32-bit numbers the compiler calls helper routines, near, in its own
  code segment. `__U4M`, which multiplies, and `__U4D`, which divides
  unsigned numbers, are functions of `src/c/arith.c`, written without
  what they are there for; of `__U4D` the scenarios run only a division
  by 16 bits. They are held against Python's arithmetic by the
  test `c-arith`, `harness/arithcheck.py`, which runs their bytes on a
  model of the processor, and with `--pairs 200000` as many numbers as
  they were written against: do that after a change there or to the
  compiler. A helper
  that is missing shows as an undefined symbol when the translation is linked:
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
  was set: `*(u16 STACK *)&regs.name`, as `DX_OF` in `src/c/dirfunc.c`.
- `INC AX` and `CBW` on what DOS left in AL is
  `(u16)(signed char)(u8)(value + 1)`.
- A text without its zero byte is an array of its length:
  `static const u8 CODE Hist_S[9] = " History ";`.
- `far` and `near` are words of the compiler: a variable of that name
  is refused with "Invalid declarator", and every use of it with a
  dozen errors more. What `MvsBw` and `MvsColr` return is a far pointer;
  kept in a near one it is "Pointer truncated", which stops the build.
- Warnings stop the build, and a parameter that is not read is one. A
  function gets of the registers that the original has only those it
  reads:
  `parm [cx]` alone where the panel in AX is of no use to it.
- A far call from one of the two code segments to the other costs a
  relocation, four bytes of the file, where one within a segment costs
  none: `src/c/commands.c`, 2,479 bytes of code in CTEXT2 that calls the
  dialogs and the bars in CTEXT, made the program 2,848 bytes longer.
- The scratchpad directory of a session is one for all the agents that
  run side by side: a file there may be another agent's by the same
  name. Keep temporary files in the checkout's `build/`.
- A far pointer whose segment is the constant 0 is compiled as a near
  one, without a warning: the table of interrupt vectors was read from
  the main data. Take the segment from a `volatile` variable, as
  `F65_31` of `src/c/memory.c` does.
- CTEXT has under a thousand bytes left below its limit. A new file
  goes into CTEXT2, by its name in `port_c_second`, unless it multiplies
  or divides numbers of 32 bits.
- An address that the main data keeps as two words, offset and segment,
  `ErrVct`, is a `u32` there and a far pointer by a cast, which costs
  nothing: `(const u8 __far *)md.ErrVct`.
- DOS, the BIOS and the mouse driver are called in `src/c/dos.c` and
  nowhere else, and that file alone has inline assembly, but for the
  entries of the interrupts and INT 2Eh in `src/c/resident.c` and one
  instruction that loads DS in `src/c/main.c`: `Int21d` and
  `Int21f` for DOS, `Int10` and `TopView` for the screen, `Int16` for
  the keyboard, `Int2F`, `Int33` and `MousAt` for the mouse, and `Int28`
  and `DvPause`, by which the program says that it is idle, all
  declared in `src/c/vc.h`. Each is a line of C around a `#pragma aux`
  that holds the `INT` and as little else as will do: the registers are
  its `parm` and its `value`, and the compiler loads them. C code that
  needs a call of DOS fills a `struct DosRegs` and calls `Intr21` of
  `src/c/system.c`, which looks for DOS's critical error, or `Int21d`,
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
  through far pointers in plain C: `Timer` of `src/c/system.c`, `Window`
  of `src/c/window.c`. No entry of the assembly calls an interrupt for C
  code any more: a call of DOS that an entry made is a few lines around
  `Int21d` in the C file that wants it, as `Dos52` of `src/c/memory.c`.
- Calls of an interrupt between which no compiled code may run are one
  piece of inline assembly: DESQview is asked for a stack of its own,
  for a pause and for the program's stack back, and the compiler may use
  the stack between any two calls of C. `DvPause` of `src/c/dos.c`.
- A number of 16 bits times another, as a number of 32, is a call of
  `__U4M`, which a file of the second segment cannot reach. Where one
  factor is below 256 the product can be had in halves of 16 bits:
  `Below` of `src/c/blank.c`.
- A table for `Case` that has keys with their scan codes besides
  characters that count with any, as the main loop's had, is a `switch`
  on the key and then one on the character, if every character of the
  second kind stood behind the keys that have it too: Tab before ^I.
  `Func12` of `src/c/main.c`.
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
  aux` of its own: `CaseMapFn` in `src/c/dos.c`.
- A far pointer to a fixed place of the machine's memory is a constant
  with its segment: `(const u32 __far *)0x0040006CUL` is the BIOS's
  count of timer ticks, and costs no relocation.

## Notes on the portable C

What the work on `src/portable_c/` has written down for whoever goes on
with it. Like the notes above they are notes, not rules; the rules are
in "The portable C".

- The C translation is built by `add_c_tree` of `CMakeLists.txt`,
  which takes the tree's name, its model and the files of its second
  code segment: the notes above say `port_c_second` and `c_second` for
  that list, which is the end of the call now. The portable C is built
  by `add_portable_c`, which takes that list too.
  The harness takes the tree where it had `src/c` written in:
  `datacheck.py` and `reads.py` go by the name of the build's directory,
  `notice.py` by the file it is given, `play.py` has `--tree`. The
  tables of `datacheck.py` and `reads.py` are one for both trees: an
  entry that only one tree wants will need them told apart.
- The portable C is compiled in the medium model, `-mm`: a function is
  far unless it is static, by the compiler's rule and with no
  `#pragma aux`, and a pointer to a function is four bytes. A static
  function is called near, as before. The compiler calls `__U4M` and
  `__U4D` far then, with a relocation for each call from the second
  segment; a `__near` in their `#pragma aux` is taken without a word and
  the program links, and returns near to a far call. `_asm` takes the
  name of a function for a far place: `jmp Exec2E` in `Int2E` of
  `resident.c` is a far jump there.
- What the compiler does with a function that has no `#pragma aux`,
  read in the listings: the arguments come in AX, DX, BX and CX in
  their order, a far pointer in two of them, and what does not fit
  there on the stack; a value in AX or in DX:AX; a structure of four
  bytes or fewer in DX:AX as well, and a larger one through a pointer
  in SI to room in the caller's frame, which the function fills through
  SS, so it works with the stack outside the data. The function keeps
  every register that carries neither, but ES.
- The registers went from a function in every file at once. A file
  that calls a function of another often declares it for itself, with
  a line of its own, and the name went with the registers, `Name_`
  where it was `Name_c`: a declaration left behind is an undefined
  symbol at the linker. A line that kept the name and lost the
  registers in one file only would link, and call wrongly.
  `harness/ported.py` finds a function under either name.
- A pointer to a function has the convention of its type, and nothing
  holds that against the function at the linker: the type and every
  function called through it change together. `FncFn` of `dirfunc.c`
  and its five functions did; the casts in their table went, since the
  functions are of that type now.
- Before the registers of a function go, two things are worth knowing,
  and were looked at for all of them by a script: that every
  declaration of it is its definition word for word, and whether a
  call has arguments whose order of working out could matter, one that
  calls something next to one that calls or reads memory. There were
  20 such calls, and none where it matters.
- A local variable that is read before it is set holds what a register
  or the stack happened to, and by the compiler's convention that is
  another thing than by the registers of the assembly: no build says
  so, since Open Watcom does not warn of it, and a scenario only by
  chance. A compiler of this computer does warn. The tree was put
  through `clang -m32 -Wsometimes-uninitialized
  -Wconditional-uninitialized`, in a copy with the words of Open Watcom
  defined away, `__far`, `__based(x)` and the like, and the `LAID_OUT`
  lines taken out, all files but `resident.c` with its `_asm`: it named
  two variables, in `edit.c` and `read.c`, and both are set on every
  path that reads them. `gcc` at `-O2` named none, and none either in a
  function written to have one.
- A comment in front of the lines often said what they were for, "How
  the assembly passes arguments to these", "saves every register", and
  is stale when they go: look at what stood in front of each line
  taken out.
- What still has registers in a `#pragma aux`, and stays so until the
  thing itself changes: the inline assembly of `dos.c`; `__U4M` and
  `__U4D`, whose registers are the compiler's; `CaseMapFn`, DOS's
  routine; and `Int2E`, whose command is in SI.
- An object has the compiler's name in the map, `_Name`, where the C
  translation takes the underscore off with `#pragma aux Name "*"`:
  `harness/datacheck.py` knew an object by either, and no name of the
  portable C comes from a `#pragma aux` any more. A text that code reads
  as a far object and a file of constants names by a near word is
  declared `TEXT_FAR` in `vc.h`, which is `__far` but in a file that
  has `tables.h`.
- A function that returned two things as one number of 32 bits returns
  what it means: all of them do now, and the README has what became of
  the 114. Whatever is retyped next is changed so that a call which was
  not changed does not compile: a structure returned by value does
  that, since no shift or cast takes one apart, and costs nothing
  where it is two words. A value with a
  pointer for the second thing does too, by the count of arguments,
  but the pointer is to a local variable and wants `STACK`, which is
  this compiler's. What does not: the same function returning one of
  the two as a `u16`, which every old caller still compiles against,
  and reads as the low word.
- How a deviation goes through the tests, as the first one did. It
  is declared in `src/portable_c/DEVIATIONS.md` and then, for the
  harness, in three tables that each fail the test for an entry that
  no longer applies:
  - `TREES` of `harness/deviations.py`: the scenarios that it changes,
    each with what changes; those that are not run for the tree; and a
    step that is left out of every scenario for it, where the keys
    choose something that is gone. A program knows its tree by the
    directory of the build that its file is in.
  - `DECLARED` of `harness/datacheck.py`: a table of the program that
    differs, as the places where it departs from the original's, each
    with the bytes that the tree has there or none for bytes left out.
    The object is held against the original's with that done to it,
    so the rest of it is still held. `GONE` of `harness/reads.py` has
    a constant that the tree no longer has and that `REVIEWED` lists.
  - `GONE` of `harness/ported.py`: a function of a batch that the tree
    no longer has.
  Then the goldens: build, and `python3 harness/compare.py
  build/portable_c/VC.EXE` says which scenarios differ from the
  original's goldens, which is the list to declare, each looked at.
  `python3 harness/goldens.py record --tree portable_c` records those
  that are declared, and refuses any other; `python3 harness/goldens.py
  changes --tree portable_c` prints of every screen the rows that
  differ from the original's golden, which is how to read them: a
  change that was not meant shows there as a row that the declaration
  does not explain. A scenario whose steps choose what is gone needs
  the step left out, not a golden of the keys going astray.
- The portable C is one program, and the notes above on the part that
  stays in the memory, on `tools/mkexe.c` and on string literals and
  static variables are of the C translation alone. In the portable C DS
  is on the compiler's group, `DGROUP`, in every function, an interrupt
  function and a `__loadds` one included, since that is what the
  compiler loads; a file may have variables and strings, which are of
  that group; and `md` is a variable of `main.c`. The README has the
  layout, under "Staying in the memory whole". What making it so met:
  - `#pragma data_seg("NAME", "CLASS")` makes a segment of that name and
    class and puts it into the group that the file's data is of:
    `DGROUP`, or with `-nd=S` the group `S_GROUP`, and then under the
    name `SNAME`. An object written behind the line is of that segment.
    Nothing gives a segment an alignment: it is that of a word.
  - The linker takes a segment of the class `STACK` for the stack and
    has the file's header name its end, with SS on the paragraph that
    the segment begins in, which is the stack's group if it is one.
    Without such a segment the header has SS and SP 0, and the program
    would start on a stack laid over its own first bytes.
  - A group is as long as from its first segment to its last, whatever
    lies between, and the linker stops at 64K: the stack and the tables
    cannot be of `DGROUP` beside the main data, each is a group.
  - The file holds the program up to the last byte that has a value,
    with zeros for what has none in front of that: the main data, which
    has to be first, is 62.6 KB of zeros in the file. The linker's
    `segaddr`, which starts a class on a paragraph that the command
    names, breaks that: it wrote all of what followed into the file and
    a header that asked DOS for a megabyte.
  - A variable declared `CODE` and `static` has the compiler's name for
    data, and the data's test takes it for a constant of the original's
    name: `VARIABLES` of `harness/reads.py` has `Vect21`.
  - A text of the original's name may be a function's in another file:
    `Command`, "COMMAND.COM", and `Command` of `main.c`. It is declared
    in the file that reads it, not in `vc.h`.
  - DOS loads a program into the largest block that it has, not into
    the first that fits: COMMAND.COM, 73 KB on the bench, goes behind a
    program that has stayed, and the gap that an earlier one left in
    front of it is not used again. And FreeDOS's COMMAND.COM keeps its
    73 KB while a program runs from it. The bench has 500,128 bytes
    from where the program is loaded.
  - The program reads its panels in a block that it has just given back
    to DOS, to move them to the largest block that DOS then has
    (`TakeMem` of `start.c`): DOS writes nothing into a block that it
    is given back, and has it as the start of the largest if it lies in
    front of it.
  - What the tests know of a program's file they get from
    `harness/coverage.py`: `made_by_mkexe` tells the C translation's
    file from one that the linker wrote, by its header, and `origin` is
    what to add to an address of the linker's map for the place in the
    file. The limits that `tools/mkexe.c` holds the C translation's
    build to, for QEMU, `check_layout` of `harness/ported.py` holds the
    portable C to, at the start of the scenarios' test.

- What retyping the first 22 of those functions met, each of which
  the compiler took without a word:
  - A variable that several calls share, `pair` as a rule, is split
    when one of the calls returns a structure. Every call but one took
    its number apart on the next lines; `Dialog` did not: four branches
    set `pair` and one line behind them all read it. The branch whose
    call got a variable of its own no longer set `pair`, and the line
    read what was there before. Look at every read of the old variable
    that is left, not only at those behind the call that changed.
  - A call may take apart what the call before returned, in its own
    arguments: `pair = F34_40(HIGH(pair), 1)`. With two variables that
    is the first one's member, and the second one's compiles as well.
  - `clang` finds the first of the two as a variable that may be read
    before it is set, by the audit of the note above, when nothing set
    the variable earlier in the function; neither it nor the compiler
    finds the second. Both were found by reading every changed line.
  - A file may have a structure of the name already: `struct Mouse` of
    `menu.c`, which is `struct MenuMouse` now. That one the compiler
    says.
  - `Function(arguments).member` is taken, and reads well where only
    one of the two things is wanted. A call that ignores what is
    returned needs no change, and compiles.
  - A structure of two words comes back as the number did, in DX:AX:
    the program is a few bytes longer for the functions that make one,
    `Changed` and `Unchanged` of `input.c` in place of two macros.
  - Open Watcom does say "has been referenced but never assigned a
    value", W200, for a variable that nothing sets at all.
- What the nine of `edit.c` added to that:
  - Where every call that sets a shared variable returns the same
    structure, the variable keeps its name and takes the structure's
    type, and neither of the two mistakes can be made: a read that was
    not changed does not compile. `answer` of `T80_00` is set by three
    functions and read behind them all, and the three return
    `struct Answer`. Divide a variable only where its calls return
    different things.
  - A macro may make the number that such a function returns, and be
    taken apart in more places than the function: `ADDRESS_OF` made
    the address that `T62_00` returned, and 22 more. It went with the
    function, and is `AddressOf` with `struct Address`. Called where
    the macro was expanded, it made the program 304 bytes shorter.
  - Two functions that make a structure in the same way are one piece
    of code under two names in the listing: `EndsAt` and `Done`,
    `NoEnd` and `Failed` of `edit.c`.
- What the ten met that a file kept to itself:
  - Not every such function has two things to return: read every
    `return` of one before it gets a structure. `F36_04` of `copy.c`
    returned the same place of the loop from each, with nothing above
    it: it is the end of a command in the assembly, a jump. It returns
    nothing now, its caller says where the command goes on, and
    `return F36_04();` left as it was is refused. `MenuItem` of
    `usermenu.c` put into a number what `FndItem` returns as a
    structure: it is gone.
  - Where the calls that set a shared variable return structures of
    their own, each with the thing that a line behind them read, the
    variable goes, and that line: each branch of `Dialog` sets the key
    itself. Nothing but reading holds that every branch does, since
    the key is set earlier in the function and `clang` has nothing to
    say.
  - That an old call is refused can be tried: put it back in a copy of
    the file and compile the copy. Define `LOW` and `HIGH` in the copy
    first. A file loses them with its last number, and then the
    compiler refuses the macro it does not know, whatever the function
    returns.
- What the commands met, and what else comes back to the main loop,
  49 functions, while those of `enter.c` were still to do:
  - What a command of the keys returns, `struct Onward` of `vc.h`, is
    two bytes and a word, four bytes, and comes back in DX:AX as the
    number did, the bytes where they were: `To` of `keys.c`, which
    makes one of a place, is three instructions with its return, and
    `ToMenu` two. A structure of more than four bytes comes back
    through a pointer to room in the caller's frame; this one was kept
    to four for that.
  - A member that is a byte holds a byte, where the number took
    whatever was shifted into it: `table << 8 | held` of the menu bar
    has the bits of `held` above the eighth in `table`. The two are
    the same only where every value fits its byte, and that is read
    from every assignment to it, as for `F39_00` of `menu.c`.
  - A flag next to a place may be a place of its own: the editor was
    the viewer's place with 1 in the high word, and is `NEXT_EDIT`.
    The list of the places is an `enum`, so a place put in moves the
    numbers of those behind it, in every file that is compiled.
  - Where one place of a family takes a second thing, it has a
    function of its own that makes it, and the others one that takes
    the place alone: `Shows` and `To` of `find.c`, `Masked` and `Go` of
    `copy.c`. One function with an argument that is 0 at every call
    but one says less.
  - Each file makes its structures by static functions of its own, as
    `To` is in `keys.c` and in `view.c`: a macro cannot make one, and
    a function of another file is a far call, with a relocation from
    the other code segment.
  - While one file still returns the number, what takes the structure
    has a function that makes it of the number, as `OfEnter` of
    `main.c` was, and the calls of that file go through it. An old call
    is refused
    both ways then: a number where the structure is taken, and later a
    structure where `OfEnter` takes a number. Twelve lines put back as
    they were, one of each kind, were all refused, with the trial of
    the note above.
  - The variable that two calls of `Command` shared, `pair`, went, and
    each call has a variable of its structure: every read stood right
    behind its call there.
  - A number of 16 bits may have two things in it as well, a thing in
    each byte or a flag above something, and its type does not say so.
    `grep -n '^\(static \)\?u32 \w*(' src/portable_c/*.c` lists the
    functions that return 32 bits, which was all that `tools/pairs.py`
    did, gone since: three, each a number, `Timer`, `__U4M` and `Dos35`.
- What the last of them met, the commands of `enter.c` and three
  numbers of 16 bits, `T00_00` of `edit.c` and `Command` and `Func21` of
  `main.c`:
  - A function that `vc.h` has among the commands may be none.
    `F24_02` of `enter.c`, which Enter and the changes of directory
    end in and the main loop calls for the file search, returned one
    place from every return: it returns nothing, and `main.c` says the
    place of the loop itself there, without `CmdEnd`.
  - A flag above something is one thing or another, not two things.
    Where the two are of a kind they are one enumeration: the places of
    the loop and, behind them, what the loop is left for, the `END_` of
    `main.c`, where `LEAVE` was. The numbers of the second kind are
    others then, so look at whatever compares with them. Where they are
    of two kinds, a command of `vc.h` or a place of the loop, it is a
    structure with a member that says which: `struct Asked` there. An
    old line is refused either way, the name of the flag being gone.
  - A variable that the changed call shared with a call that stays a
    number, `what` of `V00_03`, is there still, and a read of it that
    was overlooked compiles: its three reads behind the editor's call
    were each changed by hand, and read again.
  - The trial of an old line, of the note above, is one command for a
    copy of the file next to copies of the two headers, the compiler
    being in `build/_deps/open_watcom-src`, which `WATCOM` is set to:
    `binl64/wcc -q -0 -mm -os -s -zu -zl -zls -wx -we -fo=x.obj` and
    the copy. Try the copy as it is first: it has to be taken.
- A build can be tested while the tree goes on changing: copy
  `build/portable_c/VC.EXE`, `VC.MAP` and the object files elsewhere
  in `build/`, and run `harness/datacheck.py` (at once: it reads the
  source), `harness/arithcheck.py` and `harness/ported.py` on the copy.
  The commit has to be made when the copy is. The copy is in a
  directory called `portable_c`, `build/copy/portable_c`: the harness
  knows the tree by that name, and holds a build of any other name to
  the original's goldens with nothing declared for it.
- For the audit with `clang`, of the note on local variables above:
  the `LAID_OUT` lines go from `tables.h` as well as from `vc.h`, and
  `-fsyntax-only -ffreestanding -Wno-everything` with the three
  warnings wanted, `-Wuninitialized` the third, leaves the two
  variables that are known and nothing else; `_asm` is defined away
  like the other words.
- What taking out four things met that the program did only for the PC
  and for the programs around it, and that show on no screen: TopView,
  DESQview, the turn given to Windows, and the wait for a display that
  shows snow (`src/portable_c/DEVIATIONS.md`, 2 to 5).
  - Such a removal wants nothing in the three tables of the harness, as
    long as no function that a batch lists in `functions`, no constant
    and no scenario goes or changes. What `harness/ported.py` lists as
    out of reach are instructions of the original, which stay listed;
    that list is also where to read that no scenario shows a thing: an
    instruction of it that the bench cannot get to is there, with the
    reason.
  - A feature is found by the byte that the main data keeps of it, in
    the assembly, `grep -a` over `src/asm` with all of its output, and
    not by its routine: `DesqVie` was read in three places, of which
    the pause in `LeaveTm` was one. The others were in `Input0` and in
    the editor, where the program does something otherwise under
    DESQview, and what it does without is what stays, unconditionally.
    The byte keeps its place in `struct MainDat`, with "of the
    original. Not set or read here" in its comment; a byte of the
    setup, Snow mode, is "kept as it is found", and its default stays
    in the table of `start.c`, so the setup file is the original's.
  - An instruction that stands with the feature may do more than the
    feature needs. `Window` and `MovScr` let the machine interrupt
    again, `STI`, behind what they copy with interrupts off for a
    display with snow, and do so whatever the display is; and DOS calls
    `FatErr` with interrupts off, whose box is put on the screen
    through `Window`. No scenario would have said whether the box
    needs that. The `IntOn` stayed, once behind each copy, with a
    comment that says why. Before one of a pair like that goes, look
    for who comes there in the other state.
  - Where a call of DOS or the BIOS goes, the registers that it left
    for the next call go too: read what the next call takes. `IniData`
    hands a structure of registers from call to call.
  - `Exit05` of `resident.c` is no routine for INT 5: it is where DOS
    goes on when it ends the program. The program does not take that
    interrupt. What it does for Print Screen is one call of the BIOS in
    EGA lines, `F69_00` of `commands.c`, which was taken out and put
    back: it is a part of that command, and goes or stays with it
    (`TODO.md`).
  - A thing that is half of a command which shows is not of this kind,
    however little of it shows: the command stays whole until it is
    decided.
  - The figures that follow the size of the file were set again by
    hand with every one of these, in three documents, and were a
    conflict in every commit at the rebase. They are in one place now:
    see the note on the figures below.
  - The builds were each tested as a copy while the next was made, by
    the note above, two or three at a time: the scenarios' test took
    five to six minutes each that way.
- A name that is declared in `vc.h` can change the program where
  nothing uses it. What has no value the compiler lays out in an order
  of its own, and of `SavePtr` and `Vect27` in `resident.c` that order
  went by how many names were declared in front of them: three to six
  typedefs more in `vc.h`, or as many prototypes, and the two changed
  places; one, two, seven or eight, and they did not. The tag of a
  structure does not count, nor a macro. So a change that is to leave
  the program as it was byte for byte, and declares something in a
  header, may not: hold the listings against those of the build
  before, `build/portable_c/*.lst`, and if `resident.lst` alone
  differs, in the order of two variables of `_BSS` and nothing else,
  it is this, and the program is as right as it was. `HOLDS` of `vc.h`
  is a tag for that reason.
- The test `portable_c-host` has gcc read the tree for a machine of 64
  bits and holds every file to its counts in `harness/hostcheck.txt`,
  by kind: the top of `harness/hostcheck.py` says what is counted, and
  `TODO.md` what each kind waits for. Using it and keeping it up:
  - It fails for a file that has another count than the record, fewer
    as well as more. Read what it names, `python3 harness/hostcheck.py
    --show FILE` for the lines, and if the change was to do that, write
    the record anew with `--record` and commit it with the change. The
    record's lines in the commit then say what the change did: a kind
    that went, a file that became clean. The test does not forbid a
    count that goes up, or a file that is clean no more: it has the
    change show in the commit.
  - After a rebase the record is written anew, not merged: the others
    have changed the files.
  - The kind `other` is what gcc says that `SAYS` of the script does
    not know. None is in the record. A file with a place of that kind
    usually does not parse: something of Open Watcom's that nothing
    takes out of gcc's sight, and gcc then says much about the lines
    that follow. Find the first of them with `--show other`.
  - The counts are gcc's, of any version: 13 and 15 agree place for
    place. clang counts otherwise, 480 numbers made pointers where gcc
    has 436 in the same source, and nothing for the address of a
    function kept as that of data, so the test does not ask it.
  - A cast that a macro makes is counted where the macro is used, so a
    file is not clean while it uses `PANEL` or `AT` of `vc.h`. A macro
    that is made right makes every file right that uses it, and
    `--macros` says which macros there are and how many places each
    makes.
  - What gcc cannot say, since it only reads, is in `TODO.md`: above
    all arithmetic that counts on an int of 16 bits.
- The figures of the portable C that change with every step, the
  length of its file and what follows from that, what it keeps of the
  memory, where its code segments end, and the counts of what only
  Open Watcom takes, are in one place: the table of the README under
  "The figures", which `python3 tools/figures.py --write` writes of the
  build and the source, between two marks. Run it when a piece of work
  lands, after the build and the rebase, and commit the README with the
  work; `--check` says whether the table is the build's. Write no such
  figure anywhere else, in the README, `TODO.md`, `DEVIATIONS.md` or
  these notes: say the thing without the number, or point to the
  table. A figure that is history, what a step cost when it was made,
  stays where it tells that, and a commit message keeps its own. A new
  figure is a row of the table, added in the script. Three things the
  script does not measure but takes: what the bench has for the
  program, its panels and a command together, 425,536 bytes, seen once
  on the Info panel as its top says; that the block of the panels is
  two panels of 256 files for a command; and the counts of `__far`,
  `STACK` and `CODE`, which are the host check's, by its function.
  What else that test counts is in its own table in `TODO.md`, which
  its `--record` flow goes with, and is not copied.
- What taking out the first feature that a user sees met, the map of
  the memory with the watch on DOS that it rested on
  (`src/portable_c/DEVIATIONS.md`, 7):
  - A line taken out of a menu moves every item below it, and an item
    is known by its number in more places than the tables that the
    data's test holds: the items that a menu shows dark are numbers in
    the code, `F39_58` of `menu.c`, and a command may be handed the
    number of its item. The data's test said nothing of those. `python3
    harness/goldens.py changes` did, as a minus in front of the wrong
    line, in the one scenario that shows that menu with the panels off:
    read every row of every screen that shows the menu, and `grep` the
    code for the menu's number. The tables that change with a line are
    in `DECLARED` of `harness/datacheck.py`, the eight of this one:
    among them where the lines and the pages of the next menu begin,
    two words of `MnuTab` that the test holds against a distance in the
    original.
  - A step that goes to the thing, with the steps that leave it again,
    is left out of one scenario by `left_out_of` of `TREES`: steps that
    follow one another there, since Esc is another key at the panels. A
    scenario whose whole point is the thing is not run.
  - What such a scenario had besides, which the tree still does and no
    other scenario shows, becomes a scenario of its own, for the
    original and both C trees: `asked`, which was a part of `stay`.
    `python3 harness/goldens.py derive NAME` derives the golden of a new
    scenario from the original in seconds, and both C trees have to
    pass it. A function of a batch that no scenario of a tree runs any
    more fails that tree's test, "not executed in the translation":
    either it is gone, and listed in `GONE` of `harness/ported.py`, or
    a scenario has to reach what is left of it.
  - A scenario that shows the Info panel on D: shows how much the file
    of the program under test leaves free there, which is another
    number for every tree.
  - What a program of a scenario prints of the registers may be an
    address in the program, once the program leaves another thing
    there than the original: it follows the layout and changes with
    every step of the work. `ended` prints DS:DX as the program leaves
    them, and `IGNORED` of `harness/deviations.py` has the two words
    left out for the line that only the portable C prints. Look for
    such a figure in every golden of the tree's own before it is
    recorded: it passes today and fails for the next agent.
  - What leaned on the watch and stays: the entry of INT 21h for the
    program's own call, with nothing of the watch in it; the prompt
    between two commands, which the original writes where the watch
    notes the interrupts, `PromptLine` of `resident.c`; and `SetStat`
    with 0 where the program begins. Read what a buffer or a routine of
    the thing is used for elsewhere before it goes: `grep` its name in
    the whole tree, the routine's and the buffer's.
  - A handler that no longer goes on in another handler needs no room
    on the stack for a second return address: `Int21` returns to
    whoever called, and `New21` jumps to it without the three
    instructions that made the room. `Exit01` still has them.

### Moving a file to the system's functions

What the program asks of the system is to be asked through
`src/portable_c/sys.h`, functions named for what is asked, with
`dos.c` as their one file for DOS: read the top of `sys.h` before
anything else. `system.c`, `window.c`, `mouse.c` and `input.c` went
first, with a few callers elsewhere for what those four do not have.
The other files still fill a `struct DosRegs` or a `struct BiosRegs`
and hand it to an interrupt, `Intr21`, `Int21d`, `Int10`, and these
notes are for whoever moves one of them. `python3 tools/calls.py`
lists what is left, by file.

For one function, in this order:

1. Find every call in it, and for each the service: the value last put
   in AX. The table below has the function for it.
2. For each call, read on from it to where each register is next
   written, and note every register that is read on the way, the
   function's own arguments to the next call included. That is what the
   call has to hand back, or what the next call has to be handed.
3. Write the calls, each with a variable for what it answers; `regs`
   goes when its last call has. Then read every line that still named
   `regs`.
4. Build, and run the tests: `portable_c-data` first, then
   `portable_c-scenarios`. No golden changes.

The calls of DOS, by the value in AH or AX. "Asked" is a call through
`Intr21` or the `CALL` that `copy.c` had, "bare" one through `Int21d` or
`Int21e`: the function has to be of the same kind, and where a service
is called both ways the bare function says so in its name.

| AH, AX | was | is |
|---|---|---|
| 06h, 09h | bare | `ConsoleChar(character)`, `ConsoleText(text)` |
| 0Eh | bare, `SelDrv` | `DriveSelect(drive)` |
| 13h | `Intr21f` | `FcbDelete(fcb, flagged)` |
| 19h | bare | `DriveCurrent()`, which is the low byte already |
| 1Ah | `SetDTA` | stays `SetDTA()`, which is `FindRecord(&md.DTA)` |
| 29h | bare | `FcbParse(name, fcb, how)` |
| 2Ch | `DosTime` | `TimeOfDay()` |
| 30h, 3306h | bare | `SystemVersion()`, `SystemTrueVersion()` |
| 3300h, 3301h | bare | `BreakCheck()`, `SetBreakCheck(on)` |
| 36h | `Intr21`, `Intr21f` | `DriveSpace(drive, flagged)` |
| 38h | bare | `CountryInfo(to)` |
| 39h, 3Ah, 3Bh | asked | `DirMake(name)`, `DirRemove(name)`, `DirChange(name)` |
| 3Ch | asked | `FileCreate(name, attributes)` |
| 3Dh | asked, bare | `FileOpen(name, mode)`, `FileOpenBare`; `OpenFil` stays what it is |
| 3Eh | asked, bare | `FileClose(handle)`, `FileCloseBare` |
| 3Fh | asked, bare | `FileRead(handle, to, count)`, `FileReadBare` |
| 40h | asked | `FileWrite(handle, from, count)` |
| 41h | asked | `FileDelete(name)` |
| 4200h to 4202h | asked | `FileSeek(handle, from, offset)`, with `from` 0 to 2 |
| 4300h, 4301h | asked | `FileAttributes(name)`, `FileSetAttributes(name, attributes)` |
| 4400h | asked | `FileDevice(handle)` |
| 4408h, 4409h | bare | `DriveFixed(drive)`, `DriveRemote(drive)` |
| 440Eh, 440Fh | bare | `DriveLetter(drive)`, `DriveSetLetter(drive)` |
| 47h | asked, bare | `DirCurrent(drive, to)`, `DirCurrentBare` |
| 48h, 49h, 4Ah | bare | `MemTake(paragraphs)`, `MemGive(block)`, `MemResize(block, paragraphs)` |
| 4B00h | asked, bare | `ProgramRun(name, block)`, `ProgramRunBare` |
| 4Ch | bare | `ProgramEnd(code)` |
| 4Eh, 4Fh | asked | `FindFirst(pattern, attributes)`, `FindNext()` |
| 56h | asked | `FileRename(from, to)` |
| 5700h, 5701h | asked | `FileTime(handle)`, `FileSetTime(handle, time, date)` |
| 60h | asked | `FileTrueName(name, to)` |
| 25h, 35h | bare | `SetVector(number, to)`, `Vector(number)`, of `vc.h` and not of `sys.h` |

The BIOS for the screen, `Int10`, by AH: 0Fh is `ScreenState()`, 0
`ScreenSetMode(mode)`, 1 `CursorShape(shape)`, 2 `CursorTo(page,
place)`, 3 `CursorWhere(page)`, 6 and 7 `ScreenScroll(down, lines,
color, upper, lower)`, 8 `ScreenCell(page)`, 9 `ScreenFill(page,
character, color, count)`, 0Eh `ScreenType(character, page, color)`, 12h
with 10h in BL `ScreenEga()`, 1130h `ScreenFont()`, 1112h
`ScreenSmallFont()`. For the keyboard, `Int16`: 0 is `KeyRead()`, 100h
`KeyWaiting()`, 200h `KeysHeld()`, which is the low byte already. The
mouse is done. `Int2F` with 150Bh is `DriveCd(drive)`, `Int28` is
`Idle()`. `Timer`, `Beep`, `SetDTA`, `OpenFil` and the others of
`system.c` and `window.c` are the program's routines as before, and
are called as before; `Int10m`, which scrolled a window by a line with
the mouse hidden, is `Scroll` there.

What is not in the table stays as it is, on `Int21d` and `Int21e`, and
`struct DosRegs` stays in `vc.h` for it: what is of DOS alone and is
likely to go, `SetStat` and the PSP in `Alloc0` of `resident.c`, with
the calls 51h and 58h. The map of the memory and the watch on DOS,
which were of that kind, with 52h, 62h and 4452h, are gone. So does the
BIOS's 12h with 20h in BL, by
which the program has the BIOS print a screen of more lines than 25:
it is of the PC alone, as TopView, DESQview and the wait for snow
were, which are gone.

What a call hands back, and where the traps are:

- AX is `value`, also where the call did not fail: `DirFunc` hands the
  high byte of it on with a file's attributes, `F38_40` reports it,
  `DelTr0` returns it. Whether DOS set the carry flag is `failed`. A
  line that read `regs.ax` behind `if (Intr21(...))` reads `.value` of
  the same answer.
- A register that a call answers in is a member with a name: the
  attributes, the time and the date, what a handle is, the clusters.
  If the call failed, the member is what `dos.c` put in the register,
  0. The original went on with what its caller had there, since DOS
  leaves a register alone that it has nothing to say in. Where a line
  reads such a register without having looked at the flag, write down
  what was in it: `M30_50` of `dirfunc.c` does, for the time and the
  date of a file that DOS would not tell.
- A register that a call does not answer in holds after it what it
  held before, also behind an error that `Intr21` asked DOS about. So
  what was read out of `regs` three calls later is a variable of the
  caller's now, and the call in between does not touch it: CX from
  4300h to 4301h is `attributes`, BX is the handle.
- A register that the service does not take was whatever the code
  before left there: AL of most calls, `0x3E00 | regs.ax & 0xFF`, the
  half of BX that a call does not read, CX and DX behind a call that
  set them, DS where no address goes. The functions hand over what is
  asked and nothing else, 0 in AL and the main data in DS. That is the
  program doing otherwise than the original, though no screen shows
  it, and it is declared: `src/portable_c/DEVIATIONS.md` has one entry
  for all of it, "DOS and the BIOS are handed nothing that they are not
  asked", with a table of the places. Every leftover that a file drops
  as it goes over gets a row there, in the same commit: the routine,
  the call, what the original has in the register and what the
  portable C has. Find them by step 2: a register that is written, or
  left as a call answered, and that the next call does not take.
- Before a leftover is dropped, be sure that the service does not read
  it, from what is written of DOS and of the BIOS, not from the bench
  passing. Three are known to be read by something, and the entry says
  what the functions do about each: AL of 4Eh, which DOS's APPEND goes
  by, in `read.c` twice and in `treebox.c`; AL of the BIOS's call 1,
  where some BIOSes want the video mode, in `IniData` and in `Blank`;
  and CX and DL of 1130h, which a BIOS that does not know the call
  leaves as they came, in `F69_00` and in `IniData`. One that you are
  not sure of is not dropped: hand the work back with it, as "Work by
  several agents" has it for a deviation, and the session asks.
- What the BIOS is asked in comes by name. The page that is shown goes
  from `ScreenState` to the calls that take one, where the assembly
  kept BH from the call before. `ScreenType` takes the page and the
  colour that the BIOS is handed, and a caller that handed it what a
  register happened to hold, as Enter does with `COMNINI_WAS`, hands
  it the two bytes of that.
- The carry flag as it was before the call: `FcbDelete` and
  `DriveSpace` take it as `flagged`, since DOS leaves it alone at
  those two and `Intr21` takes a set flag for an error. `F38_70` has it
  from comparing two characters, `C20_00` of `copy.c` from a
  subtraction.
- A call that took something at ES takes a pointer: the new name of
  `FileRename`, where `FileTrueName` writes, the block of `ProgramRun`,
  the block of the memory of `MemGive` and `MemResize`. A near pointer
  of the main data goes in as it is.
- `DirCurrent` takes where the path goes as one pointer, where the
  call had the segment with DX and the offset in SI: `regs.name = wcb +
  drive` with `regs.si` on `WinPath + 3` is `DirCurrent(drive,
  wcb->WinPath + 3)`.
- `FileSeek` hands back the place as one number of 32 bits, and with
  an error the two words that DOS left: a caller that held AX and DX
  against the place it asked for, without the flag, compares `place`.
- A loop that began with 4E00h in AX and went on with 4F00h has a
  variable for the first time round: `FncMs1` of `dirfunc.c`.
- A helper that took the caller's `regs` to go on with its registers,
  `SetDate` of `copy.c`, `GetVol0` of `read.c`, `CurNl` of `start.c`,
  takes what it reads of them, by name.
- `BlocEr` is the program's as before. `ErrDat` is cleared by every
  function that is asked with the program's errors, as `Intr21` cleared
  it, and by no bare one.
- The test of the data knows a constant that is handed to a function
  of `sys.h` which takes a name, and to `FileWrite`: `NAMED` of
  `harness/reads.py`. A function added to `sys.h` that reads a name
  goes there.
- A variable that is left without a use, and a static function, stops
  the build; a macro does not, `DX_OF`, `NUMBER`, `DOS_CHDIR`: take
  those out by hand.
- A structure of more than four bytes comes back through a pointer to
  room in the caller's frame. Where one member is wanted,
  `CursorWhere(0).shape` costs no variable.

Three callers, before and after, the casts of `&regs` left out.
`OpenFil` of `system.c`, where the second call took its mode from what
the first had left in BX:

```c
regs.ax = 0x3D00 | mode & 0xFF;
regs.bx = regs.ax;
regs.name = name;
failed = Intr21(&regs);
if (failed != 0 && regs.ax == 0x20) {
    regs.ax = regs.bx & 0xFF0F;
    failed = Intr21(&regs);
}
opened.value = regs.ax;
opened.failed = failed;
```

```c
opened = FileOpen(name, mode);
if (opened.failed != 0 && opened.value == ERR_SHARED)
    opened = FileOpen(name, mode & 0x0F);
```

`Cursor` of `window.c`, where the page went from one call of the BIOS
to the next in BH, and AL with it, which the second call does not
read:

```c
regs.ax = 0x0F00 | place & 0xFF;
regs.bx = 0;
regs.cx = 0;
regs.dx = place;
Int10(&regs);
regs.ax = 0x0200 | regs.ax & 0xFF;
Int10(&regs);
```

```c
CursorTo(ScreenState().page, place);
```

`M30_50` of `dirfunc.c`, where the original does not look whether DOS
told the time, and sets what CX and DX hold:

```c
regs.bx = opened.value;
regs.ax = 0x5700;
Intr21(&regs);
if (AttrDat != 0xFFFF)
    DX_OF(regs) = AttrDat;
if (AttrTim != 0xFFFF)
    regs.cx = AttrTim;
regs.ax = 0x5701;
Intr21(&regs);
```

```c
stamp = FileTime(opened.value);
if (stamp.failed) {
    /* The assembly goes on with CX and DX as they are, which DOS
       has not set then: the attributes that it has just set, and
       the address of the name. */
    stamp.time = old & 0xFFFE;
    stamp.date = (u16)md.TmpBuf;
}
if (AttrDat != 0xFFFF)
    stamp.date = AttrDat;
if (AttrTim != 0xFFFF)
    stamp.time = AttrTim;
FileSetTime(opened.value, stamp.time, stamp.date);
```

What making the first files so met:

- A function of `dos.c` that goes by something which another call has
  to have set fails where the other call is still the old one:
  `CaseMap` goes by what `CountryInfo` notes, and the program ended in
  "Invalid Opcode" at its first capital until `start.c` asked for the
  country through `CountryInfo`. Nothing else of `sys.h` is like that
  but `ScreenPut` and `ScreenGet`, which go by what `ScreenMemory` has
  noted: `ScrOffs` of `window.c` calls that, which `IniData` calls.
- `dos.c` grew by 2.6 KB for the functions, of which each is some 15 to
  70 bytes, and a caller is shorter for every call that it moves: the
  four files and the callers beside them gave back 0.8 KB for 76
  calls.
- A file that goes over makes fewer pointers of numbers, and the test
  `portable_c-host` fails for a count that has changed, fewer as well
  as more: write its record anew, `python3 harness/hostcheck.py
  --record`, in every commit that moves calls, and look at what the
  record's diff says. The README's table of figures has how many calls
  are left: `python3 tools/figures.py --write`, after the build, in the
  last commit of a piece of work.
- A removal that lands beside this work takes out what a function
  here had wrapped or moved, and the rebase then stops in the same
  lines: the removal stands, and the function takes its simpler form.
  `Window` and `MovScr` lost their waits for a CGA that way, and are a
  call of `ScreenPut` or `ScreenGet` each.
- The functions that nobody calls yet need nothing to keep them: a
  function that is not static is in the program whoever calls it.
- What a row of the deviation's table says of the original is read in
  the assembly, not in the structure of registers: that has what the C
  translation handed over, which is the original's only where it kept a
  register from one call to the next. Where it set one by itself, 0 in
  AL behind a `MOV AH`, zeros in SI and DI, the address of `TmpBuf` to
  have DS on the main data, the original has something else, and the
  row says both. `iconv -f cp866` of the assembly's file into `build/`
  and `grep -n 'Intr21\|INT'` there finds the calls. AL of 4Eh, which
  APPEND goes by, is such a register in `FindFil` and in `F34_11`: 0 in
  the C translation, a leftover in the original.
- A call that takes no address was still made with DS where
  `regs.name` had it: on the text of the viewer or the editor, on the
  list of the file search. The functions have DS on the main data
  there, and that is a row too.
- The scenarios take long with several agents at work. A commit's build
  can be copied and tested as the note on copies above has it, while
  the next file is moved: `harness/ported.py` with the copy's `VC.EXE`
  and `VC.MAP` and the original's listing.
- Where the C kept the registers of the BIOS and those of DOS in two
  structures, as `IniData` of `start.c` did, what it handed on in a
  register that a call does not take was not always the original's: BL
  of the carriage returns that `IniData` types was what the BIOS had
  answered about an EGA, and in the assembly it is what DOS answered
  about its version. The row of the deviation says what the original
  has there, read from the assembly, not from the C. And a leftover may
  be something that no function hands back, so that it cannot be handed
  on even where the function takes it, as `ScreenType` takes a colour:
  it is dropped like any other, with its row, if the service does not
  read it, and the BIOS reads no colour there on a screen of text.

What moving `blank.c`, `main.c`, `enter.c`, `commands.c`, `keys.c`,
`bars.c` and `menu.c` met:

- A file says at its top what it calls of the rest of the program,
  "Int10 calls the BIOS for the screen", "SelDrv is the call of DOS
  that makes a drive the current one", and has macros for the services,
  `DOS_CHDIR`. Both are stale when the calls go, and nothing stops the
  build for either: look for the old names in the file once more when
  it compiles.
- `SelDrv` and `DosTime` were `DriveSelect` and `TimeOfDay` under an
  older name already: their callers change the name and drop nothing.
- An argument may be there for a leftover alone. `F69_00` of
  `commands.c`, EGA lines, takes what the main loop has in CX and hands
  the BIOS its high byte in a call that takes nothing there. When that
  goes the argument is not read, which stops the build, and has to go
  too: from `vc.h`, from the call in `Command` of `main.c`, and from the
  sentence at the top of `main.c` that says which registers the commands
  read. EGA lines is still on `Int10` for that and for its call for
  Print Screen: `TODO.md`.
- Many runs of the scenarios at once fill `/tmp`, which is in the memory
  and has a limit: with six checkouts testing, and one of them three
  copies at a time, the scenarios' tests of all of them failed in the
  same minute, the C translation's too, with "the machine stopped: it
  was reset, or QEMU ended" or "Disk quota exceeded". That is the
  machine and not the program. `TMPDIR` set to a directory in the
  checkout's `build/` puts the disks of the machines there instead:
  `TMPDIR=$PWD/build/tmp ctest --test-dir build`.

What moving `dirfunc.c`, `read.c`, `usermenu.c`, `treebox.c`, `drive.c`
and `setup.c` met:

- What a DOS leaves in AX where a call has nothing to answer there
  may be what it was handed. The bench's FreeDOS does that when it has
  closed a file: a program that closes one with 3E53h in AX gets 3E53h
  back. So a caller that reads the low byte of AX behind such a call,
  where the call did not fail, reads the leftover that the original
  handed over, and the function of `sys.h` hands over 0: the leftover
  is behaviour there, and the call does not go over until that is
  decided. `ReDr00` of `read.c` is the one that was met, with the file
  that it closes after a read which failed (`TODO.md`). At every call,
  look for a read of `value` where `failed` is 0. One that takes the
  high byte alone, as `DirFunc` and `GetVol` do, is not of this kind:
  the function hands the service over there as the original does.
- The directory that `TMPDIR` names for the tests, of the note above,
  wants a short path: QEMU's socket is in it, and a path of more than
  107 characters is refused, "AF_UNIX path too long". `build/tmp` does;
  a directory two levels further down did not.

What `copy.c` added to that, which went over a family of calls at a
time, a commit each:

- What a register held that a line reads without having looked at the
  flag is read from the assembly, not from the structure of registers
  that the C had. The structure may never have been given that member,
  and then the C went on with what its stack held: CX where `C20_00`
  asks for the time of a source that is a device. Or the C worked a
  value out beside the structure where the assembly changes the
  register: the attributes of a target that is appended to, `AND` and
  `OR` on CX there. The place then says what the assembly has, and the
  commit says that the portable C had something else.
- The other answers are like the time and the date in that: `kind` of
  `FileDevice` is 0 if DOS would not say, where the original reads DX
  as it was, the address of the name; `free` and `sector` of
  `DriveSpace` are 0 where it reads BX and CX as they were, a handle
  and a count. `place` of `FileSeek` needs nothing, being the two words
  that DOS left.
- While some calls of a function still have the structure, a call that
  has gone no longer sets it for them. What a call that stays takes,
  the name and the handle, is set in front of it, where it was set
  once for a row of calls; and what a line behind it reads of a
  register that the call does not answer in, too. The structure goes
  with the last of them, and the build stops for it if it is left.
- A register that went through a call which does not take it to one
  that does, the attributes of a source directory from 4300h through
  39h to 4301h, is the answer of the first call, read at the third.
- A file gives back fewer bytes for a call that answers with more than
  four bytes, which come back through a pointer to room in the
  caller's frame: `FileAttributes`, `FileDevice`, `FileSeek`,
  `FileTime`, `DriveSpace`.
- Each family was built, held to the data and to the host check, and
  copied as a build, by the note on testing a build that is a copy;
  the scenarios then ran on the copies side by side while the next
  family was made.

## Work by several agents

Work that can be divided is done by several agents at a time, without
waiting for the user between its steps.

- Each piece is the work of an agent of its own, started fresh, so that a
  run does not fill one context. The session that runs them does not do
  the work: it starts each agent, checks what the agent left, which is
  the commits, the push and `LastTest.log`, and keeps its report.
- An agent is given no time limit for work that is going well. The
  session watches what its agents are doing, and turns one round that is
  spending its time on something of little use. It also looks for what
  an agent has left running.
- An agent knows only what the repository tells it. What one learns that
  the next needs is written down before its work is committed: in the
  notes here, at the top of `harness/scenarios.py`, in `TODO.md`.
- Agents that work at the same time each have a checkout of their own, a
  plain clone of the same origin with its own build, and a part of the
  program that the others leave alone. The session names both.
- Work gets onto `main` one piece at a time, in a queue that the session
  runs: an agent commits in its checkout when the tests pass and says
  that it is ready; at its turn it does `git pull --rebase`, settles what
  the others changed keeping what both sides added, builds, passes the
  tests once more, and pushes. Nothing else lands meanwhile.
- An agent whose work changes only `src/portable_c/`, or what is the
  portable C's alone in the harness, runs the tests of the portable C,
  `ctest --test-dir build -R portable_c`, and not those of the frozen
  trees, which cannot change by it. A change to what the trees share, the
  harness, the build or a tool, wants the whole suite. The session runs
  the whole suite on `main` itself, after what has landed.
- Two agents have made the same thing under one name with different
  arguments, and git saw no conflict in it. After a rebase, look at
  everything the others added that names what is yours.
- Nothing in the harness takes turns: recordings of the original and
  tests run at the same time, in as many checkouts as there are, and
  other machines only make them slower. Where a recording or a test does
  come out different under load, that is a flaw to report, not something
  to wait out.
- The tools and the harness are improved as the work goes, without
  waiting for confirmation, each improvement a commit of its own. What
  decides is the total cost: an improvement is made when it saves more
  time over the work still to come than it takes to make, counting time
  that has actually been lost, not time one might lose.
- Stop and ask, with the work at hand left unpushed, before any of
  these. An agent that meets one hands its work back as it is, and the
  session asks the user:
  - in the C translation, behaviour that differs from the original's;
  - in the portable C, a deviation that is not the removal of something
    that only made sense on DOS;
  - a change to what the comparison leaves out, other than a figure of
    the kind that "Scenarios" has left out, or to a limit that a test
    holds the build to;
  - a golden of the original that comes out changed when it is derived
    again;
  - anything new to download;
  - a change to these instructions, other than a note on what was
    learned.

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
  executes, and a C tree takes more of them than the original to get to
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
- The tests run the build of a C tree that is shipped; no build is made
  for them. The original and the C trees are driven alike, by a program
  of ours that stays resident in the machine, `harness/DRIVER.ASM`: it puts
  the keys of a step in through the keyboard controller, so that the BIOS
  makes of them what it makes of a keyboard's, and writes the screen out
  when the program idles with none left. It watches and feeds, and
  answers nothing in the program's place or the BIOS's.
- A test does not put a stand-in where something is that the project does
  not own: DOS, the BIOS, the machine. What a test knows of them it gets
  from the real ones. A part of the program itself may be replaced in a
  test build, since its source says what it is supposed to do.
