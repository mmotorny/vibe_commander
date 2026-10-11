# To do

Things that are known and not done. Standing instructions are in
`CLAUDE.md`; this is the list of loose ends.

## Not yet proven

- C code calling assembly. The first batch needed none of it. The next
  batch should include a routine that does, to prove that direction: a far
  entry in the assembly segment that calls the near routine and returns
  far, called from C like any far function.
- Two paths of the start-up and reload code that changed for the EXE are
  reached by no scenario: the one taken when DOS cannot resize the
  program's memory block in place and a separate block is allocated, and
  the retry after the program file was not found for reloading.
- The signal T, for a program that waits and will act by itself after a
  while, has not been seen in use: no scenario makes the program follow
  the cursor to another directory after its delay.
- `harness/goldens.py` refuses to record a scenario that takes the guest's
  clock into the next minute. No scenario is long enough to have tried it.

## Suspected bugs in the original

Found by reading, not reproduced. Ported as they are until decided
otherwise.

- When the program file cannot be found for reloading after a command, the
  program asks to press ENTER to try again. `NotFind` returns with DS on
  the environment segment, and the retry at `Exec12` then stores the size
  it expects through DS, at the offset that `ComnIni.LoadSiz` has in the
  resident part: a word written into or past the DOS environment block.
  (`VC.ASM`, `Exec12` and `Exec13`.)

## Ideas put off

- Running scenarios side by side. Measured on the port's test with six
  scenarios: 13.8 s one after another, 7.7 s two at a time, 6.9 s four at a
  time. Booting DOS for the two countries then takes 3.1 s, which is the
  floor. `Bench._program_disk` in `harness/setups.py` is not safe to call
  from two threads at once and has to be made so first; reading the
  execution logs is Python and gains nothing from threads.
- Switching the clock off for the whole test bench, with one scenario that
  turns it on through the menus. Its minutes are on every screen, which is
  why a golden cannot be recorded across a minute boundary, and its colon
  is why recording waits 0.8 s after every step. Worth doing when a
  scenario has to be longer than a minute of recording.
- Dropping the tests `start-screen` and `start-screen-shareware-layout`.
  They take 4 s each and say little that the byte comparison does not.
