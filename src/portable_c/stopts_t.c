/*
 * The tables of the option dialogs of the setup: the boxes and the
 * dialogs that stopts.c hands to Dialog by their offsets, their texts and
 * their buttons.
 *
 * From VCSETUP.ASM, in the order that they have there, each under the
 * name of the assembly with St in front. How a line is marked up is said
 * in line.c: \377 and a count stand for so many spaces; \377, a count
 * with 200 added to it, in octal as here, and a character for so many of
 * that character; and ^b and ^r switch a colour. \332, \277, \300 and
 * \331 are the corners of a frame, \304 is its line across and \263 its
 * upright one, \303 and \264 where a line meets the frame from inside:
 * characters of CP866. \030 and \031 are the arrows up and down, \020
 * and \021 those to the right and to the left.
 *
 * Four settings of the setup program are not here, since they are of
 * what the portable C has no more: Memory allocation, Quick execute
 * commands and TSR manager of the Configuration, which were all of its
 * Execute options, and Screen output of Screen and Mouse (see
 * DEVIATIONS.md). Their lines and their fields are gone from the two
 * dialogs, and what stood below them in the right half of a box stands
 * further up: a line of such a box is one text for the two groups that
 * are side by side, so its left half is the assembly's of that name and
 * its right half that of a line further down, or spaces where the right
 * half of the box has nothing left. The fields go to each other as they
 * stand now. Their bytes of the setup are kept as they are found.
 */
#include "tables.h"

/* What this file defines that other files may name, declared as it
   is defined: clang asks for a declaration in front of each such
   object (-Wmissing-variable-declarations). */
extern const DIALOG(16) StConf_B;
extern const DIALOG(6) StScrn_B;
extern const u8 StOkMn_S[];
extern const DIALOG(9) StPanOp_B;
extern const DIALOG(1) StNoTim_B;
extern u8 StInf02_S[];
extern const u8 StInf05_S[];
extern u8 StInf06_S[];
extern u8 StInf07_S[];
extern const u8 StInf09_S[];
extern u8 StInf10_S[];
extern u8 StInf11_S[];
extern const u8 StOk_S[];
extern const DIALOG(1) StInfo_B;

/* What is not static here, others name: stopts.c the dialogs and the
   lines that it reads and writes; and the help of the setup the two
   texts of a button Ok, StOkMn_S and StOk_S. */

/* Of the main program, main_t.c: the button of a box that has Ok alone,
   which is the setup program's byte for byte. */
extern const u8 OkMn_D[];

/* The buttons of Country info, which stand behind the dialog that names
   them: of each its width and its column, up to a width of 0. */
static const u8 StOk_D[3];

/* The Configuration: 15 boxes to tick, and Ok and Cancel. The setup
   program has 18, and a group more: Execute options, at the top of the
   right half, with Memory allocation, Quick execute commands and TSR
   manager. Without it the Keyboard options stand five rows further up,
   beside the first of the General options, which leaves the last five
   rows of the right half empty. */
static const u8 StCnf00_S[] = " Configuration ";
static const u8 StCnf01_S[] =
    " \332 ^bGeneral options^b \377\212\304\277  \332 ^bKeyboard option"
    "s^b \377\216\304\277 ";
static const u8 StCnf02_S[] =
    " \263  [ ] Auto menus\377\013\263  \263  [ ] Alt alone selects men"
    "u\377\004\263 ";
static const u8 StCnf03_S[] =
    " \263  [ ] Path prompt\377\012\263  \263  [ ] Esc toggles screen "
    "on/off \263 ";
static const u8 StCnf04_S[] =
    " \263  [ ] Key bar\377\016\263  \263  [ ] Alternative viewer\377"
    "\010\263 ";
static const u8 StCnf05_S[] =
    " \263  [ ] Menu bar visible\377\005\263  \263  [ ] Alternative ed"
    "itor\377\010\263 ";
static const u8 StCnf06_S[] =
    " \263  [ ] Full screen\377\012\263  \263  [ ] Clear keyboard buf"
    "fer\377\005\263 ";
static const u8 StCnf07_S[] =
    " \263  [ ] Clock\377\020\263  \300\377\240\304\331 ";
static const u8 StCnf08_S[] =
    " \263  [ ] Confirmation on quit \263\377\045";
