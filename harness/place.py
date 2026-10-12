#!/usr/bin/env python3
"""Say where the stacks of the original and of the C translation end in the
memory of the test bench, and where the driver of the tests would put them.

QEMU translates the code that the machine runs, a page of 4,096 bytes at
a time, and has to look at every write to a page with code that it has
translated: such writes are many times slower than others. Volkov
Commander has its stack at the end of the part that stays in memory
while a command runs, and its code right behind it, so every push is
slow unless the stack ends where a page ends. That does not show while
the program waits by the clock of a real second, and with the machine's
clock counted in instructions it is most of a scenario's time.

Where DOS loads the program follows from what is loaded before it, and
the driver of the tests can keep some memory more than it needs:
setups.PLACE, in paragraphs of 16 bytes, for each country's bench. This
prints, for each bench, where the stack of the original and of the
translation ends in its page, and the PLACE that would end the original's
stack on a page. The original is the one to go by: it does not change.

    place.py

Needs the project built.
"""
import sys

import deps
import driver
import scenarios
import setups

PAGE = 4096


def stack_end(program, bench):
    """Where the stack of `program` ends on `bench`: its address in the
    machine's memory."""
    known = driver.facts(program, stack=True)
    run = scenarios.run(scenarios.Scenario(""), program, bench, stack=True)
    return run.stack_segment * 16 + known.stack + known.stack_size


def main():
    if len(sys.argv) > 1:
        sys.exit(__doc__)
    programs = [setups.original_vc(), setups.Program.load(
        deps.ROOT / "build" / "c" / "VC.EXE")]
    with setups.Benches() as benches:
        for country in setups.COUNTRIES:
            ends = [stack_end(program, benches[country])
                    for program in programs]
            for program, end in zip(programs, ends):
                print(f"{country}: the stack of {program.filename} ends at "
                      f"{end:05X}, byte {end % PAGE} of its page")
            more = -ends[0] % PAGE // 16
            print(f"{country}: PLACE is {setups.PLACE[country]}; "
                  f"{(setups.PLACE[country] + more) % (PAGE // 16)} would "
                  f"end the original's stack on a page")


if __name__ == "__main__":
    main()
