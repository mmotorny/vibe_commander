/*
 * Volkov Commander in C: how its tables are written.
 *
 * The program has texts, boxes, dialogs and lines of buttons that name one
 * another by address: a box its lines, a dialog its box and its buttons.
 * The addresses are words, offsets in the one segment that the assembly
 * has all of them in, and the routines that show them, DialBox, Dialog,
 * MinMnu, OncZoom, take such an offset and find the rest from there.
 *
 * In C such a thing is a constant like any other, written with the names
 * of what it names, in a file of nothing else: help_t.c has those of
 * help.c. The compiler puts a constant into its group of data, DGROUP,
 * so a pointer in it is the offset of what it names in that group: the
 * word that the assembly has there, counted from another place. A file
 * of constants holds no code, and a file of code no constant of this
 * kind: the code has DS on the main data, not on that group, and would
 * read a constant that it names in the wrong segment, without a
 * warning. It declares what it uses as `extern const u8 __far`, and so
 * reads it in the right one. tools/mkexe.c stops the build for an
 * object file that holds both.
 *
 * A constant that only its own file names is static, and one that code
 * or another file names is not. Either has the name that it has in the
 * assembly, and the compiler's name for it will do for the linker.
 *
 * Every object of such a file is held against the original's bytes by
 * the test `portable_c-data` (harness/datacheck.py), by its name, and so
 * is what it names.
 */
#ifndef TABLES_H
#define TABLES_H

#include "vc.h"

/* The numbers of the colours, for Color: the places of a palette. vc.h
   has the first five, those of a panel. */
#define KeyNu_C 5                       /* the key bar: a number, */
#define KeyTi_C 6                       /* and what the key does */
#define Menu_C  7                       /* the menu bar, */
#define MnCur_C 8                       /* and its cursor */
#define MnuBx_C 9                       /* a menu: its frame, */
#define MnuTx_C 10                      /* an item, */
#define MnuBr_C 11                      /* its letter, */
#define MnuCu_C 12                      /* the cursor, */
#define MnuCB_C 13                      /* the letter under it, */
#define MnuDr_C 14                      /* an item that cannot be chosen, */
#define MnuMi_C 15                      /* and the cursor on one */
#define Box_C   16                      /* a box, */
#define BoxBr_C 17                      /* what is bright in it, */
#define BoxCu_C 18                      /* and its cursor */
#define Conf_C  19                      /* a dialog, */
#define CnfRe_C 20                      /* reversed, */
#define CnfBo_C 21                      /* bold, */
#define CnfUn_C 22                      /* and underlined */
#define Hist_C  23                      /* the history, */
#define HstCu_C 24                      /* and its cursor */
#define HlBrd_C 25                      /* the frame of the help, */
#define HlpMn_C 26                      /* its buttons, */
#define Help_C  27                      /* its text, */
#define HlpBo_C 28                      /* bold, */
#define HlpRe_C 29                      /* reversed, */
#define HlpUn_C 30                      /* and underlined */
#define Err_C   31                      /* a box of an error, */
#define ErInv_C 32                      /* reversed, */
#define ErBr_C  33                      /* and what is bright in it */
#define Clock_C 34                      /* the clock */
#define Arrow_C 35                      /* the arrows of a panel */
#define Sky1_C  36                      /* the stars of the blanker, */
#define Sky2_C  37                      /* and the other half of them */
#define ShadFl  38                      /* the shadow of a box */

/* Where in the main data something is: the word that the assembly has
   for the name of a field of MainDat. */
#define MD_AT(field) ((u16)&((struct MainDat *)0)->field)

/* Where in the segment of a panel something is, a field of WCBdef: the
   filter of a panel is a dialog that is asked for with that segment. */
#define WCB_AT(field) ((u16)&((struct WCBdef *)0)->field)

/* A word that names something, where it holds a number instead: the
   length of a line that is typed into, say. */
#define NUMBER(number) ((const void *)(number))

#pragma pack(push, 1)

/* A text that a box puts into its lines, where they have Var_S: where it
   is, in the segment of whoever asks for the box, how it is written, 4
   without the spaces at its ends, 2 in capitals, 3 in capitals but for
   the name of a file at its end, and how long it may get: a longer one
   loses its beginning. */
struct BOXTX {
    u16 from;
    u8  how;
    u8  length;
};

/* A box: the numbers of its four colours, how many lines it has and how
   many texts it puts into them, where in the main data what it covers is
   kept, those texts, and its lines: the first is the title, in the frame,
   and 0 is a line across the box. BOX(n) is a box of n lines, BOX1(n) and
   BOX2(n) one that puts in a text or two. */
#define BOX_HEAD u8 colors[4]; u8 lines; u8 texts; u16 saved
#define BOX(count) \
    struct { BOX_HEAD; const u8 *line[count]; }
#define BOX1(count) \
    struct { BOX_HEAD; struct BOXTX text[1]; const u8 *line[count]; }
#define BOX2(count) \
    struct { BOX_HEAD; struct BOXTX text[2]; const u8 *line[count]; }

/* The routine that a dialog names, for Dialog to call when the dialog is
   answered with Enter or F10. The assembly has the address of the
   routine in the header; the C has one of these numbers there, and
   Dialog of dialog.c calls the function that it stands for. The
   word is still a word. */
#define SUB_F38_10 1                    /* F38_10 of dirfunc.c: what is
                                           deleted, the selected files or
                                           the one under the cursor */
#define SUB_M30_60 2                    /* M30_60 of dirfunc.c: whether
                                           the date and the time that
                                           were typed are one */
#define SUB_F35_60 3                    /* F35_61 of copy.c, the tree to
                                           choose a target from, for a
                                           dialog whose line for it is
                                           two rows further down, */
#define SUB_F35_61 4                    /* and for one where it is not */

/* The header of a dialog: BOXHD of the assembly. */
struct BOXHD {
    const void *pDiBox;                 /* its box */
    u16 BoxBase;                        /* where the middle of its top is */
    u16 SubFunc;                        /* the routine for Enter and F10,
                                           a SUB_ number, or 0 */
    u8  bKeyBar;                        /* the key bar to show */
    u8  bMenu;                          /* the item of the menu bar, or FFh */
    u8  bHelp;                          /* the page of the help, or 0 */
    u8  bFlags;
};

/* A field of a dialog: BOXMN of the assembly. See dialog.c for the
   kinds and for what mPoint and mCtrl are in each. */
struct BOXMN {
    u8  mType;
    u8  mUp;                            /* the fields that Up, Down, Left */
    u8  mDown;                          /* and Right go to */
    u8  mLeft;
    u8  mRight;
    u16 mBase;                          /* where it is in the box */
    u16 mPoint;                         /* what it sets */
    const void *mCtrl;                  /* buttons, or a NUMBER */
};

/* A dialog of n fields: the header, the fields and a byte 0. */
#define DIALOG(count) \
    struct { struct BOXHD head; struct BOXMN field[count]; u8 end; }

#pragma pack(pop)

LAID_OUT(BOXTX, 4);
LAID_OUT(BOXHD, 10);
LAID_OUT(BOXMN, 11);

#endif
