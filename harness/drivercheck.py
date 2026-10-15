#!/usr/bin/env python3
"""Hold the driver of the tests to flat programs of 32 bits, on the bench.

    python3 harness/drivercheck.py

A flat program has its data above the megabyte, and the driver reads
there what it reads of a program: AutoTim, Clock and the stack, at
addresses that driver.py works out from the program's file and the
linker's map (see "A flat program" in DRIVER.ASM, and flat_facts in
driver.py). No test of the build holds that until a program of the
project is flat. This does, by hand, with three toys of harness/trial32/,
which it builds with DOS/32A for their stub, puts on the bench under the
program's name and takes through steps as a scenario is taken. Each of
the three has variables of its own under the names AutoTim and Clock,
which the linker's map has, and writes onto the screen what it sees:

  auto.c    sets AutoTim when t is pressed and does something by itself
            nine ticks later; switches a clock on with c, a cell that
            blinks. Held: the step of t ends when the toy has done its
            thing and not before; the step of c has the cell as
            blinking; and the two addresses that the toy writes of
            itself are those that driver.py worked out. It is run a
            second time with the driver told nothing, and the step of t
            has then to end early: that is what the toy is for. And a
            third time as the program will be watched: built with the
            two variables in a main data, `md`, at the offsets that the
            original's listing has for them, and with what driver.facts
            makes of such a file, by `_md` of the map and that listing.
  keys.c    goes round as the program does, with an idle call, the mouse
            and the keyboard. Held: each step ends with its key or its
            button on the screen, and nothing above the megabyte keeps a
            step from ending.
  wait.c    waits for its keys inside the BIOS and never idles. Held:
            the keys arrive; no stack is written out, as for any program
            that does not idle.

And of every toy that idles, the stack as the driver wrote it out: it is
as long as the map has it; below what the toy has used it is what the
file has there, zeros; the used part holds addresses of the toy's own
code, as the reader of its file places that; and where keys.c writes its
stack pointer, that is inside what was written out, with nothing used
below it but what the toy's calls go down.

A fourth toy is of the tree itself:

  sys.c     asks the system's functions of the 32-bit program, sys_dos.c,
            a page for a step, and has the tree's main data, `md`, and
            its stack, stack_dos.c. It goes the way a scenario goes, by
            scenarios.run, with what driver.facts makes of a program:
            AutoTim and Clock by `_md` of the map and the offsets of the
            original's listing. Held: each step ends with its page, and
            the stack is read. The toy sets neither variable, so this
            holds that the way is whole and not that the two are read
            right: auto.c holds that.

And a fifth is the program started from its own command line, as six
scenarios start it:

  copies.c  auto.c with a key that runs a copy of the toy through the
            shell, as a command. Each copy is elsewhere above the
            megabyte, with an AutoTim, a Clock and a stack of its own,
            and writes of itself where the two are and its stack
            pointer. Three copies deep and back. Held: each copy says
            that it is where le.Linear.copies has it, which is where the
            driver was told to read; the step of t ends when the copy
            that idles has done its thing, in each of the three, and
            again in each when the one that it ran has ended; with the
            clock of the second copy on, its cell blinks while that copy
            idles and nothing does while the third idles; the stack that
            is written out at a step holds addresses of the code of the
            copy that idles and of no other, and that copy's stack
            pointer is inside it. It is run a second time with the
            driver told that every copy is where the first is, which is
            what the driver went by before it knew of copies: the step
            of t in the second copy has then to end early. And a third
            time four copies deep, one more than the script has places
            for: the driver has to say so, and driver.read to raise.

It needs the project built, for the compiler, the assembler and the
driver, and takes a minute. It ends with what it found wrong, or with
nothing wrong.

    drivercheck.py --trace          also run auto.c with QEMU's log of
                                    the code that ran, and say where the
                                    toy was in the memory by that: the
                                    place that driver.py takes for
                                    granted, le.BENCH
    drivercheck.py --below          also say how much of the extender's
                                    stack of real mode, 512 bytes, was
                                    used while the driver read above the
                                    megabyte: its own work, the BIOS's
                                    and DOS's, without the stack written
                                    out and with
    drivercheck.py --facts FILE     run nothing: print what driver.py
                                    makes of the program FILE, with the
                                    linker's map next to it under its
                                    name, .MAP or .map, as the
                                    scenarios are given it

What this cannot hold is said in TODO.md, "Piece H2, the driver".
"""
import argparse
import re
import struct
import subprocess
import sys
from collections import Counter
from dataclasses import replace
from pathlib import Path

