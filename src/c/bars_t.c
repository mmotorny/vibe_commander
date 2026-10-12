/*
 * The table of the bars: the names of the menus in the menu bar, which
 * src/c/bars.c writes and by which src/c/menu.c finds the menu that the
 * mouse is on.
 *
 * From VCSUB2.INC.
 */
#include "tables.h"

/* What others name has the name that it has in the assembly. */
#pragma aux PtMnu_S "*"

/* The names of the five menus, each behind four spaces, and spaces to
   the end of the row: FFh and a number are that many spaces. */
const u8 PtMnu_S[] =
    "\377\004Left\377\004Files\377\004Commands"
    "\377\004Options\377\004Right\377\037";
