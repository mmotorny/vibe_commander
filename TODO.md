# To do

Things that are known and not done. Standing instructions are in
`CLAUDE.md`; this is the list of loose ends.

## Known and not dealt with

- The editor shows the memory it has free in its first line, and the port
  has less than the original, by as much as it is larger in memory:
  359,048 bytes against 360,872 when the texts, paths and file names were
  translated, and less with every batch. No scenario opens the editor yet.
  The first that does needs this declared as a deviation, with a number
  that follows the size of the build or with the number left out of the
  comparison.
- The harness can type Russian on the Russian bench, and nothing beyond
  ASCII on the other: Alt with the numeric keypad types nothing there.
- The program runs on a stack of 256 bytes, 238 in the build that the
  tests run. The scenarios leave 36 of them untouched, measured by hand by
  filling the stack with a pattern first. Compiled code takes more stack
  than the assembly it replaces, and nothing checks how much is left.
- `tools/flagscan.py` follows flags until an instruction sets them all,
  and so reports a jump that reads a flag which an instruction in between
  has set: `DEC AX` and then `JE`, `STC` and then `JNC`. It should follow
  each flag by itself.

## Ideas put off

- Running scenarios side by side. Measured on the port's test with six
  scenarios: 13.8 s one after another, 7.7 s two at a time, 6.9 s four at a
  time. Booting DOS for the two countries then takes 3.1 s, which is the
  floor; since then a seventh scenario has come, and 1.7 s for making the
  bench's setup file. `Bench._program_disk` in `harness/setups.py` is not
  safe to call from two threads at once and has to be made so first;
  reading the execution logs is Python and gains nothing from threads.