import coverage
import dos
import driver
import le
import scenarios
import setups
from dos import MachineError

ROOT = Path(__file__).resolve().parent.parent
TOYS = Path(__file__).resolve().parent / "trial32"
BUILT = ROOT / "build" / "trial32"
# The name that the bench starts: D:\VC\VC.
NAME = "VC.EXE"
LIMIT = 30                        # seconds with nothing written: stuck
MAIN_DATA = 0xF48B                # bytes of the program's main data
# The cell that auto.c has blink, and what it shows there.
BLINKS = (0, 20)
# The extender's stack of real mode, in the segment that SS is on at the
# idle call.
BELOW = 512

problems = []


def hold(condition, text):
    if not condition:
        problems.append(text)
        print(f"  WRONG: {text}")
    return condition


class Toy:
    """A toy of the trial, built: its file as le.py reads it, the map."""

    def __init__(self, name):
        subprocess.run([TOYS / "build.sh", name, "d32"], check=True,
                       stdout=subprocess.DEVNULL)
        self.name = name
        self.contents = (BUILT / f"{name}-d32.exe").read_bytes()
        self.program = setups.Program(NAME, self.contents, setups.LISTING,
                                      BUILT / f"{name}-d32.map")
        self.flat = le.Linear(self.contents)
        self.map = le.Map(self.program.linked)

    def facts(self, watched=True, stack=True):
        """What the driver is told of the toy: with `watched`, where its
        AutoTim and its Clock are, if it has them. Of a toy with a main
        data, what driver.facts makes of any program."""
        if "_md" in self.map.public:
            return driver.facts(self.program, stack)
        named = [(name, 0) if watched and name in self.map.public else None
                 for name in ("_AutoTim", "_Clock")]
        return driver.flat_facts(setups_name(), self.flat, self.map, *named,
                                 stack=stack)


def auto_in_md():
    """auto.c with its two variables in a main data as long as the
    program's, where the listing of the original has them: a source
    written into build/trial32/, where build.sh finds it, and built."""
    listing = setups.LISTING.read_text(encoding="cp866")
    auto_time = re.search(r"^ *AutoTim [ .]*([0-9A-F]+) +DWord", listing, re.M)
    clock = re.search(r"^ *Clock [ .]*([0-9A-F]+) +Byte", listing, re.M)
    BUILT.mkdir(parents=True, exist_ok=True)
    (BUILT / "automd.c").write_text(
        "/* Written by harness/drivercheck.py. */\n"
        f"unsigned char md[0x{MAIN_DATA:X}];\n"
        f"#define AutoTim (*(unsigned long *)(md + 0x{auto_time[1]}))\n"
        f"#define Clock md[0x{clock[1]}]\n"
        '#include "auto.c"\n')
    return Toy("automd")


def setups_name():
    name = Path(NAME).stem.encode("ascii")
    return name[:driver.NAME].ljust(driver.NAME, b"\0")


def drive(bench, toy, steps, known, blinking=False, trace=None):
    """Take `toy` through `steps` on `bench` with the driver told
    `known`; return the driver.Steps, the start's first. `trace` is a
    function that is handed QEMU's log of the code that ran."""
    script = driver.script(steps, known, blinking)
    with dos.workdir() as workdir:
        workdir = Path(workdir)
        log = workdir / "trace.log" if trace else None
        with bench.start(workdir, toy.program, script, log) as machine:
            machine.resume()
            try:
                machine.wait_for_end(LIMIT)
            except MachineError as error:
                print(f"  {toy.name}.c is stuck ({error}) on")
                show(machine.screen())
            done, ended = driver.read(machine.written())
        if trace:
            trace(log)
    hold(ended and len(done) == len(steps) + 1,
         f"{toy.name}.c: the driver wrote {len(done)} screens for "
         f"{len(steps) + 1} steps, and the script "
         f"{'ended' if ended else 'did not end'}")
    return done


def show(screen):
    for line in screen.lines():
        if line.strip():
            print("    |" + line.rstrip())


def text(step):
    return "\n".join(line.rstrip() for line in step.screen.lines())


