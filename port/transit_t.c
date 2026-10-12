/*
 * The first of the part of the program that goes while a command runs:
 * room for the header that DOS keeps in front of a block of memory, a
 * paragraph. That part is a block of its own, which the program makes
 * by writing the header (Alloc0 of port/resident.c): the original has
 * it loaded over that paragraph and moves it up.
 *
 * The build links this file first of that part, so the linker puts it
 * where the part that stays ends, which is on no paragraph of its own.
 * It is two paragraphs long for that: 32 bytes hold a whole paragraph
 * wherever they begin, and tools/mkexe.c has the part that goes begin
 * behind that one. It is all zeros, and none of it is read.
 */
#include "tables.h"

const u8 TrnBeg[32] = { 0 };
#pragma aux TrnBeg "*"
