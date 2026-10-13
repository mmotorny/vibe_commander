/*
 * The tables of the menus' commands: which command an item of a menu is;
 * and the boxes and the dialogs that commands.c hands to DialBox and
 * Dialog by their offsets, their texts and their buttons, and the texts
 * that it copies.
 *
 * From VCMENU.INC, in the order that they have there. How a line is
 * marked up is said in line.c: \377 and a count stand for so many
 * spaces; \377, a count with 200 added to it, in octal as here, and a
 * character for so many of that character; ^I for a text that the caller
 * names and ^V for the one at TmpErr+1; and ^b and ^r switch a colour.
 * \332, \277, \300 and \331 are the corners of a frame, \304 is its line
 * across and \263 its upright one, characters of CP866.
 */
#include "tables.h"

/* What is not static here, others name: commands.c, and for Menu_T
   main.c. */

/* Of other files: the text of the button Ok, and the button. tables.h
   has two more, Var_S, a line that is all one text put in, and Tree_S,
   the title of the tree. */
extern const u8 Ok_S[];
extern const u8 OkMn_D[];

/* The buttons, which stand behind the dialogs that name them: of each
   its width and its column, up to a width of 0. */
static const u8 OkCn_D[5];
static const u8 MnLoc_D[7];
static const u8 SvSt_D[5];

/* The commands of the menus. The menu bar, F39_00 of menu.c, says
   which item was chosen and which of these tables has it: Menu_T names
   them, one for each menu, Menu1_T for the Left and for the Right one,
   and MenuS_T and MenuC_T, which are the first eight commands of the
   Files menu with Shift and with Ctrl held down. An entry is the number
   of a command, a CMD_ of vc.h, which F39_01 of main.c hands to
   Command, and 0 for a line across the menu. The original has the
   address of the command's label there. Two menus have an item less
   than the original's (see DEVIATIONS.md): the Commands menu, Menu3_T,
   Memory info, behind the volume label, and the Options menu, Menu4_T,
   Memory allocation, behind the clock. And the Options menu has one
   that the original has not, the setup, the last, behind Save setup and
   a line across the menu. */
static const u16 Menu1_T[15];
static const u16 Menu2_T[17];
static const u16 Menu3_T[12];
static const u16 Menu4_T[17];
static const u16 MenuS_T[8];
static const u16 MenuC_T[8];

const u16 * const Menu_T[6] = {
    Menu1_T, Menu2_T, Menu3_T, Menu4_T, MenuS_T, MenuC_T
};

static const u16 Menu1_T[15] = {
    CMD_M01_00,                         /* Brief */
    CMD_M01_00,                         /* Full */
    CMD_M03_00,                         /* Info */
    CMD_M03_00,                         /* Tree */
    CMD_M06_00,                         /* On/Off */
    0,
    CMD_M08_00,                         /* Name */
    CMD_M08_00,                         /* Extension */
    CMD_M08_00,                         /* Time */
    CMD_M08_00,                         /* Size */
    CMD_M08_00,                         /* Unsorted */
    0,
    CMD_F17_01,                         /* Re-read */
    CMD_M16_01,                         /* Filter... */
    CMD_F62_01                          /* Drive... */
};

static const u16 Menu2_T[17] = {
    CMD_M21_00,                         /* Help */
    CMD_F32_00,                         /* Menu */
    CMD_F33_00,                         /* View */
    CMD_F34_00,                         /* Edit */
    CMD_F35_00,                         /* Copy */
    CMD_F36_00,                         /* Rename or move */
    CMD_F37_00,                         /* Make directory */
    CMD_F38_00,                         /* Delete */
    0,
    CMD_M30_00,                         /* File attributes */
    0,
    CMD_F09_00,                         /* Gray + */
    CMD_F10_00,                         /* Gray - */
    CMD_F76_01,                         /* Gray * */
    CMD_F78_00,                         /* ^M */
    0,
    CMD_F30_00                          /* Quit */
};

static const u16 Menu3_T[12] = {
    CMD_M41_00,                         /* Tree */
    CMD_F54_00,                         /* Volume label */
    CMD_M42_00,                         /* Directory sizes */
    CMD_F67_00,                         /* Find file */
    CMD_F68_00,                         /* History */
    CMD_F69_00,                         /* EGA lines */
    0,
    CMD_F15_00,                         /* Swap panels */
    CMD_F11_00,                         /* Panels on/off */
    CMD_M49_00,                         /* Compare directories */
    0,
    CMD_M53_00                          /* Menu file edit */
};

