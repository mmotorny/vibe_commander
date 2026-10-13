/*
 * The tables of the setup: the setup that the program has without a
 * setup file, the box that says the setup file is missing or wrong, the
 * menu of the setup, and the boxes that ask whether everything is to be
 * as the program has it without a file and whether the setup is to be
 * saved.
 *
 * From VCSETUP.ASM, in the order that they have there. Each has St in
 * front of the name that it has in the assembly: vc.h says why. What
 * the main program has under the same name with the same bytes is not
 * written again: the title of an error, the line that is a text handed
 * over, and the buttons Yes and No are the main program's, and the
 * setup program's line that is the text at TmpErr+1, Var_S, which
 * nothing names there, is not here at all. What the code of stmain.c
 * alone reads of that file's constants is with the code. How a line is marked up is said in line.c: \377 and a count
 * stand for so many spaces, ^I for a text that the caller names and ^V
 * for the one at TmpErr+1.
 */
#include "tables.h"

/* What is not static here, others name: stmain.c, and for StPalets the
   palettes too, stcolor.c. */

/* Of the main program's tables: main_t.c has the first two, keys_t.c
   the buttons. */
extern const u8 Err_S[];                /* " Error " */
extern const u8 Inc_S[];                /* "^I" */
extern const u8 YesNo_S[];              /* "Yes   No" */
extern const u8 YesNo_D[];

/* The buttons, which stand behind the dialogs that name them: of each
   its width and its column, up to a width of 0. */
static const u8 StOkEx_D[5];
static const u8 StSvSt_D[5];

/* The setup that the program has without a setup file: the colours,
   Palets, six palettes of each, and behind them DefInit, the options,
   with the 51 bytes that are reserved. The two are one object here
   because they are copied as one, CmnIni bytes from Palets on, by
   StDefault of stmain.c; the palettes take the colour of an item from
   here. */
const u8 StPalets[CmnIni] = {
    0x07, 0x07, 0x1B, 0x07, 0x07, 0x07, /*  0 - Main windows */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /*  1 - Cursor */
    0x0F, 0x0F, 0x1E, 0x0F, 0x0F, 0x0F, /*  2 - Selected files */
    0x7F, 0x09, 0x3E, 0x09, 0x70, 0x70, /*  3 - Cursor for selected files */
    0x0F, 0x0F, 0x1E, 0x0F, 0x0F, 0x0F, /*  4 - "Name   Size ..." */
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, /*  5 - Key bar numbers */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /*  6 - Key bar titles */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /*  7 - Menu bar */
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, /*  8 - Menu bar cursor */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /*  9 - Menu box border */
    0x70, 0x70, 0x3F, 0x70, 0x70, 0x70, /* 10 - Menu box text */
    0x7F, 0x70, 0x3E, 0x70, 0x70, 0x70, /* 11 - Menu box high letters */
    0x07, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, /* 12 - Menu box text cursor */
    0x0F, 0x0F, 0x0E, 0x0F, 0x0F, 0x0F, /* 13 - Menu box high letters cursor */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /* 14 - Menu box dark lines */
    0x70, 0x70, 0x33, 0x70, 0x70, 0x70, /* 15 - Menu box dark lines '-' */
    0x07, 0x07, 0x70, 0x07, 0x07, 0x07, /* 16 - Function box */
    0x0F, 0x0F, 0x7E, 0x0F, 0x0F, 0x0F, /* 17 - Function box bright */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /* 18 - Function box cursor */
    0x07, 0x07, 0x3F, 0x07, 0x07, 0x07, /* 19 - Configuration window */
    0x70, 0x70, 0x0F, 0x70, 0x70, 0x70, /* 20 - Configuration cursor */
    0x0F, 0x0F, 0x3E, 0x0F, 0x0F, 0x0F, /* 21 - Configuration window bright */
    0x7F, 0x09, 0x0E, 0x09, 0x70, 0x70, /* 22 - Configuration cursor bright */
    0x70, 0x70, 0x3F, 0x70, 0x70, 0x70, /* 23 - History window */
    0x07, 0x07, 0x0F, 0x07, 0x07, 0x07, /* 24 - History cursor */
    0x07, 0x07, 0x13, 0x07, 0x07, 0x07, /* 25 - Menu help border */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /* 26 - Menu help window */
    0x07, 0x07, 0x30, 0x07, 0x07, 0x07, /* 27 - Help window */
    0x0F, 0x0F, 0x3F, 0x0F, 0x0F, 0x0F, /* 28 - Help window bold */
    0x70, 0x70, 0x0F, 0x70, 0x70, 0x70, /* 29 - Help reversive */
    0x0F, 0x09, 0x3E, 0x09, 0x0F, 0x0F, /* 30 - Help underline */
    0x07, 0x07, 0x4F, 0x07, 0x07, 0x07, /* 31 - Error windows */
    0x70, 0x70, 0x70, 0x70, 0x70, 0x70, /* 32 - Error window cursor */
    0x0F, 0x0F, 0x4E, 0x0F, 0x0F, 0x0F, /* 33 - Error window bright */
    0x70, 0x70, 0x30, 0x70, 0x70, 0x70, /* 34 - Clock */
    0x0F, 0x0F, 0x1E, 0x0F, 0x0F, 0x0F, /* 35 - Arrows in view/edit */
    0x07, 0x07, 0x0B, 0x07, 0x07, 0x07, /* 36 - Screen blank */
    0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, /* 37 - Screen blank light stars */
    0x02, 0x02, 0x01, 0x02, 0x00, 0x00, /* 38 - Shadow flag */
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, /* 39 - Blink / High background / None */
    1,                                  /* FulScr */
    0,                                  /* MenuVs */
    1,                                  /* KeyBar */
    1,                                  /* Prompt */
    1,                                  /* InsDwn */
    0,                                  /* AutoMnu */
    1,                                  /* MiniSta */
    0,                                  /* Clock */
    1,                                  /* AutoCD */
    1,                                  /* AutoSav */
    1,                                  /* ColTyp */
    5,                                  /* BlDelay */
    1,                                  /* LoadFl */
    1,                                  /* UnSlct */
    1,                                  /* ErrSnd */
    0,                                  /* LftMous */
    1,                                  /* ZoomWin */
    1,                                  /* AltBar */
    1,                                  /* EscBar */
    0,                                  /* ExecTyp */
    0,                                  /* AutoSiz */
    1,                                  /* HidDirs */
    1,                                  /* QuitAck */
    2,                                  /* SnowMod */
    1,                                  /* AltView */
    1,                                  /* AltEdit */
    1,                                  /* ClrBuf */
    1,                                  /* Active */
    0,                                  /* Ctrl_O */
    1,                                  /* ChkTSR */
    1,                                  /* ChgTree */
    1,                                  /* DriveB */
    60, 0,                              /* DelayTm: 0.6 seconds */
    0xFA, 0xF9, 0x07, 0x04, 0x0F        /* StarTab: the five characters
                                           that are stars */
                                        /* Reserved: 51 bytes, 0 */
};