static const u8 StCnf09_S[] = " \263  [ ] Error sound\377\012\263\377\045";
static const u8 StCnf10_S[] =
    " \263  [ ] Auto save setup\377\006\263\377\045";
static const u8 StCnf11_S[] =
    " \263  [ ] Show logical drive B:\263\377\045";
static const u8 StCnf12_S[] = " \300\377\233\304\331\377\045";
static const u8 StCnf13_S[] =
    "Press ^r Space ^r to change an option, ^r \030 ^r and ^r \031 ^r";
static const u8 StCnf14_S[] = "to move between options";
static const u8 StOkCn_S[] = "[ Ok ]\377\003[ Cancel ]";

/* The buttons Ok and Cancel, of this dialog and of the two that follow. */
static const u8 StOkCn_D[5] = {
    6, 0,
    10, 9,
    0
};

/* A box to tick names its byte of the setup, in the main data, and has
   it in TmpBuf while the dialog is up. */
static const BOX(17) StConf_P = {
    { Conf_C, CnfRe_C, CnfBo_C, 0 },
    17, 0,
    MD_AT(WinBuf),
    {
        StCnf00_S, StCnf01_S, StCnf02_S, StCnf03_S, StCnf04_S, StCnf05_S,
        StCnf06_S, StCnf07_S, StCnf08_S, StCnf09_S, StCnf10_S, StCnf11_S,
        StCnf12_S, StCnf13_S, StCnf14_S, 0, StOkCn_S
    }
};
/* A field keeps the place in TmpBuf that the setup program gives it,
   the buttons too: the three places of the boxes that are gone are not
   used. */
const DIALOG(16) StConf_B = {
    { &StConf_P, 0x228, 0, 1, 4, 1, 0 },
    {
        /* General options: Auto menus, Path prompt, Key bar, Menu bar
           visible, Full screen, Clock, Confirmation on quit, Error
           sound, Auto save setup, Show logical drive B:. Right of each
           is the box on its row, or else the one above that row, which
           is the last of the right half from Clock down. */
        { 3, 15, 1, 0, 10, 0x30A, MD_AT(TmpBuf[0]),
          NUMBER(MD_AT(AutoMnu)) },
        { 3, 0, 2, 1, 11, 0x40A, MD_AT(TmpBuf[1]),
          NUMBER(MD_AT(Prompt)) },
        { 3, 1, 3, 2, 12, 0x50A, MD_AT(TmpBuf[2]),
          NUMBER(MD_AT(KeyBar)) },
        { 3, 2, 4, 3, 13, 0x60A, MD_AT(TmpBuf[3]),
          NUMBER(MD_AT(MenuVs)) },
        { 3, 3, 5, 4, 14, 0x70A, MD_AT(TmpBuf[4]),
          NUMBER(MD_AT(FulScr)) },
        { 3, 4, 6, 5, 14, 0x80A, MD_AT(TmpBuf[5]),
          NUMBER(MD_AT(Clock)) },
        { 3, 5, 7, 6, 14, 0x90A, MD_AT(TmpBuf[6]),
          NUMBER(MD_AT(QuitAck)) },
        { 3, 6, 8, 7, 14, 0xA0A, MD_AT(TmpBuf[7]),
          NUMBER(MD_AT(ErrSnd)) },
        { 3, 7, 9, 8, 14, 0xB0A, MD_AT(TmpBuf[8]),
          NUMBER(MD_AT(AutoSav)) },
        { 3, 8, 10, 9, 14, 0xC0A, MD_AT(TmpBuf[9]),
          NUMBER(MD_AT(DriveB)) },
        /* Keyboard options: Alt alone selects menu, Esc toggles screen
           on/off, Alternative viewer, Alternative editor, Clear keyboard
           buffer. Left of each is the box on its row. */
        { 3, 9, 11, 0, 10, 0x329, MD_AT(TmpBuf[13]),
          NUMBER(MD_AT(AltBar)) },
        { 3, 10, 12, 1, 11, 0x429, MD_AT(TmpBuf[14]),
          NUMBER(MD_AT(EscBar)) },
        { 3, 11, 13, 2, 12, 0x529, MD_AT(TmpBuf[15]),
          NUMBER(MD_AT(AltView)) },
        { 3, 12, 14, 3, 13, 0x629, MD_AT(TmpBuf[16]),
          NUMBER(MD_AT(AltEdit)) },
        { 3, 13, 15, 4, 14, 0x729, MD_AT(TmpBuf[17]),
          NUMBER(MD_AT(ClrBuf)) },
        /* Ok, Cancel */
        { 4, 14, 0, 14, 15, 0x111E, MD_AT(TmpBuf[18]), StOkCn_D }
    },
    0
};

