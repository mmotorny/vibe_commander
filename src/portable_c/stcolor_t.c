/*
 * The tables of the setup's colours: the texts of the list of a
 * palette's colours, F5 to F7 of the setup's menu, with the names of its
 * 39 items and of the 16 colours, the box that asks whether to take the
 * default colours, and the four pictures in which the colour of an item
 * is chosen, each with the table that says which item colours which of
 * its cells. stcolor.c reads them.
 *
 * From VCSETUP.ASM of the original's setup program, in the order that
 * they have there, each with St in front of the name that it has in the
 * assembly: the test of the data holds StX against X of the setup
 * program. Every text is taken from the assembly's bytes by a script,
 * the pictures above all, which have characters of CP866 and bytes
 * below 20h in them, marks on the screen: \020 and \021 around
 * UP--DIR, \033 where the editor shows a tab, \007, \004 and \017
 * for stars. How a line of a box is marked up is said
 * in line.c: ^I is a text that the caller names.
 *
 * Two tables of the assembly are not here, Color_T and ColSt_T, the
 * keys of the list and of a picture with the labels that each jumps to:
 * they are switch statements of stcolor.c.
 */
#include "tables.h"

/* Of keys_t.c: the buttons Yes and No, their text and where they are.
   The setup program has its own, of the same bytes. */
extern const u8 YesNo_S[];
extern const u8 YesNo_D[];

/* The title of the list, which is that of the box below as well; what
   stands in front of the kind of the display and of the palette's name,
   on the right of the screen; and the names of the three palettes and of
   the two kinds of display. */
const u8 StColor_S[] = " Color Setup ";
const u8 StMode_S[] = "Mode: ";
const u8 StPalet_S[] = "Palette: ";
const u8 StBwPal_S[] = "Black&White";
const u8 StLapTp_S[] = "LapTop";
const u8 StColMd_S[] = "Color";
const u8 StMonMd_S[] = "Monochrome";

/* The two lines of the box that F8 asks in. */
static const u8 StDfCo1_S[] = "Do you wish to set default colors";
static const u8 StDfCo2_S[] = "for current palette?";

/* The names of the 39 items, in the order of a palette's colours, the
   numbers of tables.h: StF07_65 finds one by counting zero bytes. An
   empty text ends them, the zero byte of the last line and the string's
   own, which StF07_15 stops at. */
const u8 StColrs_S[] =
    "Panels\0"
    "Current pointer\0"
    "Selected files\0"
    "Selected pointer\0"
    "Title in file panel\0"
    "Key bar numbers\0"
    "Key bar text\0"
    "Pull-down menu bar\0"
    "Pull-down menu pointer\0"
    "Pull-down menu border\0"
    "Pull-down menu text\0"
    "Pull-down menu bright\0"
    "Pull-down menu current bar\0"
    "Pull-down menu current bright\0"
    "Pull-down menu unaccessable lines\0"
    "Pull-down menu minus\0"
    "Dialog box text\0"
    "Dialog box bright text\0"
    "Dialog box reverse text\0"
    "Configuration boxes text\0"
    "Configuration boxes reverse text\0"
    "Configuration boxes bright text\0"
    "Configuration boxes bright reverse text\0"
    "History box text\0"
    "History box pointer\0"
    "Mini help in .EXT file editor border\0"
    "Mini help in .EXT file editor text\0"
    "Help box text\0"
    "Help box bright text\0"
    "Help box reverse text\0"
    "Help box underline text\0"
    "Error box text\0"
    "Error box reverse text\0"
    "Error box bright text\0"
    "Clock\0"
    "Arrows in viewer and editor\0"
    "Stars\0"
    "Explosing stars\0"
    "Shadow\0";

/* "on" and a colour: the second line of a colour's name. And the names
   of the 16 colours, which StF07_66 finds like the items, with two more
   behind them for what the item Shadow can be besides a colour. */
const u8 StOn_S[] = "on ^I";
const u8 StColNm_S[] =
    "Black\0"
    "Blue\0"
    "Green\0"
    "Cyan\0"
    "Red\0"
    "Magenta\0"
    "Brown\0"
    "White\0"
    "Gray\0"
    "Bright Blue\0"
    "Bright Green\0"
    "Bright Cyan\0"
    "Bright Red\0"
    "Pink\0"
    "Yellow\0"
    "Bright White\0"
    "No shadow\0"
    "Inv brightness";

