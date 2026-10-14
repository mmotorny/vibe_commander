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
  by hand. What a tree is declared to show otherwise is not written into
  them: see "The portable C".
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
- The road, on the user's word of 12 October 2026, in this order:
  0. The setup program, `VCSETUP`, is ported into the portable C, so that
     there is one program: setup is reached by an item of its Options
     menu, and is judged by scenarios whose goldens are recorded from
     the original setup program. The call by which the setup program
     asked a running copy for its data goes then, with the last of the
     watch on DOS. This comes before the 32 bits.
  1. The portable C becomes a 32-bit program for DOS, built by Open
     Watcom's 32-bit compiler with a DOS extender, and judged on the same
     bench by the same scenarios. When it passes them all, its 16-bit
     build goes: `src/c/` is the 16-bit program. The extender is
     DOS/32A, the one that runs on the bench: its stub is linked from the
     pinned Open Watcom download when the program is built and never
     committed, its licence text is kept in the repository, and the
     README says, as that licence asks, "This product uses DOS/32
     Advanced DOS Extender technology." This step follows the setup
     program without waiting for the user; the 16-bit build is not
     deleted before the user has said so. The user said so on 13
     October 2026, with the last two differences of the program of 32
     bits declared: the tests then judge that program as the portable
     C, and the 16-bit build goes with the switch, in one piece. That
     is done, the same day: the portable C is the program of 32 bits
     and has no other build, and steps 0 and 1 are behind it.
  2. It uses what 32 bits give: one flat memory, with pointers where
     there were segments and offsets, and numbers and values of 32 bits.
     A variable is widened wherever wrapping round at 16 bits would be
     unexpected behaviour, and stays 16 bits only where the program
     counts on the wrapping. A number that is only ever true or false
     becomes a boolean. On the user's word of 13 October 2026 the
     memory for the panels and the texts comes from the extender from
     this step on, as much as the machine has, where step 1 had 640 KB
     of the program's own: what a user then sees otherwise, an editor
     that opens whatever fits, is declared. The widening is of this
     step and does not wait for the layer: the pointers first, then
     the booleans and the widths together, with the reason written
     down for every number that stays 16 bits, and a named type for
     what crosses to the system, a size of a file or a count of bytes
     read, so that the layer can say later how wide that is. A limit
     of the original that goes when a number is widened is a
     deviation, and each is asked as the work comes to it. For an hour
     of that day the widening was put behind the layer's design: the
     reasons did not hold, since most of what is widened never crosses
     to the system, and the memory from the extender makes the limits
     of 16 bits reachable on DOS.
  3. The abstraction layer is designed: what the program asks of the
     system it is on, and how. The user approves the design before
     anything is built on it. `sys.h` is what there is until then, a
     seam that keeps the registers in one file, and not that design.
  4. The implementation for DOS moves under the layer.
  5. A second implementation is made that builds and is not meant to
     run: a stub.
  6. The stub is built with clang on Linux, for 64 bits, and whatever of
     the x86 or of DOS is still in the common code goes behind the
     layer, as Chromium has it: files `_dos.c` and `_posix.c` beside
     common headers, and macros that each platform defines in its own
     way.
- The program in the portable C is Vibe Commander 0.1, on the user's word
  of 13 October 2026, since it no longer is what the original is. Its
  title and the first line of its banner are "Vibe Commander 0.1", the
  short form, and where the original names itself without a version it
  is "Vibe Commander". It says "based on Volkov Commander 4.05", in the
  same short form, wherever the original named its version: behind the
  name in the banner, and in the Info panel on a line of its own
  between the title and the line under it, where the two lines on the
  memory, which mean nothing off DOS, are gone. The banner's second
  line keeps the author's copyright and adds the user's, "(C) 2026
  Maksym Motorny". No line is added anywhere for it. The C translation
  keeps the original's name.
- Clang is where this is going, for the platforms it reaches and for its
  sanitizers and static checks. Open Watcom builds the program for DOS
  for as long as DOS is where it is judged. No other toolchain is taken
  on.
- What judges the portable C is what judged the C translation: the
  scenarios, with goldens recorded from the original. Where the portable C
  is to behave otherwise than the original, that is a deviation.
- Every deviation is declared, in `src/portable_c/DEVIATIONS.md`: what
  the original does, what the portable C does, why, and which scenarios
  show it. What a scenario is then expected to show is the original's
  golden with the declared difference applied to it, on the user's word
  of 13 October 2026: a patch, a committed file of the rows that differ,
  with its reason, kept apart from the goldens of the original and
  applied to the golden as it is derived, so that deriving the golden
  again gives the expectation again; or, where one difference is on many
  screens, as the program's name is, a rule that the comparison applies.
  A patch that no longer fits the golden it was made for is a failure to
  look at, not to regenerate. The goldens of the original themselves are
  never edited.
- A feature that only made sense on DOS is removed, not kept as a setting
  that does nothing. The first is Memory allocation in the Options menu:
  the portable C stays in memory whole while a command runs. A removal of
  that kind is declared and made without asking. Whether a feature is of
  that kind is judged by what it gives the user, not by how DOS did it:
  Quick execute runs commands through an interrupt that only DOS has,
  and what it gives, commands run by the shell the program was started
  from, is not DOS's alone. Where that is in doubt, it is asked. Any
  other deviation, a fix of a bug of the original included, is asked
  first.
- What the user has decided of the features that were in doubt, on 13
  October 2026: Quick execute commands goes, the setting and the way
  through INT 2Eh; EGA lines goes, the command that gives the screen 43
  or 50 rows by a font of the BIOS, and the rows of the screen are to
  come from the system later: until a system gives the program a screen
  of another size, the scenarios that went to a tall screen by that
  command are not run for the portable C, and what they alone tested is
  listed as untested; the drive B: that shares the floppy drive
  with A: stays, with its prompt and its setting, for as long as DOS is
  a target. A setup file made from the defaults has zeros where the
  original has 64 bytes of whatever lay behind its table: that stands.
  Where the program of 32 bits does again what the original does and the
  portable C of 16 bits could not, a second copy that has the memory to
  run a command, it is held to the original's golden, and the deviation
  says only what still differs. When DOS ends the program by another's
  hand, the program of 32 bits is to be told as the 16-bit one is, by a
  small routine of real mode that the system's functions keep for it.
  The editor of the program of 32 bits has room for a file that the
  original's has none for: that is declared, the memory is not made
  short for it, and the scenario that has the editor refuse such a
  file, `editor-big`, is not run for that program, with what it alone
  tested listed as untested.
- What differs in the program of 32 bits from the original is decided
  by the user one difference at a time, on the user's word of 13
  October 2026: nothing gives leave to remove or break a feature of
  the original for that program without asking. Decided so far, each
  declared as it is and with no change of the program: what is inherent
  in running under the extender and shows on no screen; that a command
  finds no memory above the megabyte while the extender holds it; and
  that a program of another's which has stayed in the memory, started
  from this one, is cut off from its interrupts when this one is left,
  with the extender's lines of warning on the screen. A mend of the
  last was built and dropped on the user's word: `TODO.md` has it.
- Of the limits of the original that step 2 comes to, each asked as
  the work reaches it, the user has decided, on 13 October 2026: the
  cut of each panel to 256 files while a command runs goes, since a
  panel keeps its memory through a command now, and what was selected
  beyond the 256th file stays selected; that is the removal of what
  only made sense on DOS, and the piece that has `enter.c` makes it
  and declares it.
- A bug of the original stays as it is until its fix has been decided.
- DOS stays a target of the portable C for as long as DOS is where it is
  judged: every step builds for DOS and passes its scenarios there before
  it lands.
- The order holds here as it did for the translation, and it is now the
  point of the work: portable C first, then C tied to a compiler or a
  machine, then intrinsics, then assembly inline in a C file. What the
  README lists as resting on Open Watcom is to shrink, not grow.
- What the program asks of the system it is on, it asks in one place; the
  rest of it does not know which system that is. Today that place is
  `sys.h`, with `dos32.c` behind it, the one file of the system's
  functions since the 16-bit build went.
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
- QEMU 11.1 has a second fault with segments, which a flat 32-bit
  program for a DOS extender does not meet in itself and may meet in
  its extender. In a code segment whose base is not 0, while DS, ES and
  SS all have the base 0, an instruction with 32-bit addressing that
  reads memory through `cs:` reads at the same offset from 0 instead.
  CauseWay enters every interrupt through such code, and stops every
  program on the bench with exception 0Dh at its first interrupt;
  DOS/32A is not touched by it. QEMU's log of interrupts, `-d int`,
  shows the fault. `TODO.md`, "The portable C in 32 bits: what a trial
  on the bench showed", has how it was shown and what else that trial
  found.
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
- CTEXT of the C translation ends at D32Eh, 7,378 bytes below its
  limit, and CTEXT2 at B740h, 14,528 below it: the linker's map,
  `build/c/VC.MAP`, has where each begins and how long it is. This note
  had CTEXT with under a thousand bytes left, and a new file going into
  CTEXT2 for that, by its name in `port_c_second`, unless it multiplied
  or divided numbers of 32 bits.
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

- Since the 16-bit build went, on 13 October 2026, the portable C is
  the program of 32 bits and that alone (`TODO.md`, "W and Z1, the
  switch and the end of the 16-bit build"). The notes below were
  written as the work went, most of them while the tree was a program
  of 16 bits or was built both ways, and they are kept as they were
  written, for what the work met. What a note says of two builds, or of
  the tree as a program of 16 bits, is of before, and is read so:
  - `dos.c`, of the portable C, is the file that is gone, and what it
    held is `dos32.c`'s: the one file of the system's functions, which
    keeps its name, since the C translation has a `dos.c` and the
    tables of the harness that are one for both trees tell the two
    files apart by their names (`SYSTEM_FILE` of `harness/reads.py`).
    `resident.c` is `exec.c`, which the harness knows under the C
    translation's name (`FILES` of `harness/reads.py` and of
    `harness/ported.py`). `arith.c` is gone with the test
    `portable_c-arith`: `harness/arithcheck.py` is of the C
    translation.
  - The compiler is `wcc386`, in the flat model: one segment of code
    and one of data, a pointer an offset of 32 bits, every function
    near, and the arguments in EAX, EDX, EBX and ECX. What a note says
    of the medium model, of far functions and far pointers, of the two
    code segments with their limit, of a relocation that a call costs
    and of what the compiler did with a function's registers is of
    `wcc`, the 16-bit one. The words `__far`, `STACK`, `FAR`, `CODE`
    and `TEXT_FAR` are gone from the source, and `#ifdef __I86__` with
    them: each word of the addresses in `vc.h` has one body, the flat
    one, which the notes call the second.
  - `build/portable_c/VC.EXE` is the program that was
    `build/portable_c32/VC.EXE`, with `debug/` in it, and the tree
    `portable_c` of the harness is that program. There is no tree
    `portable_c32`, no `SOURCE` in `harness/deviations.py`, no
    directory of patches but `harness/goldens/portable_c/`, and no
    record: `harness/pass32.txt` and `ported.py --record` are gone, and
    the test `portable_c-scenarios` holds the program to every scenario
    that it is taken through. The tests of the layout and of the data
    are `portable_c-layout`, `portable_c-debug-layout` and
    `portable_c-data`; `harness/notice.py` has no `--build`; and
    `harness/trial32/both.py` is gone, `asked.py` there running the
    toy with `dos32.c` alone.
  - Where a note has "the same bytes of `build/portable_c/VC.EXE`" for
    a check, the file meant was the 16-bit one. The check itself holds
    of the program that there is, by `sha256sum` before and after, for
    a change that is to be no code. What is no longer to be had is the
    16-bit file as the witness of a change that is code in 32 bits: a
    cast that was "no code on DOS", a word of the addresses written
    where a cast stood. The scenarios judge those now.
  - The build compiles a file again when the compiler's command has
    changed, by a file that the configuration writes into the build's
    directory, `compiled.txt`, and every object follows: without it an
    object that another compiler had left under the same name was
    linked as it was. A custom command of the build does not notice
    by itself that it has changed.
  - A piece that takes a name of the harness away, a tree, a
    directory, a test, looks for it in `harness/trial32/` too, whose
    scripts and toys have the commands that run them in their
    comments.

