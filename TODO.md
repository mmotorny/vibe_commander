# To do

Things that are known and not done. Standing instructions are in
`CLAUDE.md`; this is the list of loose ends.

## Known and not dealt with

- The number of bytes that the editor has free is not compared with the
  original's (see `harness/deviations.py`). The routine that works it out
  is still assembly. When it is translated, the number wants a check of
  its own: that it falls by one for every character typed, at least.
- The harness can type Russian on the Russian bench, and nothing beyond
  ASCII on the other: Alt with the numeric keypad types nothing there.
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
