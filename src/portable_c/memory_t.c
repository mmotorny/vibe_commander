/*
 * The tables of the map of the memory: what src/c/memory.c hands to
 * MinMnu by its offset.
 *
 * From VCMEM.INC.
 */
#include "tables.h"

/* What others name, src/c/memory.c here, has the name that it has in the
   assembly. */
#pragma aux Memo1_P "*"
#pragma aux Memo_D "*"

/* The colours of the buttons, and the buttons: of each its width and its
   column, up to a width of 0. */
const u8 Memo1_P[] = { Conf_C, CnfRe_C, CnfRe_C };
const u8 Memo_D[] = {
    11,  0,                             /* Release */
    10, 14,                             /* Cancel */
     0
};
