/*
 * The tables of src/c/dirfunc.c: the texts, the boxes, the dialogs and the
 * buttons of making a directory, of deleting, of the attributes and of
 * the volume label, and of going through a directory. The code hands them
 * to Dialog and OncZoom by their offsets.
 *
 * From VCF1.INC.
 */
#include "tables.h"

/* What others name has the name that it has in the assembly: src/c/copy.c
   names MkErr_B, boxes of other files Var2_S and NoFnd_S, and
   src/c/dirfunc.c the rest. */
#pragma aux MkDir_S "*"
#pragma aux CrDir_S "*"
#pragma aux MkDi1_B "*"
#pragma aux MkDi2_B "*"
#pragma aux MkErr_B "*"
#pragma aux Del_S "*"
#pragma aux DelFs_S "*"
#pragma aux Subdi_S "*"
#pragma aux DlWr4_S "*"
#pragma aux Var2_S "*"
#pragma aux DelFl_P "*"
#pragma aux DelDr_P "*"
#pragma aux DlMas_B "*"
#pragma aux DelSe_B "*"
#pragma aux DelCo_B "*"
#pragma aux DelFi_B "*"
#pragma aux DelDi_B "*"
#pragma aux RdOnl_B "*"
#pragma aux NoEmp_B "*"
#pragma aux DlWrn_B "*"
#pragma aux DelEr_B "*"
#pragma aux CntDi_B "*"
#pragma aux Attr8_S "*"
#pragma aux NoFnd_S "*"
#pragma aux NoAtr_S "*"
#pragma aux StAt1_B "*"
#pragma aux StAt2_B "*"
#pragma aux StAtr_P "*"
#pragma aux NoDat_B "*"
#pragma aux AtrEr_B "*"
#pragma aux Label_B "*"
#pragma aux DelVo_B "*"
#pragma aux ErVol_B "*"
#pragma aux PthEr_B "*"
#pragma aux NoFnc_B "*"

/* Of other files: lines and buttons that the boxes of every file use.
   Var_S, the line that is a text which the box puts in, is another, and
   src/c/tables.h has it. */
extern const u8 Ok_S[];
extern const u8 OkMn_D[];
extern const u8 Inc_S[];
extern const u8 Err_S[];
extern const u8 Empty_S[];
extern const u8 Kov_S[];
extern const u8 OkCan_S[];
extern const u8 OkCan_D[];
#pragma aux Ok_S "*"
#pragma aux OkMn_D "*"
#pragma aux Inc_S "*"
#pragma aux Err_S "*"
#pragma aux Empty_S "*"
#pragma aux Kov_S "*"
#pragma aux OkCan_S "*"
#pragma aux OkCan_D "*"

/* A box to tick, by the byte of TmpBuf that has its value: where that is
   while the dialog is up, and where it is taken from and left, which is
   the same place for the dialogs here. */
#define TICK(at) MD_AT(TmpBuf[at]), NUMBER(MD_AT(TmpBuf[at]))

/* Make directory: a path is typed, or in a panel that shows the tree a
   name, of twelve characters; and the directory that could not be made. */
const u8 MkDir_S[] = " Make directory ";
static const u8 MkDi1_S[] = "^I\377\054";
const u8 CrDir_S[] = "Create the directory";
static const u8 MkErr_S[] = "Can't create the directory";

static const BOX(3) MkDi1_P = {
    { Box_C, BoxCu_C, 0, 0 },
    3, 0,
    MD_AT(WinBuf),
    { MkDir_S, MkDi1_S, Empty_S }
};
const DIALOG(1) MkDi1_B = {
    { &MkDi1_P, 0x528, 0, 1, 2, 22, 0 },
    { { 1, 0, 0, 0, 0, 0x305, MD_AT(TmpBuf), NUMBER(0x200 + LenStr) } },
    0
};

static const BOX(3) MkDi2_P = {
    { Box_C, BoxCu_C, 0, 0 },
    3, 0,
    MD_AT(WinBuf),
    { MkDir_S, Inc_S, Empty_S }
};
const DIALOG(1) MkDi2_B = {
    { &MkDi2_P, 0x528, 0, 1, 2, 22, 0 },
    { { 1, 0, 0, 0, 0, 0x305, MD_AT(TmpBuf), NUMBER(0x20C) } },
    0
};

