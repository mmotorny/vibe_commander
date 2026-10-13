# A trial of a 32-bit program on the bench

Toys of a few dozen lines each, built with Open Watcom's 32-bit compiler
in the flat model, with a DOS extender and no C library, and run on the
test bench through the harness as it is. They were made on 12 October
2026, before the portable C was taken to 32 bits, to find out whether
the bench can run and drive such a program and what the harness needs
for it. What they showed is in `TODO.md`, "The portable C in 32 bits:
what a trial on the bench showed"; they are kept so that it can be seen
again, and asked further.

Nothing here is built or tested by the project: `CMakeLists.txt` does
not know the directory, and no test reads it. Nothing here is the
program's, and none of it is to grow into the harness as it stands:
`run.py` reaches into the harness where it has no way in, and
`DRIVERX.ASM` is a copy of the driver as it was that day.

Only what was written for the trial is here. The extenders are in the
Open Watcom that the build fetches, `build/_deps/open_watcom-src/binw/`:
`dos32a.exe`, DOS/32A, and `cwstub.exe`, CauseWay. DOS/4GW and PMODE/W
are there too and are not open: they are not linked and not run.

## Running one

The project has to be built once, for the compiler and the assembler.

```sh
harness/trial32/build.sh keys d32
mkdir -p build/tmp
TMPDIR=$PWD/build/tmp python3 harness/trial32/run.py \
    build/trial32/keys-d32.exe a F5 Alt+F1 Click@3,5 q
```

`build.sh` puts what it makes into `build/trial32/`: `NAME d32` and
`NAME cw` a toy with DOS/32A or with CauseWay for its stub, `NAME com`
a program of real mode, `driver` the copy of the driver. Its top says
the rest, and it is where the compiler's and the linker's options are,
with why.

`run.py` puts a file on the bench under the program's name, has the
bench start it and the driver of the tests take it through the steps
that follow its name, written as a scenario's are, and prints every
screen that the driver wrote. Several files run one after the other on
one bench, with `--` between them. `--help` has its options; those of
its last group change the machine or the driver, and what is seen with
them is not seen on the bench.

A toy writes what it finds onto the screen, in hex unless it says
otherwise, and most end at once: the screen that is printed then also
has DOS's prompt, where the cursor was. Under
CauseWay every toy ends in CauseWay's report of an exception, which is
the finding: see `csover.c`.

## The files

| File | What it asks |
|---|---|
| `toy.h`, `lib.c` | What the toys share: the interrupts as inline assembly, texts and numbers onto the screen, the entry without a C library, the stack |
| `build.sh` | How a toy is built |
| `quiet.py` | Switches DOS/32A's banner off in a file that has it for its stub; `build.sh` runs it |
| `run.py` | Runs toys on the bench and prints the screens |
| `entry.c` | Does it run, and what is the program handed: registers, segments, the extender's DPMI, what the extender says of itself, the privilege level |
| `start.c`, `big.py` | When is the program entered, by a clock that the extender does not set; with `big.c` or `fat.c`, which `big.py` writes, for a longer file with relocations or without: `build.sh start d32 big.c` |
| `timer.c` | What the extender has done to the timer's counter 0 |
| `zero.c` | Is what the file does not hold zero at the entry, and where do the code and the data land |
| `memory.c` | What memory there is, by DPMI 0500h, 0501h and 0100h and by INT 21h function 48h |
| `keys.c` | The idle call, keys with the shift state, the mouse: steps `a F5 Shift+F6 Alt+F1 Click@3,5 q`; with `--stack 0,512`, how much of the extender's stack of real mode the driver's work uses; with `--driver` and `--debug`, where the idle call comes from |
| `wait.c` | A key waited for inside the BIOS: steps `a q` |
| `auto.c` | What the driver reads of the program, above the megabyte: steps `a t b c q`, with the driver as it is and with `--driver build/trial32/DRIVERX.COM --lauto ADDRESS --lclock ADDRESS --clock 0`, the two addresses being those of the toy's first line |
| `DRIVERX.ASM` | The driver of the tests with the BIOS's block move for those two, and a record of where an idle call comes from; its top says what was added |
| `doscalls.c` | Calls of DOS with addresses above the megabyte: which does the extender translate |
| `realcalls.c` | Those that it does not, made in real mode; the BIOS's and the mouse's calls from flat code; what a call costs |
| `errors.c` | The critical error and Ctrl-Break taken in protected mode: steps `a Ctrl+Break w Ctrl+c q` |
| `command.c` | A command run by INT 21h function 4Bh, DOS's memory meanwhile, a second copy inside a command, INT 2Eh, and INT 22h afterwards: steps of `1 2 3 4 m v r q`, as its top has them |
| `csover.c` | The fault of QEMU that stops CauseWay, shown without CauseWay |
| `cwtables.c` | For CauseWay only: its tables read before its fault; it does not end, so `--limit 5` |
| `hello.c` | The one program with Open Watcom's C library: `build.sh hello cw` |
| `rmbench.asm` | Real mode: what the bench has for an extender, XMS, DPMI, EMS and memory |
| `rmstart.asm` | Real mode: when DOS enters a program, to hold `start.c` against |
| `rmcalls.asm` | Real mode: what a call costs, to hold `realcalls.c` against |

The three of real mode are run under the program's name: `run.py --name
VC.COM build/trial32/rmbench.com`.

## Three scripts of the plan

When the step was planned, on 13 October 2026 (`TODO.md`, "The portable
C in 32 bits: the plan"), the tree itself was asked three things, by
scripts that read and compile it and run nothing. They are here for the
pieces of that plan to start from, and what each printed that day is in
the plan.

| File | What it asks |
|---|---|
| `casts.py` | Of the places where a pointer is kept in a number or made of one, which `harness/hostcheck.py` counts: what is each, a place of the main data, a table by its offset, a panel by its segment? It prints the classes of the plan with their files, and the places of one. It needs gcc, as the host check does |
| `int16.py` | Where can the program tell an int of 32 bits from one of 16: where is arithmetic on numbers of 16 bits compared, divided, shifted right or made wider before it is put back into 16? It needs clang, which reads each file for a machine of 32 bits. Its top says what it cannot see |
| `whole.py` | Does the tree compile and link as a flat program, and how long is that? It copies the tree to `build/trial32/whole/`, changes the two lines that `wcc386` refuses, puts functions that do nothing in place of `dos.c`, and prints the file's length, its objects and its relocations. Nothing that it makes runs |
