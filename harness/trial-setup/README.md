# The trial of the setup program

The scripts by which the plan for translating the setup program,
`VCSETUP`, was tried on the bench of the tests before any of it was
translated: `TODO.md`, "The setup program in the portable C: the plan",
is that plan, and says what each of these showed. They are kept for the
piece of that work that makes the harness able to judge the setup
program, 0a there, to start from.

Nothing here is built or tested by the project, and nothing of the
harness or of the tests uses it. When the harness does these things
itself, this directory goes.

Each is run from the checkout, after the build, and needs what the
harness needs: `python3 harness/trial-setup/trial.py --help`.

| Script | What it does | Runs machines |
|---|---|---|
| `trial.py` | Takes the original `VC.COM` through steps on a bench that has the distribution's `VCSETUP.COM` next to it, so that a step can start the setup program, and prints the screens, `show`, or writes a golden and what ran of the setup program into `build/trial-setup/goldens/`, `record`. What the harness has to do otherwise for that is overridden in it: the file on the disk, `Bench`; what ran of a program that runs next to another with routines of the same bytes, `executed`; and the driver told of the setup program, `setup_facts` | Yes |
| `phantom.py` | Shows that `coverage.executed`, as it stands, counts code of the main program as the setup program's, and that `executed` of `trial.py` does not | Yes |
| `clock.py` | Shows that the setup program's clock is recorded as blinking only if the driver is told of the setup program | Yes |
| `procs.py` | Holds the procedures of the setup program against the main program's, by the text of `src/asm`: which are the same, which differ and how, which the main program has not | No |
| `data.py` | The same for the data: texts, boxes, dialogs and tables | No |
| `classes.py` | Counts the instructions of the setup program in `build/shareware-layout/VCSETUP.LST` by those classes, and how many of each ran in recordings of `trial.py` | No |

What `record` writes is no golden of the project: it is in `build/`,
which is not committed, and the first golden of the setup program is
derived by the harness once it can.