- The C translation is built by `add_c_tree` of `CMakeLists.txt`,
  which takes the tree's name, its model and the files of its second
  code segment: the notes above say `port_c_second` and `c_second` for
  that list, which is the end of the call now. The portable C is built
  by `add_portable_c`, which takes a target, the directory of the build
  and the compiler's options that are that build's own, once for the
  program and once for the one of `debug/`: it has one code segment and
  no such list.
  The harness takes the tree where it had `src/c` written in:
  `datacheck.py` and `reads.py` go by the name of the build's directory,
  `notice.py` by the file it is given, `play.py` has `--tree`. The
  tables of `datacheck.py` and `reads.py` are one for both trees: an
  entry that only one tree wants will need them told apart.
- The portable C was compiled in the medium model, `-mm`, while it was
  a program of 16 bits, and the notes from here to the one on the
  figures are of that compiler where they speak of one: a function is
  far unless it is static, by the compiler's rule and with no
  `#pragma aux`, and a pointer to a function is four bytes. A static
  function is called near, as before. The compiler calls `__U4M` and
  `__U4D` far then, with a relocation for each call from the second
  segment; a `__near` in their `#pragma aux` is taken without a word and
  the program links, and returns near to a far call. `_asm` takes the
  name of a function for a far place: `jmp Exit05` in `Exit01` of
  `dos.c` is a far jump there. (`Int2E` had one to `Exec2E`, while it
  did not return.)
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
  `__U4D`, whose registers are the compiler's; and `CaseMapFn`, DOS's
  routine. `Int2E`, whose command was in DX:SI, in `dos.c` since the
  piece S3, went with Quick execute commands.
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
  - `TREES` of `harness/deviations.py`: the scenarios that are not run
    for the tree, each of which an entry of `DEVIATIONS.md` has to name,
    as `` `NAME` ``; a step that is left out of every scenario for it,
    where the keys choose something that is gone, or steps of one
    scenario; a step that the tree is taken through in place of
    another, in every scenario or in one; and a rule for what is
    otherwise on many screens. A program knows its tree by the
    directory of the build that its file is in.
  - `DECLARED` of `harness/datacheck.py`: a table of the program that
    differs, as the places where it departs from the original's, each
    with the bytes that the tree has there or none for bytes left out.
    The object is held against the original's with that done to it,
    so the rest of it is still held. `GONE` of `harness/reads.py` has
    a constant that the tree no longer has and that `REVIEWED` lists.
  - `GONE` of `harness/ported.py`: a function of a batch that the tree
    no longer has.
  Then the scenarios: build, and `python3 harness/compare.py
  build/portable_c/VC.EXE` says which differ from the original's
  goldens, which is the list to declare, each looked at. A scenario
  whose steps choose what is gone needs the step left out, not a patch
  of the keys going astray; one that then shows every screen as the
  original does after the same step needs nothing more. What a
  scenario shows otherwise is its patch, the next note.
- How a patch is made and what it is, since a deviation is held by one
  (`harness/goldens/portable_c/NAME.patch`; the top of
  `harness/goldens.py` has the form and why it is no unified diff):
  - `python3 harness/goldens.py patch --tree portable_c NAME` takes the
    build through the scenario and writes every row that differs from
    the original's golden, the original's row behind a `-` and the
    build's behind a `+`, and prints them with the cells marked. That
    is what the build did. It is a patch when whoever declared the
    deviation has read every row against the entry of `DEVIATIONS.md`,
    has written the entry's number and a sentence of why into the
    header, and commits it; a header that is not filled in fails the
    test, and so does one that names an entry which does not name the
    scenario. A row that the declaration does not explain is a change
    that was not meant.
  - The test reads every patch before it runs anything, and holds it
    to the golden row for row. What it then says is one of two things,
    and they are told apart by their first lines. "does not fit ...
    Nothing was run": the original's golden is otherwise than the
    patch has it, which says nothing of the program; the message has
    both rows with the cells marked, and whether the golden moved in
    cells that the patch changes or only around them. "differs from
    the original with NAME.patch", with rows called "the patch": the
    patch fits and the program shows otherwise than it says, which is
    the program's to mend.
  - When goldens have been derived again on purpose, as for a change
    of the bench or of a scenario's steps, every patch on a row that
    came out otherwise fails in the first way, and is made again by
    the same command. It keeps the header and says how the new patch
    stands to the one that was there: the rows that expect the same
    in the same cells on a row of the original that is otherwise now,
    which need no second reading, and by name those that say something
    else, are new or are gone, which are read like a new patch. With
    all of them made again, `git diff` of the directory is the reading
    list.
  - A scenario where only steps are left out has no patch: it is held
    against the original's golden without the screens of those steps,
    `left_out` and `left_out_of` of `TREES`, and `python3
    harness/goldens.py changes --tree portable_c` lists them next to
    what each patch says. Four of the sixteen goldens of the tree's
    own that there were needed nothing else.
  - A patch is of rows, and a row that differs in one cell is in it
    whole, with its colours: 293 rows where the whole files had 9,050.
    The cell in the middle of the screen whose colours are exchanged is
    the mouse's pointer, which a program that has ended does not show:
    `lost` has it as a row of its own.
  - A rule of one tree is `rules` of its entry in `TREES`, made to
    every screen of a golden when it is read for that tree and before
    its patch: the rows of a patch are then the golden's as the rules
    leave them. The program's name is said by four (`DEVIATIONS.md`,
    10), each a `Part` of `harness/deviations.py`, the kind of rule
    for a text that is of another length in the tree, or stands
    elsewhere on its row, or for rows that the tree has other rows in,
    and that a menu, a box or a panel may cover
    in part: it says the text as the original shows it and as the tree
    does, and on a screen where enough of the original's shows, each
    cell that the golden has as the original's text has it, character
    and colour, is expected as the tree's, and every other cell as the
    golden has it. What making them met:
    - How much is enough. Eight letters in their places were too few:
      a line of text in the editor, in the colour of a panel, had nine
      of the Info's title, and the rule wrote the name into it. It is
      twelve, and the two screens that show fewer, of a title under
      a menu and of the banner beside a panel, have the rows in their
      scenarios' patches. A rule for one screen is a patch.
    - Such a rule is wrong of a cell in two ways, and both fail the
      row, which then goes into the scenario's patch. Something that
      covers the text has a space in the text's colour where the
      tree's text is longer: the setup's sample of the colours, made
      grey on black, over the end of the banner. Or the text shows in
      another colour, under the mouse's pointer: the rule takes that
      for a cover. It is not to be taught the pointer's colour, which
      was tried: the pointer turns grey on black into black on grey,
      which is a box, and the rule wrote the banner into the box of
      F10. The shadow of a box, which makes `(colour & 8Fh) ^ 8` of a
      colour as the program is distributed, a rule knows only where it
      is told to, `shaded`, which the rule for the top of the Info
      panel is: see below. No screen has the banner in one.
    - On a row that a rule changes, the line with `-` of a patch is
      what the rule made, not what the original shows: read it so. Of
      the two patches that there were on such rows, `second-copy` and
      `setup-lines`, the tool said "does not fit" and made them again.
    - A box whose width follows a text, as that of F10 follows the
      name, cannot be a rule: where it is narrower, the screen shows
      what was behind it, which the golden of that screen has not.
      Those are patches, the same seven rows in eight scenarios.
    - The test `portable_c-scenarios` says how many screens each rule
      expects otherwise, and fails for a rule that finds none.
    - `deviations.expected` makes the rules of every build too, when a
      golden is derived. After a change to it, derive every golden
      again: `git status` of `harness/goldens/` is then clean, as it
      was after this one.
    - In the data, a text that the tree has longer than the original
      reached into the room that is reserved for what only the shipped
      binary has, and was "not compared" for that, declared or not:
      what counts now is how much of the original's object it is held
      against, `held` of `harness/datacheck.py`. A text that only one
      tree has is in `OWN_OF` there.
    - The top of the Info panel is one `Part` of four rows of 40 cells
      (`DEVIATIONS.md`, 10 and 11): the title, the line and the two
      lines on the memory in the original; the name, what it is based
      on, the line and an empty row in the portable C. A part of rows
      took what a text did not. Colours cell by cell, since the two
      figures are yellow: `colour` and `now_colour` are a number or,
      for each row, its runs as a golden writes them. The frame and
      the line across left out of what the part is known by, `frame`:
      the foot of any panel has such a line between such ends, and
      forty cells of it are more than twelve. And the shadow.
    - `shaded` has a part known in the shadow of a box as well: a cell
      that has the part's character in what the shadow makes of the
      part's colour is expected with the tree's character in what the
      shadow makes of the tree's colour, and counts among the twelve.
      It was needed: the Right menu over the Info leaves ten cells of
      the title and five letters of the two lines below, four of them
      in the menu's shadow. It is not what teaching the pointer's
      colours was: the shadow makes cyan on black of the panel's
      colour, and nothing that the program draws over a panel as it is
      distributed has that. It is switched on for that one rule.
    - A part may stand with its first rows above the screen: an error
      beeps, the BIOS moves the screen up a row for the beep, and the
      panel is a row further up without its title (`move`). And no
      place of a part lies over another: what the program puts back
      after such a beep leaves a row of the panel twice, the part was
      known at two places that overlapped, and each wrote into the
      other, so that the row which the rule made was one that neither
      program shows. The place where more of the part shows is taken;
      the rows that are left twice are `move`'s patch.
    - A figure that the comparison leaves out is in a part as the
      golden has it, `#######`, since a golden is blanked when it is
      derived and a rule is made to the golden as it is read. What
      the machine has of memory, 653,296, is in the rule as a text: a
      bench with another figure fails every screen of the Info, which
      is the notice to change the rule.
    - `compared` takes the tree: a build's screen is blanked of what
      every tree leaves out, `IGNORED`, and of what its own tree does,
      `IGNORED_OF`, and a golden's of all of it. The count of free
      memory in the Info is of the C translation alone there, so a
      portable C that showed such a figure again would fail.
    - When a rule changes, a patch on a row that it now changes says
      "does not fit" before anything is run: make it again with the
      tool, which says whether it states what it stated. `mouse-menu`
      was the one here, and `menu` lost the row that the rule now
      knows.
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
    name: `VARIABLES` of `harness/reads.py` is for such a one, and had
    `Vect21` until that went with the entry of INT 21h.
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
    `harness/coverage.py`: `made_by_mkexe` told the C translation's
    file from one that the linker wrote, by its header, and `origin` is
    what to add to an address of the linker's map for the place in the
    file. The limits that `tools/mkexe.c` holds the C translation's
    build to, for QEMU, `check_layout` of `harness/ported.py` held the
    16-bit portable C to, at the start of the scenarios' test. The
    file of the program that there is, a flat one, is read by
    `harness/le.py`, `le.linear` tells it from one of 16 bits, and
    `check_layout` asks `harness/layoutcheck.py` of it.

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
    the copy. Try the copy as it is first: it has to be taken. That
    was the command of the 16-bit build. For the program as it is, the
    copy wants `tables.h` beside it as well where the file has tables,
    and the command is `binl64/wcc386 -q -3r -mf -s -zl -zls -wx -we
    -os -i=$WATCOM/h -fr -fo=x.obj` and the copy;
    `build/portable_c/compiled.txt` has the command that the build
    uses.
- A build can be tested while the tree goes on changing: copy
  `build/portable_c/VC.EXE`, `VC.MAP` and the object files elsewhere
  in `build/`, and run `harness/datacheck.py` (at once: it reads the
  source), `harness/layoutcheck.py` and `harness/ported.py` on the copy,
  each as its test in `CMakeLists.txt` runs it.
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
  - `Exit05`, of `resident.c` then and of `dos.c` since the piece S3,
    is no routine for INT 5: it is where DOS
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
  is a tag for that reason. (The two variables went with the watch on
  DOS, and the compiler was `wcc`: of `wcc386` this has not been seen,
  nor looked for.)
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
    file is not clean while it uses a macro that casts, as `DOS_AT` of
    `vc.h` was, or one of its own. A macro that is made right makes every file
    right that uses it, and `--macros` says which macros there are and
    how many places each makes. The words of the addresses in `vc.h`
    are made so: gcc reads their second bodies, which cast nothing.
  - What gcc cannot say, since it only reads, is in `TODO.md`. Of
    that, arithmetic that counts on an int of 16 bits has a test of
    its own, the next note.
