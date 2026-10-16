# The assembly and the C translation

How `src/asm/`, the original source, is built byte for byte with free
tools, and how `src/c/`, the C translation, is made and laid out. Both
trees are frozen: the assembly changes only as far as building it with
free tools takes, and the C translation only where it is found not to
behave as the original.

## Matching the shipped binaries

The binaries Volkov shipped in 2000 are the reference: what is built
from `src/asm/` must match them byte for byte. His own toolchain,
Borland's TASM and TLINK, is commercial software that this project does
not fetch or run; matching his output with a free assembler shows that
nothing was lost by replacing it. The test `asm-bytes` holds this.

The byte comparison has one complication. The binaries Volkov shipped were
shareware, and contain registration code, tables and strings that the source
he later published leaves out. That material is his and is not reproduced
here. Instead, the source can be assembled with room reserved for it: the
same number of bytes in the same places, filled with NOPs and zeros. Every
other byte then sits at the address it has in the original, and must be
identical to it. The reserved room is 109 of 65,090 bytes in `VC.COM` and
691 of 24,278 in `VCSETUP.COM`, in five places each.

## Building the original and the C translation

The original is assembled. The build fetches
[JWasm](https://github.com/Baron-von-Riedesel/JWasm), a free
MASM-compatible assembler, at a pinned commit, builds it, and assembles
`src/asm/`, the source as its author published it, into `build/VC.COM` and
`build/VCSETUP.COM`. It also assembles a second copy of both into
`build/shareware-layout/`, with the room reserved that the byte
comparison needs, and both there once more as object files, for their
relocations; only the tests use those. That is what the test
`asm-bytes` holds against the binaries that were shipped, and
what says where the original has its routines and its data. The
assembler also builds the driver of the tests, `harness/DRIVER.ASM`,
which is no part of either program.

The translation is compiled: `build/c/VC.EXE`, which is what the tests run
and hold against the original. It is C and nothing else, `src/c/*.c`,
and no assembler has a hand in it. What the compiler made of each C
file is written out as a listing, `build/c/*.lst`. For this the build
fetches a pinned
snapshot of [Open Watcom](https://github.com/open-watcom/open-watcom-v2), a
download of 150 MB. Open Watcom provides tools for Linux and macOS on x86-64
and ARM and for Windows on x86-64; the build has been run on Linux x86-64
only.

## Changes to the source

Everything changed since the source was imported can be listed with
`git diff $(git rev-list --max-parents=0 HEAD):src HEAD:src/asm`: the first
commit has the files in `src/` itself. There are three kinds of change.

**Syntax.** The original source is written for TASM; these changes make
JWasm accept it:

- The `JUMPS` directive is gone: JWasm lengthens out-of-range conditional
  jumps by itself. One `LOOP` that is out of range is spelled out as a jump
  over a jump.
- Structure fields are global names and labels inside procedures are global,
  as in TASM (`OPTION OLDSTRUCTS`, `OPTION NOSCOPED`).
- `LABEL` inside a `STRUC` became a zero-length field or, where the structure
  has initialised instances, an alias defined after it.
- Structure instances are written `<>` instead of `?`.
- Procedure headers use MASM's spelling for the language, `USES` and `LOCAL`.
- Code and code labels that sit in a data segment are preceded by
  `ASSUME CS:DGROUP`, an indirect `JMP` states its operand size, and
  `VcVers` is a text equate.
- `VCSETUP.ASM` declares its segments up front so that uninitialised data
  comes last and takes no space in the file.

**Encodings.** Where JWasm and TASM encode the same instruction
differently, the source now says which encoding to use, so that the output
matches the shipped binaries:

- Arithmetic on `AX` with a small constant carries `WORD PTR`, which selects
  the accumulator form of the opcode.
- A handful of jumps that TASM left in their long form are written that way
  explicitly, as `JMP NEAR PTR` or as a conditional jump over a jump.
- Two `LEA reg,address` are written as the `MOV` TASM turned them into, and
  one displacement is written so that it fits a byte.
- The byte-sized locals of `Input0` are declared two bytes long, because
  TASM gives each a whole word of stack.

**Layout.** `VCLAYOUT.INC` defines two macros, `ShwCode` and `ShwData`,
which are placed wherever the shipped binaries have something this source
does not. Each names a number of bytes. Normally they generate nothing.
When `SHAREWARE_LAYOUT` is defined they generate that many NOPs or zeros, so
that what follows lands where it does in the shipped binaries. The filler is
harmless: the program laid out this way still runs.

## The C translation

`src/c/` started as a copy of the assembly in `src/asm/` and ends as a C
program. In between it was both. A routine that had been translated was
deleted from `src/c/asm/` and left a few instructions behind that passed
its callers on to the C function, and where C code needed a routine that
was still assembly, the assembly got an entry for it that C could call.
All of the code is C now, and those are gone, and `src/c/asm/` with them:
the last file there held no instruction and no data any more, only the
order of the program's segments. The C code keeps the names
the routines and their data have in the assembly, so that either can be
found from the other, and a function still takes its arguments in the
registers that the assembly called it with. The two routines that the
compiler calls to multiply and to divide numbers of 32 bits, which a C
library would bring, are C too, `src/c/arith.c`: the program has no C
library.

DOS, the BIOS and the mouse driver are called by software interrupts,
which C cannot express. The C code calls them in one file, `src/c/dos.c`,
and nowhere else: twelve small functions, each a line of C around the
instruction, which is inline assembly there, with the registers as its
arguments; one has three such instructions that may not be parted. Four more are there for a port of the machine and its
interrupt flag, for which the compiler has functions of its own, and for
DOS's routine that makes capitals. Everything else is plain C that calls
these, and reads the BIOS's data and the memory of the display through
pointers. That file hides nothing of DOS: the portable C, not this
tree, is where the program meets other systems.

The part of the program that stays in the memory while a command runs is
C in `src/c/resident.c`, with its data. The
rest of the program is not in the memory then, `src/c/dos.c` with it, so the
build compiles that file a second time, into that part: one source, two
copies of its code. `src/c/resident.c` has the only other inline assembly of
the translation, four instructions where an interrupt arrives that the
program has taken, INT 21h and INT 27h: they make room on the stack for a
second return address, and an interrupt function of the compiler's takes
over, which saves the registers and finds the program's data. Such a
function has to go on in the handler that had the interrupt before, with the
stack as the interrupt left it, and C cannot change how deep the stack is
that it returns on: so it writes that handler's address into the room,
and returns there. The same four stand where DOS goes on when it has
ended the program by itself. In front of the four of INT 21h stand
thirteen more: every call of DOS that any program makes comes there
while the program is in the memory, and those that it does not deal
with, nearly all, are told from the others and passed on before
anything is pushed, by six comparisons, their jumps, and a far jump to
the handler before. So the program takes nothing of the stack of whoever
calls DOS, which may be short, and DOS is entered with the flags of
those comparisons, as in the original; compiled code saves registers
before it compares and promises nothing of the flags. And nine
instructions are the interrupt by which the COMMAND.COM in the memory
runs a command, INT 2Eh: it comes back with no stack and no data
segment, so both are set there, from the code segment, before any
compiled instruction runs, and the program goes on in a function of its
own on an empty stack. `src/c/main.c` has one instruction, which loads
DS with the segment of the main data, known only when the program runs:
where DOS calls the program for a critical error, and where the part
that stays in the memory calls the rest of the program, which is one
function to it, and gets DS back as it was.

The program is linked as `VC.EXE`, which is not built the way a C
program usually is. It still loads and starts like the `.COM` file it
was, keeps its data where the original has it, and gives up and reloads
most of itself at run time, compiled code included. `src/c/vc.h` describes
what that asks of the C code, and `tools/mkexe.c` how the file is made.
The code that begins the program and reloads it after a command had to
change for this: it measures in paragraphs and relocates what it has
loaded.

The file is the program as it lies in the memory. First comes the part
that stays there while a command runs: room for the PSP, which
`tools/mkexe.c` takes off, its compiled code, and its data
with the stack at the end, which are nothing but room in the file. That
data is of `src/c/resident.c`, the one C file
with data of its own: its texts, the numbers that `tools/mkexe.c`
writes, which it finds by their names in the linker's map, one
structure for what the part keeps, in the order of the original, and
the stack. Code and data of that part are one group to the linker,
which begins at the PSP, as the program's one segment did when it was a
`.COM` file: CS, DS and SS are on the PSP there, and every offset
counts from it. Then, a
paragraph further on, the part that goes while a command runs: its
constants first, which are a group of their own, then its code, in two
segments. The
original has that part where the data of the first will be and moves it
up as it starts, over itself; the translation is loaded with it in place, on
its own stack from the first instruction, and has only to make a block of
memory of it. That costs 6,384 bytes of the file, and it is what let
compiled code stand in the first part at all: nothing can lay
data over code that the compiler has yet to make. It also leaves the
code that begins the program in the memory, where the original lays its
data over that code.

None of that layout is in the C language. The order of the segments is
the linker's to be told, in its command; that the code and the data of
the first part are one group, and where each kind of data goes, are
options of the compiler; and `tools/mkexe.c` holds what comes out
against what the program needs, and stops the build where it differs.
[What rests on Open Watcom](#what-rests-on-open-watcom) has each such
place.

Every step was checked by the `c-scenarios` test, and nothing else
decides whether a translation is right. The routines are listed there, batch
by batch, in `harness/ported.py`.

The program's texts and tables are checked by themselves, by the `c-data`
test: a table that no screen shows can be wrong in every scenario and
pass. The boxes and dialogs name their texts, their buttons and one
another by address, words that are offsets in the one segment the
assembly has them in. In C they are constants written with the names of
what they name, in files that hold nothing else (`src/c/help_t.c`, with
`src/c/tables.h` for how they are written): the compiler puts them into
a group of their own, where a word that names one is its offset, as it
is in the original's one segment. They are apart from the code because the code has its data
segment on memory that the program asks DOS for, and would read a
constant that it names there. The test holds each against the original
by its name: the bytes, and for a word that holds an address the thing
that it names, which the original's source, assembled once more as an
object file, tells by its relocations.

The tests run the program that is shipped, as they run the original: a
driver in the machine takes either through the scenarios (see [When a step
is finished](testing.md#when-a-step-is-finished)). The driver also writes the
program's stack out whenever a step ends, and the `c-scenarios` test reports
for every scenario how much of it the program never used.

The stack is where the translation departs from the original in what it
takes of the memory. The original has 256 bytes of it. Compiled code uses
more stack than the assembly it replaces, and a stack used beyond its bottom
would overwrite data without a trace on the screen, so the translation has
4,096 bytes, of which the scenarios use some 200. The stack is the end of
the part of the program that stays in memory while a command runs: that part
is 19,728 bytes where it was 12,704, and a command has 7,024 bytes less
than the original leaves it: 3,840 for the stack, and the rest because
compiled code is longer than the assembly it replaces, and because the
code that starts the program stays.

### What rests on Open Watcom

The translation is C, but it is not all C that any compiler would take or
would make the same program of. These are the places, so that whoever
carries the program to another compiler has them in one list. Each has
a comment where it is done.

First what the code is written in, which shows wherever it is used and
which a 16-bit program for DOS with the original's layouts cannot do
without:

| What | Where | How many |
|---|---|---|
| `__far` and `__near` pointers, and pointers based on a segment, `CODE` and `STACK` of `src/c/vc.h` | every file | some 1,200 `__far`, 475 `STACK`, 97 `CODE` |
| `#pragma aux`: the registers that a function takes and returns, kept from the assembly, and the name that the linker has for it | every file | 866 |
| `#pragma pack(1)` for the structures that are the original's | `src/c/vc.h`, `src/c/tables.h` and five files with structures of their own | 8 pairs |
| `__interrupt` functions, which save the registers and load DS | `FatErr` of `src/c/main.c`; `CtrlC`, `Int21`, `Int27`, `Exit05` of `src/c/resident.c` | 5 |
| `__loadds` functions, which load DS when they are called from the other part | `SetBrk`, `Release` of `src/c/resident.c` | 2 |
| `__declspec(naked)` functions, which are their inline assembly and nothing else | `New21`, `New27`, `Exit01`, `Int2E` of `src/c/resident.c` | 4 |
| Intrinsic functions: `inp`, `_disable`, `_enable` | `src/c/dos.c` | 3 |
| The two routines that the compiler calls for arithmetic on 32 bits, under its names and with its registers | `__U4M`, `__U4D` of `src/c/arith.c` | 2 |

Then what cannot be seen in the code: where the program is right only
because of what this compiler and its linker do with it. The language
promises none of these. `tools/mkexe.c` stops the build where one of
the first eleven does not hold; the others would show in the tests.

| | What is relied on | Where it is done | What holds it |
|---|---|---|---|
| 1 | The option `-nd=R` gives the data of a file a group of its own, `R_GROUP`, which its code reads through DS without loading a segment, in four segments of the class `FAR_DATA`: `RCONST`, `RCONST2`, `R_DATA`, `R_BSS` | `CMakeLists.txt`, for `src/c/resident.c` | `tools/mkexe.c` knows the four names, and refuses any other data |
| 2 | The option `-g=R_GROUP` puts the code segment into a group of that name, and the linker makes one group of two of the same name: the part that stays is one segment to the program, the PSP's | `CMakeLists.txt`, for `src/c/resident.c` and the second copy of `src/c/dos.c` | the group has to begin at 0 |
| 3 | The linker's `order` directive, which its help does not list for DOS: the order of the classes and of the segments within one | `CMakeLists.txt`, the linker's command | the checks of 2, 6, 8 and 11 |
| 4 | A segment holds what the object files have of it in their order: `resident.c` is linked first, `transit_t.c` first of the part that goes | `CMakeLists.txt` | the checks of 6 and 10 |
| 5 | The compiler writes the objects of the code segment that have a value in the order of the file, and those without in front of them: so `Vect21` is given a value, `= 0` | `src/c/resident.c` | the check of 6 |
| 6 | So the room of the PSP, `Psp`, is the first thing of the code segment and of the program | `src/c/resident.c`, its first lines | `Psp` has to be at 0 |
| 7 | What has a value is in `R_DATA` in the order of the file, so `Stays`, written last and with a value, zeros, is the last of it: the part is nothing but zeros from there on, which the check sum of the file leaves out | `src/c/resident.c` | from `Stays` to the part that goes the file has to be zeros, and the seven words that are written in have to stand in front |
| 8 | What has no value is in a segment of its own behind the others, `R_BSS`: the stack, `DataSav`, is the one object without, and so the end of the part that stays | `src/c/resident.c` | `R_BSS` may hold no other symbol, and `DataSav` has to be at its start |
| 9 | A far pointer to something has the linker's number of its segment in the high word, and a word of the part that stays that names the part that goes is left as that number: `TRN_SEG` | `src/c/resident.c` | a number that is not 0 has to be a segment behind the part that stays |
| 10 | The constants of the files without code are in `CONST2` of the compiler's group, `DGROUP`, and begin with `TrnBeg`, 32 bytes in which there is a whole paragraph | `src/c/transit_t.c`, `src/c/*_t.c` | `TrnBeg` has to stand where the stack ends; a file with code and such constants is refused |
| 11 | No code of the part that goes is within 4K of the stack, the constants being in front of it; and no segment of code reaches into the last 4K of its 64K. Both are for QEMU, not for the program | the `order` of 3; `c_second` in `CMakeLists.txt` | read in the linker's map |
| 12 | `md`, the main data, is declared as an object and named to the linker as `Psp`: the linker takes the offset of an object in the group that the object is of, 0 here | `src/c/vc.h` | every scenario |
| 13 | The segment of a far pointer made of a near one is DS, `MAIN_SEG`, and of one to a local variable it is SS, with the option `-zu`: `ResSeg` | `src/c/vc.h`, `src/c/misc.c` | every scenario |
| 14 | An `__interrupt` function saves the registers in a fixed order, which `struct Saved` is, and loads DS with its group, by a number that is relocated; `SecDat`, the one variable of `DGROUP`, is where `FatErr` finds the main data by that | `src/c/vc.h`, `src/c/main_t.c`, `src/c/main.c`, `src/c/resident.c` | the scenarios of the critical error and of the programs that stay |
| 15 | A variable with a value in a file without code is in `_DATA` of `DGROUP`, in the file | `src/c/main_t.c`, `SecDat` | `tools/mkexe.c` refuses one without a value |
| 16 | `_enable` is `STI` and `CLD`: the first call of the program, `IntOn`, clears the direction flag, which the compiled code needs clear and DOS does not promise | `src/c/resident.c`, `Alloc0` | every scenario |
| 17 | A far call to a function of the same segment is made `push cs` and a near call by the linker, and needs no relocation: so the part that stays can call its own far functions before its relocations are applied | `src/c/resident.c`, `Alloc0` | every scenario |

What else the notes on the translation have, in `DIARY.md`, is how to
write for this compiler, not what the program rests on: a register that
it ignores in a `#pragma aux`, a pointer that it truncates.

### Inline assembly

Every piece of it, 81 instructions in 14 pieces. The first nine are a
`#pragma aux` each in `src/c/dos.c`, put in place where the function of
that file calls them; no other file calls DOS, the BIOS or the mouse
driver.

| File | Piece | Instructions | Why it is not C |
|---|---|---|---|
| `src/c/dos.c` | `Dos` | 20: `NEG AX` for the carry flag, `PUSH` of BP and DS, nine that load the registers from a structure on the stack, `INT 21h`, five that store what DOS returns, `SBB AX,AX` for the carry flag, two `POP` | A software interrupt is no call that C has, DOS takes and returns more registers than a function can, DS among them, and its answer is the carry flag |
| `src/c/dos.c` | `Video` | 7: `PUSH BP`, `INT 10h`, `POP BP`, four that store AX to DX on the stack | The BIOS for the screen; some lose BP |
| `src/c/dos.c` | `VideoBuffer` | 3: `PUSH BP`, `INT 10h`, `POP BP` | The same, with an address in ES:DI |
| `src/c/dos.c` | `Keyboard` | 3: `INT 16h`, `PUSHF`, `POP DX` | The BIOS for the keyboard answers in the zero flag |
| `src/c/dos.c` | `Multiplex` | 1: `INT 2Fh` | The interrupt that the mouse driver and Windows are asked by |
| `src/c/dos.c` | `DosIdle` | 1: `INT 28h` | The program says that it is idle |
| `src/c/dos.c` | `DesqviewPause` | 6: `MOV AX` and `INT 15h`, three times | DESQview takes the program off its stack between the first and the third: no compiled instruction may stand between them |
| `src/c/dos.c` | `MouseDriver` | 1: `INT 33h` | The mouse driver |
| `src/c/dos.c` | `MouseState` | 4: `INT 33h`, three that store BX, CX and DX on the stack | The same, which answers in three registers |
| `src/c/main.c` | `DataSeg` | 1: `MOV DS,AX` | C has no word for a segment register. DS is put on the main data, whose segment is known only when the program runs: where DOS calls the program for a critical error, and around the main loop |
| `src/c/resident.c` | `New21` | 17: six `CMP` with their jumps, a far `JMP` through `Vect21`, and `PUSHF`, `PUSH AX`, `PUSH AX`, `JMP` to the interrupt function | Every call of DOS by any program comes here. One that the program does not deal with is passed on with no register changed, nothing taken of the caller's stack and the flags of the original's comparisons; compiled code saves registers first and promises nothing of flags |
| `src/c/resident.c` | `New27` | 4: `PUSHF`, `PUSH AX`, `PUSH AX`, `JMP` | Room on the stack for a second return address, so that the interrupt function can go on in the handler before by returning: C cannot change how deep the stack is that it returns on |
| `src/c/resident.c` | `Exit01` | 4: the same | Where DOS goes on when it has ended the program by itself, on the stack of the program that started this one, to which it goes on in the same way |
| `src/c/resident.c` | `Int2E` | 9: `INT 2Eh`, `CLI`, `MOV AX,CS`, `MOV SS,AX`, `MOV SP`, `STI`, `MOV DS,AX`, `CLD`, `JMP` | COMMAND.COM's interrupt for a command comes back with no stack and no data segment: both are set from CS before any compiled instruction can run, and the function that called cannot be returned into |

Three more instructions are the compiler's own, by its intrinsic
functions in `src/c/dos.c`: `IN` for a port of the machine, `CLI`, and
`STI` with `CLD`.
