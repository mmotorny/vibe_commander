#!/usr/bin/env python3
"""Show that coverage.executed(), as it stands, takes code of the main
program for the setup program's: the original VC.COM is taken through
steps that never start the setup program, and what ran is held against
the file of VCSETUP.COM, as it stands and with the change that
trial.executed() has.

When this was written it said 1003 bytes as it stands, and 0 with the
change.
"""
import trial

# Dialogs, boxes and the help, which both programs have routines for.
STEPS = ("F9 o Enter", "Esc", "F7", "Esc", "F1", "Esc", "F5", "Esc",
         "Alt+F1", "Esc")


def main():
    result, ran, as_it_stands = trial.run(STEPS, trace=True)
    print("the setup program was not run; bytes of VCSETUP.COM counted as "
          f"run: {as_it_stands} by coverage.executed() as it stands, "
          f"{sum(ran.values())} with the change")
    print(f"bytes of VC.COM that ran: {sum(result.executed.values())}")


if __name__ == "__main__":
    main()
