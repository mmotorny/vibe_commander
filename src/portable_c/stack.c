/*
 * The stack of the program: LenStk bytes, sixteen times the 100h that
 * the original has in the program of 16 bits and four times as many in
 * the one of 32 (see vc.h). Compiled code uses more of the stack than
 * the assembly did, and with this much its use needs no care.
 *
 * It is a file of its own because it is a segment of its own, which the
 * language has no word for. The line below names the segment and its
 * class, STACK: the linker takes a segment of that class for the stack
 * and has the header of the program name its end, so the program is
 * started on it, by DOS or by the extender.
 *
 * For 16 bits the build compiles this file with an option that gives
 * its data a group of its own, S_GROUP (-nd=S, see CMakeLists.txt), and
 * the segment is of that group. The linker's command has it last. So
 * the program ends where its stack does, which is how it knows how long
 * it is (MemOwn of dos.c). The stack is not of the segment that DS is
 * on because it does not fit there beside the main data. A pointer to a
 * local variable is declared STACK for that (see vc.h).
 *
 * For 32 bits it is data like any other, and the linker's command has
 * it in front of the rest of the data (see CMakeLists.txt, which says
 * why there).
 *
 * It has a value, zeros, so that it is in the program's file: the tests
 * tell how much of it a scenario has never used by what is still as it
 * was loaded (harness/driver.py).
 */
#define STACK_C                         /* vc.h declares it far for the
                                           others */
#include "vc.h"

#pragma data_seg("STACK", "STACK")
u8 Stack[LenStk] = { 0 };
#pragma data_seg()
