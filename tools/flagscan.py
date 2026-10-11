#!/usr/bin/env python3
"""Find callers that read the flags a routine leaves behind.

An assembly routine hands back the processor flags along with everything
else, and a caller may branch on them even where no comment says so. A
routine translated to C leaves the flags in no particular state, so before
one is translated each of its callers has to be looked at.

For every CALL of the routines named, this follows the code behind it
until the flags are dead: to the first instruction that sets all the
arithmetic flags. It follows them wherever they travel: through a jump,
into a routine that is called and back out of it, and through a return to
every place that calls the routine returned from, or that calls a routine
which jumps into it. An instruction that reads a flag on the way is
reported, with the way to it. So is a place where the flags cannot be
followed: an indirect jump or call, a far return, a return from code that
nothing is seen to call.

A call of a C function ends the way: whatever follows it has been looked
at when that function was translated.

The direction flag is another matter. C code needs it clear when it is
entered and leaves it clear, so no call may be made while it is set. That
is checked for the whole program, whatever routines are named: from every
STD the code is followed to a CLD, and a call or a return on the way is
reported.

Usage: flagscan.py <routine>... (scans port/asm)
"""
import re
import sys
from dataclasses import dataclass
from pathlib import Path

SOURCES = Path(__file__).resolve().parent.parent / "port" / "asm"

SETS_ALL = {"ADD", "SUB", "CMP", "TEST", "AND", "OR", "XOR", "NEG", "SHL",
            "SHR", "SAR", "SAL", "MUL", "DIV", "IMUL", "IDIV", "CMPSB",
            "CMPSW", "SCASB", "SCASW", "POPF", "SAHF", "INT"}
READS = re.compile(r"(J(?!MP|CXZ)[A-Z]+|ADC|SBB|RCL|RCR|PUSHF|LAHF|CMC|"
                   r"LOOPN?[EZ]|INTO)$")
BRANCHES_ON_CX = {"JCXZ", "LOOP"}       # go two ways without reading a flag
RETURNS = {"RET", "RETN"}
FAR_RETURNS = {"RETF", "IRET"}
CALLS_C = {"CALLC", "CALLLINE"}         # macros of VC.ASM that call C code
DEPTH = 8                               # calls followed into one another


@dataclass
class Line:
    file: str
    number: int
    code: str                           # without label and comment
    mnemonic: str
    procedure: str                      # the PROC it is in, or a name
                                        # for the code between two

    @property
    def place(self):
        return f"{self.file}:{self.number}"


class Program:
    """The assembly of the port, line by line, with its labels."""

    def __init__(self, directory):
        self.lines = []
        self.labels = {}                # name -> index of its line
        self.procedures = {}            # name -> (first index, last index)
        for path in sorted(directory.iterdir()):
            text = path.read_text(encoding="cp866").split("\n")
            # Code outside any PROC is taken as a routine of its own, from
            # one PROC to the next: such code is jumped into and out of.
            loose = f"the code from {path.name}:1"
            procedure, start = loose, len(self.lines)
            for number, raw in enumerate(text, 1):
                code = re.sub(r"""('[^']*'|"[^"]*")|;.*""",
                              lambda m: m[1] or "", raw).strip()
                while (label := re.match(r"(\w+):\s*", code)):
                    self.labels[label[1]] = len(self.lines)
                    code = code[label.end():]
                words = code.split()
                if len(words) > 1 and words[1].upper() == "PROC":
                    self.procedures[procedure] = (start, len(self.lines))
                    procedure, start = words[0], len(self.lines)
                    self.labels[procedure] = start
                    code = ""
                elif len(words) > 1 and words[1].upper() == "ENDP":
                    self.procedures[procedure] = (start, len(self.lines))
                    loose = f"the code from {path.name}:{number + 1}"
                    procedure, start, code = loose, len(self.lines) + 1, ""
                elif len(words) > 1 and words[1].upper() == "LABEL":
                    self.labels[words[0]] = len(self.lines)
                    code = ""
                mnemonic = code.split()[0].upper() if code else ""
                if mnemonic in ("REP", "REPE", "REPZ", "REPNE", "REPNZ"):
                    mnemonic = code.split()[1].upper()
                self.lines.append(Line(path.name, number, code, mnemonic,
                                       procedure))
            self.procedures[procedure] = (start, len(self.lines))

    def target(self, line):
        """The index of the label that a jump or a call goes to, or None
        if it does not name one."""
        operand = re.sub(r"\b(SHORT|NEAR PTR)\b", "", line.code,
                         flags=re.IGNORECASE).split(None, 1)
        if len(operand) == 2 and operand[1].strip() in self.labels:
            return self.labels[operand[1].strip()]
        return None

    def entries(self, procedure, seen=None):
        """Where the flags go when `procedure` returns: the indices behind
        every call of it, with how they were found, and reasons why some
        may be missing. A routine that jumps into it passes its own callers
        on."""
        seen = seen if seen is not None else set()
        if procedure in seen:
            return [], []
        seen.add(procedure)
        first, last = self.procedures[procedure]
        after, unknown = [], []
        for index, line in enumerate(self.lines):
            target = self.target(line)
            if target is None or not first <= target < last:
                continue
            if line.mnemonic == "CALL":
                after.append((index + 1, f"the return of {procedure} to "
                                         f"{line.place}"))
            elif line.mnemonic.startswith(("J", "LOOP")) \
                    and not first <= index < last:
                more, why = self.entries(line.procedure, seen)
                after += more
                unknown += why
        return after, unknown


