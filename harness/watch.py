"""Where the original is, watched from outside.

The port says when it has finished a step (see "Signals to the tests" in
port/asm/VC.ASM). The original cannot, but its code is known: the build of
the source that is laid out like it is the same byte for byte, and its
listing says where every routine and every variable is. So the harness
looks at the places where the port signals, through the debugger that QEMU
has for its guests:

  Inpt31    in Input0, where the program goes round while it waits for a
            key: the port signals W, S or T there
  NotFn1    where it waits for an answer when it cannot find its own file
  Exit04    where it ends: the port signals X

The machine is stopped when the program comes to one of them, and then
its memory says the rest. The program has finished a step when it is at
Inpt31 with every key taken, none of Shift, Ctrl and Alt held and nothing
that it will do by itself after a while (AutoTim), and has gone round once
more from there without changing the screen. A key is pressed when the
program waits for one and has taken those before it, so no key is ever
found waiting when the program comes to ask; and Shift, Ctrl and Alt are
held until the program has gone round once and seen them.

None of that is measured in time. Time on the host only decides how often
the harness looks, and when it gives up.

Where time is what the program goes by, it is the machine's: with its
clock on the program draws by the timer, and what blinks is found by
looking at the screen each time the program comes round, until that has
covered more than a second of the BIOS's count of timer ticks without a
gap in it.

At NotFn1 the program waits inside the BIOS and does not come round.
It is at rest when it comes there with every key taken. So that it cannot
pass NotFn1 or Exit04 unseen, the machine never runs unwatched while a key
is on its way: it runs with stops at both while a key is pressed, and one
round at a time while a key is waiting to be taken.

A scenario may want a key pressed while the program is busy, a search
that looks for keys as it runs (Scenario.ahead). Such a key is pressed
when the one before it has been taken, and where the program is in its
work by then is not decided here: what it runs for that key may differ
from one recording to the next, though the screens have not.
"""
import re
import struct
import sys
import time
from dataclasses import replace

import deps
from dos import COLUMNS, KEY_TIME, ROWS, MachineError

# The listing of the build that is laid out as the original is.
LISTING = deps.ROOT / "build" / "shareware-layout" / "VC.LST"

CONVENTIONAL_MEMORY = 0xA0000     # where DOS programs live, below this
BIOS_SHIFT_KEYS = 0x417           # low four bits: Shift, Ctrl and Alt held
BIOS_TICKS = 0x46C                # timer ticks, 18.2 a second
# How often the program goes round before a Shift, Ctrl or Alt that the
# BIOS does not show counts as seen.
SHIFT_ROUNDS = 20
SIGNATURE = 16                    # bytes of code that a place is known by

# How long the program may take to come to wait for a key before it
# counts as stuck, in seconds, and for how long the machine runs between
# two looks at where the program is in its memory.
STEP_TIME = 20
SLICE = 0.05
# With the clock on, the screen is looked at for this many timer ticks,
# with no more than GAP of them between two looks: the colon of the clock
# shows for half a second and is gone for half a second.
BLINK_SPAN = 20
BLINK_GAP = 3


def available(program):
    """Whether `program` (a setups.Program) is watched: it is the
    original."""
    import setups                       # which itself needs this module
    return program == setups.original_vc()


def warn_if_guessed(program):
    """Say so if `program`, a build that does not signal, is not the
    original either: its steps are then timed by still screens."""
    if not available(program):
        print(f"{program.filename} neither signals nor is the original: "
              "the harness waits for its screen to stand still after "
              "every step, and on a busy computer that may be before the "
              "program has finished", file=sys.stderr)


