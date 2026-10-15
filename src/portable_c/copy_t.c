/*
 * The tables of copying and renaming: the texts, boxes and dialogs that
 * copy.c hands to OncZoom and Dialog by their offsets, and the texts
 * that it copies words from.
 *
 * From VCCOPY.INC.
 */
#include "tables.h"

/* What this file defines that other files may name, declared as it
   is defined: clang asks for a declaration in front of each such
   object (-Wmissing-variable-declarations). */
extern const u8 Copy0_S[];
extern const u8 Move0_S[];
extern const u8 Ren0_S[];
extern const u8 RnMv_S[];
extern const u8 CopTo_S[];
extern const u8 CopFi_S[];
extern const DIALOG(2) Copy1_B;
extern const DIALOG(3) Copy2_B;
extern const DIALOG(1) Renam_B;
extern const BOX2(6) CopFi_P;
extern const BOX2(6) MovFi_P;
extern const BOX2(4) RenFi_P;
extern const BOX2(5) CopDi_P;
extern const BOX2(5) MovDi_P;
extern const DIALOG(1) OvrCo_B;
extern const DIALOG(1) Cont_B;
extern const DIALOG(1) OpnEr_B;
extern const DIALOG(1) Itslf_B;
extern const DIALOG(1) RenEr_B;
extern const DIALOG(1) Prot_B;
extern const u8 OvrSk_D[];

/* What is not static here, others name: copy.c, and for Prot_B edit.c
   too. */

/* Of other files: the second line that a box puts a text into, an empty
   line, the line that it puts ^I into, and the buttons Ok and Cancel,
   Yes and No. The first such line, Var_S, is in tables.h. */
extern const u8 Var2_S[];
extern const u8 Empty_S[];
extern const u8 Inc_S[];
extern const u8 OkCan_S[];
extern const u8 OkCan_D[];
extern const u8 YesNo_S[];
extern const u8 YesNo_D[];

/* Where in the main data the commands keep what the boxes show and the
   dialogs set: see copy.c. */
#define CpVar1  MD_AT(TmpBuf[120])      /* Variable 1 */
#define CpVar2  MD_AT(TmpBuf[140])      /* Variable 2 */
#define CpInpt  MD_AT(TmpBuf[0])        /* Input buffer */
#define CpMenu  MD_AT(TmpBuf[230])      /* Menu variable */
#define CpSrc   MD_AT(TmpBuf[0])        /* Source filename */
#define CpDest  MD_AT(TmpBuf[130])      /* Destination filename */

/* The buttons, which stand behind the dialogs. */
static const u8 Copy_D[7];
static const u8 OvrAp_D[9];
static const u8 Cont_D[5];

/* The titles. The commands copy the title of their dialog into CpVar1 as
   far as CpVar2, 20 bytes whatever its length, so Copy0_S and Ren0_S are
   written with what follows them in the assembly. */
const u8 Copy0_S[20] NONSTRING = " Copy \000 Move \000 Renam";
const u8 Move0_S[] = " Move ";
const u8 Ren0_S[20] NONSTRING = " Rename \000Rename \"^V\"";
static const u8 Ren1_S[] = "Rename \"^V\" to";
const u8 RnMv_S[15] NONSTRING = "Rename or move ";
const u8 CopTo_S[] = "^V to";
static const u8 CpTo1_S[] = { 't', 'o', 0xFF, LenStr - 2, 0 };
static const u8 CoMnu_S[] = "[^I]\377\003[ F10-Tree ]\377\003[ Cancel ]";
const u8 CopFi_S[] = "Copying the file";
static const u8 MovFi_S[] = "Moving the file";
static const u8 RenFi_S[] = "Renaming or moving the file";
static const u8 CopDi_S[] = "Copying the directory";
static const u8 MovDi_S[] = "Renaming or moving the directory";
static const u8 Diagr_S[] = "\377\030";
static const u8 OvRn1_S[] = "The following file exists";
static const u8 OvRn2_S[] = "Do you wish to write over the old file?";
static const u8 OvrAp_S[] = "Overwrite\377\003All\377\003aPpend\377\003Skip";
static const u8 Prot1_S[] = "is a read-only file.";
static const u8 Prot2_S[] = "Do you still wish to overwrite it?";
static const u8 Cont1_S[] = "There isn't enough room to copy";
static const u8 Cont2_S[] = "Abort\377\003Continue";
static const u8 OpnEr_S[] = "Can't open the file";
static const u8 Itslf_S[] = "You can't copy a file to itself";
static const u8 RenEr_S[] = "Can't rename the file";