def new_lines(done, at):
    """The lines that the screen after step `at` has and the one before
    it has not."""
    before = set(done[at - 1].screen.lines()) if at else set()
    return [line.rstrip() for line in done[at].screen.lines()
            if line not in before and line.strip()]


def stack(toy, known, done, pointer=None):
    """Hold the stack that the driver wrote out at the steps `done` of
    `toy` against what is known of it. `pointer` is the stack pointer
    that the toy wrote of itself in its Main, if it did."""
    written = [step for step in done if step.stack is not None]
    if not hold(written, f"{toy.name}.c: no stack was written out"):
        return
    code = toy.flat.objects[0]
    for step in written:
        held = step.stack
        if not hold(len(held) == known.stack_size and step.stack_segment == 0,
                    f"{toy.name}.c: a stack of {len(held)} bytes in the "
                    f"segment {step.stack_segment:04X}, where the map has "
                    f"{known.stack_size} and a flat program no segment"):
            return
    left = min(driver.stack_left(step.stack, known) for step in written)
    held = written[-1].stack
    words = struct.unpack(f"<{len(held) // 4}I", held)
    in_code = [word for word in words
               if 0 <= word - le.BENCH < code.size]
    in_stack = [word for word in words
                if 0 <= word - known.stack < known.stack_size]
    print(f"  the stack: {known.stack_size} bytes at {known.stack:08X}, "
          f"{left} of them never used; of the rest, {len(in_code)} words "
          f"are addresses in the toy's code and {len(in_stack)} in its "
          "stack")
    hold(0 < left < known.stack_size,
         f"{toy.name}.c: {left} bytes of its stack are as the file has "
         "them: all of it or none")
    hold(in_code, f"{toy.name}.c: no word of its stack is an address in "
                  "its code, as a return address is")
    if pointer is not None:
        # Main has the pointer below its own variables; what it calls
        # goes further down, and nothing has been above the pointer that
        # the extender entered with.
        hold(known.stack + left <= pointer <= known.stack + known.stack_size,
             f"{toy.name}.c: its stack pointer was {pointer:08X}, and the "
             f"stack that was written out is used from "
             f"{known.stack + left:08X} to "
             f"{known.stack + known.stack_size:08X}")


def auto(bench, toy):
    known = toy.facts()
    steps = ["a", "t", "b", "c", "b"]
    name = "auto.c" if toy.name == "auto" else "auto.c with a main data"
    print(f"{name}, told that AutoTim is at {known.auto_time:08X} and "
          f"Clock at {known.clock:08X}")
    done = drive(bench, toy, steps, known, blinking=True)
    if len(done) != len(steps) + 1:
        return
    said = re.search(r"AutoTim at ([0-9A-F]{8}) Clock at ([0-9A-F]{8})",
                     text(done[0]))
    hold(said and (int(said[1], 16), int(said[2], 16))
         == (known.auto_time, known.clock),
         f"{name} says of itself: {said[0] if said else 'nothing'}")
    # The step of t: the key, and then what the toy did by itself.
    lines = new_lines(done, 2)
    print(f"  the step of t ended with: {' / '.join(lines)}")
    key = re.search(r"key 1474 at ([0-9A-F]{8})", "\n".join(lines))
    did = re.search(r"done by itself at ([0-9A-F]{8})", "\n".join(lines))
    if hold(key and did, f"{name}: the step of t ended before the toy had "
                         "done its thing"):
        hold(int(did[1], 16) - int(key[1], 16) >= 9,
             f"{name}: the toy did its thing less than nine ticks behind "
             "the key")
    hold(not done[1].screen.blinking and not done[3].screen.blinking,
         f"{name}: a cell was taken for blinking with the clock off")
    for at in (4, 5):
        blinking = done[at].screen.blinking
        print(f"  the step of {steps[at - 1]}, with the clock on: blinking "
              f"{[cell[:2] for cell in blinking]}")
        hold([cell[:2] for cell in blinking] == [BLINKS],
             f"{name}: with its clock on, the cell {BLINKS} blinks and no "
             f"other; the driver has {[cell[:2] for cell in blinking]}")
    stack(toy, known, done)
    if toy.name != "auto":
        return

    print("auto.c again, with the driver told nothing of it")
    done = drive(bench, toy, steps[:2], toy.facts(watched=False, stack=False))
    if len(done) == 3:
        lines = new_lines(done, 2)
        print(f"  the step of t ended with: {' / '.join(lines)}")
        hold("key 1474" in "\n".join(lines)
             and "done by itself" not in "\n".join(lines),
             "auto.c: the step of t did not end early with the driver "
             "told nothing: the toy shows nothing")