static const u16 Menu4_T[17] = {
    CMD_M60_00,                         /* Configuration... */
    CMD_M61_00,                         /* Advanced options... */
    CMD_M62_00,                         /* Extension file edit... */
    CMD_M63_00,                         /* Viewer... */
    CMD_M64_00,                         /* Editor... */
    0,
    CMD_M66_00,                         /* Auto menus */
    CMD_M67_00,                         /* Path prompt */
    CMD_F16_00,                         /* Key bar */
    CMD_M69_00,                         /* Full screen */
    CMD_M70_00,                         /* Auto directory sizes */
    CMD_M71_00,                         /* Mini status */
    CMD_M72_00,                         /* Clock */
    0,
    CMD_M75_00,                         /* Save setup */
    0,
    CMD_SETUP                           /* More setup... */
};

static const u16 MenuS_T[8] = {
    CMD_M21_00,                         /* Help */
    CMD_F42_00,                         /* Menu */
    CMD_F43_00,                         /* View... */
    CMD_F44_00,                         /* Edit... */
    CMD_F45_00,                         /* Copy... */
    CMD_F46_00,                         /* Rename or move... */
    CMD_F47_00,                         /* Make directory... */
    CMD_F48_00                          /* Delete... */
};

static const u16 MenuC_T[8] = {
    CMD_M21_00,                         /* Help */
    CMD_F32_00,                         /* Menu */
    CMD_F33_00,                         /* View... */
    CMD_F34_00,                         /* Edit... */
    CMD_F55_00,                         /* Copy... */
    CMD_F56_00,                         /* Rename or move... */
    CMD_F37_00,                         /* Make directory... */
    CMD_F58_00                          /* Delete... */
};
/* The filter of a panel:

     Select files to display
     Custom:
     [ ] Hidden files
     [ ] Executable files
     ----------------------
     [ Ok ]   [ Cancel ]

   The line to type into and the two boxes to tick are in the segment of
   the panel, which the dialog is asked for with: its mask of twelve
   characters, taken with NormStr, and two bytes of its setup. OkCn_S and
   OkCn_D are the buttons of the configuration and of the advanced
   options too. */
static const u8 Filt0_S[] = " Filter ";
static const u8 Filt1_S[] = "Select files to display";
static const u8 Filt2_S[] = "Custom:\377\016";
static const u8 Filt3_S[] = "[ ] Hidden files\377\005";
static const u8 Filt4_S[] = "[ ] Executable files ";
static const u8 OkCn_S[] = "[ Ok ]\377\003[ Cancel ]";

static const BOX(7) Filt_P = {
    { Box_C, BoxCu_C, Box_C, 0 },
    7, 0,
    MD_AT(WinBuf),
    { Filt0_S, Filt1_S, Filt2_S, Filt3_S, Filt4_S, 0, OkCn_S }
};
const DIALOG(4) Filt_B = {
    { &Filt_P, 0x514, 0, 1, 1, 14, 0 },
    {
        { 1, 3, 1, 0, 0, 0x30E, WCB_AT(DskMas), NUMBER(0x20C) },
        { 3, 0, 2, 1, 1, 0x407, MD_AT(TmpBuf), NUMBER(WCB_AT(Hidden)) },
        { 3, 1, 3, 2, 2, 0x507, MD_AT(TmpBuf[1]), NUMBER(WCB_AT(MasTyp)) },
        { 4, 2, 0, 3, 3, 0x70A, MD_AT(TmpBuf[2]), OkCn_D }
    },
    0
};
static const u8 OkCn_D[5] = {
     6, 0,                              /* Ok */
    10, 9,                              /* Cancel */
     0
};

/* What is shown while the sizes of the directories are added up, with
   the letter of the drive put in from TmpBuf. */
static const u8 DiSi1_S[] = "Scanning sizes of subdirectories";
static const u8 DiSi2_S[] = "on drive ^V:";

const BOX1(3) DiSiz_P = {
    { Box_C, 0, 0, 0 },
    3, 1,
    MD_AT(WinBuf),
    { { MD_AT(TmpBuf), 0, 25 } },
    { Tree_S, DiSi1_S, DiSi2_S }
};

