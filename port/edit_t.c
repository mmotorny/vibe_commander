/*
 * The tables of the editor: the texts that port/edit.c reads, and the
 * boxes and dialogs that it hands to DialBox and Dialog by their offsets.
 *
 * From VCEDIT.INC.
 */
#include "tables.h"

/* What others name, port/edit.c here, has the name that it has in the
   assembly. */
#pragma aux Line_S "*"
#pragma aux FrBuf_S "*"
#pragma aux EdtH_S "*"
#pragma aux EdtH1_S "*"
#pragma aux EdtH2_S "*"
#pragma aux EdtH_P "*"
#pragma aux SavFi_P "*"
#pragma aux SavAs_B "*"
#pragma aux SavCn_B "*"
#pragma aux BigFi_B "*"
#pragma aux NewFi_B "*"
#pragma aux SavEr_B "*"

/* Of other files: the title " Edit "; the empty line that an input is
   typed in; "Can't find the file"; and the text of the button Ok with
   the button. Var_S, a line of the text that the box puts in, is of
   tables.h. */
extern const u8 EdFl_S[];
extern const u8 Empty_S[];
extern const u8 NoFnd_S[];
extern const u8 Ok_S[];
extern const u8 OkMn_D[];
#pragma aux EdFl_S "*"
#pragma aux Empty_S "*"
#pragma aux NoFnd_S "*"
#pragma aux Ok_S "*"
#pragma aux OkMn_D "*"

/* The buttons, which stand behind the dialogs that name them. */
static const u8 SavCn_D[7];
static const u8 BigFi_D[5];
static const u8 NewFi_D[5];

/* Of the status line. */
const u8 Line_S[] = "Line";
const u8 FrBuf_S[] = "Free";

/* In the lines of buttons and elsewhere, FFh and a number are that many
   spaces. */
static const u8 BgFi1_S[] = "The file ^V";
static const u8 BgFi2_S[] = "is too large for Edit.";
static const u8 BgFi3_S[] = "View\377\003Cancel";
static const u8 NewFi_S[] = "New-file\377\003Cancel";
static const u8 SvCn1_S[] = "You've made changes since the last save.";
static const u8 SvCn2_S[] = "Save\377\003Don't save\377\003Continue editing";
/* The line is as wide as the name that is typed below it: LenStr less
   the 17 of the text. */
static const u8 SavAs_S[] = "Save this file as\377\057";
static const u8 SavFi_S[] = "Saving the file";
static const u8 SavEr_S[] = "There was an error writing the file";

/* The help that the editor shows below a menu file or an extension
   file: six lines on the right, which are the same for both, and for
   each a title and six lines on the left, one text behind the other.
   \263 is an upright line, \300 a corner and \304 a line across, of
   which FFh with 80h and a number makes that many; \036 is an arrow
   that points up. */
const u8 EdtH_S[] =
    "\263  !.!  Filename with extension\0"
    "\263  !:\377\003Drive letter\0"
    "\263  !\\\377\003Path name\0"
    "\263  !!\377\003!\0"
    "\263  !\377\004Filename w/o extension\0"
    "\263  !@\377\003File list";
const u8 EdtH1_S[] =
    " User Menu Help \0"
    "File format for user defined menus:\0"
    "\0"
    "' comment\377\007Comment line\0"
    "m: Menu Label\377\003Appears in the pop-up menu\0"
    "\377\005edit !.!\377\003Any DOS command\0"
    "\377\005cls\377\010Any additional commands";
const u8 EdtH2_S[] =
    " Extension File Help \0"
    "Format of the extension file:\0"
    "\0"
    "' comment\377\012Comment line\0"
    "txt: edit !.!\377\006Any DOS command\0"
    " \036\377\003cls\377\013Any additional commands\0"
    " \300\377\220\304 File extension";

/* The colour of the help: one, where a line may ask for four. None of
   the help's lines asks for another. */
const u8 EdtH_P[] = { HlpMn_C };

/* What is shown while the file is saved. */
const BOX1(3) SavFi_P = {
    { Box_C, 0, 0, 0 },
    3, 1,
    MD_AT(WinBuf),
    { { MD_AT(ViewFil), 7, 64 } },
    { EdFl_S, SavFi_S, Var_S }
};

/* Save as: the dialog asks for the name. */
static const BOX(3) SavAs_P = {
    { Box_C, BoxCu_C, 0, 0 },
    3, 0,
    MD_AT(WinBuf),
    { EdFl_S, SavAs_S, Empty_S }
};
const DIALOG(1) SavAs_B = {
    { &SavAs_P, 0x528, 0, 1, 0xFF, 0, 0 },
    { { 1, 0, 0, 0, 0, 0x305, MD_AT(ViewFil), NUMBER(0x200 + LenStr) } },
    0
};

/* Leaving a text that has been changed. */
static const BOX(3) SavCn_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    3, 0,
    MD_AT(WinBuf),
    { EdFl_S, SvCn1_S, SvCn2_S }
};
const DIALOG(1) SavCn_B = {
    { &SavCn_P, 0x528, 0, 1, 0xFF, 0, 1 },
    { { 4, 0, 0, 0, 0, 0x315, MD_AT(TmpErr), SavCn_D } },
    0
};

/* A file that is too large for the editor. Its name stands in a line
   behind "The file ", so it may be nine characters shorter than where it
   has a line of its own. */
static const BOX1(4) BigFi_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(ViewFil), 7, 64 - 9 } },
    { EdFl_S, BgFi1_S, BgFi2_S, BgFi3_S }
};
const DIALOG(1) BigFi_B = {
    { &BigFi_P, 0x928, 0, 1, 0xFF, 0, 0 },
    { { 4, 0, 0, 0, 0, 0x420, MD_AT(TmpErr), BigFi_D } },
    0
};

/* A file that is not there: a new one, or not. */
static const BOX1(4) NewFi_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(ViewFil), 7, 64 } },
    { EdFl_S, NoFnd_S, Var_S, NewFi_S }
};
const DIALOG(1) NewFi_B = {
    { &NewFi_P, 0x928, 0, 1, 0xFF, 0, 0 },
    { { 4, 0, 0, 0, 0, 0x41E, MD_AT(TmpErr), NewFi_D } },
    0
};

/* The file could not be written. What the box covers is kept 2,000
   bytes into WinBuf, behind what "Saving the file" has there. */
static const BOX1(4) SavEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(WinBuf[1000]),
    { { MD_AT(ViewFil), 7, 64 } },
    { EdFl_S, SavEr_S, Var_S, Ok_S }
};
const DIALOG(1) SavEr_B = {
    { &SavEr_P, 0x928, 0, 1, 0xFF, 0, 2 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpErr), OkMn_D } },
    0
};

/* The buttons of the three dialogs that have their own: of each its
   width and its column, up to a width of 0. */
static const u8 SavCn_D[7] = {
     6,  0,                             /* Save */
    12,  7,                             /* Don't save */
    18, 20,                             /* Continue editing */
     0
};
static const u8 BigFi_D[5] = {
     6,  0,                             /* View */
     8,  7,                             /* Cancel */
     0
};
static const u8 NewFi_D[5] = {
    10,  0,                             /* New-file */
     8, 11,                             /* Cancel */
     0
};