/* Screen and Mouse: two groups, two boxes to tick and a line of five
   characters. The setup program has a third group, Screen output, at the
   top of the right half. Without it the Other options and the Mouse
   options stand five rows further up, which leaves the right half empty
   from there down. */
static const u8 StScr00_S[] = " Screen and Mouse ";
static const u8 StScr01_S[] =
    " \332 ^bScreen colors^b \377\206\304\277  \332 ^bOther options^b "
    "\377\216\304\277 ";
static const u8 StScr02_S[] =
    " \263  ( ) Black & White  \263  \263  [ ] Zooming boxes\377\012"
    "\263 ";
static const u8 StScr03_S[] =
    " \263  ( ) Color\377\012\263  \263  Screen saver chars:\377\010"
    "\263 ";
static const u8 StScr04_S[] =
    " \263  ( ) Laptop\377\011\263  \300\377\235\304\331 ";
static const u8 StScr05_S[] =
    " \300\377\225\304\331  \332 ^bMouse options^b \377\216\304\277 ";
static const u8 StScr06_S[] =
    " \332 ^bScreen blank delay^b \304\277  \263  [ ] Left-handed mouse"
    "\377\006\263 ";
static const u8 StScr07_S[] =
    " \263  ( ) 40 minutes\377\005\263  \300\377\235\304\331 ";
static const u8 StScr08_S[] = " \263  ( ) 20 minutes\377\005\263\377\042";
static const u8 StScr09_S[] = " \263  ( )  5 minutes\377\005\263\377\042";
static const u8 StScr10_S[] = " \263  ( )  3 minutes\377\005\263\377\042";
static const u8 StScr11_S[] = " \263  ( )  1 minute\377\006\263\377\042";
static const u8 StScr12_S[] = " \263  ( ) Off\377\014\263\377\042";
static const u8 StScr13_S[] = " \300\377\225\304\331\377\042";
static const u8 StScr14_S[] =
    "Press ^r Space ^r to change an option, ^r \030 ^r and ^r \031 ^r";
static const u8 StScr15_S[] = "to move between options";

/* A group has the line chosen, and behind it that of the cursor, in
   TmpBuf. The line is typed into the setup itself, the five characters
   of the screen saver at StarTab. The minutes of the second group, from
   its first line down, are StDelay_T of stopts.c, which stands here in
   the assembly, behind the dialog. A field keeps the place in TmpBuf
   that the setup program gives it: the two bytes of the group that is
   gone are not used. */
static const BOX(18) StScrn_P = {
    { Conf_C, CnfRe_C, CnfBo_C, 0 },
    18, 0,
    MD_AT(WinBuf),
    {
        StScr00_S, StScr01_S, StScr02_S, StScr03_S, StScr04_S, StScr05_S,
        StScr06_S, StScr07_S, StScr08_S, StScr09_S, StScr10_S, StScr11_S,
        StScr12_S, StScr13_S, StScr14_S, StScr15_S, 0, StOkCn_S
    }
};
const DIALOG(6) StScrn_B = {
    { &StScrn_P, 0x228, 0, 1, 4, 2, 0 },
    {
        /* Screen colors, with Zooming boxes to its right, and Screen
           blank delay, with Left-handed mouse, the last of the right
           half, which ends above it. Up from the first group are the
           buttons, and down from the second. */
        { 2, 5, 0x81, 0, 2, 0x30A, MD_AT(TmpBuf[0]), NUMBER(3) },
        { 2, 0xC0, 5, 1, 4, 0x80A, MD_AT(TmpBuf[2]), NUMBER(6) },
        /* Zooming boxes: above it, as above the right half in the setup
           program, the last line of the second group; to its left the
           first group, on its first line */
        { 3, 0xC1, 3, 0x80, 2, 0x323, MD_AT(TmpBuf[6]),
          NUMBER(MD_AT(ZoomWin)) },
        /* Screen saver chars */
        { 1, 2, 4, 0x80, 3, 0x436, MD_AT(StarTab), NUMBER(5) },
        /* Left-handed mouse: to its left the second group, on its first
           line */
        { 3, 3, 5, 0x81, 4, 0x723, MD_AT(TmpBuf[7]),
          NUMBER(MD_AT(LftMous)) },
        /* Ok, Cancel */
        { 4, 4, 0x80, 5, 5, 0x121E, MD_AT(TmpBuf[9]), StOkCn_D }
    },
    0
};