/* The one colour that a colour's name is written in: the panels'. */
const u8 StColNm_P[1] = { Back_C };

/* F8 in the list: "Do you wish to set default colors for current
   palette?", Yes or No. What it covers is kept in the setup program's
   BoxBuf, which the main data has as ErrBuf. */
static const BOX(4) StDfCol_P = {
    { Box_C, BoxCu_C, BoxCu_C, 0 }, 4, 0, MD_AT(ErrBuf),
    { StColor_S, StDfCo1_S, StDfCo2_S, YesNo_S }
};
const DIALOG(1) StDfCol_B = {
    { &StDfCol_P, 0x728, 0, 1, 0xFF, 0xFF, 4 },
    { { 4, 0, 0, 0, 0, 0x423, MD_AT(TmpErr), YesNo_D } },
    0
};

/* The four pictures: 25 rows of 58 characters each, with no zero byte
   anywhere. The first is the panels with the boxes of the drives, of
   the tree and of the user menu; */
static const u8 StPage1_S[25 * 58] NONSTRING =
    "\311\315\315\315\315\315\315\315\315\315\315\315"
        "\315\321\315\315 C:\\DOS \315\315\315\315\315\315"
        "\315\315\321\315 4:55p                  "
    "\272    Name    \263   Size  \263  Da"
        "te  \263 Time \272                  "
    "\272..          \263\020UP--DIR\021\2631"
        "0-29-91\263 2:55p\272                  "
    "\272command  com\263    54619\311\315\315\315\315\315"
        "\315\315\315\315\315\315\315\315\315\315\315 Tree "
        "\315\315\315\315\315\315\315\315\315\315\315 "
    "\272append   exe\263     8170\272 \\                                "
    "\272as                    \272 \303\304"
        "\304DOS                           "
    "\272at   \311\315\315\315\315\315\315 Drive let\272"
        " \303\304\335VC          \336                 "
    "\272ba   \272    Choose left \272 \303"
        "\304\304TOOLS                         "
    "\272ch   \272  A   B   C   D \272 \303\304"
        "\335ARC         \336                 "
    "\272ch   \310\315\315\315\315\315\315\315\315\315\315\315\315\315"
        "\315\315\315\272 \303\304\304TEST                          "
    "\272co                    \272                                  "
    "\272debug    exe\263    15718\272   \311\315\315\315\315\315"
        "\315\315 User Menu \315\315\315\315\315\315\315\273    "
    "\272deltree  exe\263     7930\272   \272"
        " F1  Archives...         \272    "
    "\272diskcomp com\263     7249\272   \272"
        " F2  Format floppy disk  \272    "
    "\272diskcopy com\263     8763\272   \272"
        " F4  Edit file           \272    "
    "\272doskey   com\263     4729\272   \310\315\315\315"
        "\315\315\315\315\315\315\315\315\315\315\315\315\315"
        "\315\315\315\315\315\315\315\315\315\274    "
    "\272edlin    exe\263    10007\272                                  "
    "\272exe2bin  exe\263     6755\272 \303"
        "\304\304DOCS                          "
    "\272expand   exe\263    16129\272 \300"
        "\304 TMP                           "
    "\272fasthelp exe\263     8199\272                                  "
    "\307\304\304\304\304\304\304\304\304\304\304\304\304\301\304\304\304"
        "\304\304\304\304\304\304\272                                  "
    "\272   32,891 bytes in 3 s\272                                  "
    "\310\315\315\315\315\315\315\315\315\315\315\315\315\315\315\315\315"
        "\315\315\315\315\315\315\272                                  "
    "C:\\DOS>                \307\304\304\304\304\304\304\304"
        "\304\304\304\304\304\304\304\304\304\304\304\304\304\304"
        "\304\304\304\304\304\304\304\304\304\304\304\304 "
    "1Help   2Menu   3View  \272C:\\DOS                            ";

/* the second the menu bar with a menu open, over the stars of the
   blanker; */
