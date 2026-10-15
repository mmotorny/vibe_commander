/*
 * The tables of the setup's help: what sthelp.c hands to Dialog and
 * MinMnu by its offset.
 *
 * From VCSTHLP.INC, of the original's setup program. A name here is the
 * assembly's with St in front, as for everything of that program: the
 * main program has things of the same names with other contents.
 */
#include "tables.h"

/* What this file defines that other files may name, declared as it
   is defined: clang asks for a declaration in front of each such
   object (-Wmissing-variable-declarations). */
extern const u8 StHlpMn_D[];
extern const DIALOG(1) StHlpEr_B;

/* What is not static here, sthelp.c names. */

/* Of the main program, and the same bytes in the setup program: the
   title of the window, of help_t.c, and the button Ok. */
extern const u8 Help_S[];               /* " Help " */
extern const u8 OkMn_D[];

/* The button Ok as a line of a box, and as the window of a page has it
   below its text, which sthelp.c names. The setup program has the two
   among the tables of its dialogs, in VCSETUP.ASM, and its help reads
   both: they are of stopts_t.c. */
extern const u8 StOkMn_S[];             /* " Ok " */

static const u8 StHlEr1_S[] = "Help contents is not available";

/* The button under a page, for MinMnu: its width and its column, up to
   a width of 0. Its colours are the main program's, HlpMn_P. */
const u8 StHlpMn_D[] = {
     6, 20,                             /* Ok */
     0
};

/* The box that says there is no such page. It has three lines, and the
   dialog has its button on the third row: that is the frame of the box,
   and the line " Ok " stands above it as a text. What the box covers is
   kept where the window of a page is kept: BoxBuf of the setup program,
   which the main data has as ErrBuf. */
static const BOX(3) StHlpEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    3, 0,
    MD_AT(ErrBuf),
    { Help_S, StHlEr1_S, StOkMn_S }
};
const DIALOG(1) StHlpEr_B = {
    { &StHlpEr_P, 0x928, 0, 2, 0xFF, 0xFF, 2 },
    { { 4, 0, 0, 0, 0, 0x326, MD_AT(TmpErr), OkMn_D } },
    0
};
