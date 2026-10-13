#!/usr/bin/env python3
"""Hold the arithmetic that the compiler calls for against Python's.

For a product or a quotient of numbers of 32 bits the compiled code calls
two routines, __U4M and __U4D, which are functions of arith.c in a C
tree, src/c/ or src/portable_c/. No
scenario shows a wrong one for what it is, and of the division the
scenarios run little: nothing divides by more than 16 bits. So the two
are run here, as the bytes that they are in the linked program, on a
model of the 8086 that knows the instructions they are made of, with
many pairs of numbers, and what comes back is held against what Python
makes of the same numbers: the product in DX:AX, the quotient in DX:AX
and the remainder in CX:BX. The compiler calls them near in the model
that the C translation is compiled in, and far in that of the portable
C: which it is, each routine says by how it returns.

It also fails if a routine changes a register that it is to keep, BX and
CX among them for __U4M, if it writes to the stack of its caller or
reads anything but the stack, if it calls something, and if a division
by 0 is not the processor's divide error. An instruction that the model
does not know stops it: add the instruction.

It is the test `c-arith` of the build, with the pairs of numbers at the
edges, 729 of them, and 4,000 random ones, which takes two seconds:
about 2,200 of the divisions are by more than 16 bits. `--pairs` asks
for more; the routines were held to 200,000 when they were written,
which takes two and a half minutes. The numbers are the same from run
to run.

Exits with status 0 if everything is as Python has it, and 1 otherwise,
with the routine and the numbers.
"""
import argparse
import random
import re
import sys
from pathlib import Path

import coverage
STACK = 0x100               # of the model, and where a call begins in it
ENTERED = 0xC0
AX, CX, DX, BX, SP, BP, SI, DI = range(8)
NAMES = "ax cx dx bx sp bp si di".split()


CALLER = 0xCA11             # the segment that a far call comes from


class Unknown(Exception):
    pass


class CalledFar(Exception):
    """A routine that was called near returns far."""


class Fault(Exception):
    pass


