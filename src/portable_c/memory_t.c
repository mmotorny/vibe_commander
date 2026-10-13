/*
 * The tables of the map of the memory: what memory.c hands to
 * MinMnu by its offset.
 *
 * From VCMEM.INC.
 */
#include "tables.h"

/* What is not static here, memory.c names. */

/* The colours of the buttons, and the buttons: of each its width and its
   column, up to a width of 0. */
const u8 Memo1_P[] = { Conf_C, CnfRe_C, CnfRe_C };
const u8 Memo_D[] = {
    11,  0,                             /* Release */
    10, 14,                             /* Cancel */
     0
};