/* What Compare directories says where it finds no difference. */
static const u8 Cmp0_S[] = " Compare ";
static const u8 Cmp1_S[] = "The two directories appear";
static const u8 Cmp2_S[] = "to be identical.";

static const BOX(4) CmpDi_P = {
    { Box_C, BoxCu_C, BoxCu_C, 0 },
    4, 0,
    MD_AT(WinBuf),
    { Cmp0_S, Cmp1_S, Cmp2_S, Ok_S }
};
const DIALOG(1) CmpDi_B = {
    { &CmpDi_P, 0x728, 0, 1, 3, 37, 0 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpBuf), OkMn_D } },
    0
};

/* The configuration, with the frames drawn in ASCII here:

     + Screen colors ------+  + Panel options ----------------+
     |  ( ) Black & White  |  |  [ ] Ins moves down           |
     |  ( ) Color          |  |  [ ] Auto change directory    |
     |  ( ) Laptop         |  +-------------------------------+
     +---------------------+  + Other options ----------------+
     + Screen blank delay -+  |  [ ] Menu bar always visible  |
     |  ( ) 40 minutes     |  |  [ ] Auto save setup          |
     |  ( ) 20 minutes     |  |  [ ] Confirmation on quit     |
     |  ( )  5 minutes     |  |  [ ] Zooming boxes            |
     |  ( )  3 minutes     |  +-------------------------------+
     |  ( )  1 minute      |  + Mouse options ----------------+
     |  ( ) Off            |  |  [ ] Left-handed mouse        |
     +---------------------+  +-------------------------------+
     Press  Space  to change an option,  ^  and  v
     to move between options

   What begins a box to tick, with the frame to its left and the two
   spaces between the frames, is put in for ^I: Chk1_S, which the
   assembly has as two spaces in front of Chk2_S and reads as far as the
   end of that. \030 and \031 are the arrows up and down. */
static const u8 Cnf00_S[] = " Configuration ";
static const u8 Cnf01_S[] =
    " \332 ^bScreen colors^b \377\206\304\277"
    "  \332 ^bPanel options^b \377\220\304\277 ";
static const u8 Cnf02_S[] =
    " \263  ( ) Black & White  \263"
    "^IIns moves down\377\013\263 ";
static const u8 Cnf03_S[] =
    " \263  ( ) Color\377\012\263"
    "^IAuto change directory\377\004\263 ";
static const u8 Cnf04_S[] =
    " \263  ( ) Laptop\377\011\263"
    "  \300\377\237\304\331 ";
static const u8 Cnf05_S[] =
    " \300\377\225\304\331"
    "  \332 ^bOther options^b \377\220\304\277 ";
static const u8 Cnf06_S[] =
    " \332 ^bScreen blank delay^b \304\277"
    "^IMenu bar always visible  \263 ";
static const u8 Cnf07_S[] =
    " \263  ( ) 40 minutes\377\005\263"
    "^IAuto save setup\377\012\263 ";
static const u8 Cnf08_S[] =
    " \263  ( ) 20 minutes\377\005\263"
    "^IConfirmation on quit\377\005\263 ";
static const u8 Cnf09_S[] =
    " \263  ( )  5 minutes\377\005\263"
    "^IZooming boxes\377\014\263 ";
static const u8 Cnf10_S[] =
    " \263  ( )  3 minutes\377\005\263"
    "  \300\377\237\304\331 ";
static const u8 Cnf11_S[] =
    " \263  ( )  1 minute\377\006\263"
    "  \332 ^bMouse options^b \377\220\304\277 ";
static const u8 Cnf12_S[] =
    " \263  ( ) Off\377\014\263"
    "^ILeft-handed mouse\377\010\263 ";
static const u8 Cnf13_S[] =
    " \300\377\225\304\331"
    "  \300\377\237\304\331 ";
static const u8 Cnf14_S[] =
    "Press ^r Space ^r to change an option, ^r \030 ^r and ^r \031 ^r";
static const u8 Cnf15_S[] = "to move between options";
const u8 Chk1_S[] = "  \263  [ ] ";
const u8 Chk2_S[] = "\263  [ ] ";

/* A group has the line chosen, and behind it that of the cursor, in
   TmpBuf. A box to tick names its byte of the setup, in the main data,
   and has it in TmpBuf while the dialog is up. */
