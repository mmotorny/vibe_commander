/*
 * The tables of the main file: the program's name, what it says to a DOS
 * that is too old, and the boxes of port/main.c, which are that of DOS's
 * critical error and that of a drive that is not ready. With them the
 * texts and the button that the boxes of every file name: Error, Ok, and
 * the two lines that stand for a text put in. And one word that is no
 * constant: SecDat, where the main data is.
 *
 * From VC.ASM.
 */
#include "tables.h"

/* What others name has the name that it has in the assembly: the code of
   port/main.c, port/start.c, port/resident.c and of those that show the
   title, and the boxes of other files, of the assembly and of C. Title_S
   and Var_S have theirs by port/tables.h, which gives each a near name
   for a file of constants, and no line here. */
#pragma aux Vers_S "*"
#pragma aux SecDat "*"
#pragma aux Err_S "*"
#pragma aux OnDr_S "*"
#pragma aux OnDv_S "*"
#pragma aux Errs_S "*"
#pragma aux FatEr_S "*"
#pragma aux Ok_S "*"
#pragma aux Inc_S "*"
#pragma aux FtEr1_B "*"
#pragma aux FtEr2_B "*"
#pragma aux OkMn_D "*"
#pragma aux NoRd_P "*"

/* The title: of the help's window as it is, with the two bytes that set
   its colour, and without them in the panel of the Info and in the
   banner. */
const u8 Title_S[] = "\377\016The Volkov Commander, Version 4.05";
const u8 Vers_S[] = "\r\nThis program requires DOS 3.20 or later.\r\n";

/* The segment of the main data, which the program asks DOS for when it
   starts and after every command: port/start.c writes it here. FatErr of
   port/main.c, which DOS calls with DS anywhere, finds the main data by
   it: an interrupt function has DS on this group when it begins, and on
   nothing else that the program could keep a word in. It is the one
   variable of the group, with a value, 0, for the compiler to put it
   where the file has contents; tools/mkexe.c allows one in a file like
   this, which has no code that would read it through DS on the main
   data. */
u16 SecDat = 0;

/* The critical error of DOS. The title, of which the box of every error
   is headed, and what goes in front of the text: the drive, whose letter
   is counted up from A, or the device, whose name is put behind the ten
   bytes, which end in no zero. */
const u8 Err_S[] = " Error ";
const u8 OnDr_S[] = "on drive A";
const u8 OnDv_S[10] = "on device ";

/* What is wrong, by the number that DOS gives: the last is for every
   number that has no text. */
const u8 Errs_S[] =
    "Attempt to write on write-protected disk.\0"
    "Unknown disk drive.\0"
    "Drive or device not ready.\0"
    "Unknown command.\0"
    "Data error.\0"
    "Bad request.\0"
    "Seek error.\0"
    "Non-DOS disk.\0"
    "Sector not found.\0"
    "Printer out of paper.\0"
    "Write fault.\0"
    "Read fault.\0"
    "General failure.\0"
    "Sharing violation.\0"
    "Lock violation.\0"
    "Invalid disk change.\0"
    "FCB unavailable.\0"
    "Sharing buffer overflow.\0"
    "Code page mismatch.\0"
    "Out of input.\0"
    "Insufficient disk space.\0"
    "Unknown error.";
const u8 FatEr_S[] = "File allocation table bad.";
static const u8 RtAb_S[] = " Retry   Abort ";

/* The button Ok, and the lines of a box that are a text put in: at the
   address that the box has, and at the one that is handed over with the
   box. */
const u8 Ok_S[] = " Ok ";
const u8 Var_S[] = "^V";
const u8 Inc_S[] = "^I";

/* The buttons Retry and Abort: of each its width and its column, up to a
   width of 0. That of Ok follows the dialogs. */
static const u8 RtAb_D[] = {
    7, 0,                               /* Retry */
    7, 8,                               /* Abort */
    0
};
extern const u8 OkMn_D[];

/* The box of a critical error, where DOS lets the call be tried again
   and where it does not: the drive or the device, from TmpErr, and under
   it the text of the error, which is handed over. */
static const BOX1(4) FtEr1_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf1),
    { { MD_AT(TmpErr[1]), 0, 70 } },
    { Err_S, Var_S, Inc_S, RtAb_S }
};
static const BOX1(4) FtEr2_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf1),
    { { MD_AT(TmpErr[1]), 0, 70 } },
    { Err_S, Var_S, Inc_S, Ok_S }
};
const DIALOG(1) FtEr1_B = {
    { &FtEr1_P, 0x928, 0, 2, 0xFF, 0xFF, 7 },
    { { 4, 0, 0, 0, 0, 0x420, MD_AT(TmpErr), RtAb_D } },
    0
};
const DIALOG(1) FtEr2_B = {
    { &FtEr2_P, 0x928, 0, 2, 0xFF, 0xFF, 7 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpErr), OkMn_D } },
    0
};
const u8 OkMn_D[] = {
    4, 0,                               /* Ok */
    0
};

/* A drive that is not ready: the box asks for another letter, and has
   that of the drive from TmpErr. */
static const u8 NoRd1_S[] = "Can't read the disk in drive ^V:";
static const u8 NoRd2_S[] = "Press ENTER to try again, ESC to abort,";
static const u8 NoRd3_S[] = "or enter a different drive letter here ^r^V^r:";
const BOX1(4) NoRd_P = {
    { Err_C, ErInv_C, 0, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(TmpErr[1]), 0, 55 } },
    { Err_S, NoRd1_S, NoRd2_S, NoRd3_S }
};