class Watch:
    """Watches the original in one machine. Machine.press() takes it as
    `watch`."""

    def __init__(self, program):
        if not LISTING.exists():
            raise MachineError(
                f"the original cannot be watched without {LISTING}: "
                "build the project first")
        text = LISTING.read_text(encoding="cp866")

        def symbol(name, kind):
            return int(re.search(rf"^ *{name} [ .]*{kind}", text,
                                 re.M)[1], 16)

        code = r"L Near +([0-9A-F]+)h +_TEXT"
        self._addresses = {"round": symbol("Inpt31", code),
                           "asks": symbol("NotFn1", code),
                           "exit": symbol("Exit04", code)}
        self._places = {}               # where they are in the memory
        self._pending = symbol("AutoTim", r"([0-9A-F]+) +DWord")
        self._clock = symbol("Clock", r"([0-9A-F]+) +Byte")
        self._contents = program.contents
        self._ended = False             # it has come to Exit04
        self._inside = False            # it waits inside the BIOS
        self._busy = False              # it is at work, and looks for keys

    def _find(self, machine):
        """Where in the machine's memory the three places are now:
        address -> which."""
        memory = machine._memory(0, CONVENTIONAL_MEMORY)
        self._places = {}
        for name, address in self._addresses.items():
            code = self._code(address)
            at = memory.find(code)
            while at >= 0:
                self._places[at] = name
                at = memory.find(code, at + 1)

    def _code(self, address):
        """The bytes that the code at `address` of the program is known
        by."""
        return self._contents[address - 0x100:address - 0x100 + SIGNATURE]

    def _arrive(self, machine, deadline):
        """Let the paused machine run until the program is at one of the
        three places, and return which: "round", "asks" or "exit". The
        machine is paused again."""
        while time.monotonic() < deadline:
            at = machine.run_to(list(self._places), SLICE)
            if at is not None:
                name = self._places[at]
                # The program loads itself again after a command, and may
                # then be elsewhere: what was found is still there.
                if machine._memory(at, SIGNATURE) == \
                        self._code(self._addresses[name]):
                    return name
            self._find(machine)
        raise MachineError(f"the program did not come to wait for a key "
                           f"within {STEP_TIME} s")

    def _asks(self, machine):
        """The program is at NotFn1 with no key to take: from here it
        waits inside the BIOS, and does not come to any of the places
        until a key has been pressed."""
        machine.step()
        self._inside = True

    def before_key(self, machine, busy=False):
        """Wait until the program waits for a key and has taken every key
        pressed so far. If it is `busy`, and looks for keys while it is,
        only until it has taken them, and from then on it counts as busy
        until it has been waited for. The machine is left paused."""
        if self._ended:
            return
        machine.pause()
        self._busy = self._busy or busy
        if self._busy:
            self._send(machine, lambda: machine.until(
                machine.keys_taken, STEP_TIME,
                "the program did not take the key before"))
        if self._inside or self._busy:
            return
        deadline = time.monotonic() + STEP_TIME
        while True:
            at = self._arrive(machine, deadline)
            if at == "exit":
                self._ended = True
                machine.resume()
                return
            if machine.keys_taken():
                break
            machine.step()
        if at == "asks":
            self._asks(machine)

    def _send(self, machine, send):
        """Let the paused machine run while send() presses or releases
        keys, and pause it again. A machine that is paused gets no keys.
        The machine stops by itself if the program comes to ask for its
        file or to end, so that neither is missed."""
        machine.start([at for at, name in self._places.items()
                       if name != "round"])
        send()
        at = machine.stopped()
        if at is not None and self._places[at] == "asks":
            self._inside = False

    def key(self, machine, send):
        """Have send() press a key. The machine is paused, before and
        after."""
        if self._ended:
            send()
            return
        self._send(machine, send)
        # The program has the key, or will have it.
        self._inside = False

    def shift(self, machine, send, shown):
        """Have send() press or release one of Shift, Ctrl and Alt, and
        wait until the program has seen it: it has gone round once since
        shown() said that the BIOS has it so. A keyboard driver may keep a
        key from the BIOS, so after SHIFT_ROUNDS rounds that is no longer
        waited for. The machine is paused, before and after."""
        if self._ended:
            send()
            time.sleep(KEY_TIME)
            return
        if self._inside or self._busy:
            # It does not come round: the key is held for a while.
            self._send(machine, lambda: (send(), time.sleep(KEY_TIME)))
            return
        # A key that went with it is acted on first.
        self.before_key(machine)
        if self._ended or self._inside:
            return self.shift(machine, send, shown)
        self._send(machine, send)
        deadline = time.monotonic() + STEP_TIME
        rounds = 0
        while self._arrive(machine, deadline) == "round":
            there = shown()
            machine.step()
            if self._arrive(machine, deadline) != "round":
                break
            rounds += 1
            if there or rounds >= SHIFT_ROUNDS:
                break

    def wait(self, machine):
        """Wait until the program has finished what the keys pressed made
        it do, and return the screen. The machine is left paused."""
        if self._ended:
            return machine.wait_for_prompt(STEP_TIME)
        machine.pause()
        self._busy = False
        if self._inside:
            return machine.screen()
        deadline = time.monotonic() + STEP_TIME
        before = None
        while True:
            at = self._arrive(machine, deadline)
            if at == "exit":
                self._ended = True
                machine.resume()
                return machine.wait_for_prompt(STEP_TIME)
            if not machine.keys_taken():
                # It takes the key now, and is watched while it does.
                before = None
                machine.step()
                continue
            if at == "asks":
                screen = machine.screen()
                self._asks(machine)
                return screen
            data = machine.registers["ds"] * 16
            if machine._memory(BIOS_SHIFT_KEYS, 1)[0] & 15 or \
                    machine._memory(data + self._pending, 2) != b"\xFF\xFF":
                # A key has still to come up, or the program will do
                # something by itself after a while. It has no key to act
                # on meanwhile, and can do without being watched.
                before = None
                machine.resume()
                time.sleep(0.001)
                machine.pause()
                continue
            screen = machine.screen()
            if screen == before:
                break
            before = screen
            machine.step()
        if machine._memory(data + self._clock, 1)[0]:
            return self._blinking(machine, deadline)
        return screen

    def _blinking(self, machine, deadline):
        """The screen of the program that waits with its clock on, with
        the cells that blink: of the two states the one with the greater
        bytes, as Machine.wait_for_screen() has it. The machine is paused
        before and after."""
        def look():
            self._arrive(machine, deadline)
            ticks = struct.unpack("<I", machine._memory(BIOS_TICKS, 4))[0]
            return ticks, machine.screen()

        began = last = None
        states = []
        while True:
            ticks, screen = look()
            if last is None or ticks - last > BLINK_GAP or \
                    (screen.cells not in states and len(states) == 2):
                began, states = ticks, []
            last = ticks
            if screen.cells not in states:
                states.append(screen.cells)
            if ticks - began >= BLINK_SPAN:
                break
            machine.resume()
            time.sleep(0.01)
            machine.pause()
        shown, other = max(states), min(states)
        while screen.cells != shown:
            machine.resume()
            time.sleep(0.01)
            machine.pause()
            _, screen = look()
        size = COLUMNS * ROWS * 2
        blinking = tuple((i // 2 % COLUMNS, i // 2 // COLUMNS,
                          other[i], other[i + 1])
                         for i in range(0, size, 2)
                         if shown[i:i + 2] != other[i:i + 2])
        return replace(screen, blinking=blinking)