# copies.c: the rows of the screen that a copy has, and where its clock
# blinks, the last cell of its first row.
COPY_ROWS = 6


def copies(bench):
    """copies.c: see the top."""
    toy = Toy("copies")
    known = toy.facts()
    bases = toy.flat.copies(driver.COPIES)
    places = [(known.auto_time, known.clock, known.stack), *known.copies]
    # Each step, and the copy that idles when it has ended.
    steps = [("a", 1), ("t", 1), ("r", 2), ("t", 2), ("c", 2), ("r", 3),
             ("t", 3), ("q", 2), ("t", 2), ("o", 2), ("q", 1), ("t", 1)]
    blinks = {5: 2, 8: 2, 9: 2}         # the steps that end with a clock on
    print("copies.c, told that its copies are at "
          + ", ".join(f"{base:08X}" for base in bases))
    done = drive(bench, toy, [step for step, _ in steps], known,
                 blinking=True)
    if len(done) != len(steps) + 1:
        return
    code = toy.flat.objects[0].size
    for at, (step, copy) in enumerate([("", 1)] + steps):
        auto_time, clock, stack = places[copy - 1]
        first = done[at].screen.lines()[COPY_ROWS * (copy - 1)]
        said = re.match(rf"copy {copy} AutoTim at ([0-9A-F]{{8}}) Clock at "
                        r"([0-9A-F]{8}) ESP ([0-9A-F]{8})", first)
        if not hold(said and (int(said[1], 16), int(said[2], 16))
                    == (auto_time, clock),
                    f"copies.c, behind step {at}: copy {copy} says of "
                    f"itself {first.rstrip()!r}, and the driver was told "
                    f"{auto_time:08X} and {clock:08X}"):
            continue
        if step in ("r", ""):
            print(f"  copy {copy} says: AutoTim at {said[1]} Clock at "
                  f"{said[2]}")
        lines = "\n".join(new_lines(done, at)) if at else ""
        if step == "t":
            key = re.search(r"key 1474 at ([0-9A-F]{8})", lines)
            did = re.search(r"done by itself at ([0-9A-F]{8})", lines)
            if hold(key and did, f"copies.c: step {at}, t in copy {copy}, "
                                 "ended before the copy had done its thing"):
                hold(int(did[1], 16) - int(key[1], 16) >= 9,
                     f"copies.c: step {at}: copy {copy} did its thing less "
                     "than nine ticks behind the key")
        blinking = [cell[:2] for cell in done[at].screen.blinking]
        wanted = [(79, COPY_ROWS * (blinks[at] - 1))] if at in blinks else []
        hold(blinking == wanted,
             f"copies.c: behind step {at}, {step} with copy {copy} idling, "
             f"the driver has {blinking} as blinking, and what blinks is "
             f"{wanted}")
        # The stack that was written out is that of the copy that idles.
        held = done[at].stack
        if not hold(held is not None and len(held) == known.stack_size,
                    f"copies.c: no stack of {known.stack_size} bytes was "
                    f"written out behind step {at}"):
            continue
        words = struct.unpack(f"<{len(held) // 4}I", held)
        own = [word for word in words if 0 <= word - bases[copy - 1] < code]
        others = [word for word in words
                  if any(0 <= word - base < code for base in bases
                         if base != bases[copy - 1])]
        left = driver.stack_left(held, known)
        hold(own and not others and 0 < left < known.stack_size
             and stack + left <= int(said[3], 16) <= stack + len(held),
             f"copies.c: the stack written out behind step {at} is not "
             f"that of copy {copy}: {len(own)} words of it are addresses "
             f"in that copy's code and {len(others)} in another's, {left} "
             f"bytes of it were never used, and the copy's stack pointer "
             f"was {said[3]} with its stack at {stack:08X}")
    print(f"  {len(steps)} steps, three copies deep and back: t ended with "
          "the copy's own thing done in each, the clock of the second "
          f"blinked at steps {sorted(blinks)} and nothing else did, and "
          "each stack was that of the copy that idled")

    print("copies.c again, with the driver told that every copy is where "
          "the first is")
    blind = replace(known, copies=(places[0],) * len(known.copies))
    done = drive(bench, toy, ["r", "t"], blind)
    if len(done) == 3:
        lines = "\n".join(new_lines(done, 2))
        print("  the step of t in the second copy ended with: "
              + " / ".join(new_lines(done, 2)))
        hold("key 1474" in lines and "done by itself" not in lines,
             "copies.c: the step of t in the second copy did not end early "
             "with the driver reading the first copy: the toy shows "
             "nothing")

    print(f"copies.c again, {driver.COPIES + 1} copies deep")
    try:
        drive(bench, toy, ["r"] * driver.COPIES, known)
    except MachineError as error:
        print(f"  {error}")
        hold("no place for" in str(error), f"copies.c: {error}")
    else:
        hold(False, f"copies.c: copy {driver.COPIES + 1} idled, which the "
                    "script has no places for, and the driver did not say "
                    "so")


