# To do

Things that are known and not done. Standing instructions are in
`CLAUDE.md`; this is the list of loose ends.

## Not yet proven

- C code calling assembly. The first batch needed none of it. The next
  batch should include a routine that does, to prove that direction: a far
  entry in the assembly segment that calls the near routine and returns
  far, called from C like any far function.
- A path of the start-up code that changed for the EXE is reached by no
  scenario: the one taken when DOS cannot give the program's memory block
  the size it asks for, and a separate block is allocated for the part
  that is unloaded while a command runs. It takes a machine whose largest
  free block holds the program file but not the 10 KB more that the
  program spreads out to, and which has a second free block besides.
  Nothing on the test bench arranges that from the keyboard; a helper
  program written for it could.
- A retry that succeeds after the program file was not found for
  reloading. The scenario `lost` reaches the retry, and it fails again: no
  key brings the file back while the program waits.

## Ideas put off

- Running scenarios side by side. Measured on the port's test with six
  scenarios: 13.8 s one after another, 7.7 s two at a time, 6.9 s four at a
  time. Booting DOS for the two countries then takes 3.1 s, which is the
  floor; since then a seventh scenario has come, and 1.7 s for making the
  bench's setup file. `Bench._program_disk` in `harness/setups.py` is not
  safe to call from two threads at once and has to be made so first;
  reading the execution logs is Python and gains nothing from threads.