- The test `portable_c-int16` has clang read the tree for a machine
  whose int has 32 bits and finds where a number that the two builds
  may have otherwise is looked at before it is back in 16 bits: the
  top of `harness/int16.py` says how it knows, and what it cannot
  see. `harness/int16.txt` has every such place with the reason why
  it is the same in both builds. The two builds are, since the program
  of 16 bits went, the code as it was written and judged, with an int
  of 16 bits, and the code as it is compiled now. Using it and keeping
  it up:
  - It fails for a line that is new or changed and is such a place:
    "not in the record". Read the place, `--show FILE` for the lines.
    Where the number can leave 16 bits or go below 0, or stays within
    only by something that is not to be seen at the place, write the
    cast that says what DOS does, `(u16)(a - b) < c`, and the place is
    gone. That was no code on DOS, so the 16-bit `VC.EXE` was the same
    bytes, which was held against the file of before; in the program
    that there is the cast is code, and the scenarios judge it. Where
    it cannot, run
    `--record` and write the reason into `harness/int16.txt` by hand,
    under the place's line, in the words of the others: what at the
    place or just above it keeps the number in.
  - It fails for a place of the record that is gone, also where a
    line was only written otherwise: `--record` takes it out, and
    keeps the reason of every place whose line is as it was. A place
    is its file, its function, its kind and the text of its line; the
    number of the line is not in the record.
  - A cast is the answer where the number is a place or an address of
    16 bits, a difference of two such, or a sum that the code itself
    cuts elsewhere. A reason is the answer where the line above asks
    that it fits, or a loop counts it. A cast where nothing can wrap
    says to the reader of step 2 that something does.
  - Added to a pointer, a cut number is what DOS has only where the
    pointer is the beginning of a segment: a panel, `ENTRY` of `vc.h`,
    and the viewer's text, `WORD_AT` of `view.c`, which cut what they
    are handed. Of any other pointer the two builds step back alike
    for a number below 0, kind `back`, and nothing written at the
    place makes a flat memory wrap round at 64K.
  - A place of the kind `minus` is no arithmetic: a number written
    below 0 and kept in 16 bits without a sign, `(u16)-160`, where it
    is cast, handed to a function, returned or given to a variable.
    It is the same number in both builds. Follow it to where it is
    used: added to a number of 16 bits it is the way back in both,
    added to a pointer it is that on DOS alone. The reason in the
    record says where it goes and that the sum is made in a number.
    One that is worked out and has wrapped round, a difference kept
    in a variable, is found nowhere.
  - A variable is taken to hold any number of its type, but one that
    a loop counts with, inside that loop: `for (i = 0; i < 13; i++)`
    with `i` the function's own, changed nowhere else in the loop and
    no label in it. A loop of another shape, `i += 3` or a second
    condition, is not known, and its places want a reason.
  - What is written in a word of the addresses of `vc.h` is no place,
    and the finder knows those words by their names, `WORDS` of the
    script: a word that is added there, with a shift or a difference
    of two addresses in it, is a place at every use until its name is
    in that list. And `dos32.c` is read like any file: it was written
    for an int of 32 bits, so the reason of a place there says that
    the number is meant as it is.
  - It needs clang, any that gives the account of a file that the
    finder was written for: it holds itself against
    `harness/int16check.c` first, lines with the kinds that are to be
    found in each, and says so where they are not. A line added there
    is worked out by hand first. Five of those written for it were
    wrong and the finder right: 65,530 written in decimal is a long on
    DOS, and what is added to it is added in 32 bits there too.
- With one build of the portable C, the bytes of the program say
  nothing of a change that is to leave it doing what it did. What says
  it is the data's image: what the program has in its main data, in the
  memory that it was handed and in its variables at the end of every
  scenario, which `harness/dataimage.py` keeps and holds against what
  another build has there. `TODO.md`, "F0, the net and the record", has
  what was tried and what is left out. Using it:
  - Before the first change of a piece, `python3 harness/dataimage.py
    save build/portable_c/VC.EXE build/before`. The scenarios' test
    writes the images of the build that it judges into
    `build/portable_c/images`, anew at every run; then `python3
    harness/dataimage.py compare build/before build/portable_c/images`,
    and its last lines go into the commit.
  - It names what differs: a field of the main data, by the original's
    listing, a field of a panel or an entry of its list, a place behind
    the panels, a variable by its file and its name. Read each. What a
    piece means to change, it names with `--changed`; a word that it
    takes out of the main data for a pointer gets the pointer's symbol
    in `ADDRESSES` there, and is then held by what it names.
  - Two builds differ with no fault in what follows the time, since
    they get to a step by different counts of instructions: the random
    numbers, the blanker's stars, the time of a file that the scenario
    made. Those are in `UNSTEADY` with their reasons, and are counted
    as "may differ". Before another is put there, `save --early` of
    the same build says whether it follows the time: the driver then
    writes less, the program gets to every step a little earlier, and
    what differs from the build's own images follows the time.
  - The same build gives the same image in every run, under load too.
    One that does not has met a flaw of the bench, to report.
  - The machine is not left to end for it: QEMU pauses it where the
    driver resets it, `linger` of `harness/dos.py`, and the harness
    waits for QEMU to say so. Asking QEMU for its state a hundred times
    a second cost a quarter of a run.
- The test `portable_c-words` holds every file of the tree to how often
  it writes each word of the addresses, in `harness/words.txt`, fewer
  as well as more, as `portable_c-host` holds it to its record:
  `python3 harness/words.py --record` in the commit that moves a count,
  and after a rebase the record is written anew, not merged. A run
  prints which piece of step 2 is to have each word at nothing. A word
  is counted where it is written, in the body of another word's
  definition too, and a segment under another name than `panel` or
  `segment` is not found.
