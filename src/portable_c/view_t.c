/*
 * The tables of the viewer and of the commands that view or edit a file:
 * the texts that src/c/view.c and src/c/edit.c read, and the boxes and
 * dialogs that they hand to DialBox and Dialog by their offsets.
 *
 * From VCVIEW.INC.
 */
#include "tables.h"

/* What is not static here, others name: the code, and the boxes of other
   files. */

/* Of other files: the title " Error "; ^I, a line that the caller gives,
   the title as a rule; ^V in quotation marks, a line of the text that
   the box puts in; the empty line that an input is typed in; "Can't
   find the file"; and the text of the button Ok with the button. Var_S,
   ^V without the quotation marks, is of tables.h. */
extern const u8 Err_S[];
extern const u8 Inc_S[];
extern const u8 Empty_S[];
extern const u8 Kov_S[];
extern const u8 NoFnd_S[];
extern const u8 Ok_S[];
extern const u8 OkMn_D[];

/* The names of the extension files of the viewer and of the editor. The
   commands copy 13 bytes of a name, which has 11: the name and the two
   bytes that follow it in the original, the start of the next text. So
   each is written as long as what is copied. */
const u8 ViewE_F[13] = "VCVIEW.EXT\0VC";
const u8 EditE_F[13] = "VCEDIT.EXT\0Co";

const u8 Col_S[] = "Col";
const u8 ViFl_S[] = " View ";
static const u8 ViFl1_S[] = "Loading the viewer:";
const u8 EdFl_S[] = " Edit ";
static const u8 EdFl1_S[] = "Loading the editor:";
static const u8 ViFi0_S[] = " ^I";
/* The line is as wide as the name that is typed below it: FFh and a
   number are that many spaces, here LenStr less the 14 of the text. */
static const u8 ViFi1_S[] = "^Ithe file:\377\062";
static const u8 RdFil_S[] = "Reading the file";
static const u8 Srch_S[] = "Search for the string\377\053";    /* LenStr - 21 */
static const u8 SrFor_S[] = "Searching for";
static const u8 SrEr_S[] = "Could not find the string";
const u8 RunEr_S[] = "Not enough memory to load";
const u8 Run1_S[] = "Can't run commands for some reason.";
/* The command copies 80 bytes of this text, which has 15, for the box to
   put in: the text and the 65 bytes that follow it in the original,
   which are the first texts of the editor, the last cut short. So it is
   written as long as what is copied. */
const u8 Run2_S[80] =
    "Try rebooting.\0"
    "Line\0"
    "Free\0"
    "The file ^V\0"
    "is too large for Edit.\0"
    "View\377\003Cancel\0"
    "New-fil";

/* A program that an extension file names as the viewer or the editor
   could not be run: the box puts in the text that the command has at
   TmpBuf. */
static const BOX1(4) RunEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf), 0, 50 } },
    { Err_S, Inc_S, Var_S, Ok_S }
};
const DIALOG(1) RunEr_B = {
    { &RunEr_P, 0x928, 0, 2, 0xFF, 0xFF, 2 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpBuf), OkMn_D } },
    0
};

/* View... and Edit... of the menu ask for the file. */
static const BOX(3) ViFil_P = {
    { Box_C, BoxCu_C, 0, 0 },
    3, 0,
    MD_AT(WinBuf),
    { ViFi0_S, ViFi1_S, Empty_S }
};
const DIALOG(1) ViFil_B = {
    { &ViFil_P, 0x528, 0, 1, 2, 0, 0 },
    { { 1, 0, 0, 0, 0, 0x305, MD_AT(ViewFil), NUMBER(0x200 + LenStr) } },
    0
};

/* What is shown while a program is loaded as the viewer or as the
   editor, and while the file is read. */
const BOX1(3) Viewr_P = {
    { Box_C, 0, 0, 0 },
    3, 1,
    MD_AT(WinBuf),
    { { MD_AT(TmpBuf), 7, 64 } },
    { ViFl_S, ViFl1_S, Var_S }
};
const BOX1(3) Editr_P = {
    { Box_C, 0, 0, 0 },
    3, 1,
    MD_AT(WinBuf),
    { { MD_AT(TmpBuf), 7, 64 } },
    { EdFl_S, EdFl1_S, Var_S }
};
const BOX1(3) RdFil_P = {
    { Box_C, 0, 0, 0 },
    3, 1,
    MD_AT(WinBuf),
    { { MD_AT(ViewFil), 7, 64 } },
    { Inc_S, RdFil_S, Var_S }
};

/* The search: the dialog that asks for the text, and what is shown
   while it is looked for. */
static const BOX(3) Srch_P = {
    { Box_C, BoxCu_C, 0, 0 },
    3, 0,
    MD_AT(WinBuf),
    { Inc_S, Srch_S, Empty_S }
};
const DIALOG(1) Srch_B = {
    { &Srch_P, 0x528, 0, 1, 0xFF, 0, 0 },
    { { 1, 0, 0, 0, 0, 0x305, MD_AT(SrchStr), NUMBER(LenStr) } },
    0
};
const BOX1(3) SrFor_P = {
    { Box_C, 0, 0, 0 },
    3, 1,
    MD_AT(WinBuf),
    { { MD_AT(SrchStr), 0, LenStr } },
    { Inc_S, SrFor_S, Kov_S }
};

/* The file to view or to edit cannot be opened. */
static const BOX1(4) FndEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(ViewFil), 7, 64 } },
    { Inc_S, NoFnd_S, Var_S, Ok_S }
};
const DIALOG(1) FndEr_B = {
    { &FndEr_P, 0x928, 0, 1, 0xFF, 0, 2 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpBuf), OkMn_D } },
    0
};

/* The text that was looked for is not there. What the box covers is
   kept 2,000 bytes into WinBuf, behind what the box of the search has
   there. */
static const BOX1(4) SrEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(WinBuf[1000]),
    { { MD_AT(SrchStr), 0, LenStr } },
    { Inc_S, SrEr_S, Kov_S, Ok_S }
};
const DIALOG(1) SrEr_B = {
    { &SrEr_P, 0x928, 0, 1, 0xFF, 0, 0 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpBuf), OkMn_D } },
    0
};