/* F5 and F6: where to. */
static const BOX2(5) Copy1_P = {
    { Box_C, BoxCu_C, Box_C, 0 },
    5, 2,
    MD_AT(MenuBuf[500]),
    { { CpVar1, 0, 70 }, { CpVar2, 0, 70 } },
    { Var_S, Var2_S, Empty_S, 0, CoMnu_S }
};
const DIALOG(2) Copy1_B = {
    { &Copy1_P, 0x528, SUB_F35_61, 8, 2, 0, 8 },
    { { 1, 1, 1, 0, 0, 0x305, CpInpt, NUMBER(0x200 + LenStr) },
      { 4, 0, 0, 1, 1, 0x516, CpMenu, Copy_D } },
    0
};

/* Shift-F5 and Shift-F6: what, and where to. */
static const BOX2(7) Copy2_P = {
    { Box_C, BoxCu_C, Box_C, 0 },
    7, 2,
    MD_AT(MenuBuf[500]),
    { { CpVar1, 0, 70 }, { CpVar2, 0, 70 } },
    { Var_S, Var2_S, Empty_S, CpTo1_S, Empty_S, 0, CoMnu_S }
};
const DIALOG(3) Copy2_B = {
    { &Copy2_P, 0x528, SUB_F35_60, 8, 2, 0, 8 },
    { { 1, 2, 1, 0, 0, 0x305, MD_AT(ViewFil), NUMBER(0x300 + LenStr) },
      { 1, 0, 2, 1, 1, 0x505, CpInpt, NUMBER(0x200 + LenStr) },
      { 4, 1, 0, 2, 2, 0x716, CpMenu, Copy_D } },
    0
};

/* F6 in a tree: the new name of the directory. */
static const BOX1(3) Renam_P = {
    { Box_C, BoxCu_C, 0, 0 },
    3, 1,
    MD_AT(WinBuf),
    { { CpDest, 0, 30 } },
    { Ren0_S, Ren1_S, Empty_S }
};
const DIALOG(1) Renam_B = {
    { &Renam_P, 0x528, 0, 1, 2, 0, 0 },
    { { 1, 0, 0, 0, 0, 0x305, CpDest, NUMBER(0x20C) } },
    0
};

/* What is shown while a file or a directory is copied, moved or
   renamed. */
const BOX2(6) CopFi_P = {
    { Box_C, 0, 0, 0 },
    6, 2,
    MD_AT(WinBuf),
    { { CpSrc, 7, 64 }, { CpDest, 4, 64 } },
    { Copy0_S, CopFi_S, Var_S, CopTo_S + 3, Var2_S, Diagr_S }
};
const BOX2(6) MovFi_P = {
    { Box_C, 0, 0, 0 },
    6, 2,
    MD_AT(WinBuf),
    { { CpSrc, 7, 64 }, { CpDest, 4, 64 } },
    { Move0_S, MovFi_S, Var_S, CopTo_S + 3, Var2_S, Diagr_S }
};
const BOX2(4) RenFi_P = {
    { Box_C, 0, 0, 0 },
    4, 2,
    MD_AT(WinBuf),
    { { CpSrc, 7, 64 - 3 }, { CpDest, 7, 64 } },
    { Ren0_S, RenFi_S, CopTo_S, Var2_S }
};
const BOX2(5) CopDi_P = {
    { Box_C, 0, 0, 0 },
    5, 2,
    MD_AT(WinBuf),
    { { CpSrc, 6, 64 }, { CpDest, 6, 64 } },
    { Inc_S, CopDi_S, Var_S, CopTo_S + 3, Var2_S }
};
const BOX2(5) MovDi_P = {
    { Box_C, 0, 0, 0 },
    5, 2,
    MD_AT(WinBuf),
    { { CpSrc, 6, 64 }, { CpDest, 6, 64 } },
    { Inc_S, MovDi_S, Var_S, CopTo_S + 3, Var2_S }
};