def keys(bench):
    toy = Toy("keys")
    known = toy.facts()
    steps = ["a", "F5", "Shift+F6", "Alt+F1", "Ctrl+o", "Up", "Click@3,5",
             "RightClick@40,12", "Wait*18", '"dir"']
    shown = ["key 1E61 shift 00", "key 3F00 shift 00", "key 5900 shift 02",
             "key 6800 shift 08", "key 180F shift 04", "key 48E0 shift 00",
             "mouse 01 at 3,5", "mouse 02 at 40,12", None,
             "key 2064 shift 00"]
    print("keys.c")
    done = drive(bench, toy, steps, known, blinking=True)
    if len(done) != len(steps) + 1:
        return
    for at, (step, wanted) in enumerate(zip(steps, shown), 1):
        lines = new_lines(done, at)
        if wanted is None:
            hold(not lines, f"keys.c: {step} wrote {lines}")
        else:
            hold(any(wanted in line for line in lines),
                 f"keys.c: the step {step} ended with {lines}, without "
                 f"{wanted}")
    hold(not any(step.screen.blinking for step in done),
         "keys.c: a cell was taken for blinking, and its Clock is 0")
    said = re.search(r"ESP ([0-9A-F]{8})", text(done[0]))
    print(f"  {len(steps)} steps, each with its key or its button on the "
          f"screen; the toy says {said[0] if said else 'nothing'}")
    stack(toy, known, done, int(said[1], 16) if said else None)
    return toy


def wait(bench):
    toy = Toy("wait")
    print("wait.c")
    done = drive(bench, toy, ["a", "b"], toy.facts())
    if len(done) != 3:
        return
    for at, wanted in ((1, "key 1E61"), (2, "key 3062")):
        hold(any(wanted in line for line in new_lines(done, at)),
             f"wait.c: step {at} ended with {new_lines(done, at)}")
    hold(all(step.stack is None for step in done),
         "wait.c: a stack was written out for a program that does not idle")
    print("  2 keys waited for inside the BIOS, and no stack written out")


def system(bench):
    """sys.c: see the top."""
    print("sys.c, by scenarios.run and driver.facts")
    try:
        subprocess.run([TOYS / "build.sh", "sys", "32"], check=True,
                       stdout=subprocess.DEVNULL)
    except subprocess.CalledProcessError:
        hold(False, "sys.c did not build for 32 bits: see "
                    "harness/trial32/build.sh")
        return
    program = setups.Program(NAME, (BUILT / "sys-32.exe").read_bytes(),
                             setups.LISTING, BUILT / "sys-32.map")
    known = driver.facts(program, stack=True)
    steps = {"a": "key: KeyWaiting 1E61 0000 KeyRead 1E61 KeysHeld 00",
             "Shift+F6": "key: KeyWaiting 5900 0000 KeyRead 5900 KeysHeld 02",
             "Click@3,5": "button of the mouse: MouseWhere 0001 0018 0028",
             "Alt+F1": "key: KeyWaiting 6800 0000 KeyRead 6800 KeysHeld 08"}
    try:
        run = scenarios.run(scenarios.Scenario("", steps=tuple(steps)),
                            program, bench, blinking=True, stack=True,
                            limit=LIMIT)
    except MachineError as error:
        hold(False, f"sys.c: {error}")
        return
    for screen, (step, wanted) in zip(run.screens[1:], steps.items()):
        first = screen.lines()[0].rstrip()
        hold(first == "turned by a " + wanted,
             f"sys.c: the page of {step} begins with {first!r}")
    hold(not any(screen.blinking for screen in run.screens),
         "sys.c: a cell was taken for blinking")
    print(f"  told that AutoTim is at {known.auto_time:08X} and Clock at "
          f"{known.clock:08X}, in its main data; {len(steps)} steps, each "
          "with its page")
    print(f"  the stack: {known.stack_size} bytes at {known.stack:08X}, "
          f"{run.stack_left} of them never used")
    hold(run.stack_segment == 0 and run.stack_left is not None
         and 0 < run.stack_left < known.stack_size,
         f"sys.c: {run.stack_left} bytes of its stack never used, in the "
         f"segment {run.stack_segment}")