static const BOX1(4) MkErr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf[130]), 6, 64 } },
    { Inc_S, MkErr_S, Var_S, Ok_S }
};
const DIALOG(1) MkErr_B = {
    { &MkErr_P, 0x928, 0, 1, 0xFF, 0, 6 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpBuf[200]), OkMn_D } },
    0
};

/* Delete. DlWr4_S is copied with what follows it, 81 bytes in all, by
   F48_00 of src/c/dirfunc.c, and so it is written as long as that, with
   the 56 bytes that the original has behind it: the four texts that
   follow, which are constants of their own besides, and seven bytes of
   the fifth. */
const u8 Del_S[] = " Delete ";
static const u8 DlMas_S[] = "Delete the file:\377\060";
static const u8 DelFl_S[] = "Deleting the file^I";
const u8 DelFs_S[] = "s";
static const u8 DelDr_S[] = "Deleting the subdirectory";
static const u8 DelSl_S[] = "You have selected";
static const u8 Del1_S[] = "Do you wish to delete^I";
const u8 Subdi_S[] = " the directory";
static const u8 RdOn_S[] = "The following file is marked read-only.";
static const u8 NoEmp_S[] = "The following subdirectory is not empty.";
static const u8 StDel_S[] = "Do you still wish to delete it?";
static const u8 CnDl_S[] = "Can't delete the file";
static const u8 CnSb_S[] = "Can't delete the subdirectory";
static const u8 DlWr1_S[] = "WARNING!";
static const u8 DlWr2_S[] = "Do you want to delete all the files";
static const u8 DlWr3_S[] = "^V?";
const u8 DlWr4_S[81] =
    "in the current directory\0" "You are ^uDELETING^u\0" "^V from\0" "^W\0"
    " Delete\377\003Cancel \0" " Delete";
static const u8 DlSel_S[] = "You are ^uDELETING^u";
static const u8 From_S[] = "^V from";
const u8 Var2_S[] = "^W";
static const u8 DelCa_S[] = " Delete\377\003Cancel ";
static const u8 DelAl_S[] = " Delete\377\003All\377\003Cancel ";
static const u8 DelSk_S[] = "Delete\377\003Skip\377\003Cancel";

/* The buttons: of each its width and its column, up to a width of 0. */
static const u8 DelCa_D[] = {
     8,  0,                             /* Delete */
     8,  9,                             /* Cancel */
     0
};
static const u8 DelAl_D[] = {
     8,  0,                             /* Delete */
     5,  9,                             /* All */
     8, 15,                             /* Cancel */
     0
};
static const u8 DelSk_D[] = {
     8,  0,                             /* Delete */
     6,  9,                             /* Skip */
     8, 16,                             /* Cancel */
     0
};

/* What is being deleted, a file or a subdirectory. */
const BOX1(3) DelFl_P = {
    { Box_C, 0, 0, 0 },
    3, 1,
    MD_AT(WinBuf),
    { { MD_AT(TmpBuf), 7, 64 } },
    { Del_S, DelFl_S, Var_S }
};
const BOX1(3) DelDr_P = {
    { Box_C, 0, 0, 0 },
    3, 1,
    MD_AT(WinBuf),
    { { MD_AT(TmpBuf), 6, 64 } },
    { Inc_S, DelDr_S, Var_S }
};

/* Shift-F8: which files. */
static const BOX(3) DlMas_P = {
    { Box_C, BoxCu_C, 0, 0 },
    3, 0,
    MD_AT(WinBuf),
    { Del_S, DlMas_S, Empty_S }
};
const DIALOG(1) DlMas_B = {
    { &DlMas_P, 0x528, 0, 1, 2, 23, 0 },
    { { 1, 0, 0, 0, 0, 0x305, MD_AT(ViewFil), NUMBER(LenStr) } },
    0
};

/* The files that are selected, and once more, as a warning. Enter and
   F10 in the first go through F38_10. */