/* The setup file is missing, or is not one:

     ------------ Error ------------
     The setup file D:\VC\VC.INI
     not found.
          Ok   Exit

   with the path of the file put in from NcPath, and the second line
   handed over: StNoFnd_S or StNoIni_S. */
static const u8 StStFil_S[] = "The setup file ^V";
const u8 StNoFnd_S[] = "not found.";
const u8 StNoIni_S[] = "contains data that is not correct.";
static const u8 StOkEx_S[] = "Ok\377\003Exit";

static const BOX1(4) StIniEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 1,
    MD_AT(WinBuf),
    { { MD_AT(NcPath), 7, 64 } },
    { Err_S, StStFil_S, Inc_S, StOkEx_S }
};
const DIALOG(1) StIniEr_B = {
    { &StIniEr_P, 0x928, 0, 1, 0xFF, 0xFF, 5 },
    { { 4, 0, 0, 0, 0, 0x422, MD_AT(TmpErr), StOkEx_D } },
    0
};
static const u8 StOkEx_D[5] = {
    4, 0,                               /* Ok */
    6, 5,                               /* Exit */
    0
};

/* The menu of the setup, a box of nine lines under its title. The keys
   that it takes and what each of its items does are in stmain.c: the
   original has two tables for them here, Main_T and Menu_T, with the
   addresses of the code. */
static const u8 StSetUp_S[] = " Setup ";
static const u8 StMain1_S[] = "F2   Configuration\377\015";
static const u8 StMain2_S[] = "F3   Screen and Mouse\377\012";
static const u8 StMain3_S[] = "F4   Panel options\377\015";
static const u8 StMain4_S[] = "F5   Set Black & White palette ";
static const u8 StMain5_S[] = "F6   Set Color palette\377\011";
static const u8 StMain6_S[] = "F7   Set Laptop palette\377\010";
static const u8 StMain7_S[] = "F8   Set default configuration ";
static const u8 StMain8_S[] = "F9   Country info\377\016";
static const u8 StMain9_S[] = "F10  Exit\377\026";

const BOX(10) StMain_P = {
    { Hist_C, 0, 0, 0 },
    10, 0,
    MD_AT(WinBuf),
    {
        StSetUp_S, StMain1_S, StMain2_S, StMain3_S, StMain4_S, StMain5_S,
        StMain6_S, StMain7_S, StMain8_S, StMain9_S
    }
};

/* Set default configuration, with the buttons Yes and No, which are
   the main program's. */
static const u8 StDef1_S[] = "Do you wish to reset all options";
static const u8 StDef2_S[] = "to default value?";

static const BOX(4) StDeflt_P = {
    { Box_C, BoxCu_C, BoxCu_C, 0 },
    4, 0,
    MD_AT(WinBuf),
    { StSetUp_S, StDef1_S, StDef2_S, YesNo_S }
};
const DIALOG(1) StDeflt_B = {
    { &StDeflt_P, 0x728, 0, 1, 0xFF, 5, 4 },
    { { 4, 0, 0, 0, 0, 0x423, MD_AT(TmpErr), YesNo_D } },
    0
};

/* The setup is left with something changed: save it? */
static const u8 StSvSt0_S[] = " Setup ";
static const u8 StSvSt1_S[] = "Do you wish to save";
static const u8 StSvSt2_S[] = "the current setup?";
static const u8 StSvSt3_S[] = "Save\377\003Cancel";

static const BOX(4) StSvSt_P = {
    { Box_C, BoxCu_C, BoxCu_C, 0 },
    4, 0,
    MD_AT(WinBuf),
    { StSvSt0_S, StSvSt1_S, StSvSt2_S, StSvSt3_S }
};
const DIALOG(1) StSvSt_B = {
    { &StSvSt_P, 0x528, 0, 1, 4, 7, 0 },
    { { 4, 0, 0, 0, 0, 0x420, MD_AT(TmpBuf), StSvSt_D } },
    0
};
static const u8 StSvSt_D[5] = {
    6, 0,                               /* Save */
    8, 7,                               /* Cancel */
    0
};