static const BOX(18) Conf_P = {
    { Conf_C, CnfRe_C, CnfBo_C, 0 },
    18, 0,
    MD_AT(WinBuf),
    {
        Cnf00_S, Cnf01_S, Cnf02_S, Cnf03_S, Cnf04_S, Cnf05_S, Cnf06_S,
        Cnf07_S, Cnf08_S, Cnf09_S, Cnf10_S, Cnf11_S, Cnf12_S, Cnf13_S,
        Cnf14_S, Cnf15_S, 0, OkCn_S
    }
};
const DIALOG(10) Conf_B = {
    { &Conf_P, 0x228, 0, 1, 4, 40, 0 },
    {
        /* Screen colors, and Screen blank delay */
        { 2, 9, 0x81, 0, 2, 0x30A, MD_AT(TmpBuf), NUMBER(3) },
        { 2, 0xC0, 9, 1, 4, 0x80A, MD_AT(TmpBuf[2]), NUMBER(6) },
        /* Ins moves down, Auto change directory */
        { 3, 9, 3, 0x80, 2, 0x323, MD_AT(TmpBuf[4]),
          NUMBER(MD_AT(InsDwn)) },
        { 3, 2, 4, 0x80, 3, 0x423, MD_AT(TmpBuf[5]),
          NUMBER(MD_AT(AutoCD)) },
        /* Menu bar always visible, Auto save setup, Confirmation on
           quit, Zooming boxes */
        { 3, 3, 5, 0x81, 4, 0x723, MD_AT(TmpBuf[6]),
          NUMBER(MD_AT(MenuVs)) },
        { 3, 4, 6, 0x81, 5, 0x823, MD_AT(TmpBuf[7]),
          NUMBER(MD_AT(AutoSav)) },
        { 3, 5, 7, 0x81, 6, 0x923, MD_AT(TmpBuf[8]),
          NUMBER(MD_AT(QuitAck)) },
        { 3, 6, 8, 0x81, 7, 0xA23, MD_AT(TmpBuf[9]),
          NUMBER(MD_AT(ZoomWin)) },
        /* Left-handed mouse */
        { 3, 7, 9, 0x81, 8, 0xD23, MD_AT(TmpBuf[10]),
          NUMBER(MD_AT(LftMous)) },
        /* Ok, Cancel */
        { 4, 8, 0x80, 9, 9, 0x121E, MD_AT(TmpBuf[11]), OkCn_D }
    },
    0
};

/* The advanced options:

     + Key configuration ----------------+
     |  [ ] Alt alone selects menu       |
     |  [ ] Escape toggles panels on/off |
     |  [ ] Alternative viewer           |
     |  [ ] Alternative editor           |
     +-----------------------------------+
     + Other options --------------------+
     |  [ ] Quick execute commands       |
     |  [ ] Clear keyboard buffer        |
     +-----------------------------------+
     Press  Space  to change an option,
     arrows to move between options

   Chk2_S is put in for ^I here. The assembly has no Set06_S: the line
   that ends the first frame is the one that ends the second. */
static const u8 Set00_S[] = " Advanced Options ";
static const u8 Set01_S[] = "\332 ^bKey configuration^b \377\220\304\277";
static const u8 Set02_S[] = "^IAlt alone selects menu\377\007\263";
static const u8 Set03_S[] = "^IEscape toggles panels on/off \263";
static const u8 Set04_S[] = "^IAlternative viewer\377\013\263";
static const u8 Set05_S[] = "^IAlternative editor\377\013\263";
static const u8 Set07_S[] = "\332 ^bOther options^b \377\224\304\277";
static const u8 Set08_S[] = "^IQuick execute commands\377\007\263";
static const u8 Set09_S[] = "^IClear keyboard buffer\377\010\263";
static const u8 Set10_S[] = " \300\377\243\304\331 ";
static const u8 Set11_S[] = "Press ^r Space ^r to change an option,";
static const u8 Set12_S[] = "arrows to move between options";