static const u8 StPage2_S[25 * 58] NONSTRING =
    "    Left    Files    Commands    Options    Right         "
    "  \332\304\304\304\304\304\304\304\304\304\304\304\304\304\304\304"
        "\304\304\304\304\304\304\304\304\277                               "
    "  \263  Brief                \263  "
        "              \372         \372    "
    "  \263  Full                 \263                               "
    "  \263  Info                 \263       \372                       "
    "  \263  Tree                 \263                               "
    "  \263\373 On/Off       Ctrl-F1 \263 "
        "       \007              \372       "
    "  \263 \304\304\304\304\304\304\304\304\304\304\304\304\304\304\304"
        "\304\304\304\304\304\304 \263                               "
    "  \263  Name                 \263                               "
    "  \263  eXtension            \263                               "
    "  \263  tiMe                 \263                               "
    "  \263  Size                 \263            \372                  "
    "  \263  Unsorted             \263                        \372      "
    "  \263 \304\304\304\304\304\304\304\304\304\304\304\304\304\304\304"
        "\304\304\304\304\304\304 \263                               "
    "  \263  Re-read              \263                               "
    "  \263 -fiLter...            \263                   \017           "
    "  \263  Drive...     Alt-F1  \263                               "
    "  \300\304\304\304\304\304\304\304\304\304\304"
        "\304\304\304\304\304\304\304\304\304\304\304\304"
        "\304\331          \372                    "
    "                                                   \372      "
    "                                             \371            "
    "                          \372                               "
    "       \372                          \004                       "
    "                                                          "
    "                    \372                    \372                "
    "                                                          ";

/* the third the editor of an extension file, with boxes of errors, of
   a tree, of the deleting and of a dialog over it; */
static const u8 StPage3_S[25 * 58] NONSTRING =
    "Edit: C:\\VC\\                                              "
    "\033asm:    tas   \311\315\315\315\315\315\315\315\315\315\315\315 "
        "Choose Directory \315\315\315\315\315\315\315\315\315\315\315\273 "
    "\033obj:    lin   \272  \303\304\304TO"
        "OLS                              \272 "
    "\033sys:    dl    \272  \303\304 ARC  "
        "                            \021 \272 "
    "zip:    pkun   \272  \303\304\304TEST"
        "                               \272 "
    "arc:                                                      "
    "ice:     \311\315\315\315\315\315\315\315\315\315\315\315\315"
        "\315\315\315\315\315\315 Error \315\315\315\315\315\315\315"
        "\315\315\315\315\315\315\315\315\315\315\315\273    "
    "lzh:     \272      Can't read the disk in drive A:      \272    "
    "arj:     \272  Press ENTER to try again, ESC to abort,  \272    "
    "         \272 or enter a different drive letter here A: \272    "
    "         \310\315\315\315\315\315\315\315"
        "                                         "
    "                    \311\315\315\315\315\315\315\315\315\315\315\315"
        " Delete \315\315\315\315\315\315\315\315\315\315\315\273      "
    "               \272    \272       Y"
        "ou are DELETING       \272    \272 "
    "               \272    \272 5 files"
        " and 1 directory from \272    \272 "
    "\334\334\334\334\334\334\334\334\334\334\334\334   \272"
        "    \272            C:\\TMP            \272    \272 "
    " \311\315\315\315\315\315\315\315\315\315\315   \272"
        "    \272     Delete   All   Cancel    \272    \272 "
    " \272 Format of   \272    \310\315\315\315\315\315\315\315"
        "\315\315\315\315\315\315\315\315\315\315\315\315\315\315"
        "\315\315\315\315\315\315\315\315\315\274    \272 "
    " \272             \307\304           "
        "                           \304\266 "
    " \272 ' comment   \272 C:\\ARC                                 \272 "
    " \272 txt: edit   \272 Speed search: ARC                      \272 "
    " \272  \036   cls    \310\315\315\315\315\315\315"
        "\315\315\315\315\315\315\315\315\315\315\315\315\332"
        " Screen colors \304\304\304\304\304\304\277 "
    " \272  \300\304\304\304\304\304\304\304       "
        "               \263  ( ) Black & White  \263 "
    " \310\315\315\315\315\315\315\315\315\315\315\315\315\315"
        "\315\315\315\315\315\315\315\315\315\315\315\315\315\315"
        "\315\315\315\315\315\263  (\007) Color          \263 "
    "\337\337\337\337\337\337\337\337\337\337\337\337\337\337"
        "\337\337\337\337\337\337\337\337\337\337\337\337\337\337"
        "\337\337\337\337\337\337\263  ( ) Laptop         \263 "
    "1Help   2Save   3       4Hex    5 \300\304\304\304\304\304\304\304"
        "\304\304\304\304\304\304\304\304\304\304\304\304\304\304\331 ";