def follow(program, start):
    """Follow the flags from the line with index `start`. Returns what was
    found: (place, what, the way there)."""
    found = {}
    seen = set()
    pending = [(start, (), ())]         # index, calls to return from, way
    while pending:
        index, stack, way = pending.pop()
        while index < len(program.lines):
            if (index, stack) in seen:
                break
            seen.add((index, stack))
            line = program.lines[index]
            mnemonic = line.mnemonic

            def report(what):
                found.setdefault((line.place, what), way)

            if mnemonic in SETS_ALL or mnemonic in CALLS_C \
                    or mnemonic == "CALL" and "FAR PTR" in line.code.upper():
                break
            if READS.match(mnemonic):
                report(f"reads a flag: {line.code}")
                break
            if mnemonic in FAR_RETURNS:
                report(f"cannot be followed: {line.code}")
                break
            if mnemonic in RETURNS:
                if stack:
                    index, stack = stack[-1], stack[:-1]
                    continue
                after, unknown = program.entries(line.procedure)
                if not after and not unknown:
                    report(f"cannot be followed: nothing is seen to call "
                           f"{line.procedure}")
                for why in unknown:
                    found.setdefault((line.place, f"cannot be followed: "
                                                  f"{why}"), way)
                for there, how in after:
                    pending.append((there, (), way + (how,)))
                break
            if mnemonic in ("JMP", "CALL") or mnemonic in BRANCHES_ON_CX:
                target = program.target(line)
                if target is None:
                    report(f"cannot be followed: {line.code}")
                    break
                if mnemonic == "CALL":
                    if len(stack) == DEPTH:
                        report(f"not followed any deeper: {line.code}")
                        break
                    index, stack = target, stack + (index + 1,)
                    continue
                if mnemonic == "JMP":
                    index = target
                    continue
                pending.append((target, stack, way))
            index += 1
    return [(place, what, way) for (place, what), way in found.items()]


def direction(program):
    """Report where the direction flag is set when a call is made or a
    routine returns. Returns the number of STD instructions looked at."""
    count = 0
    for start, std in enumerate(program.lines):
        if std.mnemonic != "STD":
            continue
        count += 1
        seen = set()
        pending = [start + 1]
        while pending:
            index = pending.pop()
            while index < len(program.lines) and index not in seen:
                seen.add(index)
                line = program.lines[index]
                mnemonic = line.mnemonic
                if mnemonic == "CLD":
                    break
                if mnemonic in ("CALL", "ENDM") or mnemonic in CALLS_C \
                        or mnemonic in RETURNS or mnemonic in FAR_RETURNS:
                    print(f"{std.place}: the direction flag is still set "
                          f"at {line.place}: {line.code}")
                    break
                if mnemonic.startswith(("J", "LOOP")):
                    target = program.target(line)
                    if target is None:
                        print(f"{std.place}: the direction flag is still "
                              f"set at {line.place}: {line.code}")
                        break
                    if mnemonic == "JMP":
                        index = target
                        continue
                    pending.append(target)
                index += 1
    return count


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    names = set(sys.argv[1:])
    program = Program(SOURCES)
    for name in sorted(names - program.labels.keys()):
        sys.exit(f"no routine {name} in {SOURCES}")
    calls = clean = 0
    for index, line in enumerate(program.lines):
        if line.mnemonic != "CALL":
            continue
        target = program.target(line)
        called = line.code.split()[-1]
        if target is None or called not in names:
            continue
        calls += 1
        found = follow(program, index + 1)
        clean += not found
        for place, what, way in sorted(found):
            via = "".join(f", by {step}" for step in way)
            print(f"{line.place} after {called}: {place} {what}{via}")
    print(f"{calls} calls of {', '.join(sorted(names))} followed; behind "
          f"{clean} of them nothing reads the flags they leave")
    print(f"{direction(program)} STD instructions followed to their CLD")


if __name__ == "__main__":
    main()