/* The file is there already: write over it, over all, append, skip. */
static const BOX1(5) OvrCo_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    5, 1,
    MD_AT(ErrBuf),
    { { CpDest, 4, 64 } },
    { Inc_S, OvRn1_S, Var_S, OvRn2_S, OvrAp_S }
};
const DIALOG(1) OvrCo_B = {
    { &OvrCo_P, 0x928, 0, 1, 0xFF, 0, 5 },
    { { 4, 0, 0, 0, 0, 0x517, CpMenu, OvrAp_D } },
    0
};

/* The disk is full: abort, or continue. */
static const BOX2(5) Cont_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    5, 2,
    MD_AT(ErrBuf),
    { { CpSrc, 7, 64 }, { CpDest, 4, 64 } },
    { Inc_S, Cont1_S, CopTo_S, Var2_S, Cont2_S }
};
const DIALOG(1) Cont_B = {
    { &Cont_P, 0x928, 0, 1, 0xFF, 0, 5 },
    { { 4, 0, 0, 0, 0, 0x51F, CpMenu, Cont_D } },
    0
};

/* The file cannot be opened. */
static const BOX1(4) OpnEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { CpDest, 4, 64 } },
    { Inc_S, OpnEr_S, Var_S, OkCan_S }
};
const DIALOG(1) OpnEr_B = {
    { &OpnEr_P, 0x928, 0, 1, 0xFF, 0, 6 },
    { { 4, 0, 0, 0, 0, 0x422, CpMenu, OkCan_D } },
    0
};

/* A file cannot be copied to itself. */
static const BOX1(4) Itslf_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { CpSrc, 7, 64 } },
    { Inc_S, Itslf_S, Var_S, OkCan_S }
};
const DIALOG(1) Itslf_B = {
    { &Itslf_P, 0x928, 0, 1, 0xFF, 0, 6 },
    { { 4, 0, 0, 0, 0, 0x422, CpMenu, OkCan_D } },
    0
};

/* The file cannot be renamed. */
static const BOX1(4) RenEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { CpSrc, 7, 64 } },
    { Ren0_S, RenEr_S, Var_S, OkCan_S }
};
const DIALOG(1) RenEr_B = {
    { &RenEr_P, 0x928, 0, 1, 0xFF, 0, 6 },
    { { 4, 0, 0, 0, 0, 0x422, CpMenu, OkCan_D } },
    0
};

/* The file to write over is read-only: the editor asks so too. */
static const BOX1(5) Prot_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    5, 1,
    MD_AT(WinBuf[1000]),
    { { CpDest, 4, 64 } },
    { Inc_S, Var_S, Prot1_S, Prot2_S, YesNo_S }
};
const DIALOG(1) Prot_B = {
    { &Prot_P, 0x928, 0, 1, 0xFF, 0, 5 },
    { { 4, 0, 0, 0, 0, 0x523, MD_AT(TmpErr), YesNo_D } },
    0
};

/* The buttons of the dialogs: of each its width and its column, up to a
   width of 0. Nothing names OvrSk_D, in the original either: it is not
   static, since the compiler warns of a static constant without a use. */
static const u8 Copy_D[7] = {
     8,  0,                             /* Copy, Move or Rename: ^I */
    12, 11,                             /* F10-Tree */
    10, 26,                             /* Cancel */
     0
};
static const u8 OvrAp_D[9] = {
    11,  0,                             /* Overwrite */
     5, 12,                             /* All */
     8, 18,                             /* aPpend */
     6, 27,                             /* Skip */
     0
};
const u8 OvrSk_D[] = {
    11,  0,
     5, 12,
     6, 18,
     0
};
static const u8 Cont_D[5] = {
     7,  0,                             /* Abort */
    10,  8,                             /* Continue */
     0
};