/* and the fourth the help, a page over the index. */
static const u8 StPage4_S[25 * 58] NONSTRING =
    "                                                          "
    "                                                          "
    "   \311\315\315\315\315\315\315\315\315\315\315\315"
        "\315\315\315\315\315\315\315\315\315\315\315\315\315"
        "\315\315\315\315\315 Help \315\315\315\315\315\315\315"
        "\315\315\315\315\315\315\315\315\315\315\315 "
    "   \272 Configuration...                                     "
    "   \307\304\304\304\304\304\304\304\304\304\304\304\304"
        "\304\304\304\304\304\304\304\304\304\304\304\304\304\304"
        "\304\304   \311\315\315\315\315\315\315\315\315\315\315"
        "\315\315\315\315\315\315\315\315\315\315\315 "
    "   \272  This dialog box  allows yo   \272               The Vo "
    "   \272  options.  Use the cursor k   \307\304\304\304\304\304\304"
        "\304\304\304\304\304\304\304\304\304\304\304\304\304\304\304 "
    "   \272  to change, and use the Spa   \272  About the Commander "
    "   \272  Enter to accept the dialog   \272  Keyboard reference  "
    "   \272                               \272  View -- Keyboard re "
    "   \272  \332 Screen colors \304\304\304\304"
        "\304\304\277  T   \272  View -- Status line "
    "   \272  \263  ( ) Black & White  \263"
        "  w   \272  Edit -- Keyboard re "
    "   \272  \263  (\007) Color          "
        "\263  t   \272  Edit -- Status line "
    "   \272  \263  ( ) Laptop         \263"
        "  l   \272  Left/Right menu     "
    "   \272  \300\304\304\304\304\304\304\304\304"
        "\304\304\304\304\304\304\304\304\304\304\304\304"
        "\304\331  c   \272       Brief/Full     "
    "   \272                               \272       Info           "
    "   \272  \332 Screen blank delay \304"
        "\277  T   \272       Tree           "
    "   \272  \263  ( ) 40 minutes     \263"
        "  b   \272       On/Off         "
    "   \307\304\304\304\304\304\304\304\304\304\304\304"
        "\304\304\304\304\304\304\304\304\304\304\304\304\304"
        "\304\304\304\304   \272       Sorting order  "
    "   \272        [ Next ]   [ Previou   \272       Re-read a pane "
    "   \310\315\315\315\315\315\315\315\315\315\315\315\315"
        "\315\315\315\315\315\315\315\315\315\315\315\315\315\315"
        "\315\315   \307\304\304\304\304\304\304\304\304\304\304"
        "\304\304\304\304\304\304\304\304\304\304\304 "
    "                                   \272                      "
    "                                   \310\315\315\315\315\315\315"
        "\315\315\315\315\315\315\315\315\315\315\315\315\315\315\315 "
    "                                                          "
    "                                                          ";

/* The colours of the pictures: for each of the 25 rows, how many cells
   and the number of the item that colours them, up to 0. With SHADED
   the cells lie in the shadow of a box and are coloured as Shadow says,
   with BLINKS they blink, and GREY is no item: white on black. */
#define SHADED 0x40
#define BLINKS 0x80
#define GREY   0xFF

