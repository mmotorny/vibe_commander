/*
 * The tables of what is asked of DOS at every turn: the names of files
 * that the program reads, which port/read.c, port/setup.c, port/start.c
 * and others take from here, and the dialog that port/system.c shows for
 * the disk of a drive that shares the floppy drive with another.
 *
 * From VCSUB3.INC.
 */
#include "tables.h"

/* What others name has the name that it has in the assembly: the code,
   and the boxes of other files, which have Ok and Cancel from here. */
#pragma aux Mask_S "*"
#pragma aux Ext_F "*"
#pragma aux DrIn_F "*"
#pragma aux Init_F "*"
#pragma aux OkCan_S "*"
#pragma aux Fant_B "*"
#pragma aux OkCan_D "*"

/* The names of files. Each is copied with what follows it: the mask of
   every file from its second byte on and the extension file by MvzLine,
   which takes the 68 bytes of a path, the file of a directory's
   description and the setup file as the 13 bytes of a name. So each is
   written as long as what is copied of it, with what the original has
   behind it there: the next names, the first five bytes of a tree's
   file, the texts of the box of a tree that is read, the start of a
   line of the Info and that of the title below. Nothing is known to
   read those bytes where they get to. */
const u8 Mask_S[1 + LenPath] =
    "\\*.*\0"
    "VC.EXT\0"
    "\\TREEINFO.NCD\0"
    "PNCI\0"
    "Scanning the directory\0"
    "structure on dr";
const u8 Ext_F[LenPath] =
    "VC.EXT\0"
    "\\TREEINFO.NCD\0"
    "PNCI\0"
    "Scanning the directory\0"
    "structure on drive ";
const u8 DrIn_F[13] = "DIRINFO\0 byte";
const u8 Init_F[13] = "VC.INI\0 Chang";

/* The disk of a drive that is none of its own: the box asks for it, with
   the letter of the drive from TmpErr, and has the buttons Ok and
   Cancel: of each its width and its column, up to a width of 0. */
static const u8 Fant0_S[] = " Change drive ";
static const u8 Fant1_S[] = "Insert diskette for drive ^V:";
const u8 OkCan_S[] = " Ok\377\003Cancel";
extern const u8 OkCan_D[];

static const BOX1(3) Fant_P = {
    { Err_C, ErInv_C, ErInv_C, 0 },
    3, 1,
    MD_AT(ErrBuf1),
    { { MD_AT(TmpErr[1]), 0, 25 } },
    { Fant0_S, Fant1_S, OkCan_S }
};
const DIALOG(1) Fant_B = {
    { &Fant_P, 0x928, 0, 2, 0xFF, 0xFF, 5 },
    { { 4, 0, 0, 0, 0, 0x322, MD_AT(TmpErr), OkCan_D } },
    0
};
const u8 OkCan_D[] = {
    4, 0,                               /* Ok */
    8, 5,                               /* Cancel */
    0
};