static const BOX1(4) DelSe_P = {
    { Box_C, BoxCu_C, BoxCu_C, 0 },
    4, 1,
    MD_AT(WinBuf),
    { { MD_AT(TmpBuf), 0, 64 } },
    { Del_S, DelSl_S, Var_S, DelCa_S }
};
const DIALOG(1) DelSe_B = {
    { &DelSe_P, 0x528, SUB_F38_10, 1, 2, 23, 1 },
    { { 4, 0, 0, 0, 0, 0x41F, MD_AT(TmpBuf[200]), DelCa_D } },
    0
};
static const BOX2(5) DelCo_P = {
    { Err_C, ErInv_C, ErInv_C, ErBr_C },
    5, 2,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf), 0, 64 }, { MD_AT(TmpBuf[100]), 6, 64 } },
    { Del_S, DlSel_S, From_S, Var2_S, DelAl_S }
};
const DIALOG(1) DelCo_B = {
    { &DelCo_P, 0x928, 0, 1, 0xFF, 0, 1 },
    { { 4, 0, 0, 0, 0, 0x51C, MD_AT(TmpBuf[100]), DelAl_D } },
    0
};

/* The file or the directory under the cursor. */
static const BOX1(4) DelFi_P = {
    { Box_C, BoxCu_C, BoxCu_C, 0 },
    4, 1,
    MD_AT(WinBuf),
    { { MD_AT(TmpBuf), 7, 64 } },
    { Del_S, Del1_S, Var_S, DelCa_S }
};
const DIALOG(1) DelFi_B = {
    { &DelFi_P, 0x528, 0, 1, 2, 23, 1 },
    { { 4, 0, 0, 0, 0, 0x41F, MD_AT(TmpBuf[100]), DelCa_D } },
    0
};
static const BOX1(4) DelDi_P = {
    { Box_C, BoxCu_C, BoxCu_C, 0 },
    4, 1,
    MD_AT(WinBuf),
    { { MD_AT(TmpBuf), 6, 62 } },
    { Del_S, Del1_S, Kov_S, DelCa_S }
};
const DIALOG(1) DelDi_B = {
    { &DelDi_P, 0x528, 0, 1, 2, 23, 1 },
    { { 4, 0, 0, 0, 0, 0x41F, MD_AT(TmpBuf[100]), DelCa_D } },
    0
};

/* A file that is read-only, and a subdirectory with something in it. */
static const BOX1(5) RdOnl_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    5, 1,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf), 7, 64 } },
    { Inc_S, RdOn_S, Var_S, StDel_S, DelSk_S }
};
const DIALOG(1) RdOnl_B = {
    { &RdOnl_P, 0x928, 0, 1, 0xFF, 0, 5 },
    { { 4, 0, 0, 0, 0, 0x51C, MD_AT(TmpBuf[230]), DelSk_D } },
    0
};
static const BOX1(5) NoEmp_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    5, 1,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf), 6, 64 } },
    { Del_S, NoEmp_S, Var_S, StDel_S, DelAl_S }
};
const DIALOG(1) NoEmp_B = {
    { &NoEmp_P, 0x928, 0, 1, 0xFF, 0, 1 },
    { { 4, 0, 0, 0, 0, 0x51C, MD_AT(TmpBuf[100]), DelAl_D } },
    0
};

/* Shift-F8 with a mask that takes every file. */
static const BOX1(5) DlWrn_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    5, 1,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf[100]), 4, 64 } },
    { Del_S, DlWr1_S, DlWr2_S, DlWr3_S, DelCa_S }
};
const DIALOG(1) DlWrn_B = {
    { &DlWrn_P, 0x528, 0, 1, 0xFF, 0, 1 },
    { { 4, 0, 0, 0, 0, 0x51F, MD_AT(TmpBuf[100]), DelCa_D } },
    0
};

/* What could not be deleted. */
static const BOX1(4) DelEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf[130]), 7, 64 } },
    { Inc_S, CnDl_S, Var_S, OkCan_S }
};
const DIALOG(1) DelEr_B = {
    { &DelEr_P, 0x928, 0, 1, 0xFF, 0, 6 },
    { { 4, 0, 0, 0, 0, 0x422, MD_AT(TmpBuf[230]), OkCan_D } },
    0
};
static const BOX1(4) CntDi_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf), 6, 64 } },
    { Inc_S, CnSb_S, Var_S, OkCan_S }
};
const DIALOG(1) CntDi_B = {
    { &CntDi_P, 0x928, 0, 1, 0xFF, 0, 2 },
    { { 4, 0, 0, 0, 0, 0x422, MD_AT(TmpBuf[100]), OkCan_D } },
    0
};