static const BOX(15) Sets_P = {
    { Conf_C, CnfRe_C, CnfBo_C, 0 },
    15, 0,
    MD_AT(WinBuf),
    {
        Set00_S, Set01_S, Set02_S, Set03_S, Set04_S, Set05_S, Set10_S,
        Set07_S, Set08_S, Set09_S, Set10_S, Set11_S, Set12_S, 0, OkCn_S
    }
};
const DIALOG(7) Sets_B = {
    { &Sets_P, 0x328, 0, 1, 4, 41, 0 },
    {
        /* Alt alone selects menu, Escape toggles panels on/off,
           Alternative viewer, Alternative editor */
        { 3, 6, 1, 0, 0, 0x30A, MD_AT(TmpBuf), NUMBER(MD_AT(AltBar)) },
        { 3, 0, 2, 1, 1, 0x40A, MD_AT(TmpBuf[1]), NUMBER(MD_AT(EscBar)) },
        { 3, 1, 3, 2, 2, 0x50A, MD_AT(TmpBuf[2]), NUMBER(MD_AT(AltView)) },
        { 3, 2, 4, 3, 3, 0x60A, MD_AT(TmpBuf[3]), NUMBER(MD_AT(AltEdit)) },
        /* Quick execute commands, Clear keyboard buffer */
        { 3, 3, 5, 4, 4, 0x90A, MD_AT(TmpBuf[4]), NUMBER(MD_AT(ExecTyp)) },
        { 3, 4, 6, 5, 5, 0xA0A, MD_AT(TmpBuf[5]), NUMBER(MD_AT(ClrBuf)) },
        /* Ok, Cancel */
        { 4, 5, 0, 6, 6, 0xF1E, MD_AT(TmpBuf[6]), OkCn_D }
    },
    0
};

/* The box that asks which file to edit, the main one or that of the
   current directory: its title is put in from TmpBuf, " User Menu " or
   " Extension File ", and what it calls the file for ^I, "user menu" or
   "extension file".

   The title is copied to TmpBuf as 80 bytes, so ExFl_S has the 63 that
   the original has behind its 17 as well: the three texts that follow
   it and the first eight letters of the fourth. Each of the four is a
   constant of its own besides. */
const u8 ExFl_S[80] =
    " Extension File \0"
    "Do you wish to edit the main,\0"
    "or a local ^I?\0"
    "user menu\0"
    "extensio";
static const u8 MnLc1_S[] = "Do you wish to edit the main,";
static const u8 MnLc2_S[] = "or a local ^I?";
const u8 UsMnu_S[] = "user menu";
const u8 ExtFl_S[] = "extension file";
static const u8 MnLc3_S[] = "Main\377\003Local\377\003Cancel";

static const BOX1(4) MnLoc_P = {
    { Box_C, BoxCu_C, BoxCu_C, 0 },
    4, 1,
    MD_AT(WinBuf),
    { { MD_AT(TmpBuf), 0, 50 } },
    { Var_S, MnLc1_S, MnLc2_S, MnLc3_S }
};
const DIALOG(1) MnLoc_B = {
    { &MnLoc_P, 0x528, 0, 1, 0xFF, 0, 0 },
    { { 4, 0, 0, 0, 0, 0x41C, MD_AT(TmpBuf), MnLoc_D } },
    0
};
static const u8 MnLoc_D[7] = {
    6,  0,                              /* Main */
    7,  7,                              /* Local */
    8, 15,                              /* Cancel */
    0
};

/* Save setup, and the box that says the setup could not be saved. */
static const u8 SvSt0_S[] = " Setup ";
static const u8 SvSt1_S[] = "Do you wish to save";
static const u8 SvSt2_S[] = "the current setup?";
static const u8 SvSt3_S[] = "Save\377\003Cancel";
static const u8 StEr1_S[] = "There was an error while";
static const u8 StEr2_S[] = "saving the setup file";

static const BOX(4) SvSt_P = {
    { Box_C, BoxCu_C, BoxCu_C, 0 },
    4, 0,
    MD_AT(WinBuf),
    { SvSt0_S, SvSt1_S, SvSt2_S, SvSt3_S }
};
const DIALOG(1) SvSt_B = {
    { &SvSt_P, 0x528, 0, 1, 4, 53, 0 },
    { { 4, 0, 0, 0, 0, 0x420, MD_AT(TmpBuf), SvSt_D } },
    0
};
static const BOX(4) SetEr_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    4, 0,
    MD_AT(ErrBuf),
    { SvSt0_S, StEr1_S, StEr2_S, Ok_S }
};
const DIALOG(1) SetEr_B = {
    { &SetEr_P, 0x928, 0, 1, 0xFF, 0, 2 },
    { { 4, 0, 0, 0, 0, 0x426, MD_AT(TmpErr), OkMn_D } },
    0
};
static const u8 SvSt_D[5] = {
    6, 0,                               /* Save */
    8, 7,                               /* Cancel */
    0
};
