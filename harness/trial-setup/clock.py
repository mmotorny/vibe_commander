#!/usr/bin/env python3
"""Show what the driver of the tests makes of the setup program's clock:
the clock is switched on in its Configuration, where it shows at once,
with the driver told of the main program, as the harness tells it now,
and then with the driver told of the setup program (trial.setup_facts).
Only the second way has the colon of the clock, which blinks, recorded
as blinking; the first has it in whichever state it was.
"""
import trial

STEPS = ('"d:\\vc\\vcsetup" Enter', "F2", "Down Down Down Down Down Space",
         "Enter", "F3", "Esc")


def main():
    for watch in (False, True):
        result, _, _ = trial.run(STEPS, watch_setup=watch)
        print("the driver told of",
              "VCSETUP and its Clock" if watch else "VC.COM, as now")
        for number, screen in enumerate(result.screens):
            cells = [(column, row) for column, row, *_ in screen.blinking]
            print(f"  screen {number}: row 0 ends "
                  f"|{screen.lines()[0][70:]}|, blinking cells {cells}")


if __name__ == "__main__":
    main()
