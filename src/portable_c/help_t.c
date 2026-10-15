/*
 * The tables of the help: what help.c hands to Dialog and MinMnu by
 * its offset, and the title of its window.
 *
 * From VCHELP.INC.
 */
#include "tables.h"

/* What this file defines that other files may name, declared as it
   is defined: clang asks for a declaration in front of each such
   object (-Wmissing-variable-declarations). */
extern const u8 Help_S[];
extern const u8 HlpMn_P[];
extern const u8 HlMn1_D[];
extern const u8 HlMn2_D[];
extern const DIALOG(1) HlpEr_B;

/* What is not static here, help.c names. */

/* Of another file: the text of the button Ok, and the button. */
extern const u8 Ok_S[];
extern const u8 OkMn_D[];

const u8 Help_S[] = " Help ";
static const u8 HlEr1_S[] = "There was error reading data";
static const u8 HlEr2_S[] = "from the help file.";

/* The colours of the buttons, and the buttons under the index and under
   a page: of each its width and its column, up to a width of 0. */
const u8 HlpMn_P[] = { Help_C, HlpRe_C, HlpRe_C };
const u8 HlMn1_D[] = {
     8, 14,                             /* Help */
    10, 25,                             /* Cancel */
     0
};
const u8 HlMn2_D[] = {
     8,  0,                             /* Next */
    12, 11,                             /* Previous */
     9, 26,                             /* Index */
    10, 38,                             /* Cancel */
     0
};

/* The box that says the help file could not be read. What it covers is
   kept behind the 32 bytes that the help has in front of a page: HlpInfo
   of struct HlpData in help.c. */
static const BOX(4) HlpEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 0,
    MD_AT(HelpBuf[32]),
    { Help_S, HlEr1_S, HlEr2_S, Ok_S }
};
const DIALOG(1) HlpEr_B = {
    { &HlpEr_P, 0x928, 0, 2, 0xFF, 0xFF, 2 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpErr), OkMn_D } },
    0
};