static const u8 StColr1_T[259] = {
    16, Back_C, 8, Cursr_C, 10, Back_C, 2, Clock_C, 1, Clock_C + BLINKS,
        3, Clock_C, 18, GREY, 0,
    1, Back_C, 12, Title_C, 1, Back_C, 9, Title_C, 1, Back_C, 8, Title_C,
        1, Back_C, 6, Title_C, 1, Back_C, 18, GREY, 0,
    40, Back_C, 18, GREY, 0,
    57, Back_C, 1, GREY, 0,
    57, Back_C, 1, GREY, 0,
    3, Back_C, 20, Box_C, 34, Back_C, 1, GREY, 0,
    3, Back_C, 20, Box_C, 4, Back_C, 14, Slct_C, 16, Back_C, 1, GREY, 0,
    3, Back_C, 15, Box_C, 4, BoxBr_C, 1, Box_C, 34, Back_C, 1, GREY, 0,
    3, Back_C, 13, Box_C, 3, BoxCu_C, 4, Box_C, 4, Back_C, 14, CurSl_C,
        16, Back_C, 1, GREY, 0,
    3, Back_C, 20, Box_C, 34, Back_C, 1, GREY, 0,
    3, Back_C, 20, Box_C, 1, Back_C, 33, Hist_C, 1, GREY, 0,
    5, Back_C, 18, Back_C + SHADED, 1, Back_C, 33, Hist_C, 1, GREY, 0,
    23, Back_C, 1, Back_C, 8, Hist_C, 20, HstCu_C, 5, Hist_C, 1, GREY, 0,
    23, Back_C, 1, Back_C, 33, Hist_C, 1, GREY, 0,
    1, Back_C, 22, Cursr_C, 1, Back_C, 33, Hist_C, 1, GREY, 0,
    23, Back_C, 1, Back_C, 33, Hist_C, 1, GREY, 0,
    1, Back_C, 22, CurSl_C, 1, Back_C, 33, Hist_C, 1, GREY, 0,
    1, Back_C, 22, Slct_C, 3, Back_C, 31, Back_C + SHADED, 1, GREY, 0,
    1, Back_C, 22, Slct_C, 4, Back_C, 14, Cursr_C, 16, Back_C, 1, GREY, 0,
    57, Back_C, 1, GREY, 0,
    57, Back_C, 1, GREY, 0,
    4, Back_C, 19, Slct_C, 34, Back_C, 1, GREY, 0,
    57, Back_C, 1, GREY, 0,
    23, GREY, 34, Back_C, 1, GREY, 0,
    1, KeyNu_C, 6, KeyTi_C, 2, KeyNu_C, 6, KeyTi_C, 2, KeyNu_C, 6, KeyTi_C,
        34, Back_C, 1, GREY, 0
};

static const u8 StColr2_T[317] = {
    2, Menu_C, 8, MnCur_C, 47, Menu_C, 1, GREY, 0,
    2, Sky1_C, 25, MnuBx_C, 30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 2, MnuTx_C, 1, MnuBr_C, 20, MnuTx_C, 1, MnuBx_C,
        30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 2, MnuTx_C, 1, MnuBr_C, 20, MnuTx_C, 1, MnuBx_C,
        30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 2, MnuTx_C, 1, MnuBr_C, 20, MnuTx_C, 1, MnuBx_C,
        30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 2, MnuTx_C, 1, MnuBr_C, 20, MnuTx_C, 1, MnuBx_C,
        30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 2, MnuCu_C, 1, MnuCB_C, 20, MnuCu_C, 1, MnuBx_C,
        8, Sky1_C, 1, Sky2_C, 21, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 25, MnuBx_C, 30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 2, MnuTx_C, 1, MnuBr_C, 20, MnuTx_C, 1, MnuBx_C,
        30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 3, MnuTx_C, 1, MnuBr_C, 19, MnuTx_C, 1, MnuBx_C,
        30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 4, MnuTx_C, 1, MnuBr_C, 18, MnuTx_C, 1, MnuBx_C,
        30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 2, MnuTx_C, 1, MnuBr_C, 20, MnuTx_C, 1, MnuBx_C,
        30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 2, MnuTx_C, 1, MnuBr_C, 20, MnuTx_C, 1, MnuBx_C,
        30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 25, MnuBx_C, 30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 2, MnuTx_C, 1, MnuBr_C, 20, MnuTx_C, 1, MnuBx_C,
        30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 1, MnuDr_C, 1, MnuMi_C, 21, MnuDr_C, 1, MnuBx_C,
        19, Sky1_C, 1, Sky2_C, 10, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 1, MnuBx_C, 2, MnuTx_C, 1, MnuBr_C, 20, MnuTx_C, 1, MnuBx_C,
        30, Sky1_C, 1, GREY, 0,
    2, Sky1_C, 25, MnuBx_C, 30, Sky1_C, 1, GREY, 0,
    57, Sky1_C, 1, GREY, 0,
    45, Sky1_C, 1, Sky2_C, 11, Sky1_C, 1, GREY, 0,
    57, Sky1_C, 1, GREY, 0,
    34, Sky1_C, 1, Sky2_C, 22, Sky1_C, 1, GREY, 0,
    57, Sky1_C, 1, GREY, 0,
    57, Sky1_C, 1, GREY, 0,
    57, Sky1_C, 1, GREY, 0
};

