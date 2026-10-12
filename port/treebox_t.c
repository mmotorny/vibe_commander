/*
 * The tables of the tree: its title, the name of the file that it is
 * kept in, and the boxes that port/treebox.c shows while a disk is read
 * for it and while the file is written.
 *
 * From VCSUB2.INC, the title, and VCSUB3.INC.
 */
#include "tables.h"

/* What others name has the name that it has in the assembly: the code of
   port/treebox.c, port/dirfunc.c and port/panel.c, and the boxes of
   other files. Tree_S has its by port/tables.h, which gives it a near
   name for a file of constants, and no line here. */
#pragma aux Tree_F "*"
#pragma aux Tree_P "*"
#pragma aux TrSav_P "*"

const u8 Tree_S[] = " Tree ";
const u8 Tree_F[] = "\\TREEINFO.NCD";
static const u8 Scan1_S[] = "Scanning the directory";
static const u8 Scan2_S[] = "structure on drive ^V:";
static const u8 Scan3_S[] = "0";
static const u8 Scan4_S[] = "Saving the";
static const u8 Scan5_S[] = "directory information...";

/* The box of a disk that is read: the letter of the drive is taken from
   MenuBuf, and the last line is where the directories are counted. And
   the box of the file that is written. */
const BOX1(4) Tree_P = {
    { Box_C, 0, 0, 0 },
    4, 1,
    MD_AT(ErrBuf),
    { { MD_AT(MenuBuf), 0, 25 } },
    { Tree_S, Scan1_S, Scan2_S, Scan3_S }
};
const BOX(3) TrSav_P = {
    { Box_C, 0, 0, 0 },
    3, 0,
    MD_AT(ErrBuf),
    { Tree_S, Scan4_S, Scan5_S }
};