- The figures of the portable C that change with every step, the
  length of its file and what follows from that, what it has of the
  memory, how far its tables may grow, and the counts of what only
  Open Watcom takes, are in one place: the table of the README under
  "The figures", which `python3 tools/figures.py --write` writes of the
  build and the source, between two marks. Run it when a piece of work
  lands, after the build and the rebase, and commit the README with the
  work; `--check` says whether the table is the build's. Write no such
  figure anywhere else, in the README, `TODO.md`, `DEVIATIONS.md` or
  these notes: say the thing without the number, or point to the
  table. A figure that is history, what a step cost when it was made,
  stays where it tells that, and a commit message keeps its own. A new
  figure is a row of the table, added in the script. What the test
  `portable_c-host` counts is in its own table in `TODO.md`, which its
  `--record` flow goes with, and is not copied.
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
    them, and `IGNORED` of `harness/deviations.py` had the two words
    left out for the line that only the portable C prints, while they
    were where INT 21h went before; it has one of them left out since
    (the note on the program's own call, below). Look for such a
    figure in every patch before it is committed: it passes today and
    fails for the next agent.
  - What leaned on the watch and stayed: the entry of INT 21h for the
    program's own call, with nothing of the watch in it, until that
    call went too; the prompt between two commands, which the original
    writes where the watch notes the interrupts, `PromptLine` of
    `resident.c`; and `SetStat` with 0 where the program begins. Read
    what a buffer or a routine of the thing is used for elsewhere
    before it goes: `grep` its name in the whole tree, the routine's
    and the buffer's.
  - A handler that no longer goes on in another handler needs no room
    on the stack for a second return address: `Int21` returned to
    whoever called, and `New21` jumped to it without the three
    instructions that made the room. `Exit01` still has them.
- What taking three settings out of two dialogs of the setup met,
  Memory allocation, TSR manager and Screen output
  (`src/portable_c/DEVIATIONS.md`, 1, 7 and 5; `TODO.md`, "Piece E2"):
  - A field of a dialog names the four fields that the keys go to from
    it, by their numbers, and a group by a flag whether on its first
    line or its last (`LINK_` of `dialog.c`). With a field gone every
    field behind it has another number, and its neighbours may be
    others: write down where each field stands now, and each of its
    four links from that, by the rule that the original's follow, not
    by taking one from every number. The data's test holds what is
    declared, which is what was written, and says nothing of a link
    that leads to the wrong field: only a scenario that goes that way
    does, as a cursor on another row.
  - A box with two groups side by side has one text for each row, both
    halves in it. A line taken out of the right half makes every text
    below it another, of the left half that it had and the right half
    of a line further down. Such a text keeps its name, and is declared
    in `DECLARED` of `harness/datacheck.py` without the bytes from the
    end of its left half to where that right half begins in the
    original, a stretch that runs on through the objects that follow:
    the test then holds the right half against the original's too, and
    the entry is two numbers. `_lines` there makes the entries, and
    `_field` those of a dialog's fields.
  - Steps that get to a line of such a dialog are others for the tree,
    and the same keys mean what they did elsewhere in the scenario:
    `in_place_in` of `deviations.TREES` has, for a scenario, steps of
    the original that follow one another and the tree's step for each,
    or None. Give a step with enough of those around it that the
    scenario has them once; a step that is alone is put in wherever it
    is, which is right for a click. `python3 harness/goldens.py
    changes --tree portable_c NAME` says of each screen which step the
    tree was taken through.
  - The mouse pointer exchanges the colours of the cell that it is on,
    and the setup puts it in the middle of the screen: lines that move
    under it are in the patch for that cell too, on the row that it is
    on and on the row that the text went to.
  - A patch of a dialog whose lines have moved is the same eleven rows
    on every screen that shows it, hundreds of lines. They were read
    by a script as well as by eye: it made of each screen of the
    original's golden what the declaration says, the rows of the right
    half moved and the cursor with them, held that against the golden
    with its patch (`goldens.read(name)` and `goldens.read(name,
    tree)`, screen for screen by `places`), and printed what was left,
    which is then little enough to read: the pointer's cell, the
    cursors that are on another field, the disks.
  - While a test runs, the next build can be tried beside it without
    touching `build/portable_c`: copy its object files into a
    directory of `build/` that is called `portable_c`, compile the
    files that have changed into it by their lines of
    `build/CMakeFiles/portable_c.dir/build.make`, and link by the line
    of the linker there with the paths changed. `harness/show.py
    --program` and `harness/datacheck.py` with `--source` take it.
- What taking EGA lines out met, the second command to go
  (`src/portable_c/DEVIATIONS.md`, 12; `TODO.md`, "The portable C: what
  EGA lines has left", which has all of it):
  - The screen of more than 25 rows is not tested in the portable C
    for now, on the user's word of 13 October 2026, until a system
    gives the program its rows: EGA lines was the one way there. The
    scenarios that were about such a screen are not run for the tree,
    those that passed through it have the steps left out, and `TODO.md`
    lists each with what of the program it tested, under "The tall
    screen: untested in the portable C until a system gives it rows".
    The code that goes by the rows stays as it is.
  - A second line out of a menu moves a click onto another item, where
    the first had moved it onto nothing: `mouse-menu` clicked a dark
    item by its place, and the place became Menu file edit. Look at
    every step that clicks or counts in a menu that changes, also
    those that an earlier removal looked at.
  - A scenario written twice in a table of `harness/deviations.py` is
    the second entry alone, and Python says nothing: add to the entry
    that the scenario has.
  - A command that changes what DOS has shown, as the original's EGA
    lines does when the screen gets shorter, changes screens many
    steps on, wherever a panel is off: leaving its steps out wants
    those rows in a patch.
- What taking Quick execute commands out met, the first setting to go
  from a dialog of the main program (`src/portable_c/DEVIATIONS.md`,
  14; `TODO.md`, "The portable C: without Quick execute commands"):
  - A dialog of one column that loses a line is a row shorter, and the
    row that it no longer covers shows what is behind it, another thing
    in every scenario: a patch for each scenario that shows the dialog,
    and no rule. Some thirty open the Advanced options only to untick
    Clear keyboard buffer for the editor, which stood below the line
    that went: each has a patch for that and a step in place of
    another. One entry of `in_place_of` for the same keys everywhere
    would have been wrong: the same keys tick the Clock in the setup's
    Configuration, in `setup-clock`. They are `in_place_in`, by the
    scenario's name, behind the step that opens the dialog.
  - A patch that is made again for a second deviation is read against
    the one before, not against the original: the patch of before,
    `git show HEAD:harness/goldens/portable_c/NAME.patch` in a file of
    `build/`, is made to the golden by `goldens.read(name, tree,
    patched=goldens.read_patch(file, tree))`, what the new declaration
    says is made to that, and what the golden with the new patch has
    otherwise is the reading list. That works where the steps are as
    they were; where steps are left out now, the old patch does not
    fit, and the screens are held against the original with the whole
    declaration made.
  - A function of `sys.h` that goes is in more places than those which
    the notes on the system's functions list: the toy that asks every
    function of `sys.h`, `harness/trial32/sys.c`; a table
    of `dos32.c` alone in `OWN_OF` of `harness/datacheck.py` and
    in `REVIEWED` of `harness/reads.py`; the count at the top of
    `dos32.c` of those that are made in real mode; the count of the
    functions in the README, in its text and in its table by group;
    and the layout of the block below the megabyte, where what stands
    behind the bytes that go moves down.
  - The piece waited while two others landed, and what bringing it
    onto them met is in its section of `TODO.md`, "What landing it
    met": above all a count that both sides had each taken one from,
    which git merged without a word and which was wrong.
  - A call of DOS that was made for the feature alone goes with it and
    is said in the entry: DOS was asked for the current drive in two
    places for the command that went there first. No test counts the
    calls that the portable C makes for itself.
- The setup program, step 0 of the road, has its plan in `TODO.md`,
  "The setup program in the portable C: the plan": what the program is,
  what the portable C has of it already, how it becomes a part of the
  one program, the pieces of the work with the files of each, and how
  its scenarios are held against the original. Read it before any
  piece of that work. `harness/vcsetup/` has three scripts that hold
  the setup program's assembly against the main program's, with a
  README of their own: which routine is the main program's word for
  word, and how another differs.
- The harness judges the setup program, since the first piece of that
  plan. What making it so met, and what the pieces go by:
  - The original is two programs to the harness. A scenario is of one
    of them, `setup=True` for the setup program's, and the top of
    `harness/scenarios.py` says how such a scenario is written, started,
    tried, derived, held and declared: read "The scenarios of the setup
    program" there before writing one.
  - Two programs of the original in one machine have routines of the
    same bytes, and a trace cannot tell whose a block is that both
    have: `coverage.executed` places a program only by blocks that the
    other's file has not, `others`. Whatever runs a second program next
    to the one under test has to say so there.
  - A `.ran` next to a golden is of `VC.COM`, or of `VCSETUP.COM` for a
    scenario of the setup program, under the same name. `goldens.ran`
    keeps them apart; a tool that reads the lists of all scenarios by
    itself mixes offsets of two files.
  - The C translation is taken through no scenario of the setup
    program, by their kind, `no_setup` of `deviations.TREES`: a piece
    that adds one lists it for the portable C alone, which is the one
    tree whose tests it runs.
  - The test of the data has two originals, and knows a constant of the
    setup program by `St` in front of its name. Its top says what a
    word may hold that names a thing of the setup program: the tree's
    `StX`, the main program's `X` where that is the same bytes, and for
    a variable the place of the field of that name in the main data.
    Where it says "the original names X there, which the translation
    has not by that name", one of the three is missing, or the thing
    is kept under another name and wants a line in
    `SETUP_NAMED_OTHERWISE`. That table, not `NAMED_OTHERWISE`, is
    where a word of the setup program's object goes: an entry of
    `NAMED_OTHERWISE` for an object that a tree has not fails that
    tree's test, and the C translation has no object of the setup
    program. And a word that names a text which has no zero byte and is
    read by a count, as a picture of the colours is through `PagTab`,
    has its count in `SETUP_COUNTED`: without that the test takes a
    text that a word names to be read as far as a zero.
    `harness/reads.py` is the same in its way: an entry of `REVIEWED`
    for a file of the setup program, one of `SETUP_FILES` there, is
    not held against a tree that has no setup program, where an entry
    for a file that does not name its constant fails the test.
  - The test of the data can be tried before any code of a piece is
    there: a copy of the tree with one file of constants more, built as
    `harness/notice.py` builds its copy, `cmake -S` the copy with the
    two `FETCHCONTENT_SOURCE_DIR_` of `build/_deps`, and
    `harness/datacheck.py --source` the copy's `src/portable_c`. The
    copy's build is in a directory called `portable_c`, as always.
  - A screen of the setup program that is a dialog can be tried before
    the skeleton of setup is there, since `Dialog` is the main
    program's: in a change that is not kept, a command of the main
    program calls the piece's function, and `python3 harness/show.py
    --program build/portable_c/VC.EXE` with the keys of that command
    and then the steps of the scenario shows the box, to hold against
    the golden's rows inside its frame. Piece A did that for its four
    dialogs, and when the skeleton was there its scenarios passed as
    they were written (`TODO.md`, "Piece A, the option dialogs: done,
    and what a later piece should know").
  - Where the disks of a scenario differ in a file, the test says the
    two sums of its contents and no more. To see the bytes, a script
    of a dozen lines, kept in `build/`, runs the scenario by
    `scenarios.run` on the original, `setups.original_vc()`, and on
    the build, with `fat.hashlib` replaced by an object whose `sha256`
    keeps what it is given; `scenarios.of(name, "portable_c")` has the
    steps as that tree is taken through them. That is how the setup
    file of `setup-missing-saved` was found to differ in the 64 bytes
    behind the path of each panel and nowhere else, which is declared.
  - A line of a box that the setup program writes into before it shows
    the box, as Country info does, is an object without `const` in the
    file of tables, with the bytes that the assembly has there; the
    data's test holds it like a constant, and the code writes it
    through a far pointer made of `AsmSeg` and the object's offset.
  - A text or a button of the setup program that two pieces name is
    defined once, not static, in the file of the piece whose part of
    the assembly has it, and the other declares it `extern`: `StOk_S`
    and `StOkMn_S` of `stopts_t.c`, which the help of setup has too.
  - `python3 harness/goldens.py derive` derives all goldens again side
    by side, in a minute and a half on four processors, where it took
    them one behind the other. `git status` of `harness/goldens/` is
    clean afterwards, and that is the proof that a change to the
    harness has left the judging alone: it is cheap enough to do for
    every such change.
- The setup is entered and left, since the second piece of that plan:
  `stmain.c`, with a section of `vc.h` for all of setup and the plan
  itself for what the next pieces go by. What making a program of its
  own a part of another met:
  - Two routines of one name are held against each other before one
    is made to do for both: a diff of the two procedures, each cut out
    of its file after `iconv -f cp866`, says every line, where
    `harness/vcsetup/procs.py` only says which differ and by how many
    lines. What differs may be more than the flags that the other
    program lacks: the setup program's `Dialog` loads the place of its
    box whatever a flag says, and keeps a typed line elsewhere. Then
    each difference is one of three kinds, on the user's word that the
    setup is to use the main program's routines: what a scenario of
    setup shows and is no bug is an `if` on `StOpen`; what nothing of
    setup can show is left out, the main program's holding for both,
    and said in the comment at the routine; and a bug of the copy is
    not carried by a switch but asked about, as the 81st cell of the
    key bar was, which is fixed. The setup program's instructions that
    are left out so are still of the batch, as out of reach.
  - What a program does when it starts is not done twice, but look at
    what of it shows. The setup program sets the mouse up anew, which
    puts its pointer in the middle of the screen; it notes the row of
    the cursor, which is where it leaves it. And what the main program
    does around a command is done around setup too, where the next
    thing rests on it: the two interrupts are given back before
    `IniData` takes them, or it notes the program's own handlers as
    those to give back.
  - The screen behind setup is that of a command run: the main
    program writes prompt and command into the row of its command
    line, types the end of the line, and after the command the screen
    goes up by two rows where there is a key bar, one in `IniData` and
    one in `Init40`. With an
    empty row in place of that line and the same end of the line
    typed, every later screen is the original's too, so a scenario
    may start setup several times. A line of such a command that a box
    covers in part is not known to the harness for what it is, and
    stays in the golden: the top of `harness/scenarios.py` says where
    the lines get to.
  - A copy that runs on behind its table, as the notes above have it
    for the main program's defaults, may get into a file: `Default`
    copies 88 bytes of a panel from a table of 24, and the 64 behind
    it are in `VC.INI` when that is saved. Hold a file that the
    original writes against what the published tables can make before
    the scenario is written: `fat.read_tree` hashes every file of a
    disk, and a shim around its `hashlib` keeps the bytes. Here 17 of
    the 64 are of the binary alone, so no build can write that file,
    and the scenario has the main program read it and save its own.
  - What setup has of its own is in the main data, in buffers that
    the main program makes anew after a command: the original has
    nothing behind `DosBuf` left then, so nothing there is counted on
    across a command, here either. `struct StData` is laid over
    `MenuBuf`, with a `HOLDS` line that it fits. The file's image in
    it is an array of bytes with the panels and the sum by their
    places, not members: members would want `#pragma pack`.
  - An object of the setup program has `St` in front of its name, and
    the program has objects whose names begin so already: its
    `StErr_S`, the line for a file that could not be written, is
    `StStErr_S`, and `StErr_S` would be its `Err_S`. That one is not
    written: where the main program has an object of the same name
    and the same bytes that a file of tables can name, the setup's
    tables name it, `Err_S`, `Inc_S`, `YesNo_S`. And once one piece
    names the main program's, no piece may write the `St` one: the
    test of the data takes a word for the setup's own object if the
    tree has one, and fails the piece that named the other.
  - A screen that is still to come can be a function that waits for a
    key and returns: the item and Esc in one step then leave the menu
    as the real screen will, and the scenario of the menu runs the
    code that leads to every item before any of them is there.
  - The critical error of DOS is the main program's box whoever has
    asked for a file. `FtErr1` of `main.c` clears `StOpen` around its
    dialog, as it does the clock and the blanker, and has the next key
    bar drawn anew: the numbers of the two programs' bars mean other
    rows, and `BarPrev` would take one for the other.
  - The calls of DOS and the BIOS that setup makes have rows in entry
    6 of `DEVIATIONS.md` like any others, read from the setup
    program's assembly: a piece adds the rows of its calls.
  - A routine that the setup program alone has by its name may be the
    main program's by its text: `Main50`, the mouse at its menu, is
    `F32_70` word for word. `harness/vcsetup/procs.py` goes by names.
    Before translating such a routine, hold it against the main
    program's with the labels taken out of both. One that is the same
    and names nothing but routines that are the same too is called,
    not written again, and is listed in the setup's batch all the
    same, so that all of it has to have run in the setup program. One
    that names data is another matter: `HlpLine` is `Help77` by its
    text, and the table of colours that it names is the setup
    program's own.
  - What the original's setup program prints through DOS is sent
    nowhere in a scenario, all of it: the banner, and with it the line
    for a setup file that could not be written. A tree writes that
    line on the screen, so what DOS has shown differs from then on,
    and a scenario does not look at it after a save that failed.
  - A golden that would hold the original's edition, because a bug
    has changed the line that `harness/deviations.py` knows the banner
    by, is not mended there: the scenario first has that line gone
    from the screen. `setup-lines` does, for the corner that the key
    bar's 81st cell blanks.
- What taking out the program's own call met, with the entry of INT
  21h that was left for it (`src/portable_c/DEVIATIONS.md`, 8; "Piece
  E1" of the plan in `TODO.md`):
  - What a thing leaves behind when it goes may be a path that the
    original has too, by a setting: with no interrupt to give back,
    the registers that the program leaves when DOS ends it are those
    that the original leaves with TSR manager off. Such a path can be
    seen on the bench before it is written down: a scenario that is
    not kept, the steps of another behind the steps that set the
    original up so, derived from the original by `python3
    harness/goldens.py derive NAME`, read, and taken out again with
    its golden.
  - A figure that is left out of the comparison is looked at again
    when what made it follow the program's size goes: of the two words
    of `ended` one is the same on every build now and is compared, and
    the other, a segment as it came, is that of a program that lies
    behind the program under test and still follows its size. Leaving
    less out needs no asking; the patch is made again and both rows
    read.
  - A scenario that showed the thing through a second scenario's eyes
    says so where it is declared not run: `passed-on` named
    `handed-on`, and `stay` named `asked`, in `harness/deviations.py`
    and in two entries of `DEVIATIONS.md`. `grep` the names of the
    scenarios that go, not only of the functions.
  - An entry of `VARIABLES` of `harness/reads.py`, or of any table of
    the harness that is by name, fails the data's test when the object
    has gone: take it out with the object. A function of a batch that
    has gone is in `GONE` of `harness/ported.py`, and a batch may have
    every function there.
  - `harness/hostcheck.txt` has fewer places for such a change, and
    the test fails for fewer as for more: `--record`, in the same
    commit.
- What the help of the setup program met, the first of its pieces to
  have code, before the setup could be entered:
  - Every C file of `src/portable_c/` is in the next build, by its
    being there: a file that is written and not finished stops the
    build for whoever builds next, and a test that is running reads
    the tree in its first seconds. Compile a new file by itself first,
    with the command of the note on the trial of an old line above.
  - Code that nothing calls yet can still be tried: in a copy of the
    tree, made and built as `harness/notice.py` makes its own, with a
    few lines that call it from somewhere the main program gets to, F1
    of `Input` for the help, and `harness/show.py --program` on the
    copy's `VC.EXE`. What it draws is held by eye, or by a few lines of
    Python, against what `harness/show.py --setup` prints of the
    original for the same steps. The copy is not kept.
  - A routine of the setup program that walks through memory with
    addresses of 16 bits may go by where its buffer is in the
    original's one segment, not only by how long it is: the carry of
    an addition is a test there. The translation then counts in
    numbers of 16 bits from the original's address, and has that
    address as a number with where it comes from: `StHelp` of
    `sthelp.c`.
  - Where the original reads memory that it has put nothing into, a
    buffer behind a read that failed, what it shows on the bench is by
    what was there. The translation has zeros there, the scenario says
    in a comment that the original's screen is of that kind, and the
    plan has it among what was found.

- The words of the addresses, `AT`, `OFFSET`, `PANEL`, `MAIN_SEG`,
  `NEAR`, `SEG_OF`, `OFFSET_IN_SEG`, `MD_PLACE`, `MD_POINTER`, are in
  `vc.h` once, each with a body for DOS in 16 bits and one for a flat
  memory while there were two builds, and with the flat one alone
  since, and no file defines one for itself: `TODO.md`, "The portable
  C in 32 bits: the plan", has what each takes and gives, with a line
  as it was and as it becomes for every kind of place. What putting
  them there met:
  - A cast that is made a word is the same bytes of `VC.EXE`, and that
    is the check: `sha256sum build/portable_c/VC.EXE` before and after,
    which takes the build's second. Every form that the plan's table
    has was the same bytes at the first try, with the files' own
    definitions differing among themselves in a `const`, in a cast of
    the offset and in a macro between. One thing was not: the
    difference of two far pointers written out where `(u16)(u32)` took
    the offset, which is why `OFFSET_IN_SEG` is a word.
  - What a far pointer says beyond its address is lost where the
    memory is flat, and no second body gets it back: `WCB_OF` took the
    panel from the segment of a pointer to one of its files. Such a
    thing is handed on as an argument, which is other code in 16 bits
    too, and the scenarios judge it, not the bytes.
  - A second body is read by nothing but the compilers until the
    program runs in 32 bits. To try one, a file of a few lines that
    includes `vc.h`, has gcc take it for this computer, defines `md`
    after the include as a field of a structure that has 64K in front
    of it and a megabyte behind, and holds each word against the
    address that the plan's account gives: `#define md (arena.main)`
    behind the `#include` is enough, since a macro is expanded where it
    is used.
  - `harness/reads.py` goes by the text of a file of code. A constant
    that a file named and now names through a word of `vc.h` is one
    that the file "does not name": `AsmSeg` hands `Title_S` to
    `TABLES_BY` for that, where a word without an argument would have
    done for the compilers.
  - `wcc386` writes a file `NAME.err` into the directory it is run in
    when a file has a warning: run a trial of it from `build/`.
  - A name that `vc.h` takes is taken in every file: `dos.c` had an
    `OFFSET` of its own, which is `OFFSET_OF` there now, and `view.c`
    and `usermenu.c` each have a `WORD_AT` that is not the one that
    `dialog.c` had, which is why that word is in no header.
  - The host check does not count a pointer that is made of a number
    which gcc can work out. `MD_PLACE` of the address of a field is
    such a number in its second body, so `(struct DIRENT *)TRELIST`
    went uncounted in ten places of `treebox.c` and `tree.c`, and a
    file that is clean in the record may have one. To see them, have
    gcc read a copy of the tree, in `build/`, whose `vc.h` has
    `MD_PLACE` as a function, `extern u16 md_place(const void *);`,
    with the options of `harness/hostcheck.py`, and hold what it says
    against what it says of the tree.
  - A pointer to nothing has no place: `MD_PLACE` of a pointer that is
    0 is 0 on DOS and another number in a flat memory, and
    `MD_POINTER(0)` is no pointer to nothing there. Before a cast is
    made `MD_PLACE`, look whether the pointer can be 0, and before one
    is made `MD_POINTER`, whether the pointer is held against 0 behind
    it. The user menu cleared a pointer and handed it on as a number:
    it is set to `MD_POINTER(0)` now, which is the same bytes.
  - `OFFSET_IN_SEG` wants the segment that the pointer was made with,
    and a routine that is handed a far pointer alone has none to name:
    `NearFl` of `files.c` names the one that its only caller has the
    list in. The first body does not read the segment, so naming it
    is the same bytes, and nothing in 16 bits says whether it is the
    right one.
  - A cast with a step in it, `*(const u8 *)entry--`, is the same
    bytes with the step as a statement of its own behind it.
- A word of a table that names something, a line of a box, the box
  and the buttons of a dialog, is a pointer in `tables.h`, as long as
  a pointer is, and code never reads through it: it asks `NAMED` of
  `vc.h` what the word names, as an offset in the tables' segment, and
  `NUMBER_OF` where the word holds a number. `TODO.md`, "P3, the
  tables read as what they are", has the two. What reading the tables
  so met:
  - Whether the bytes of the 16-bit program stay is a matter of how
    the read is written. A structure laid over what was read by its
    offsets, `head->lines` where `head[4]` stood, is the same bytes. A
    far pointer that is stepped where an offset was added to is not:
    `DialBox` is 6 bytes longer. And a far pointer to a table kept in
    a variable of its own was 10 bytes more in `F39_01` of `main.c`
    and moved every local variable of the function, where the same
    read in one expression, `NAMED(*(type __far *)AT(code,
    OFFSET(table) + index * sizeof *table))`, is the bytes that the
    function had.
  - A word may be a name or a number by the kind of the thing that it
    is a part of, as `mCtrl` of a dialog's field is, and code may read
    it before it has looked at the kind: the mouse's loop of `Dialog`
    did. Such a read is made once for each kind, behind the test.
  - The objects of two builds differ in every file, by what the
    compiler writes into them besides the code. The listings do not:
    `build/portable_c/*.lst` against a copy of the build before says
    which files changed, and within one, the bytes of each routine,
    without their addresses, say which routines.

- The program of 32 bits is linked by the build since the piece B2
  (`TODO.md`, "B2, the program"), `build/portable_c/VC.EXE` and
  `debug/VC.EXE` there, and the tests `portable_c-layout` and
  `portable_c-debug-layout` hold where the linker has put things. What
  linking it met:
  - The order of the segments is the linker's command's, by classes,
    and a class is linked whole: the constants and what has a value
    are both `DATA`, `CONST2` and `_DATA`, and nothing can be put
    between them. The stack and the main data have classes of their
    own by the two `#pragma data_seg`, `STACK` and `MAIN`.
  - What has no value is not in the file only where nothing with a
    value stands behind it, and of the class `BSS`. The main data has
    none and is in the file all the same, as zeros, also with nothing
    but that class behind it: the segment that its `#pragma data_seg`
    names is one with a value to the linker.
  - Within a segment the objects are in the order in which the linker
    is given them, so which file's variables stand behind which is the
    order of the link's files: `dos32.obj` is last for that.
  - `option stack=` makes a segment of the class `STACK` as long as it
    says, also one that the source has shorter. The build does not
    say it: `LenStk` is the stack's length.
  - The name of the file is in the file: two links of the same
    objects under two names differ in one byte. Under one name they
    are the same bytes, so the program of 32 bits can be held against
    itself as the one of 16 was.
  - `wcc386 -os` gives a function no instructions of its own where
    the function behind it in the file has the same: one that does
    nothing, or that is the same call. The map has several names at
    one place then, nineteen of `dos32.c` at three. With `-od` each has
    its own.
  - It puts pieces of a function in front of its label, behind the
    function before: an address of QEMU's log is not always of the
    function whose name precedes it in the map. The listing says.
  - `wdis -s=FILE` puts the lines of the source into a listing of an
    object that was compiled with `-d2`; without the linker's `debug`
    directive nothing of that is in the program.
  - A check that is to fail for a wrong link is tried with wrong
    links, by hand, from `build/`: the objects of
    `build/portable_c/` with another `order`, or with a toy object
    of constants or of memory without a value among them. The table
    in the plan has which link broke what.

- The tables of the program of 32 bits are held by `portable_c-data`,
  which was a test of their own, `portable_c32-data`, beside that of
  the 16-bit program: `harness/datacheck.py` with `build/portable_c`
  and `--source src/portable_c` (`TODO.md`, "H4, the test of the
  data"). What "Moving the data of a file to C" says
  of `c-data` holds for it, its messages and its tables, with this
  besides:
  - A word of a table that names something is a pointer of four bytes
    there, so a box, a dialog and a table of pointers are longer than
    the original's and are held field by field. Which fields are
    pointers the test reads in the declaration: `BOX`, `BOX1`, `BOX2`,
    `DIALOG` with a count, at the start of a line, an array of
    pointers, or an array of a structure of nothing but pointers. An
    object with pointers that is declared in another way is taken for
    bytes and fails, "has an address at +N ..., where its declaration
    has no pointer": `declared_shapes` and `SHAPES` there want it.
  - The place in a message is counted as the original lays the object
    out, which is what `DECLARED`, `NAMED_OTHERWISE` and `COUNTED`
    count in, and the message says where that is in the object itself:
    "StDfCol_P+10, which is +12 of it as it is here".
  - Where the original names something, the pointer has to be an
    address that the extender sets: "the translation has the number
    1234h, in a word that could name something, and no address that
    the extender sets" is a `NUMBER` or a 0 where a name belongs. Where
    the original has a number, it has to be none: "the translation has
    the address of X, ..., where the original has a number and names
    nothing". The first of the two passes in no program; the second
    is a mistake that only this test names for what it is.
  - A change to a file of tables was held by both tests while there
    were two, and the 16-bit one said the same in fewer words. There
    is the one now.
  - Data that only this program has is that of `dos32.c`. A constant
    of that file wants a line in `OWN_OF` there, and one in
    `REVIEWED` of `reads.py` for where code reads it; `reads.py` leaves
    out the file of the system's functions that a program is not built
    of, `SYSTEM_FILE`. A variable of that file wants nothing, with a
    value or without: those are left out as what they are. In any other
    file a variable with a value is held like a constant.
  - `wcc386` puts nothing between the fields of a box or a dialog, nor
    between two objects of a file: every object is as long as the
    original's but for its pointers. It does round what a file has of
    a segment up to four bytes, so the linker's map does not say how
    long the last object of a file is; the object file does.
  - A box cannot be declared before it is defined: each `BOX(n)` is a
    structure of its own, and a second declaration is "Type does not
    agree with previous definition". Buttons can, being arrays.

- The program of 32 bits went through the scenarios by a test of its
  own, `portable_c32-scenarios`, from the piece H3 (`TODO.md`, "Piece
  H3, the scenarios' test") until it became the portable C, and goes
  through them by `portable_c-scenarios` since. What that met:
  - While it failed scenarios it was held to a record,
    `harness/pass32.txt`, and not to passing everything, and was a tree
    of its own to the harness, `portable_c32`, with the source of
    another and with what was declared for it alone in an entry of its
    own. All of that went with the switch: the program is held to
    every scenario that the portable C is taken through, a fix wants
    no record written, and what was declared for that build alone is
    declared for the portable C, `editor-big`, which is not run, entry
    13 of `DEVIATIONS.md`. A scenario that is not run has to be named
    by an entry there, or the test fails. Nothing holds that the
    program still does otherwise in it.
  - That program has 640 KB of its own where the program of 16 bits
    has what DOS leaves it, and a scenario that takes the original to
    the end of its memory may pass without taking this program there:
    a count of free memory is left out of the comparison, and such
    steps were written to end on the same screen whatever room a build
    has. `editor-full` and `copy-big` are so (`TODO.md`, "The editor's
    room: untested in the program of 32 bits"). Before a scenario that
    passes is taken for a test of a limit of memory, look at what the
    program shows as free in it, `harness/show.py --program`.
  - A scenario of that test was ended at its first screen that
    differs, while the record was; `harness/compare.py` does so with
    `--first`.
    What the keys of a scenario do in a program that has left, or that
    stands in another box than the scenario takes it to, is not worth
    reading, and such a program waits for keys that do not come: DOS's
    prompt takes no click of the mouse, and a step with one waits
    until its time runs out.
  - When the program leaves by itself, DOS goes on in the bench's batch
    file to the command that switches the machine off, and QEMU ends
    with the driver's script half played. A check that looks at what
    the driver has written must look once more after the machine has
    ended, and a pause asked of a machine that has just ended fails.
  - What is looked for in a program's image for every block of QEMU's
    log is looked for in its code alone. With the 640K that the
    program hands out in the image, a scenario took 14 seconds where
    it takes 3: Python's threads take turns at such work, so four
    scenarios side by side waited on one another.
  - A second copy of the program, started from the command line of
    the first, is loaded behind it, where `copies` of `harness/le.py`
    works it out from the program's length: the place moves with every
    change of the program, and is written nowhere. The driver of the
    tests is told the places of three copies and tells which of them
    idles by the programs of that name in DOS's memory in front of it
    (`TODO.md`, "C2"): not by who ran whom, since FreeCOM names itself
    as the one that ran it, at 16h of its PSP, also when a program has
    run it for one command.
  - The time that a step has before the program counts as stuck is
    of the host's clock and of the machine's own, both
    (`Machine.wait_for_end` of `harness/dos.py`): a step that is given
    up on a busy host has also had that many seconds of the machine's
    clock, counted in instructions, and is no slow machine. A step
    that has the driver wait has the seconds of its wait besides.
  - While the program of 32 bits idles, a second of the machine costs
    the host four times what it costs for the 16-bit one. QEMU pays
    some ten microseconds for every change of the processor's mode,
    thirteen at a round, and as much again for a block of the
    extender's that it translates anew at every interrupt, since
    DOS/32A writes the interrupt's number into an `INT` of its own.
    `TODO.md`, "T, the time of a wait", has the measurements and why
    nothing in the program mends it. Such a thing is measured on a
    host by itself or counted, in instructions, flushes and blocks:
    the processor time of a QEMU follows the load as well.
  - The extender takes all of the memory above the megabyte while the
    program runs, and a command of the program is told that there is
    none: the scenario `command` shows that in what `mem` prints, and
    is held by a patch of that build's own, the difference being
    declared as it is on the user's word (entry 1 of `DEVIATIONS.md`;
    `TODO.md`, the plan, 9 of what was open).
  - A number of 16 bits that stands for one below 0, FF60h for 160
    back, and is added to a pointer goes that far on in a flat
    memory, and nothing at the place says so: `T71_00` of `edit.c`,
    which moves the rows of a window in the screen buffer, was nine
    of the fourteen scenarios that the program failed at first. The
    sum is made in the number and the pointer of that: for a pointer
    into the main data `MD_POINTER(MD_PLACE(to) + step)`, for one
    into a segment `AT` with the sum as its offset. On DOS that is
    the code it was, and the file the same bytes.
  - Such a write shows by where it lands, or not at all. That one
    made three kinds of screen, by how many rows the editor's window
    had, and none in the viewer, which made it at every line that it
    went up. Scenarios that fail alike by the look of their first
    screen are not thereby of one cause, nor those that fail
    otherwise of several: mend the first and run them all, `python3
    harness/compare.py build/portable_c/VC.EXE NAME --first`, a few
    seconds each and as many side by side as there are.
  - To see what a mend changed in the memory, keep the program of
    before beside the build in a directory named as its tree is,
    `build/NAME/portable_c/VC.EXE` with its `VC.MAP`: the harness
    takes it for the same tree, and `harness/show.py --program` shows
    the two side by side. The editor's F4 is a view of the memory at
    `ViewSeg` in hex.
  - Which functions of the batches ran is asked of that program, as
    of the C translation. A function that the program has under
    another name may be static in its file, as the entry of the
    critical error is, `DeviceEntry` of `dos32.c` for `FatErr`:
    `RENAMED` of `harness/ported.py`, which finds a static name in the
    linker's map (`TODO.md`, "W, the switch").
  - What that program does otherwise than the original, declared or
    not, is in one list with what the user decided of each: `TODO.md`,
    "The program of 32 bits: what differs from the original, for the
    user". A difference of it that a piece meets goes there, and to
    the user, before it is declared.
  - What a program asks of DOS, call for call, with the segments that
    DOS gets in real mode, `harness/trial32/calls.asm` writes to a
    file: it takes INT 21h and runs the program under the program's
    name, so the driver drives it. That is how the registers and the
    calls of entry 6 of `DEVIATIONS.md` were seen for the program of
    32 bits. Under the extender, DS of real mode holds what AX holds
    at a call that takes no address, and is not on the program's data
    at any.
  - `harness/notice.py` tries a change on the program, through its
    scenarios, as on the C translation: the file that it is given says
    which tree.

- The numbers of 8 and 16 bits are held by a test of their own,
  `portable_c-width`, since the piece N0 of step 2 (`TODO.md`, "N0, the
  finder, the record, the types", which has what a cluster does, in
  order). `harness/width.py` finds every such number that a file
  declares and every cut, where a number that may not fit by the types
  is put into so few bits; `harness/width.txt` has them, with a reason
  for each in what has been sorted and the counts of the rest. Using it
  and keeping it up:
  - It fails for a count that has moved, fewer as well as more, in a
    file that is not sorted, and prints the lines of the record that
    differ: `python3 harness/width.py --record`, and read what moved.
    A number or a cut that is new in something sorted is written
    without a reason, and the test fails until the reason is under it
    in the record, or the number is widened.
  - A function that is widened moves the counts of other files: one
    that returns 32 bits where it returned 16 is a cut more in every
    file that keeps its answer in 16 bits, and one that takes 32 bits
    is a cut fewer where a caller counted on it to cut. Those are for
    the cluster of that file, and are committed as counts.
  - Before a parameter is widened, `--show NAME` lists the calls of the
    tree that hand the function more than it takes. Afterwards nothing
    lists them.
  - A file may declare another file's function for itself, and that
    line changes with the function: `grep` the name in the whole tree.
    The compiler stops for one that was left.
  - What the finder does not name, and a widening can get wrong: a mask
    that was written for 16 bits and now cuts, `length & 0xFF00` for
    all but the low byte, which is `length & ~(u32)0xFF` of 32 bits; a
    `~` or a `-` of the variable; a line that holds it against 0FFFFh,
    for a number that got there by stepping 0 back; and that a number
    is only ever 0 or 1. Each is read, at every line that names the
    variable.
  - A number that the program counts on wrapping at one place is
    widened, and the cut is written there with `wraps:` in front of its
    reason: `more = (u16)((second << 8 | second & 0x7F) - 1);` in
    `LnStr` of `line.c`, where `more` was a `u16` and the line had no
    cast.
  - A character, a colour, a place of a row and a column, and what the
    system answers in stay as wide as the translation has them, 16 bits
    as a rule, with one word for the reason: half of the numbers of the
    first five files did.
  - A reason is written into `harness/width.txt` by hand, or for many
    at once by a script of a dozen lines that is kept in `build/`: the
    entries are divided by empty lines, the first line of one is its
    key, a cut has its line behind `    | `, and what follows is the
    reason. `--record` puts the file into its form again and keeps
    them.
  - `harness/int16.py` knows `bool` and `i32`. Its test stays, and a
    widening moves its places too: one that makes 32 bits of a
    difference of two numbers of 16 is a new place, `widen`, with its
    reason.

- The panels and the room are three blocks since the piece F1 of step
  2 (`TODO.md`, "F1, three blocks where there was one"), and nothing
  is moved: a panel has 64K from the start. What that met:
  - A piece that changes where things lie in the memory cannot be held
    to the images of the build before: the room differed in 142 of 155
    scenarios, by where the old list of a directory lies. What held it
    was a third build, not kept: the source of before with three lines
    changed so that it lays the memory out as the piece does and is
    otherwise the old code. Against that the images were equal. Make
    the build that separates the layout from the code before reading
    17,000 differences.
  - What is declared as changed is not looked at. `--changed "behind
    the panels"` or `--changed TmpBuf` passes anything there: declare
    a word or a count, not a buffer.
  - A piece that changes the image's tool saves its "before" with the
    tool as it was.
  - The clock's separator is a colon in the second half of a second:
    it is in the program's copy of the screen, and a build that is
    much slower or faster to a step has the other. `save --early` is
    too fine a trial for it.
  - The 16 bytes of a pattern round each block are looked at when a
    scenario ends, and a scenario that has written into them fails
    `portable_c-scenarios` with the block and the byte. The line says
    "in front of block 1" of a write behind the end of block 0: they
    are the same bytes.

### Moving a file to the system's functions

What the program asks of the system it asks through
`src/portable_c/sys.h`, functions named for what is asked, with
`dos32.c` as their one file for DOS: read the top of `sys.h` before
anything else. Every file has gone over. None fills a `struct DosRegs`
or a `struct BiosRegs` and hands it to an interrupt, as each did and as
the C translation does: `Intr21`, `Int21d`, `Int10` and their like are
gone from the tree, the two structures are `dos32.c`'s own, and `python3
tools/calls.py` counts no such call. These notes are what the move met.
They are kept for whoever writes a call of the system anew, brings code
over from the C translation, or adds a function to the interface: the
steps and the table say how one call goes over, and the last notes how
a function is added.

Since the 16-bit build went, on 13 October 2026: `dos.c` in the notes
below is the file that the calls moved into, which went with that
build, and what it held is `dos32.c`'s, written for a flat memory
under the extender (`TODO.md`, the pieces D1 and D2 of the plan for the
32 bits; what D2 met is further down); `resident.c` is `exec.c`. What a note says of `sys.h`, of what a call hands back and
of the traps holds as it stands. What it says a function of `dos.c`
does, with registers, segments, a stack or `_asm`, is of that file and
is history. The table was held against `sys.h` by a script when that
build went: every function that it names as what a call is, is
declared there, but for those that it says went with a command, and
the rows that named something of `dos.c` say what `dos32.c` has.

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
| 4Ah, of the program's own block | bare | `MemOwn()`: the block that DOS loaded the program into made as long as the program is. Which block that is and how long the program is, `dos.c` knew; in `dos32.c` there is nothing to make shorter, the program's memory being the extender's, and it answers that nothing failed |
| 5801h, 5803h | bare | `MemSetSearch(how)`, `MemSetUpper(on)` |
| 4B00h | asked, bare | `ProgramRun(name, tail)`, `ProgramRunBare`: the block that DOS takes with it is the system's file's to make, `Run` of `dos32.c` |
| INT 2Eh, COMMAND.COM's own | | Was `ShellRun(command)`, which came back, the stack kept and put back around the interrupt in `dos.c`: it went with Quick execute commands (`src/portable_c/DEVIATIONS.md`, 14) |
| 4Ch | bare | `ProgramEnd(code)` |
| INT 20h, entered at the start of the PSP | | `ProgramEndOld()`, for a DOS before 3 |
| The PSP at 0Ah, where DOS goes on when the program has ended; no call of DOS | | `ProgramEndedTake()` and `ProgramEndedGive()`: the PSP names a routine of the system's file between the two, `Exit01` of `dos.c` then and the routine of real mode `Ended_T` of `dos32.c` now, which has DOS told what the program has said beforehand, `ProgramEndedBreakCheck(on)`, when DOS has ended the program. It called a function of the program's, `told`, until the program of 32 bits had to do the same |
| 51h | bare | No function of `sys.h`, and no call any more: the entry of the 16-bit program, `Alloc0` of `dos.c`, asked where the PSP is, as the first thing that DOS was asked, by `ProgramPrefix`, static there; `Start` of `dos32.c` has it from the extender, by the selector that the program is entered with. What is read in the PSP is `ProgramTail()` and `ProgramEnvironment()`, which ask DOS nothing |
| 4Eh, 4Fh | asked | `FindFirst(pattern, attributes)`, `FindNext()` |
| 56h | asked | `FileRename(from, to)` |
| 5700h, 5701h | asked | `FileTime(handle)`, `FileSetTime(handle, time, date)` |
| 60h | asked | `FileTrueName(name, to)` |
| 25h, 35h | bare | The seven by which the program takes what DOS and the BIOS enter of it: `DeviceErrorNote()`, `DeviceErrorTake(ask)` and `DeviceErrorGive()` for INT 24h, `CtrlBreakNote()`, `CtrlBreakTake()` and `CtrlBreakGive()` for INT 1Bh, `CtrlCTake()` for INT 23h. `SetVector(number, to)` and `Vector(number)` were static in `dos.c` behind them; `PutVector(number, to)` beside them wrote the machine's table without DOS, for INT 21h, and went with that. In `dos32.c` the extender is asked, not DOS: `Note`, `Go` and `Give`, static there |

The BIOS for the screen, `Int10`, by AH: 0Fh is `ScreenState()`, 0
`ScreenSetMode(mode)`, 1 `CursorShape(shape)`, 2 `CursorTo(page,
place)`, 3 `CursorWhere(page)`, 6 and 7 `ScreenScroll(down, lines,
color, upper, lower)`, 8 `ScreenCell(page)`, 9 `ScreenFill(page,
character, color, count)`, 0Eh `ScreenType(character, page, color)`, 12h
with 10h in BL `ScreenEga()`, 1130h `ScreenFont()`; 12h with 20h in BL
was `ScreenPrintAll()` and 1112h `ScreenSmallFont()`, which went with
EGA lines (`src/portable_c/DEVIATIONS.md`, 12). For the keyboard, `Int16`: 0 is `KeyRead()`, 100h
`KeyWaiting()`, 200h `KeysHeld()`, which is the low byte already. The
mouse is done. `Int2F` with 150Bh is `DriveCd(drive)`, `Int28` is
`Idle()`. `Timer`, `Beep`, `SetDTA`, `OpenFil` and the others of
`system.c` and `window.c` are the program's routines as before, and
are called as before; `Int10m`, which scrolled a window by a line with
the mouse hidden, is `Scroll` there.

Nothing stays on the old calls. What the table lacked while the files
went over got its functions with the last of them: 51h and 58h, which
are of DOS alone, for the PSP that `Alloc0` asks for and for `SetStat`
of `resident.c`; and the BIOS's 12h with 20h in BL, by which EGA lines
has the BIOS print a screen of more lines than 25, which is of the PC
alone and goes with that command if the command goes. The map of the
memory and the watch on DOS, with 52h, 62h and 4452h, are gone.

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
  passing. Four are known to be read by something, and the entry says
  what the functions do about each: AL of 4Eh, which DOS's APPEND goes
  by, a leftover at every search of the original; AL of the BIOS's
  call 1, where some BIOSes want the video mode, in `IniData` and in
  `Blank`; CX and DL of 1130h, which a BIOS that does not know the call
  leaves as they came, in `F69_00` and in `IniData`; and BL of 1112h in
  `F69_00`, which the call takes and the original does not set. One
  that you are not sure of is not dropped: hand the work back with it,
  as "Work by several agents" has it for a deviation, and the session
  asks.
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
  `FileRename`, where `FileTrueName` writes, the block of the memory of
  `MemGive` and `MemResize`. A near pointer of the main data goes in as
  it is. The block of `ProgramRun` was one such, and is `dos.c`'s own
  now: see below, what the PSP and a program run met.
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
  `commands.c`, EGA lines, took what the main loop has in CX and handed
  the BIOS its high byte in a call that takes nothing there. When that
  went the argument was not read, which stops the build, and went too:
  from `vc.h`, from the call in `Command` of `main.c`, and from the two
  comments of `main.c` that said which registers the commands read.
- Many runs of the scenarios at once filled `/tmp`, which is in the
  memory and has a limit: with six checkouts testing, and one of them
  three copies at a time, the scenarios' tests of all of them failed in
  the same minute, the C translation's too, with "the machine stopped:
  it was reset, or QEMU ended" or "Disk quota exceeded". That was the
  machine and not the program. The harness has the disks of its machines
  in the checkout's `build/tmp` since, by itself, and nothing is to be
  set for it: `workdir` of `harness/dos.py`, which every tool of the
  harness comes to for such a directory. A run removes what it made
  there, and what a run that was killed has left, the next one removes.
  A `TMPDIR` that is set is used as it is.

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
  that it closes after a read which failed, and the user decided it:
  the program says what the original does on such a DOS, in its own
  words, by the answer of the read, and entry 6 of
  `src/portable_c/DEVIATIONS.md` says what that is on a DOS that changes
  AX there. At every call, look for a read of `value` where `failed` is
  0. One that takes the high byte alone, as `DirFunc` and `GetVol` do,
  is not of this kind: the function hands the service over there as the
  original does.
- The directory of the machines' disks, of the note above, wants a
  short path: QEMU's socket is in it, and a path of more than 107
  characters is refused, "AF_UNIX path too long". `build/tmp` of a
  checkout does; where it would not, for a checkout that lies deep, the
  harness says so and uses the system's directory. A `TMPDIR` set by
  hand to a directory two levels further down did not: the harness
  names the path that is too long then, and starts no machine.

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

What the last of it met, `resident.c`, the drive B:, EGA lines, the
extension file of a directory and the old way taken out, and how a
function is added:

- A function that the interface lacks is declared in `sys.h`, in the
  group of its kind, with a comment that says what is asked, what it
  answers and what it answers when it fails, in DOS's values and with
  no register in it; and written in `dos32.c`, as it was in `dos.c`,
  where the comment at the head of its group says the service. If it
  takes a name, it goes into `NAMED` of `harness/reads.py`. The README
  counts the functions by group, under "The system in one place", and
  the table above gets its row. What is of DOS alone and no part of
  the interface, where an interrupt goes, was in the last part of
  `dos.c` and declared in `vc.h`; of `dos32.c` nothing is declared
  there, and `vc.h` declares for it what it calls, `Main`. A function
  of `sys.h` is in more places besides, which the note on one that
  goes has, in "Notes on the portable C".
- A register that a service does take may be set by nothing in the
  original: BL of the BIOS's 1112h in `F69_00` is 0 from a line of the
  screen buffer cleared at the start of the command. Step 2 finds the
  registers that a call does not take; read also, for each that it
  takes, where the assembly sets it. A function sets what it is asked,
  and such a place is declared among what is known to be read.
- What a file handed over may be neither the original's nor nothing,
  but the portable C's own: `regs.name = 0`, to have a number in DX,
  put DS on 0 at calls that take no address, in `SetBrk` and `Exit02`;
  and a `name` that was never set handed DOS DS and DX as the stack had
  them, in `SetStat` and at the entry. The row says what the original
  has, and the commit what the portable C had.
- A function of `dos.c` may be entered on a stack that is not the
  program's: `SetBreakCheck` is, from `Exit05`, when DOS has ended the
  program, as `Exit02` of `resident.c` was while `Exit05` called that.
  All of `dos.c` reaches its
  registers through `STACK` pointers, so that works; it takes a few
  words more of the other program's stack than a structure in the
  caller did.
- The table of entry 6 of `DEVIATIONS.md` is in the order of the files,
  and in a file in the order of its routines: a row is put where it
  belongs, not at the end. What the C translation handed over has a
  column of its own, filled where it was something else than the
  original's.
- No file declares the names of the old way, so a call of one does not
  compile. What `tools/calls.py` still watches in the portable C is its
  last kind, a fixed address of the machine written outside `dos32.c`,
  and not where the segment comes from a variable, as it has to for
  the table of interrupts: `PutVector` of `dos.c` was where that was,
  and nothing writes that table now.
- The whole suite takes four to eight minutes with `ctest --test-dir
  build -j8`, where it took three while the portable C was a program of
  16 bits: the tests that read the source are done within the first ten
  seconds, and from then on the source may be edited while the
  scenarios of both C trees run. The build may not change until they
  are done.

What the PSP and a program run met, when what was 16 bits in kind left
the header (`TODO.md`, the piece S1 of the plan for the 32 bits):

- A shape of the header that only DOS in 16 bits has, a block of far
  addresses, a segment to read at, becomes what is asked: the name and
  the tail for a program run, the tail and the environment that this
  one was started with, where the memory ends. What DOS takes besides
  is made in `dos.c`, where the original has it and as it has it: the
  block is `EPB` of the main data, which keeps its 14 bytes there since
  the layout of the main data is held, filled in at every run, where
  the original fills in all but the tail once, when it starts. DOS is
  handed the registers and the bytes that it was handed.
- Before a field of the main data is given to `dos.c` like that, look
  for every line that names it: nothing else reads `EPB`. `EPB_WAS` of
  `resident.c` is its address in the original, for a register that the
  BIOS is handed, and reads no field.
- A function of the header that answers with a pointer answers with a
  `FAR` one, and what is read behind what it points at is said at its
  declaration: the environment is read on behind its end, where DOS
  has the path of the program's own file, as 68 bytes whatever the
  path, by `MnPth0` of `start.c`. Another system has to have that many
  bytes there.
- A place of DOS's own that is kept as a number stays one: `ComSpec` of
  the main data is where the name of the shell is in the environment,
  and is added to what `ProgramEnvironment` answers.
- What `dos.c` goes by that something else sets was said in `vc.h`,
  with what is of DOS alone: `PspSeg`, which the entry of the program
  set by `ProgramPrefix`, and the toy of `harness/trial32/sys.c` in an
  entry of its own. The entry is `dos.c`'s since the piece S3, and the
  toy is entered there as the program is. A function of `sys.h` is also a function
  of `dos32.c`, made or not, which the build compiles for 32 bits with
  the rest; and what is of DOS alone and outside `sys.h` is in no
  `dos32.c`, so the build, which links the tree for 32 bits too,
  stops at each such name as one that the linker does not find.
  `python3 harness/trial32/both.py` took seconds and said whether the
  two files still answered alike; since `dos.c` went,
  `harness/trial32/asked.py` runs the toy with `dos32.c`, and nothing
  judges what it writes.

What the critical error, Ctrl-Break and Ctrl-C met, when where they
arrive went into `dos.c` (`TODO.md`, the piece S2 of the plan for the
32 bits):

- What a system enters of the program is two things, and they are
  kept apart. Where it arrives, with DOS's registers, a segment
  register to load and an `IRET`, is the system's: an interrupt
  function of `dos.c`, `FatErr`. What the program does about it is a
  function of the program's, of a type that `sys.h` names,
  `DeviceErrorFn`, which the program hands over where it takes the
  thing, `DeviceErrorTake(FtErr1)`, and which is called through a
  pointer as any function is. It takes what it needs by what that is,
  in DOS's shapes as everything of `sys.h` does, and no address of
  real mode: of the header of the device that DOS names, whether it is
  a drive, and a pointer to the name if not.
- Each of the seven functions is one call of DOS, and the row is the
  program's: `HookVct` notes who has the critical error and who has
  Ctrl-Break and then takes both, as the original asks, and a function
  that noted and took one of them at once would have changed the
  order.
- What `dos.c` keeps for the program's sake stays where the original
  has it while the layout of the main data is held: `ErrVct` and
  `BrkVct`, as `EPB` before them. Look for every line that names such a
  field first; nothing else read these.
- The scenarios' test holds a function of a batch to have run by its
  name in the linker's map, among the symbols that are not static:
  `FatErr` is not static in `dos.c` for that, and keeps the name of the
  routine. A function that moves and becomes static fails the test
  with "the link map has no function", behind every scenario passed.
- The call through the pointer has its arguments in registers and
  costs the stack what the call of a static function cost: every
  scenario left to the byte what it had left.
- DOS enters the critical error with the machine kept from
  interrupting, and the compiler's interrupt function leaves it so:
  the box gets its keys because `Window`, `MovScr` and `MinBx3` call
  `IntOn`, as the original has `STI` there. Three of its four callers
  being of the common code, `IntOn` is a function of `sys.h`, beside
  the critical error, and of `dos32.c`. Having the entry let the
  machine interrupt before it asks the program would take those calls
  out of the common code, and would be the program doing otherwise
  than the original between the entry and its first `Window`: that is
  asked first.
- A decision of the program's that is still in `dos.c`: `AskEs` clears
  `md.ErrDat` in front of a call and makes the error 53h of a call
  behind which it is set. What the function answered does not say
  that: it answers `DEVICE_FAIL` with `BlocEr` set and no box shown,
  too, and such a call is asked about like any that failed. `TODO.md`
  says what moving it would take.
- A macro of `dos.c` may not have a name that `vc.h` has, which the
  file includes: W140 stops the build, as it did for `ENTRY`.

What the entry, the ends and Quick execute met, when the last of DOS
outside `dos.c` went into it (`TODO.md`, the piece S3 of the plan for
the 32 bits):

- What DOS enters is `dos.c`'s, and what the program does there is the
  program's, and the cut between them is where the order of DOS's calls
  is: the entry asks DOS one thing, where the PSP is, and calls `Main`;
  the version, the block made shorter and how DOS looks for memory stay
  a row of the program, `Alloc3` of `resident.c`, in the original's
  order, and each thing in that row that only `dos.c` can do is a
  function of `sys.h`. Moving a row into `dos.c` whole would have moved
  a call in front of a message of the program's.
- A register that the program that started this one is left with when
  DOS has ended the program is `dos.c`'s to leave, and it may be what a
  call of the program's left there: DL is what DOS was last told of
  Ctrl-C. `dos.c` has that from the program, which says it beforehand,
  `ProgramEndedBreakCheck`, and reads nothing of the program's for it.
- A function that is handed over and is static in its file is compiled
  far and called through the pointer far, as `FtErr1` is.
- What the system does when it has ended the program cannot be a
  function of the program's that is called then: under an extender the
  program is gone before DOS goes on, and nothing of it can run. So the
  program says beforehand what is to be put back, a value, each time
  that it has noted it anew (`SetBrk`), and each file of the system's
  functions puts it back in its own way: `Exit05` of `dos.c` by the
  function that tells DOS, `dos32.c` by eight instructions of real mode
  in its block below the megabyte, `Ended_T`. A thing that a system may
  have to do with the program gone is asked of the program as a value,
  not as a function.
- Instructions of real mode in `dos32.c` are bytes in a table, with the
  instruction beside each as a comment, and the places that they read
  behind them in the block, read through CS: the compiler writes for a
  segment of 32 bits. `ndisasm -b16` reads the bytes back; what holds
  them is the bench: `harness/trial32/ended.asm` run against the
  program, which prints the registers that it gets back and whether
  DOS looks for Ctrl-C again, the scenario `ended`, and the toy of
  `both.py`, `asked.py` since, which ends with its end left taken so
  that DOS goes on in the routine.
- `_asm` in a function that is static and naked: the function is near,
  `ret` is a near return, and the call of it is near. `seg` of a
  variable of the program's data is the segment of its group, `DGROUP`,
  the one that DS is on: `mov ax,seg ShellSp` in `Int2E`. A far pointer
  in `parm [si dx]` has its offset in SI and its segment in DX. BP in a
  `modify` list is not saved, so the instructions push and pop it; ES
  may be listed there, DS may not.
- What does not return cannot be a function of `sys.h` that another
  system makes by a call which does. Before taking such a thing as it
  is, read what the original does behind it: it set its stack anew
  behind INT 2Eh because it never returns from running commands at
  all, and the C had a second way into its loop for that alone,
  `Again`. With the stack kept and put back, the loop of the commands
  goes on behind the call, and what DOS is asked is the same row.
- The compiler for 32 bits takes `_asm` with registers of 16 bits,
  `seg` and `offset` of a segment of 16 bits, and `__loadds`, with no
  word said. So that a file compiles for 32 bits does not say that it
  has none: `resident.c` compiled the moment its interrupt function had
  left it, with `Int2E` still in it. The host check's record is what
  says that a file has none.
- `dos.c` had a `FLAG_ZERO` of its own, for the keyboard, when the
  flags of `resident.c` came over under the same names: look for a
  name in the file before bringing a block of definitions into it.
- A function is in more places than the six that the piece S1 has: a
  row of entry 6 of `DEVIATIONS.md`, if a register that the call does
  not take goes with it; and the README's paragraphs under "The system
  in one place", "What rests on Open Watcom" and "Inline assembly",
  which say where each thing of the compiler's is, by file.
- The toy of `harness/trial32/sys.c` is entered where the program is,
  at `Start_` of `dos32.c`, as it was at `Alloc0_` of `dos.obj` in 16
  bits, and defines `Main`, the one name of the program that the file
  of the system's functions calls. The build links the objects for 32
  bits and nothing else, `build/portable_c/VC.EXE`: a name of DOS that
  the tree asks for outside `sys.h` shows there as one that the linker
  does not find.
- The stack figures of the scenarios follow the layout of the file in
  two or three scenarios, `command`, `mouse-select` and `mouse-find`,
  by 2 to 16 bytes, at every change of the code: hold a change to the
  figures of the others.

What the rest of `dos32.c` met, the functions that the extender does
not carry through as they are (`TODO.md`, the piece D2 of the plan for
the 32 bits):

- The extender puts a selector into the PSP at 2Ch, where DOS has the
  segment of the environment, and DOS reads that word itself: a program
  run by 4Bh with no environment named gets a copy of whatever lies at
  that selector's number taken for a segment. `Run` of `dos32.c` names
  the environment's segment. Run through the shell, a program has an
  environment all the same, FreeCOM's: try such a thing with a program
  that is run by itself.
- What a toy of the trial did once and came back from says little: the
  trial's command ran with no environment named, through the shell.
- Where DOS writes into a buffer of the program's and the call is made
  in real mode, the program's bytes go down to the block first and
  come up afterwards, as many as DOS may write: then the program has
  what DOS wrote and, of the rest, what it had, as when DOS writes
  there itself. FreeDOS writes 24 of the country's 34 bytes and leaves
  a path alone where 60h fails, and the toy of both builds, which
  fills each buffer first, shows either.
- The extender is entered on the program's stack at every interrupt
  that the program makes, and keeps there what it needs of the call
  until that comes back: 190 bytes below the stack pointer at a call
  of DOS that it translates, 130 at one made by DPMI 0300h, 74 at any
  other. Whatever goes on on the program's stack while such a call is
  under way, as the routine of the critical error does, starts below
  that.
- A routine that the extender enters from real mode, for an interrupt
  taken by DPMI 0205h, has the registers of real mode in their low
  words, DS and ES anywhere, and a stack of the extender's. `wcc386`
  takes it as a `__declspec(naked)` function with `_asm`: `mov
  bx,word ptr cs:Variable` reads a variable through CS, `mov
  esp,Variable` and `call Function` are taken as written, and a static
  function that only `_asm` calls is kept. Give the function that it
  calls a `#pragma aux` with its registers, so that the call does not
  rest on the compiler's options.
- Nothing promises that what has no value is zeros when a program of
  32 bits is entered. A variable of `dos32.c` has a value, `= 0`, but
  for the 640 KB.
- DOS on the bench has free blocks besides its largest, and how many
  and how long differs between the two countries: a toy that is to be
  given the same blocks in both builds takes all of them first.
- Asked for the capital of a letter before the country has been asked
  for, the toy of 16 bits ends in "Invalid Opcode": `CaseMap` of
  `dos.c` calls what its four bytes hold. A toy of both builds asks
  only what the program may ask.
- `BREAK` off and Ctrl-Break at DOS's prompt end no step of a script,
  whatever ran before: it looks like a machine that has stopped and
  is none.
- When DOS ends a program of 32 bits by another's hand, the extender
  sees the call, since it has INT 21h of real mode while the program
  runs, and takes itself out, with the interrupts that the program
  took through it. `harness/trial32/ended.asm` shows it.

What a program that a command has left in the memory met, when the
program of 32 bits is left: a mend that was made for it and dropped on
the user's word of 13 October 2026, and what stays of it (`TODO.md`,
the piece L of the plan for the 32 bits, which has the mend in words):

- A setting of the extender that was "seen" to mend something on a
  copy of the file is looked at on every screen of the run, the first
  above all. A program that the extender does not load leaves DOS's
  prompt, which takes the steps of a script as the program would: one
  bit of the settings cleared was taken for a mend so, the user was
  told that the mend was one bit, and what the steps had shown was
  DOS. What it took then was two other bits, more of `dos32.c` and a
  routine of real mode, and the user, told that, had it dropped.
- What the extender does is read in its file where nothing describes
  it. `binw/dos32a.exe` is not packed: `ndisasm -b16` of it, from where
  its header has its two segments of code, kept in `build/` and not
  committed, and nothing of the listing written down but what the
  extender does, in words. The names of the options of its environment
  variable, `DOS32A`, are in the file, and the code that takes each
  sets a bit of the settings: that names the bits. The piece L has
  what was read.
- As it leaves, the extender puts all 256 interrupts of real mode back
  as they were when the program started, with a line of warning for
  each that was otherwise, by two bits of its settings; and whatever
  its settings say, it has INT 1Bh, 1Ch, 21h, 23h, 24h and 2Fh go where
  they went when it started, and INT 15h where it has taken that,
  whoever has taken them since. And told to give INT 1Bh, 1Ch, 23h or
  24h back in protected mode, it has the interrupt of real mode go
  there too.
- INT 21h of real mode is the extender's while the program runs. A
  program that takes it meanwhile hands on to the extender, so nothing
  in the program can leave it that interrupt when the extender has
  gone.
- The scenario `stay-left` shows all of it, and the program of 32 bits
  is held to what it shows there by a patch of its own, the difference
  being declared as it is on the user's word (entry 7 of
  `DEVIATIONS.md`; `TODO.md`, the plan, 10 of what was open). Its
  second program finds the first by INT 2Fh, so where that one is lost
  it prints dashes for all five, whatever else still goes to the
  first: such a row says less than it seems to, and the entry says
  what is known by something else.
- A patch that a build has of its own may lack a row that the tree's
  patch has, where the build shows the original's row there:
  `stay-left` at DOS's prompt. `goldens.py patch` says "gone" for it,
  and the patch's reason says why.
- A scenario that goes on at DOS's prompt when the program has been
  left: with `"cls" Enter` as its last command, or the prompt that the
  program leaves is written into a row of what the command before
  printed.

What a trial of the extender's memory met, DPMI 0500h to 0503h under
DOS/32A, before the memory of the panels and the texts was taken from
it (`TODO.md`, "X, the extender's memory on the bench", which has the
facts row for row and what the piece F9 goes by):

- The extender keeps 16 bytes of its own in front of every block, in
  the memory itself, and looks at them all at every call for memory,
  0500h too. A write behind the end of a block ends the program at the
  next such call, "DOS/32A run-time (6004): extended memory blocks
  have been corrupted", with the name of the block, which is its
  address less 16. A scenario that ends so has written behind a block.
- A block is given with what the memory held, and a block made shorter
  has the extender's 16 bytes written behind its new end.
- A toy that asks what is in memory must not have taken and given back
  blocks there first: the search by halving of `blocks.c` left the
  extender's 16 bytes at two dozen places, and the first reading of
  "what a machine that has just started has in its memory" was of
  those.
- `harness/trial32/run.py` starts a machine for every file that it is
  given. Two programs on one machine are one file and steps that type
  the second at DOS's prompt, `q '"d:\vc\vc" Enter'`, with `--extra`
  for another file. A capital letter is `Shift+s` in a step.
- A program that is to be had with little memory is run from
  `harness/trial32/second.c`, which keeps all of the extender's memory
  but as much as its keys say.

## Work by several agents

Work that can be divided is done by several agents at a time, without
waiting for the user between its steps.

- Each piece is the work of an agent of its own, started fresh, so that a
  run does not fill one context. The session that runs them does not do
  the work: it starts each agent, checks what the agent left, which is
  the commits, the push and `LastTest.log`, and keeps its report. Nor
  does it run or read what prints much, on the user's word of 13
  October 2026: the suite on `main`, a search through the documents, a
  small correction of a text are an agent's too, and the session keeps
  the conclusion. Its context is for the queue and for the user.
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
- The test bench changes two things, and gives every scenario the setup
  file that the original saves after making them in its own menus. The
  clock is off. And the panel on the right lists files, where the
  distribution has the Info panel there, on the user's word of 13 October
  2026: what the Info panel shows is of the machine and of the system,
  and with it on every screen every difference there was a difference in
  every scenario. A scenario that is about the Info panel switches it on.
- A scenario that shows the time of day, with the clock switched on or in
  the time stamp of a file it has made, does so within the first minute of
  its machine. The machine's clock is counted in the instructions it
  executes, and a C tree takes more of them than the original to get to
  the same step: a tenth to a third more as a rule, and twice as many
  where a step searches a text, as counted over every scenario on 13
  October 2026 (the top of `harness/scenarios.py` and `TODO.md` have
  the figures; the sixth that stood here was of two scenarios). The
  first screen is at about five seconds past the minute. So such a
  scenario is tried in every tree that it is run for, and is done
  showing the time by about fifty seconds past in the slowest of them,
  which for most scenarios is by about forty in the original. A scenario
  that switches
  the Info panel on takes some five seconds more, once, when the panel
  is first drawn.
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