static const u8 StColr3_T[259] = {
    12, Menu_C, 45, Conf_C, 1, GREY, 0,
    1, Arrow_C, 11, Back_C, 45, Conf_C, 1, GREY, 0,
    1, Arrow_C, 11, Back_C, 45, Conf_C, 1, GREY, 0,
    1, Arrow_C, 11, Back_C, 8, Conf_C, 14, CnfUn_C, 20, Conf_C, 1, CnfBo_C,
        2, Conf_C, 1, GREY, 0,
    12, Back_C, 45, Conf_C, 1, GREY, 0,
    6, Back_C, 51, Err_C, 1, GREY, 0,
    6, Back_C, 51, Err_C, 1, GREY, 0,
    6, Back_C, 51, Err_C, 1, GREY, 0,
    6, Back_C, 51, Err_C, 1, GREY, 0,
    6, Back_C, 44, Err_C, 1, ErInv_C, 6, Err_C, 1, GREY, 0,
    6, Back_C, 51, Err_C, 1, GREY, 0,
    6, Back_C, 49, Err_C, 2, Err_C + SHADED, 1, GREY, 0,
    8, Back_C, 4, Back_C + SHADED, 5, Conf_C + SHADED, 19, Err_C,
        8, ErBr_C, 11, Err_C, 2, Conf_C + SHADED, 1, GREY, 0,
    12, Back_C, 5, Conf_C, 38, Err_C, 2, Conf_C + SHADED, 1, GREY, 0,
    12, HlBrd_C, 5, Conf_C, 38, Err_C, 2, Conf_C + SHADED, 1, GREY, 0,
    12, HlpMn_C, 5, Conf_C, 8, Err_C, 8, ErInv_C, 22, Err_C,
        2, Conf_C + SHADED, 1, GREY, 0,
    12, HlpMn_C, 5, Conf_C, 38, Err_C, 2, Conf_C + SHADED, 1, GREY, 0,
    12, HlpMn_C, 5, Conf_C, 38, Err_C, 2, Conf_C + SHADED, 1, GREY, 0,
    12, HlpMn_C, 7, Conf_C, 38, Conf_C + SHADED, 1, GREY, 0,
    12, HlpMn_C, 19, Conf_C, 12, CnfRe_C, 14, Conf_C, 1, GREY, 0,
    12, HlpMn_C, 24, Conf_C, 13, CnfBo_C, 8, Conf_C, 1, GREY, 0,
    12, HlpMn_C, 45, Conf_C, 1, GREY, 0,
    14, HlpMn_C, 20, HlpMn_C + SHADED, 23, Conf_C, 1, GREY, 0,
    34, HlBrd_C, 23, Conf_C, 1, GREY, 0,
    1, KeyNu_C, 6, KeyTi_C, 2, KeyNu_C, 6, KeyTi_C, 2, KeyNu_C, 6, KeyTi_C,
        2, KeyNu_C, 6, KeyTi_C, 2, KeyNu_C, 1, KeyTi_C, 23, Conf_C,
        1, GREY, 0
};

static const u8 StColr4_T[155] = {
    58, GREY, 0,
    57, Help_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    5, Help_C, 16, HlpBo_C, 36, Help_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    24, Help_C, 6, HlpUn_C, 27, Help_C, 1, GREY, 0,
    29, Help_C, 3, HlpUn_C, 25, Help_C, 1, GREY, 0,
    6, Help_C, 5, HlpUn_C, 26, Help_C, 20, HlpRe_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    8, Help_C, 13, HlpBo_C, 36, Help_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    8, Help_C, 18, HlpBo_C, 31, Help_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    12, Help_C, 8, HlpRe_C, 37, Help_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    57, Help_C, 1, GREY, 0,
    32, GREY, 25, Help_C, 1, GREY, 0,
    32, GREY, 25, Help_C, 1, GREY, 0,
    58, GREY, 0
};

/* Which picture each of the 39 items is shown in, from 1. */
const u8 StColPage[39] = {
    1, 1, 1, 1, 1, 1, 1,                /* the panels and the key bar */
    2, 2, 2, 2, 2, 2, 2, 2, 2,          /* the menus */
    1, 1, 1,                            /* a box */
    3, 3, 3, 3,                         /* a dialog */
    1, 1,                               /* the history */
    3, 3,                               /* the mini help */
    4, 4, 4, 4,                         /* the help */
    3, 3, 3,                            /* an error */
    1,                                  /* the clock */
    3,                                  /* the arrows */
    2, 2,                               /* the stars */
    1                                   /* the shadow */
};

/* The pictures, each with its colours. */
const struct {
    const u8 *page;
    const u8 *colors;
} StPagTab[4] = {
    { StPage1_S, StColr1_T },
    { StPage2_S, StColr2_T },
    { StPage3_S, StColr3_T },
    { StPage4_S, StColr4_T }
};
