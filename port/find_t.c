/*
 * The tables of the file search: what port/find.c hands to Dialog and
 * MinMnu by its offset, and the two texts that it copies.
 *
 * From VCFIND.INC.
 */
#include "tables.h"

/* What others name, port/find.c here, has the name that it has in the
   assembly. */
#pragma aux FnFl6_S "*"
#pragma aux FnFl7_S "*"
#pragma aux FnFl_P "*"
#pragma aux Find_B "*"
#pragma aux Find5_D "*"
#pragma aux Find6_D "*"

/* "N files found." is put together of these two, and the command copies
   15 bytes of each, the text and what follows it in the original: behind
   " files" that is " found.", and behind " found." the first seven bytes
   of the title of an error's box, " Error ", without its zero byte. So
   each is written as long as what is copied. */
const u8 FnFl6_S[15] = " files\0 found.";
const u8 FnFl7_S[15] = " found.\0 Error ";

/* The colours of the dialog, and those of the menu of a search that is
   over. */
static const u8 Find_P[] = { Conf_C, CnfRe_C, CnfBo_C, CnfUn_C };
const u8 FnFl_P[] = { Conf_C, CnfRe_C, CnfRe_C };

/* The dialog that asks what to look for: the files, and the text in
   them. It has no box of its own; the command draws one, in the colours
   of Find_P. */
const DIALOG(2) Find_B = {
    { Find_P, 0, 0, 1, 0xFF, 32, 0x10 },
    {
        { 1, 1, 1, 0, 0, 0x305, MD_AT(FindStr), NUMBER(LenStr) },
        { 1, 0, 0, 1, 1, 0x505, MD_AT(SrchStr), NUMBER(0x100 + LenStr) }
    },
    0
};

/* The buttons of a search that runs and of one that is over: of each its
   width and its column, up to a width of 0. */
const u8 Find5_D[] = {
     8,  0,                             /* Stop */
     9, 11,                             /* Go to */
     8, 23,                             /* Quit */
     0
};
const u8 Find6_D[] = {
    14,  0,                             /* New search */
     9, 17,                             /* Go to */
     8, 29,                             /* Quit */
     0
};
