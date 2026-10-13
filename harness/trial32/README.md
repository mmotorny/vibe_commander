# A trial of a 32-bit program on the bench

Toys of a few dozen lines each, built with Open Watcom's 32-bit compiler
in the flat model, with a DOS extender and no C library, and run on the
test bench through the harness as it is. They were made on 12 October
2026, before the portable C was taken to 32 bits, to find out whether
the bench can run and drive such a program and what the harness needs
for it. What they showed is in `TODO.md`, "The portable C in 32 bits:
what a trial on the bench showed"; they are kept so that it can be seen
again, and asked further.

Since then one toy has been added that is of the work itself: `sys.c`,
which asks the system's functions of the portable C,
`src/portable_c/dos32.c`, with `asked.py` to run it. While the portable
C was a program of 16 bits as well, it held those functions against the
ones of that program, `dos.c`, and the script was `both.py`.

And the driver of the tests reads above the megabyte itself since,
where the trial had `DRIVERX.ASM` do it: `harness/drivercheck.py` holds
it to `auto.c`, `keys.c`, `wait.c`, `copies.c` and `sys.c`, and its top
says how.
`run.py` still tells the driver nothing of where a toy keeps anything,
so what is said below of `auto.c` and "the driver as it is" is what
`run.py` shows today as well.

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
a program of real mode, `driver` the copy of the driver, `sys 32` the
toy of `sys.h`. Its top says
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
| `ran.py` | Which functions of a flat program ran on the bench, in the order in which each first did, by QEMU's log of the code (`run.py --trace`), the program's file and the linker's map: where a program stops that does not get to its first screen. Its top says what the order is and is not |
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
| `sys.c`, `asked.py` | The functions of the portable C's `sys.h`, each asked and what it answers written out, with the program's `dos32.c`. `asked.py` builds the toy, runs it on the bench and prints its pages; see its top, and the top of `sys.c`. It was built both ways, with the `dos.c` of the 16-bit program too, and the two had to write the same, until that build went |
| `depth.c` | How much of the program's own stack the extender writes below the stack pointer during a call of DOS, of the BIOS, of the mouse and of real mode, and for the timer's interrupts |
| `ended.asm` | Real mode: DOS ends a program by another's hand, as the scenario `ended` has it, and what is left then of a program of 32 bits and of its extender. Its top says how it is run and what it writes |
| `copies.c` | A copy of the toy run from the toy through the shell, and one from that: each with an `AutoTim`, a `Clock` and a stack of its own, elsewhere above the megabyte. `harness/drivercheck.py` holds the driver to it; by hand, steps of `t c o r q`, as its top has them |
| `above.asm` | Real mode: what a program that is run as a command is told of the memory above the megabyte, by INT 15h, the clock's memory, XMS and DPMI. Its top says how it is run |
| `calls.asm` | Real mode: every call of DOS that a program makes, as DOS gets it, in order, with AX, BX, DS and ES: it takes INT 21h, runs the program, and writes the calls to a file when the program has ended. Entry 6 of `src/portable_c/DEVIATIONS.md` has what it showed of the two programs of the portable C. Its top says how it is run |
| `rmbench.asm` | Real mode: what the bench has for an extender, XMS, DPMI, EMS and memory |
| `rmstart.asm` | Real mode: when DOS enters a program, to hold `start.c` against |
| `rmcalls.asm` | Real mode: what a call costs, to hold `realcalls.c` against |

Those of real mode are run under the program's name: `run.py --name
VC.COM build/trial32/rmbench.com`.

## A script of the plan

When the step was planned, on 13 October 2026 (`TODO.md`, "The portable
C in 32 bits: the plan"), the tree itself was asked three things, by
scripts that read and compile it and run nothing. One is here for the
pieces of that plan to start from, and what each printed that day is in
the plan. The second, `whole.py`, asked whether the tree links as a
flat program and how long that is: the project's build links that
program now, `build/portable_c/VC.EXE`, `python3 harness/le.py` with
the file and its map prints what the script printed of it, but for how
many bytes of the file its relocations take, and the script is gone.
The third, which asked where the program can tell an
int of 32 bits from one of 16, has become a test of the build,
`portable_c-int16`, and is `harness/int16.py`.

| File | What it asks |
|---|---|
| `casts.py` | Of the places where a pointer is kept in a number or made of one, which `harness/hostcheck.py` counts: what is each, a place of the main data, a table by its offset, a panel by its segment? It prints the classes of the plan with their files, and the places of one. It needs gcc, as the host check does |
| `added.py` | Where is a variable of 16 bits added to a pointer as it is? Of the convergence, not of the plan: the first piece of it met a step of -160 that was kept in such a variable and added to a pointer, which goes 64K less 160 on in a flat memory, and no rule says of a variable what it holds. It lists the places, 158 then, with the pointer and the variable, for reading by eye. It needs clang, as `harness/int16.py` does, whose finder it uses |