/* Attributes. Of the lines that have two boxes to tick, the dialog for
   one file shows what is behind the first box. */
static const u8 Attr_S[] = " Attributes ";
static const u8 Attr1_S[] = "Change file attributes^I";
static const u8 Attr2_S[] = "^uSet  Clear^u\377\025";
static const u8 Attr3_S[] = " [ ]  [ ] Read only\377\004Date\377\006";
static const u8 Attr4_S[] = " [ ]  [ ] Archive\377\020";
static const u8 Attr5_S[] = " [ ]  [ ] Hidden\377\007Time\377\006";
static const u8 Attr6_S[] = " [ ]  [ ] System\377\021";
static const u8 Attr7_S[] = "[ Set ]   [ Cancel ]";
const u8 Attr8_S[] = " for";
static const u8 StAtr_S[] = "Setting attributes:";
static const u8 NoDt1_S[] = "Can't set the attributes.";
static const u8 NoDt2_S[] = "Illegal date or time.";
const u8 NoFnd_S[] = "Can't find the file";
const u8 NoAtr_S[] = "Can't set attributes";

static const u8 SetCa_D[] = {
     7,  0,                             /* Set */
    10, 10,                             /* Cancel */
     0
};

/* One file: its four attributes, read only, archive, hidden and system,
   its date and its time, and the buttons. Enter and F10 go through
   M30_60, which looks at the date and the time. */
static const BOX1(9) StAt1_P = {
    { Box_C, BoxCu_C, Box_C, BoxBr_C },
    9, 1,
    MD_AT(WinBuf),
    { { MD_AT(TmpBuf[100]), 7, 62 } },
    { Attr_S, Attr1_S, Kov_S, &Attr3_S[5], &Attr4_S[5], &Attr5_S[5],
      &Attr6_S[5], 0, Attr7_S }
};
const DIALOG(7) StAt1_B = {
    { &StAt1_P, 0x428, SUB_M30_60, 1, 2, 24, 0 },
    {
        { 3, 6, 1, 0, 4, 0x407, TICK(0) },
        { 3, 0, 2, 1, 4, 0x507, TICK(1) },
        { 3, 1, 3, 2, 5, 0x607, TICK(2) },
        { 3, 2, 6, 3, 5, 0x707, TICK(3) },
        { 1, 3, 5, 1, 4, 0x517, MD_AT(TmpBuf[15]), NUMBER(0xA) },
        { 1, 4, 6, 3, 5, 0x717, MD_AT(TmpBuf[30]), NUMBER(0x10A) },
        { 4, 5, 0, 6, 6, 0x91E, MD_AT(TmpBuf[8]), SetCa_D }
    },
    0
};

/* A directory, or what is selected: the four attributes to set, the four
   to clear, a date and a time, and the buttons. */
static const BOX(9) StAt2_P = {
    { Box_C, BoxCu_C, Box_C, BoxBr_C },
    9, 0,
    MD_AT(WinBuf),
    { Attr_S, Attr1_S, Attr2_S, Attr3_S, Attr4_S, Attr5_S, Attr6_S, 0,
      Attr7_S }
};
const DIALOG(11) StAt2_B = {
    { &StAt2_P, 0x428, SUB_M30_60, 1, 2, 24, 0 },
    {
        { 3, 10, 1, 0, 4, 0x407, TICK(0) },
        { 3, 0, 2, 1, 5, 0x507, TICK(1) },
        { 3, 1, 3, 2, 6, 0x607, TICK(2) },
        { 3, 2, 10, 3, 7, 0x707, TICK(3) },
        { 3, 3, 5, 0, 8, 0x40C, TICK(10) },
        { 3, 4, 6, 1, 8, 0x50C, TICK(11) },
        { 3, 5, 7, 2, 9, 0x60C, TICK(12) },
        { 3, 6, 10, 3, 9, 0x70C, TICK(13) },
        { 1, 7, 9, 5, 8, 0x51C, MD_AT(TmpBuf[15]), NUMBER(0xA) },
        { 1, 8, 10, 7, 9, 0x71C, MD_AT(TmpBuf[30]), NUMBER(0x10A) },
        { 4, 9, 0, 10, 10, 0x91E, MD_AT(TmpBuf[8]), SetCa_D }
    },
    0
};

