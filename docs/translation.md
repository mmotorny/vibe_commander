# The assembly and the C translation

This page covers the two frozen trees:

- `src/asm/`, the original source, and how it is built with free tools
  into binaries that match the ones Volkov shipped, byte for byte.
- `src/c/`, the C translation, and how it is laid out as a DOS program.

The assembly changes only as far as building it with free tools takes.
The C translation changes only where it is found not to behave as the
original, and then towards the original.

## Matching the shipped binaries

The binaries Volkov shipped in 2000 are the reference. What is built
from `src/asm/` must match them byte for byte, and the test `asm-bytes`
checks that. He built them with Borland's TASM and TLINK, which are
commercial and are not fetched or run here. Matching his output with a
free assembler shows that nothing was lost by replacing them.

There is one complication. The shipped binaries were shareware, and
contain registration code, tables and strings that the published source
leaves out. That material is his and is not reproduced. Instead, the
source can be assembled with room reserved for it: the same number of
bytes in the same places, filled with NOPs and zeros. Every other byte
then sits at its original address and must be identical.

| File | Reserved | Of | Places |
|---|---|---|---|
| `VC.COM` | 109 bytes | 65,090 | 5 |
| `VCSETUP.COM` | 691 bytes | 24,278 | 5 |

`harness/bytecheck.py` lists each place with its offset, its length and
what the original has there.

## The build

