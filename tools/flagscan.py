#!/usr/bin/env python3
"""Find callers that read the flags a routine leaves behind.

An assembly routine hands back the processor flags along with everything
else, and a caller may branch on them even where no comment says so. A
routine translated to C leaves the flags in no particular state, so before
one is translated each of its callers has to be looked at.

For every CALL of the routines named, this follows the code behind it to
the first instruction that sets all the arithmetic flags, which makes the
call safe, or that reads one, which is reported. It also reports where it
cannot tell: at a jump, a call or a return, where the flags travel on.
String instructions are reported for the direction flag they use; C code
leaves it clear.

Usage: flagscan.py <routine>... (scans port/asm)
"""
import re
import sys
from pathlib import Path

SOURCES = Path(__file__).resolve().parent.parent / "port" / "asm"

SETS_ALL = {"ADD", "SUB", "CMP", "TEST", "AND", "OR", "XOR", "NEG", "SHL",
            "SHR", "SAR", "SAL", "MUL", "DIV", "IMUL", "IDIV", "CMPSB",
            "CMPSW", "SCASB", "SCASW", "POPF", "SAHF", "INT"}
READS = re.compile(r"(J(?!MP|CXZ)[A-Z]+|ADC|SBB|RCL|RCR|PUSHF|LAHF|CMC|"
                   r"LOOPN?[EZ]|REPN?[EZ]?)$")
LEAVES = {"RET", "RETF", "RETN", "IRET", "JMP", "CALL"}
REACH = 40                              # lines to follow


def scan(path, names):
    lines = path.read_text(encoding="cp866").split("\n")
    for number, line in enumerate(lines):
        call = re.search(r"\bCALL\s+(\w+)", line.split(";")[0])
        if not call or call[1] not in names:
            continue
        verdict = "nothing decisive nearby"
        for ahead in range(number + 1, min(number + REACH, len(lines))):
            code = re.sub(r"^\w+:\s*", "", lines[ahead].split(";")[0].strip())
            if not code:
                continue
            mnemonic = code.split()[0].upper()
            if READS.match(mnemonic):
                verdict = f"read {ahead - number} lines on: {code}"
            elif mnemonic in LEAVES:
                verdict = f"travel on {ahead - number} lines on: {code}"
            elif mnemonic not in SETS_ALL:
                continue
            else:
                verdict = None
            break
        if verdict:
            print(f"{path.name}:{number + 1} after {call[1]}: {verdict}")


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    for path in sorted(SOURCES.iterdir()):
        scan(path, set(sys.argv[1:]))


if __name__ == "__main__":
    main()
