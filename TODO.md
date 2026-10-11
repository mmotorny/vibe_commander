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

- Deriving goldens side by side, as the tests now run their scenarios. The
  original cannot say when it has finished a step, so the harness waits
  for the screen to stand still, and a machine slowed down by its
  neighbours may stand still before it has finished. Deriving all goldens
  takes two and a half minutes, and only a change to the bench asks for
  it.
- Keeping the booted bench from one run of a tool to the next. Booting DOS
  for the two countries and making the bench's setup file take about five
  seconds of every run. What is kept would have to be thrown away whenever
  the harness, a download or QEMU changes.
- A file or a directory that a scenario makes gets the time of day, to
  the second, and the golden lists it; deriving is slower than testing, so
  the two differ. No scenario makes one yet. Such a time stamp could be
  recorded only as made during the run.