The build fetches [JWasm](https://github.com/Baron-von-Riedesel/JWasm),
a free MASM-compatible assembler, at a pinned commit, and builds it.
JWasm then makes:

| Output | What it is |
|---|---|
| `build/VC.COM`, `build/VCSETUP.COM` | The original, assembled from `src/asm/` |
| `build/shareware-layout/` | Both again with the reserved room, as programs and as object files (for their relocations). Only the tests use these: `asm-bytes` compares them with the shipped binaries, and the listing `VC.LST` tells the harness where the original keeps its routines and data |
| the driver of the tests | `harness/DRIVER.ASM`, which is part of neither program |

The C translation is compiled by a pinned snapshot of
[Open Watcom](https://github.com/open-watcom/open-watcom-v2), a 150 MB
download, into `build/c/VC.EXE`, with a listing of each file in
`build/c/*.lst`. It is C and nothing else; no assembler is involved.
Open Watcom has tools for Linux and macOS on x86-64 and ARM, and for
Windows on x86-64. The build has been run on Linux x86-64 only.

## Changes to the assembly

The first commit holds the source exactly as published, in `src/`, so
every change since is listed by:

```sh
git diff $(git rev-list --max-parents=0 HEAD):src HEAD:src/asm
```

There are three kinds of change.

**Syntax.** The source is written for TASM. These changes make JWasm
accept it:

- The `JUMPS` directive is gone: JWasm lengthens out-of-range
  conditional jumps by itself. One `LOOP` that is out of range is
  spelled out as a jump over a jump.
- Structure fields and labels inside procedures are global, as in TASM
  (`OPTION OLDSTRUCTS`, `OPTION NOSCOPED`).
- `LABEL` inside a `STRUC` became a zero-length field or, where the
  structure has initialised instances, an alias defined after it.
- Structure instances are written `<>` instead of `?`.
- Procedure headers use MASM's spelling for the language, `USES` and
  `LOCAL`.
- Code and code labels in a data segment are preceded by
  `ASSUME CS:DGROUP`, an indirect `JMP` states its operand size, and
  `VcVers` is a text equate.
- `VCSETUP.ASM` declares its segments up front, so that uninitialised
  data comes last and takes no space in the file.

**Encodings.** Where JWasm and TASM encode the same instruction
differently, the source now says which encoding to use:

- Arithmetic on `AX` with a small constant carries `WORD PTR`, which
  selects the accumulator form of the opcode.
- A handful of jumps that TASM left long are written that way
  explicitly, as `JMP NEAR PTR` or as a conditional jump over a jump.
- Two `LEA reg,address` are written as the `MOV` that TASM turned them
  into, and one displacement is written so that it fits a byte.
- The byte-sized locals of `Input0` are declared two bytes long,
  because TASM gives each a whole word of stack.

**Layout.** `VCLAYOUT.INC` defines two macros, `ShwCode` and `ShwData`,
placed wherever the shipped binaries have something that this source
lacks. Each names a number of bytes. Normally they generate nothing.
With `SHAREWARE_LAYOUT` defined they generate that many NOPs or zeros,
so that what follows lands where it does in the shipped binaries. The
program laid out this way still runs.

## The C translation

`src/c/` is the whole program in C. Its routines and data keep the names
they have in the assembly, so either can be found from the other. A
function still takes its arguments in the registers that the assembly
called it with, declared by `#pragma aux`. The program has no C library:
the two routines that the compiler calls to multiply and divide 32-bit
numbers are C too, in `src/c/arith.c`.

Every step of the translation was checked by the `c-scenarios` test, and
nothing else decides whether a translation is right.
`harness/ported.py` lists the routines batch by batch.

### Calling DOS and the BIOS

DOS, the BIOS and the mouse driver are called by software interrupts,
which C cannot express. All such calls are in `src/c/dos.c`:

- twelve small functions, each a line of C around a piece of inline
  assembly, with the registers as arguments;
- four more for a port of the machine, the interrupt flag, and DOS's
  routine for capital letters.

Everything else is plain C that calls these, and reads the BIOS's data
and the display memory through far pointers. `dos.c` hides nothing of
DOS: the portable C, not this tree, is where the program meets other
systems.

### The part that stays in memory

The original is in two parts. One stays in memory while a command runs.
The other is given back to DOS for the command and loaded again after.

The part that stays is `src/c/resident.c` with its data. Since the rest
of the program, `dos.c` included, is not in memory during a command,
the build compiles `dos.c` a second time into the resident part.
`resident.c` also has the translation's other inline assembly:

- **Entries of INT 21h and INT 27h.** Four instructions make room on the
  stack for a second return address. A compiler interrupt function then
  takes over, saves the registers and finds the program's data. It has
  to continue in the previous handler with the stack as the interrupt
  left it, and C cannot change the depth of the stack it returns on, so
  it writes that handler's address into the room and returns there. The
  same four instructions stand where DOS continues after ending the
  program by itself.
- **The INT 21h filter.** Every DOS call made by any program passes
  through while the program is resident. Thirteen instructions in front
  of the entry pick out the few calls the program handles, and pass the
  rest on to the previous handler before anything is pushed. This takes
  nothing of the caller's stack, which may be short, and enters DOS with
  the flags of the comparisons, as the original does.
- **INT 2Eh.** COMMAND.COM's interrupt for running a command returns
  with no stack and no data segment. Nine instructions set both from
  the code segment before any compiled code runs.

`src/c/main.c` has one more instruction, which loads DS with the segment
of the main data, known only at run time. It is used where DOS calls the
program for a critical error, and where the resident part calls the
rest of the program.

### The layout of VC.EXE

`VC.EXE` is not built the way a C program usually is. It loads and
starts like the `.COM` file it was, keeps its data where the original
does, and gives up and reloads most of itself at run time, compiled code
included. `src/c/vc.h` describes what that asks of the C code, and
`tools/mkexe.c` makes the file.

The file is the program as it lies in memory:

1. **The part that stays.** Room for the PSP, which `tools/mkexe.c`
   takes off; the compiled code of `resident.c`; then its data, with the
   stack at the end, which take no room in the file. That data is the
   texts of `resident.c`, the numbers that `tools/mkexe.c` fills in
   (found by name in the linker's map), one structure for what the part
   keeps, in the original's order, and the stack. Code and data are one
   group to the linker, beginning at the PSP: CS, DS and SS are on the
   PSP and every offset counts from it, as in the `.COM` file.
2. **The part that goes**, a paragraph further on: its constants first,
   in a group of their own, then its code in two segments.

The original keeps the second part where the first part's data will be,
and moves it up over itself as it starts. The translation is loaded
with it already in place, on its own stack from the first instruction,
and only has to make a memory block of it. That costs 6,384 bytes of
the file. It is also what lets compiled code stand in the first part at
all, since nothing can lay data over code that the compiler has yet to
make. And it leaves the start-up code in memory, where the original
overwrites it with data. The start-up code had to change for this: it
measures in paragraphs and relocates what it loads.

None of this layout is in the C language. The order of the segments is
given to the linker in its command; that the first part's code and data
are one group, and where each kind of data goes, are compiler options.
`tools/mkexe.c` checks the result and stops the build where it differs
from what the program needs. [What rests on Open
Watcom](#what-rests-on-open-watcom) lists each such place.

### The texts and tables

The boxes and dialogs name their texts, buttons and each other by
address: words that are offsets in the assembly's one segment. In C they
are constants written with the names of what they refer to, in files
that hold nothing else, the `_t.c` files (`src/c/help_t.c`, with
`src/c/tables.h` for how they are written). The compiler puts them in a
group of their own, where a word that names one is its offset, as in the
original. They are kept apart from the code because the code's data
segment is on memory that the program asks DOS for, and a constant read
through it would be read there.

A table that no screen shows can be wrong in every scenario and still
pass. So the `c-data` test checks the tables by themselves: each against
the original by name, byte for byte, and for a word that holds an
address, the thing it names, which the original's object file tells by
its relocations.

### The stack and the memory

The tests run the shipped program, as they run the original: a driver
in the machine takes either through the scenarios (see [When a step is
finished](testing.md#when-a-step-is-finished)). It also writes the
program's stack out after each step, and `c-scenarios` reports for each
scenario how much of the stack was never used.

The stack is where the translation departs from the original in memory
use. The original has 256 bytes. Compiled code uses more stack than the
assembly, and an overflow would overwrite data with no trace on the
screen, so the translation has 4,096 bytes, of which the scenarios use
some 200.

The part that stays in memory is 19,728 bytes where the original's is
12,704, so a command has 7,024 bytes less than under the original: 3,840
for the larger stack, and the rest because compiled code is longer than
the assembly and the start-up code stays.

## What rests on Open Watcom

The translation is C, but not C that any compiler would accept or would
make the same program of. These are the places, so that whoever moves
the program to another compiler has them in one list. Each has a
comment where it is done.

### What the code is written in

These show wherever they are used. A 16-bit DOS program with the
original's layouts cannot do without them.

| What | Where | How many |
|---|---|---|
| `__far` and `__near` pointers, and pointers based on a segment, `CODE` and `STACK` of `src/c/vc.h` | every file | some 1,270 `__far`, 485 `STACK`, 96 `CODE` |
| `#pragma aux`: the registers that a function takes and returns, kept from the assembly, and its name for the linker | every file | 863 |
| `#pragma pack(1)` for the original's structures | `src/c/vc.h`, `src/c/tables.h` and five files with structures of their own | 8 pairs |
| `__interrupt` functions, which save the registers and load DS | `FatErr` of `src/c/main.c`; `CtrlC`, `Int21`, `Int27`, `Exit05` of `src/c/resident.c` | 5 |
| `__loadds` functions, which load DS when called from the other part | `SetBrk`, `Release` of `src/c/resident.c` | 2 |
| `__declspec(naked)` functions, which are their inline assembly and nothing else | `New21`, `New27`, `Exit01`, `Int2E` of `src/c/resident.c` | 4 |
| Intrinsic functions: `inp`, `_disable`, `_enable` | `src/c/dos.c` | 3 |
| The compiler's two helpers for 32-bit arithmetic, under its names and with its registers | `__U4M`, `__U4D` of `src/c/arith.c` | 2 |

### What cannot be seen in the code

The program is right in these places only because of what this compiler
and its linker do. The language promises none of them. `tools/mkexe.c`
stops the build where one of the first eleven does not hold; the others
would show in the tests.

| | What is relied on | Where it is done | What holds it |
|---|---|---|---|
| 1 | The option `-nd=R` gives a file's data a group of its own, `R_GROUP`, which its code reads through DS without loading a segment, in four segments of the class `FAR_DATA`: `RCONST`, `RCONST2`, `R_DATA`, `R_BSS` | `CMakeLists.txt`, for `src/c/resident.c` | `tools/mkexe.c` knows the four names and refuses any other data |
| 2 | The option `-g=R_GROUP` puts the code segment into a group of that name, and the linker merges two groups of the same name: the part that stays is one segment to the program, the PSP's | `CMakeLists.txt`, for `src/c/resident.c` and the second copy of `src/c/dos.c` | the group has to begin at 0 |
| 3 | The linker's `order` directive, which its help does not list for DOS: the order of the classes, and of the segments within one | `CMakeLists.txt`, the linker's command | the checks of 2, 6, 8 and 11 |
| 4 | A segment holds what the object files have of it, in their order: `resident.c` is linked first, and `transit_t.c` first of the part that goes | `CMakeLists.txt` | the checks of 6 and 10 |
| 5 | The compiler writes the objects of the code segment that have a value in file order, and those without one in front of them, so `Vect21` is given a value, `= 0` | `src/c/resident.c` | the check of 6 |
| 6 | So the room of the PSP, `Psp`, is the first thing of the code segment and of the program | `src/c/resident.c`, its first lines | `Psp` has to be at 0 |
| 7 | What has a value is in `R_DATA` in file order, so `Stays`, written last and with a value of zeros, is the last of it: from there on the part is zeros, which the file's checksum leaves out | `src/c/resident.c` | from `Stays` to the part that goes the file has to be zeros, with the seven filled-in words in front |
| 8 | What has no value is in a segment of its own behind the others, `R_BSS`: the stack, `DataSav`, is the one object without a value, and so the end of the part that stays | `src/c/resident.c` | `R_BSS` may hold no other symbol, and `DataSav` has to be at its start |
| 9 | A far pointer has the linker's segment number in its high word, and a word of the part that stays that names the part that goes is left as that number: `TRN_SEG` | `src/c/resident.c` | a number that is not 0 has to be a segment behind the part that stays |
| 10 | The constants of the files without code are in `CONST2` of the compiler's group, `DGROUP`, and begin with `TrnBeg`, 32 bytes that hold a whole paragraph | `src/c/transit_t.c`, `src/c/*_t.c` | `TrnBeg` has to stand where the stack ends; a file with both code and such constants is refused |
| 11 | No code of the part that goes is within 4K of the stack, the constants being in front of it, and no code segment reaches into the last 4K of its 64K. Both are for QEMU, not for the program | the `order` of 3; `c_second` in `CMakeLists.txt` | read in the linker's map |
| 12 | `md`, the main data, is declared as an object and named to the linker as `Psp`: the linker takes the offset of an object in its own group, 0 here | `src/c/vc.h` | every scenario |
| 13 | A far pointer made of a near one has DS for its segment, `MAIN_SEG`, and one to a local variable has SS, with the option `-zu`: `ResSeg` | `src/c/vc.h`, `src/c/misc.c` | every scenario |
| 14 | An `__interrupt` function saves the registers in a fixed order, which `struct Saved` is, and loads DS with its group by a relocated number; `SecDat`, the one variable of `DGROUP`, is where `FatErr` finds the main data by that | `src/c/vc.h`, `src/c/main_t.c`, `src/c/main.c`, `src/c/resident.c` | the scenarios of the critical error and of programs that stay resident |
| 15 | A variable with a value in a file without code is in `_DATA` of `DGROUP`, in the file | `src/c/main_t.c`, `SecDat` | `tools/mkexe.c` refuses one without a value |
| 16 | `_enable` is `STI` and `CLD`: the program's first call, `IntOn`, clears the direction flag, which compiled code needs clear and DOS does not promise | `src/c/resident.c`, `Alloc0` | every scenario |
| 17 | The linker turns a far call to a function of the same segment into `push cs` and a near call, with no relocation, so the part that stays can call its own far functions before its relocations are applied | `src/c/resident.c`, `Alloc0` | every scenario |

How to write code for this compiler, such as which registers it ignores
in a `#pragma aux` or which pointers it truncates, is in the notes of
`DIARY.md`, not here.

### Inline assembly

Every piece of it: 81 instructions in 14 pieces. The first nine are each
a `#pragma aux` in `src/c/dos.c`, put in place where that file's
functions call them. No other file calls DOS, the BIOS or the mouse
driver.

| File | Piece | Instructions | Why it is not C |
|---|---|---|---|
| `src/c/dos.c` | `Dos` | 20: `NEG AX` for the carry flag, `PUSH` of BP and DS, nine that load the registers from a structure on the stack, `INT 21h`, five that store what DOS returns, `SBB AX,AX` for the carry flag, two `POP` | A software interrupt is not a C call; DOS takes and returns more registers than a function can, DS among them, and answers in the carry flag |
| `src/c/dos.c` | `Video` | 7: `PUSH BP`, `INT 10h`, `POP BP`, four that store AX to DX on the stack | The BIOS for the screen; some BIOSes lose BP |
| `src/c/dos.c` | `VideoBuffer` | 3: `PUSH BP`, `INT 10h`, `POP BP` | The same, with an address in ES:DI |
| `src/c/dos.c` | `Keyboard` | 3: `INT 16h`, `PUSHF`, `POP DX` | The BIOS for the keyboard answers in the zero flag |
| `src/c/dos.c` | `Multiplex` | 1: `INT 2Fh` | The interrupt by which the mouse driver and Windows are asked |
| `src/c/dos.c` | `DosIdle` | 1: `INT 28h` | The program says that it is idle |
| `src/c/dos.c` | `DesqviewPause` | 6: `MOV AX` and `INT 15h`, three times | DESQview takes the program off its stack between the first and the third, so no compiled instruction may stand between them |
| `src/c/dos.c` | `MouseDriver` | 1: `INT 33h` | The mouse driver |
| `src/c/dos.c` | `MouseState` | 4: `INT 33h`, three that store BX, CX and DX on the stack | The same, answering in three registers |
| `src/c/main.c` | `DataSeg` | 1: `MOV DS,AX` | C has no way to set a segment register. DS is put on the main data, whose segment is known only at run time: for a critical error, and around the main loop |
| `src/c/resident.c` | `New21` | 17: six `CMP` with their jumps, a far `JMP` through `Vect21`, and `PUSHF`, `PUSH AX`, `PUSH AX`, `JMP` to the interrupt function | Every DOS call by any program comes here. A call the program does not handle is passed on with no register changed, nothing taken of the caller's stack, and the flags of the original's comparisons; compiled code saves registers first and promises nothing of flags |
| `src/c/resident.c` | `New27` | 4: `PUSHF`, `PUSH AX`, `PUSH AX`, `JMP` | Room on the stack for a second return address, so that the interrupt function can continue in the previous handler by returning: C cannot change the depth of the stack it returns on |
| `src/c/resident.c` | `Exit01` | 4: the same | Where DOS continues after ending the program by itself, on the stack of the program that started this one |
| `src/c/resident.c` | `Int2E` | 9: `INT 2Eh`, `CLI`, `MOV AX,CS`, `MOV SS,AX`, `MOV SP`, `STI`, `MOV DS,AX`, `CLD`, `JMP` | COMMAND.COM's interrupt for a command returns with no stack and no data segment: both are set from CS before any compiled instruction runs, and the calling function cannot be returned into |

Three more instructions are the compiler's own, through its intrinsic
functions in `src/c/dos.c`: `IN` for a port, `CLI`, and `STI` with
`CLD`.
