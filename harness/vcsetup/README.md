# The setup program's assembly against the main program's

The original's setup program, `VCSETUP`, has much of the main program in
it: routines that are the main program's word for word, routines of the
same name with another text, and texts and tables of both kinds. It goes
into the portable C as a part of the one program (`TODO.md`, "The setup
program in the portable C: the plan"), and what is the main program's
word for word is not translated twice. These three scripts say which is
which, by the text of `src/asm` and by the listing of the build. They
were made for the trial that the plan rests on, and are kept for the
pieces of that work: a piece looks here before it translates a routine or
writes a table.

Nothing here is built or tested by the project, and nothing of the
harness or of the tests uses it. Each is run from the checkout, and the
last needs the build.

| Script | What it does |
|---|---|
| `procs.py` | Holds the procedures of the setup program against the main program's, by the text of `src/asm`: which are the same, which differ and by how many lines, which the main program has not. `procs.py Dialog Help` also prints how each of those differs, as a diff from the main program's to the setup program's |
| `data.py` | The same for the data: texts, boxes, dialogs and tables, by name, in the three classes. The test of the data holds a tree's constants against the bytes (`harness/datacheck.py`); this says beforehand which names there are |
| `classes.py` | Counts the instructions of the setup program in `build/shareware-layout/VCSETUP.LST` by those classes, with the code between procedures, which is most of `VCSETUP.ASM`, and how many of each have run in the scenarios of the setup program |

What the trial had besides, the harness does itself now: the original's
setup program on the bench, what ran of it kept apart from what ran of
the main program, and the driver of the tests told of it for its clock.
The top of `harness/scenarios.py` says how.
