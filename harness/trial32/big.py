#!/usr/bin/env python3
"""Write two files of C into build/trial32/ that make a toy's file
longer, to see what the extender takes to load it (see start.c):

  fat.c   180,000 bytes more in the file, and no address in them
  big.c   100,000 bytes and a table of 20,000 addresses, each a
          relocation that the extender applies: 9 bytes of the file each
"""
from pathlib import Path

OUT = Path(__file__).resolve().parent.parent.parent / "build" / "trial32"


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "fat.c").write_text('#include "toy.h"\n'
                               "u8 Fat[180000] = { 1 };\n")
    lines = ['#include "toy.h"', "extern u8 Filler[];",
             "void *Table[20000] = {"]
    lines += [f"    Filler + {5 * at}," for at in range(20000)]
    lines += ["};", "u8 Filler[100000] = { 1 };"]
    (OUT / "big.c").write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