/* The file whose attributes are being set, and what went wrong. */
const BOX1(3) StAtr_P = {
    { Box_C, 0, 0, 0 },
    3, 1,
    MD_AT(WinBuf),
    { { MD_AT(TmpBuf), 7, 64 } },
    { Attr_S, StAtr_S, Var_S }
};
static const BOX(4) NoDat_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 0,
    MD_AT(ErrBuf),
    { Attr_S, NoDt1_S, NoDt2_S, Ok_S }
};
const DIALOG(1) NoDat_B = {
    { &NoDat_P, 0xC28, 0, 1, 0xFF, 0, 0 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpBuf[100]), OkMn_D } },
    0
};
static const BOX1(4) AtrEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf), 7, 64 } },
    { Attr_S, Inc_S, Var_S, OkCan_S }
};
const DIALOG(1) AtrEr_B = {
    { &AtrEr_P, 0x928, 0, 1, 0xFF, 0, 2 },
    { { 4, 0, 0, 0, 0, 0x422, MD_AT(TmpBuf[100]), OkCan_D } },
    0
};

/* The volume label: one to type, deleting the one that is there, and
   what could not be set. */
static const u8 Lab0_S[] = " Volume Label ";
static const u8 Lab1_S[] = "Change volume label^V to";
static const u8 Lab2_S[] = "Do you wish to delete volume label";
static const u8 Lab3_S[] = "^V ?";
static const u8 ErVol_S[] = "Could not set the volume label";

static const BOX1(3) Label_P = {
    { Box_C, BoxCu_C, 0, 0 },
    3, 1,
    MD_AT(WinBuf),
    { { MD_AT(TmpBuf[20]), 0, 30 } },
    { Lab0_S, Lab1_S, Empty_S }
};
const DIALOG(1) Label_B = {
    { &Label_P, 0x528, 0, 1, 3, 29, 0 },
    { { 1, 0, 0, 0, 0, 0x305, MD_AT(TmpBuf[1]), NUMBER(0x20B) } },
    0
};
static const BOX1(4) DelVo_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(Volume), 0, 15 } },
    { Lab0_S, Lab2_S, Lab3_S, DelCa_S }
};
const DIALOG(1) DelVo_B = {
    { &DelVo_P, 0x728, 0, 1, 0xFF, 0, 5 },
    { { 4, 0, 0, 0, 0, 0x41F, MD_AT(TmpBuf[100]), DelCa_D } },
    0
};
static const BOX1(4) ErVol_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf[1]), 6, 15 } },
    { Lab0_S, ErVol_S, Var_S, Ok_S }
};
const DIALOG(1) ErVol_B = {
    { &ErVol_P, 0x728, 0, 1, 0xFF, 0, 2 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpBuf[100]), OkMn_D } },
    0
};

/* Going through a directory: a path that got too long. */
static const u8 PthEr_S[] = "Path too long";
static const BOX1(4) PthEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf), 6, 64 } },
    { Err_S, PthEr_S, Var_S, Ok_S }
};
const DIALOG(1) PthEr_B = {
    { &PthEr_P, 0x528, 0, 1, 0xFF, 0, 6 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpBuf[200]), OkMn_D } },
    0
};

/* A mask that no file has. */
static const u8 NoFnc_S[] = "Can't find file^W";
static const BOX2(4) NoFnc_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 2,
    MD_AT(ErrBuf),
    { { MD_AT(TmpBuf), 7, 64 }, { MD_AT(TmpBuf[100]), 0, 50 } },
    { Inc_S, NoFnc_S, Var_S, Ok_S }
};
const DIALOG(1) NoFnc_B = {
    { &NoFnc_P, 0x928, 0, 1, 0xFF, 0, 2 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpBuf[100]), OkMn_D } },
    0
};
