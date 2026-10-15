/*
 * The tables of the keys of the panels: the dialog that asks for a mask
 * to select by and the box that says no file is like it, of keys.c;
 * the dialog that asks whether to quit, of enter.c; and what
 * usermenu.c has of the user menu, the names of the menu file and of
 * the list file, the title of the menu and the box that says no menu file
 * was found; and the keys that begin the speed search, which the main
 * loop looks for. The texts that many boxes name are here too: an empty
 * line, a name in quotes, and the buttons Yes and No.
 *
 * From VCFUNC.INC.
 */
#include "tables.h"

/* What this file defines that other files may name, declared as it
   is defined: clang asks for a declaration in front of each such
   object (-Wmissing-variable-declarations). */
extern const u8 Sel_S[];
extern const u8 Unsl_S[];
extern const u8 Inv_S[];
extern const u8 Kov_S[];
extern const u8 Empty_S[];
extern const DIALOG(1) Sel_B;
extern const DIALOG(1) ErSel_B;
extern const u8 Name_S[];
extern const u8 YesNo_S[];
extern const u8 YesNo_D[];
extern const DIALOG(1) Quit_B;
extern const u8 Menu_F[];
extern const u8 User_S[];
extern const DIALOG(1) UsrEr_B;
extern const u8 AltSr_T[];
extern const u8 List_F[];

/* What is not static here, others name: the code, and the boxes of other
   files. */

/* Of another file: the text of the button Ok, and the button. The line
   that a box puts its text into, Var_S, is in tables.h. */
extern const u8 Ok_S[];
extern const u8 OkMn_D[];

#define PrgName "Vibe Commander"

/* Selecting by a mask: what ^I stands for in the two boxes, their lines,
   and a name in quotes, which is read as far as the zero byte that is
   Empty_S in the assembly. */
const u8 Sel_S[] = "Select";
const u8 Unsl_S[] = "Unselect";
const u8 Inv_S[] = "Invert";
static const u8 IncSp_S[] = " ^I ";
static const u8 Sel1_S[] = "^I the files";
static const u8 ErSel_S[] = "Could not find a match";
const u8 Kov_S[] = "\"^V\"";
const u8 Empty_S[] = "";

static const BOX(3) Sel_P = {
    { Box_C, BoxCu_C, 0, 0 },
    3, 0,
    MD_AT(WinBuf),
    { IncSp_S, Sel1_S, Empty_S }
};
const DIALOG(1) Sel_B = {
    { &Sel_P, 0x528, 0, 1, 2, 25, 0 },
    { { 1, 0, 0, 0, 0, 0x305, MD_AT(SelMas), NUMBER(0x20C) } },
    0
};
static const BOX1(4) ErSel_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(WinBuf),
    { { MD_AT(SelMas), 4, 50 } },
    { IncSp_S, ErSel_S, Kov_S, Ok_S }
};
const DIALOG(1) ErSel_B = {
    { &ErSel_P, 0x528, 0, 1, 0xFF, 0, 0 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpBuf), OkMn_D } },
    0
};

/* F10: the name of the program, for ^I, and the buttons Yes and No, of
   each its width and its column. */
const u8 Name_S[] = PrgName;
static const u8 Quit1_S[] = " The ^I ";
static const u8 Quit2_S[] = "Do you want to quit the ^I?";
const u8 YesNo_S[] = "Yes\377\003No";
const u8 YesNo_D[] = {
    5, 0,                               /* Yes */
    4, 6,                               /* No */
    0
};
static const BOX(3) Quit_P = {
    { Box_C, BoxCu_C, BoxCu_C, 0 },
    3, 0,
    MD_AT(WinBuf),
    { Quit1_S, Quit2_S, YesNo_S }
};
const DIALOG(1) Quit_B = {
    { &Quit_P, 0x528, 0, 1, 2, 26, 0 },
    { { 4, 0, 0, 0, 0, 0x323, MD_AT(TmpBuf), YesNo_D } },
    0
};

/* The user menu. The name of the menu file is copied 13 bytes long, as
   MainFil copies any name, so it is written with what follows it in the
   assembly, the beginning of the title. */
const u8 Menu_F[13] NONSTRING = "VC.MNU\000 User ";
/* Menu file edit copies the title as 80 bytes (M64_02 of
   commands.c), so it is written with the 68 that follow it in the
   original: the line of the box below, the texts of the box of the
   drives, and the beginning of the history's title. */
const u8 User_S[80] NONSTRING =
    " User Menu \0"
    "Could not find the menu file\0"
    " Drive letter Choose leftright drive: H";
static const u8 User1_S[] = "Could not find the menu file";
static const BOX1(4) UsrEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf), 7, 64 } },
    { User_S, User1_S, Var_S, Ok_S }
};
const DIALOG(1) UsrEr_B = {
    { &UsrEr_P, 0x528, 0, 1, 0xFF, 0, 2 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpBuf), OkMn_D } },
    0
};

/* The scan codes of Alt with a digit or a letter, which begin the speed
   search: the main loop looks for a key here. The search itself, in
   cmdline.c, has them too. */
const u8 AltSr_T[38] = {
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
    0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32
};

/* The name of the list file, which is copied 13 bytes long too. What
   follows it in the original is the first line of the menus, MenuLn1. */
const u8 List_F[13] NONSTRING = "vc.lst\000 Brief";