def below(bench, toy):
    """How much of the extender's stack of real mode is used at the idle
    call of `toy`, with the driver reading above the megabyte. That stack
    is written out twice: the way of a program of 16 bits, from the
    segment that SS is on, which also says where it is; and then by its
    address, as a flat program's stack is, with the driver's work of
    writing a stack out on it meanwhile."""
    steps = ["a", "F5", "Click@3,5", "b"]
    known = replace(toy.facts(stack=False), stack_size=BELOW,
                    stack_loaded=bytes(BELOW))
    print("keys.c, for the extender's stack of real mode")
    for way in ("from its segment", "by its address"):
        done = drive(bench, toy, steps, known, blinking=True)
        written = [step for step in done if step.stack is not None]
        if not hold(written, "no stack was written out"):
            return
        left = min(driver.stack_left(step.stack, known) for step in written)
        segment = written[0].stack_segment or known.stack >> 4
        print(f"  of the {BELOW} bytes at {segment:04X}:0000, written out "
              f"{way}, {BELOW - left} were used and {left} never")
        known = replace(known, stack=segment << 4)


def placed(toy):
    """A function for drive() that says where `toy` was in the memory,
    by QEMU's log of the code that ran: where the blocks of that code
    are that the toy's file has once."""
    def trace(log):
        image = toy.flat.image()
        votes = Counter()
        for block in coverage.blocks(log):
            code = b"".join(code for _, code in block)
            at = image.find(code)
            if len(code) >= coverage.MIN_BLOCK and at >= 0 \
                    and image.find(code, at + 1) < 0:
                votes[block[0][0] - at] += 1
        found = votes.most_common(2)
        print("  by the code that ran, the toy's first object was at "
              + ", ".join(f"{base:08X} ({count} blocks)"
                          for base, count in found))
        hold(found and found[0][0] == le.BENCH
             and found[0][1] >= coverage.MIN_VOTES,
             f"the toy was not at {le.BENCH:08X}, where le.BENCH has a "
             "program on the bench")
    return trace


def facts(path):
    program = setups.Program.load(path)
    if program.linked and not program.linked.exists():
        program = replace(program, linked=program.linked.with_suffix(".map"))
    for stack in (False, True):
        known = driver.facts(program, stack)
    print(f"{path}: {'a flat program' if known.flat else '16 bits'}, "
          f"watched as {known.name.rstrip(bytes(1)).decode()}")
    print(f"  AutoTim at {known.auto_time:08X}, Clock at {known.clock:08X}")
    print(f"  the stack: {known.stack_size} bytes at {known.stack:08X}, "
          f"of which {known.stack_size - known.stack_loaded.count(0)} are "
          "not 0 in the file")


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--trace", action="store_true")
    parser.add_argument("--below", action="store_true")
    parser.add_argument("--facts", metavar="FILE")
    args = parser.parse_args()
    if args.facts:
        facts(args.facts)
        return
    with setups.Benches() as benches:
        bench = benches["us"]
        auto(bench, Toy("auto"))
        auto(bench, auto_in_md())
        toy = keys(bench)
        wait(bench)
        copies(bench)
        system(bench)
        if args.below and toy:
            below(bench, toy)
        if args.trace:
            toy = Toy("auto")
            print("auto.c, with QEMU's log of the code")
            drive(bench, toy, ["a"], toy.facts(), trace=placed(toy))
    if problems:
        sys.exit(f"{len(problems)} wrong:\n  " + "\n  ".join(problems))
    print("Nothing wrong.")


if __name__ == "__main__":
    main()