/* Panel options: seven boxes to tick and the delay of the tree, a line
   of five characters; and the box that says that what was typed is no
   delay. StOkMn_S is the button of that box, and of the box of the
   setup's help. */
static const u8 StPan00_S[] = " Panel options ";
static const u8 StPan01_S[] =
    " \332 ^bFile panel options^b \377\222\304\277 ";
static const u8 StPan02_S[] = " \263  [ ] Mini status line\377\020\263 ";
static const u8 StPan03_S[] = " \263  [ ] Auto directory size\377\015\263 ";
static const u8 StPan04_S[] = " \263  [ ] Ins moves down\377\022\263 ";
static const u8 StPan05_S[] =
    " \263  [ ] Unselect during group operation \263 ";
static const u8 StPan06_S[] = " \300\377\246\304\331 ";
static const u8 StPan07_S[] =
    " \332 ^bTree panel options^b \377\222\304\277 ";
static const u8 StPan08_S[] =
    " \263  [ ] Auto change directory\377\013\263 ";
static const u8 StPan09_S[] =
    " \263  [ ] Show hidden subdirectories\377\006\263 ";
static const u8 StPan10_S[] = " \263  [ ] Auto rescan tree\377\020\263 ";
static const u8 StPan11_S[] =
    " \263  Auto change dir time delay:\377\011\263 ";
static const u8 StPan12_S[] = " \300\377\246\304\331 ";
static const u8 StPan13_S[] = "Press ^r Space ^r to change an option,";
static const u8 StPan14_S[] = "arrows to move between options";
static const u8 StNoTm1_S[] = "Illegal format of";
static const u8 StNoTm2_S[] = "auto change dir time delay.";
const u8 StOkMn_S[] = " Ok ";

/* The line is at TmpBuf+9 when the dialog comes up, as StF04_00 of
   stopts.c has written it, and StF04_10 there reads what was typed, when
   the dialog is answered. */
static const BOX(17) StPanOp_P = {
    { Conf_C, CnfRe_C, CnfBo_C, 0 },
    17, 0,
    MD_AT(WinBuf),
    {
        StPan00_S, StPan01_S, StPan02_S, StPan03_S, StPan04_S, StPan05_S,
        StPan06_S, StPan07_S, StPan08_S, StPan09_S, StPan10_S, StPan11_S,
        StPan12_S, StPan13_S, StPan14_S, 0, StOkCn_S
    }
};
const DIALOG(9) StPanOp_B = {
    { &StPanOp_P, 0x228, SUB_StF04_10, 1, 4, 3, 0 },
    {
        /* File panel options: Mini status line, Auto directory size, Ins
           moves down, Unselect during group operation */
        { 3, 8, 1, 0, 0, 0x30A, MD_AT(TmpBuf[0]),
          NUMBER(MD_AT(MiniSta)) },
        { 3, 0, 2, 1, 1, 0x40A, MD_AT(TmpBuf[1]),
          NUMBER(MD_AT(AutoSiz)) },
        { 3, 1, 3, 2, 2, 0x50A, MD_AT(TmpBuf[2]),
          NUMBER(MD_AT(InsDwn)) },
        { 3, 2, 4, 3, 3, 0x60A, MD_AT(TmpBuf[3]),
          NUMBER(MD_AT(UnSlct)) },
        /* Tree panel options: Auto change directory, Show hidden
           subdirectories, Auto rescan tree */
        { 3, 3, 5, 4, 4, 0x90A, MD_AT(TmpBuf[4]),
          NUMBER(MD_AT(AutoCD)) },
        { 3, 4, 6, 5, 5, 0xA0A, MD_AT(TmpBuf[5]),
          NUMBER(MD_AT(HidDirs)) },
        { 3, 5, 7, 6, 6, 0xB0A, MD_AT(TmpBuf[6]),
          NUMBER(MD_AT(ChgTree)) },
        /* Auto change dir time delay */
        { 1, 6, 8, 7, 7, 0xC25, MD_AT(TmpBuf[9]), NUMBER(5) },
        /* Ok, Cancel */
        { 4, 7, 0, 8, 8, 0x111E, MD_AT(TmpBuf[7]), StOkCn_D }
    },
    0
};

