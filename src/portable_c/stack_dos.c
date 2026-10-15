/*
 * The stack of the program: LenStk bytes, sixty-four times the 100h
 * that the original has (see vc.h). Compiled code uses more of the
 * stack than the assembly did, and with this much its use needs no
 * care.
 *
 * It is a file of its own because it is a segment of its own, which the
 * language has no word for. The line below names the segment and its
 * class, STACK, by SEGMENT_BEGIN of compiler.h: the linker takes a
 * segment of that class for the stack and has the header of the
 * program name its end, so the program is started on it, by the
 * extender. It is data like any other besides, and the linker's
 * command has it in front of the rest of the data (see CMakeLists.txt,
 * which says why there). It is the build for DOS's own, _dos.c, since
 * another system has a stack of its own making.
 *
 * It has a value, zeros, so that it is in the program's file: the tests
 * tell how much of it a scenario has never used by what is still as it
 * was loaded (harness/driver.py).
 */
#include "vc.h"

SEGMENT_BEGIN("STACK", "STACK")
u8 Stack[LenStk] = { 0 };
SEGMENT_END