class Machine:
    def __init__(self, code):
        self.code = code            # the bytes of the code segment
        self.stack = bytearray(STACK)
        self.r = [0] * 8
        self.ip = 0
        self.cf = self.zf = self.sf = self.of = 0
        self.count = 0
        self.far = False            # how the routine is called

    # --- memory: the stack segment only, which is all they may touch
    def rd16(self, at):
        at &= 0xFFFF
        if at + 1 >= len(self.stack):
            raise Fault(f"read at {at:04X}h, outside the stack")
        return self.stack[at] | self.stack[at + 1] << 8

    def wr16(self, at, value):
        at &= 0xFFFF
        if at + 1 >= len(self.stack):
            raise Fault(f"write at {at:04X}h, outside the stack")
        self.stack[at] = value & 0xFF
        self.stack[at + 1] = value >> 8 & 0xFF

    def rd8(self, at):
        at &= 0xFFFF
        if at >= len(self.stack):
            raise Fault(f"read at {at:04X}h, outside the stack")
        return self.stack[at]

    def wr8(self, at, value):
        at &= 0xFFFF
        if at >= len(self.stack):
            raise Fault(f"write at {at:04X}h, outside the stack")
        self.stack[at] = value & 0xFF

    def push(self, value):
        self.r[SP] = self.r[SP] - 2 & 0xFFFF
        self.wr16(self.r[SP], value)

    def pop(self):
        value = self.rd16(self.r[SP])
        self.r[SP] = self.r[SP] + 2 & 0xFFFF
        return value

    # --- fetching
    def byte(self):
        value = self.code[self.ip]
        self.ip += 1
        return value

    def word(self):
        return self.byte() | self.byte() << 8

    def sbyte(self):
        value = self.byte()
        return value - 256 if value & 0x80 else value

    # --- registers of 8 bits: al cl dl bl ah ch dh bh
    def get8(self, n):
        return self.r[n] & 0xFF if n < 4 else self.r[n - 4] >> 8

    def set8(self, n, value):
        value &= 0xFF
        if n < 4:
            self.r[n] = self.r[n] & 0xFF00 | value
        else:
            self.r[n - 4] = self.r[n - 4] & 0xFF | value << 8

    # --- an operand by its ModRM byte
    def modrm(self):
        b = self.byte()
        mod, reg, rm = b >> 6, b >> 3 & 7, b & 7
        if mod == 3:
            return reg, ("r", rm)
        if mod == 0 and rm == 6:
            raise Unknown("an operand at a fixed address: not the stack")
        disp = 0 if mod == 0 else self.sbyte() if mod == 1 else self.word()
        if rm == 6:
            base = self.r[BP]
        elif rm == 2:
            base = self.r[BP] + self.r[SI]
        elif rm == 3:
            base = self.r[BP] + self.r[DI]
        else:
            raise Unknown("an operand through DS: not the stack")
        return reg, ("m", base + disp & 0xFFFF)

    def get(self, where, wide):
        kind, n = where
        if kind == "r":
            return self.r[n] if wide else self.get8(n)
        return self.rd16(n) if wide else self.rd8(n)

    def put(self, where, wide, value):
        kind, n = where
        if kind == "r":
            if wide:
                self.r[n] = value & 0xFFFF
            else:
                self.set8(n, value)
        elif wide:
            self.wr16(n, value)
        else:
            self.wr8(n, value)

    # --- arithmetic
    def szf(self, value, wide):
        bits = 16 if wide else 8
        value &= (1 << bits) - 1
        self.zf = int(value == 0)
        self.sf = value >> bits - 1
        return value

    def alu(self, op, a, b, wide):
        """add or adc sbb and sub xor cmp, by number. Returns the result,
        or None for cmp."""
        bits = 16 if wide else 8
        mask = (1 << bits) - 1
        top = 1 << bits - 1
        if op in (0, 2):                    # add, adc
            c = self.cf if op == 2 else 0
            full = a + b + c
            result = full & mask
            self.cf = int(full > mask)
            self.of = int(bool(~(a ^ b) & (a ^ result) & top))
        elif op in (3, 5, 7):               # sbb, sub, cmp
            c = self.cf if op == 3 else 0
            full = a - b - c
            result = full & mask
            self.cf = int(full < 0)
            self.of = int(bool((a ^ b) & (a ^ result) & top))
        else:
            result = (a | b, a & b, a ^ b)[(1, 4, 6).index(op)]
            self.cf = self.of = 0
        self.szf(result, wide)
        return None if op == 7 else result

    def shift(self, op, value, count, wide):
        bits = 16 if wide else 8
        mask = (1 << bits) - 1
        top = 1 << bits - 1
        for _ in range(count):
            if op == 0:                     # rol
                self.cf = value >> bits - 1
                value = (value << 1 | self.cf) & mask
            elif op == 1:                   # ror
                self.cf = value & 1
                value = value >> 1 | self.cf << bits - 1
            elif op == 2:                   # rcl
                carry = value >> bits - 1
                value = (value << 1 | self.cf) & mask
                self.cf = carry
            elif op == 3:                   # rcr
                carry = value & 1
                value = value >> 1 | self.cf << bits - 1
                self.cf = carry
            elif op in (4, 6):              # shl
                self.cf = value >> bits - 1
                value = value << 1 & mask
                self.szf(value, wide)
            elif op == 5:                   # shr
                self.cf = value & 1
                value >>= 1
                self.szf(value, wide)
            else:                           # sar
                self.cf = value & 1
                value = value >> 1 | value & top
                self.szf(value, wide)
        if count == 1:
            if op in (0, 2, 4, 6):
                self.of = (value >> bits - 1) ^ self.cf
            elif op in (1, 3):
                self.of = (value >> bits - 1) ^ (value >> bits - 2 & 1)
            elif op == 5:
                self.of = value >> bits - 2 & 1
            else:
                self.of = 0
        return value

    def condition(self, n):
        answer = (self.of, self.cf, self.zf, self.cf | self.zf, self.sf,
                  None, self.sf ^ self.of,
                  (self.sf ^ self.of) | self.zf)[n >> 1]
        if answer is None:
            raise Unknown("a jump on the parity flag")
        return bool(answer) != bool(n & 1)

    # --- one instruction
    def step(self):
        self.count += 1
        op = self.byte()
        if op < 0x40 and op & 7 < 6:
            kind, wide = op >> 3, op & 1
            if op & 4:                      # with AL or AX
                b = self.word() if wide else self.byte()
                result = self.alu(kind, self.get(("r", 0), wide), b, wide)
                if result is not None:
                    self.put(("r", 0), wide, result)
                return
            reg, rm = self.modrm()
            to, frm = (("r", reg), rm) if op & 2 else (rm, ("r", reg))
            result = self.alu(kind, self.get(to, wide), self.get(frm, wide),
                              wide)
            if result is not None:
                self.put(to, wide, result)
        elif 0x40 <= op < 0x50:             # inc, dec: the carry stays
            n = op & 7
            carry = self.cf
            self.r[n] = self.alu(0 if op < 0x48 else 5, self.r[n], 1, 1)
            self.cf = carry
        elif 0x50 <= op < 0x58:
            self.push(self.r[op & 7])
        elif 0x58 <= op < 0x60:
            self.r[op & 7] = self.pop()
        elif 0x70 <= op < 0x80:
            to = self.sbyte()
            if self.condition(op & 15):
                self.ip += to
        elif op in (0x80, 0x81, 0x83):
            wide = op & 1
            reg, rm = self.modrm()
            b = (self.word() if op == 0x81 else
                 self.sbyte() & (0xFFFF if wide else 0xFF))
            result = self.alu(reg, self.get(rm, wide), b, wide)
            if result is not None:
                self.put(rm, wide, result)
        elif op in (0x84, 0x85):
            wide = op & 1
            reg, rm = self.modrm()
            self.alu(4, self.get(rm, wide), self.get(("r", reg), wide), wide)
        elif op in (0x86, 0x87):
            wide = op & 1
            reg, rm = self.modrm()
            a, b = self.get(rm, wide), self.get(("r", reg), wide)
            self.put(rm, wide, b)
            self.put(("r", reg), wide, a)
        elif 0x88 <= op < 0x8C:
            wide = op & 1
            reg, rm = self.modrm()
            to, frm = (("r", reg), rm) if op & 2 else (rm, ("r", reg))
            self.put(to, wide, self.get(frm, wide))
        elif 0x90 <= op < 0x98:
            n = op & 7
            self.r[AX], self.r[n] = self.r[n], self.r[AX]
        elif 0xB0 <= op < 0xB8:
            self.set8(op & 7, self.byte())
        elif 0xB8 <= op < 0xC0:
            self.r[op & 7] = self.word()
        elif op == 0xC3:
            self.ip = self.pop()
        elif op == 0xCB:                    # retf: to where it was called
            if not self.far:                # from, a segment that is none
                raise CalledFar()
            self.ip = self.pop()
            if self.pop() != CALLER:
                raise Fault("it returns to another segment than its "
                            "caller's")
        elif op in (0xC6, 0xC7):
            wide = op & 1
            reg, rm = self.modrm()
            self.put(rm, wide, self.word() if wide else self.byte())
        elif 0xD0 <= op < 0xD4:
            wide = op & 1
            reg, rm = self.modrm()
            count = self.r[CX] & 0xFF if op & 2 else 1
            self.put(rm, wide, self.shift(reg, self.get(rm, wide), count,
                                          wide))
        elif op == 0xE2:
            to = self.sbyte()
            self.r[CX] = self.r[CX] - 1 & 0xFFFF
            if self.r[CX]:
                self.ip += to
        elif op == 0xE3:
            to = self.sbyte()
            if not self.r[CX]:
                self.ip += to
        elif op == 0xE9:
            to = self.word()
            self.ip = self.ip + to & 0xFFFF
        elif op == 0xEB:
            to = self.sbyte()
            self.ip += to
        elif op in (0xF6, 0xF7):
            wide = op & 1
            reg, rm = self.modrm()
            value = self.get(rm, wide)
            if reg == 0:
                self.alu(4, value, self.word() if wide else self.byte(), wide)
            elif reg == 2:
                self.put(rm, wide, ~value)
            elif reg == 3:
                self.put(rm, wide, self.alu(5, 0, value, wide))
            elif reg == 4 and wide:         # mul
                product = self.r[AX] * value
                self.r[AX], self.r[DX] = product & 0xFFFF, product >> 16
                self.cf = self.of = int(self.r[DX] != 0)
            elif reg == 4:
                self.r[AX] = self.get8(0) * value
                self.cf = self.of = int(self.r[AX] > 0xFF)
            elif reg == 6 and wide:         # div
                number = self.r[DX] << 16 | self.r[AX]
                if value == 0 or number // value > 0xFFFF:
                    raise Fault("divide error")
                self.r[AX], self.r[DX] = number // value, number % value
            else:
                raise Unknown(f"{op:02X}h /{reg}")
        elif op == 0xFC:
            pass                            # cld
        else:
            raise Unknown(f"the instruction {op:02X}h at "
                          f"{self.ip - 1:04X}h")

    def call(self, entry, ax, dx, bx, cx):
        """Call the routine at `entry`, far if `self.far` and else near.
        Returns the registers, and fails if it does not come back with
        the stack as it was."""
        self.r = [random.getrandbits(16) for _ in range(8)]
        self.r[AX], self.r[DX], self.r[BX], self.r[CX] = ax, dx, bx, cx
        self.r[SP] = ENTERED
        kept = self.r[:]
        self.stack[:] = random.randbytes(STACK)
        above = bytes(self.stack[ENTERED:])
        if self.far:
            self.push(CALLER)
        self.push(0xFFFF)                   # where it returns to
        self.ip = entry
        self.cf, self.zf, self.sf, self.of = (random.getrandbits(1)
                                               for _ in range(4))
        while self.ip != 0xFFFF:
            if self.count > 100000:
                raise Fault("it does not come back")
            self.step()
        if bytes(self.stack[ENTERED:]) != above:
            raise Fault("the caller's stack was written to")
        return self.r[:], kept


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("translation", type=Path,
                        help="the directory of a C tree's build, build/c "
                             "or build/portable_c: VC.EXE, VC.MAP")
    parser.add_argument("--pairs", type=int, default=4000, metavar="N",
                        help="how many random pairs of numbers, besides "
                             "those at the edges (4000)")
    args = parser.parse_args()
    directory = args.translation
    count = args.pairs
    image = (directory / "VC.EXE").read_bytes()
    segment = None
    places = {}
    for line in open(directory / "VC.MAP"):
        match = re.match(r"CTEXT +CCODE +\w+ +([0-9a-f]{4}):([0-9a-f]{4}) +"
                         r"([0-9a-f]{8})$", line)
        if match:
            segment = (int(match[1], 16) * 16 + int(match[2], 16),
                       int(match[3], 16))
        match = re.match(r"([0-9a-f]{4}):([0-9a-f]{4})[*+s]? +(__U4[MD])$",
                         line)
        if match:
            places[match[3]] = int(match[1], 16) * 16 + int(match[2], 16)
    if segment is None or len(places) != 2:
        sys.exit("arithcheck: the linker's map has no CTEXT, or not both "
                 "routines")
    begin, size = segment
    at = begin + coverage.origin(image)
    code = image[at:at + size]
    # The addresses of the map count the segment's paragraph and offset:
    # in the code they are offsets from the paragraph.
    base = begin & ~0xF
    code = bytes(begin - base) + code
    machine = Machine(code)
    random.seed(4052000)

    edges = [0, 1, 2, 3, 0xFF, 0x100, 0x7FFF, 0x8000, 0xFFFF, 0x10000,
             0x10001, 0xFFFFF, 0x7FFFFFFF, 0x80000000, 0x80000001,
             0xFFFF0000, 0xFFFFFFFE, 0xFFFFFFFF, 10, 100, 1000, 10000,
             100000, 1000000, 10000000, 100000000, 1000000000]

    def numbers():
        for a in edges:
            for b in edges:
                yield a, b
        for _ in range(count):
            # Of every length, so that small divisors and quotients come
            # up as often as large ones.
            yield (random.getrandbits(random.randint(1, 32)),
                   random.getrandbits(random.randint(1, 32)))

    tried = {"__U4M": 0, "__U4D": 0, "__U4D by 16 bits": 0,
             "__U4D by more": 0}
    longest = {"__U4M": 0, "__U4D": 0}
    keeps = {"__U4M": set(range(8)) - {AX, DX},
             "__U4D": set(range(8)) - {AX, BX, CX, DX}}
    # How each is called: near, unless it returns far from that.
    far = {}
    for name in ("__U4M", "__U4D"):
        machine.far = False
        try:
            machine.call(places[name] - base, 1, 0, 1, 0)
        except CalledFar:
            machine.far = True
        except (Unknown, Fault) as problem:
            sys.exit(f"{name}: {problem}, for 1 and 1")
        far[name] = machine.far
        print(f"{name}: called {'far' if machine.far else 'near'}")
    for a, b in numbers():
        for name in ("__U4M", "__U4D"):
            if name == "__U4D" and b == 0:
                continue
            machine.count = 0
            machine.far = far[name]
            try:
                got, kept = machine.call(places[name] - base, a & 0xFFFF,
                                         a >> 16, b & 0xFFFF, b >> 16)
            except (Unknown, Fault, CalledFar) as problem:
                sys.exit(f"{name}: {problem}, for {a:08X}h and {b:08X}h")
            longest[name] = max(longest[name], machine.count)
            result = got[DX] << 16 | got[AX]
            if name == "__U4M":
                wanted = a * b & 0xFFFFFFFF
                if result != wanted:
                    sys.exit(f"{name}: {a:08X}h * {b:08X}h is {wanted:08X}h, "
                             f"not {result:08X}h")
            else:
                rest = got[CX] << 16 | got[BX]
                if (result, rest) != (a // b, a % b):
                    sys.exit(f"{name}: {a:08X}h / {b:08X}h is {a // b:08X}h "
                             f"and {a % b:08X}h left, not {result:08X}h and "
                             f"{rest:08X}h")
                tried["__U4D by 16 bits" if b < 0x10000
                      else "__U4D by more"] += 1
            for n in keeps[name]:
                if got[n] != kept[n]:
                    sys.exit(f"{name}: {NAMES[n]} comes back changed, for "
                             f"{a:08X}h and {b:08X}h")
            tried[name] += 1
    if not tried["__U4D by more"] or not tried["__U4D by 16 bits"]:
        sys.exit("arithcheck: one half of __U4D was not run")
    for name, n in tried.items():
        print(f"{name}: {n} pairs of numbers as Python has them"
              + (f", {longest[name]} instructions at the most"
                 if name in longest else ""))
    # A division by nothing is the processor's divide error, in both.
    machine.far = far["__U4D"]
    for a in (0, 1, 0x12345678):
        try:
            machine.call(places["__U4D"] - base, a & 0xFFFF, a >> 16, 0, 0)
        except Fault as fault:
            if str(fault) != "divide error":
                raise
        else:
            sys.exit(f"__U4D: {a:08X}h divided by 0 comes back")
    print("__U4D: a division by 0 is a divide error")


if __name__ == "__main__":
    main()