/* The box comes up over the dialog, so it keeps what it covers where
   the assembly has BoxBuf, which is ErrBuf here, and has its button
   behind the line of the dialog, which is at TmpErr. */
static const BOX(4) StNoTim_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 0,
    MD_AT(ErrBuf),
    { StPan00_S, StNoTm1_S, StNoTm2_S, StOkMn_S }
};
const DIALOG(1) StNoTim_B = {
    { &StNoTim_P, 0xC28, 0, 1, 0xFF, 0xFF, 0 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpErr[240]), OkMn_D } },
    0
};

/* Country info: how the country writes the time, the date and numbers,
   and the letters beyond ASCII with their capitals and their small
   letters. Five of its lines are no constants: StF09_00 of stopts.c
   writes into them before the box is shown, the time, the date and the
   number into StInf02_S, and the capitals and the small letters of the
   characters of StInf05_S and StInf09_S into the two lines behind each.
   Here they are as the assembly has them, with spaces there. StOk_S is
   the button of the setup's help as well. */
static const u8 StInf00_S[] = " Country info ";
static const u8 StInf01_S[] =
    " \332 ^bTime format^b \304\277 \332 ^bDate format^b \304\304\277 "
    "\332 ^bNumbers format^b \304\277 ";
u8 StInf02_S[] =
    " \263              \263 \263               \263 \263              "
    "   \263 ";
static const u8 StInf03_S[] =
    " \300\377\216\304\331 \300\377\217\304\331 \300\377\221\304\331 ";
static const u8 StInf04_S[] =
    " \332 ^bCharacters table^b \377\242\304\277 ";
const u8 StInf05_S[] =
    " \263\377\006\200\201\202\203\204\205\206\207\210\211\212\213\214"
    "\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234"
    "\235\236\237\240\241\242\243\244\245\246\247\377\006\263 ";
u8 StInf06_S[] =
    " \263\377\003\332\304\020                                        "
    "\377\006\263 ";
u8 StInf07_S[] =
    " \263\377\003\263                                          \021"
    "\304\277\377\003\263 ";
static const u8 StInf08_S[] = " \263\377\003\263\377\054\263\377\003\263 ";
const u8 StInf09_S[] =
    " \263\377\003\263  \250\251\252\253\254\255\256\257\340\341\342"
    "\343\344\345\346\347\350\351\352\353\354\355\356\357\360\361\362"
    "\363\364\365\366\367\370\371\372\373\374\375\376   \263\377\003"
    "\263 ";
u8 StInf10_S[] =
    " \263\377\003\303\304\020                                         "
    " \263\377\003\263 ";
u8 StInf11_S[] =
    " \263\377\003\263                                          \021"
    "\304\264\377\003\263 ";
static const u8 StInf12_S[] =
    " \263\377\003\300\304\304\304\304 ^bUpper case\377\016Lower case^b"
    " \304\304\304\304\331\377\003\263 ";
static const u8 StInf13_S[] = " \300\377\264\304\331 ";
static const u8 StInf14_S[] = "The country information is determined by";
static const u8 StInf15_S[] =
    "the ^bcountry=^b statement in the ^bconfig.sys^b file.";
const u8 StOk_S[] = "[ Ok ]";

static const BOX(18) StInfo_P = {
    { Conf_C, CnfRe_C, CnfBo_C, 0 },
    18, 0,
    MD_AT(WinBuf),
    {
        StInf00_S, StInf01_S, StInf02_S, StInf03_S, StInf04_S, StInf05_S,
        StInf06_S, StInf07_S, StInf08_S, StInf09_S, StInf10_S, StInf11_S,
        StInf12_S, StInf13_S, StInf14_S, StInf15_S, 0, StOk_S
    }
};
const DIALOG(1) StInfo_B = {
    { &StInfo_P, 0x228, 0, 1, 4, 6, 0 },
    { { 4, 0, 0, 0, 0, 0x1225, MD_AT(TmpBuf), StOk_D } },
    0
};
static const u8 StOk_D[3] = {
    6, 0,
    0
};
