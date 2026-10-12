/*
 * What is done to files one after the other: deleting them, and setting
 * their attributes, date and time; and DirFunc, which goes through the
 * files of a directory for these and for the other commands that do.
 * With them the commands that ask for it: F8, Shift-F8 and Ctrl-F8, the
 * attributes, and those that stand next to them in the assembly, F7 and
 * Shift-F7, which make a directory, and the volume label.
 *
 * Translated from VCF1.INC.
 *
 * A command that works on files puts the routine that does its work on
 * one of them into FncAddr. The assembly then calls it for every selected
 * file of a panel, FncSel, or for every file that fits a mask, FncMas,
 * with the name at the start of TmpBuf and the file's attributes. Such a
 * routine answers with 1 if it has done its work, 0 if it has not, and
 * with 100h, which is something in the high byte, to stop the command.
 * For a directory it calls DirFunc, which calls the routine again for
 * every file of that directory, with the directory's path in front of the
 * name: so the commands go as deep as the directories do. F38_40 deletes,
 * M30_50 sets attributes; the routines of the other commands are in
 * other files, and DirFunc serves them too.
 *
 * A command is entered by a jump in the original, from the table of the
 * keys or from a menu, and jumps on when it is done: back to the panels,
 * to FncSel, or to where the panels are read again. Its function here
 * returns which, one of the TO_ of vc.h, and the main loop goes on from
 * that: F1Next of main.c.
 * FncMas and FncSel are here, and FncExt, where such a command ends: it
 * has the panels read again if TmpBuf has something at 91.
 *
 * Behind the name TmpBuf has what a command keeps while it runs. For
 * deleting: at 90, not 0 once "All" was the answer for a directory that
 * is not empty, so that the next one is not asked about; at 91, not 0
 * once something on the disk has changed, for the panels to be read
 * again; at 92, not 0 once the tree's file has been marked as out of
 * date. For attributes: at 94 the bits to keep and at 95 the bits to set,
 * at 96 the date and at 98 the time, each 0FFFFh if it is to stay.
 */
#include "vc.h"

u16 F38_10(u16 answer);
u16 F38_40(u16 attributes);
static u16 F38_70(void);
u16 M30_50(u16 attributes);
u32 M30_60(u16 answer);
static u32 F38_60(void);
static u16 FncMs1(u16 include);

/* The registers of these are those that their stubs had in the assembly,
   when it called them. DirFunc and DelTr0 are in vc.h. */
#pragma aux F38_10  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux F38_40  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M30_50  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M30_60  "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
/* The commands, with FncMas, FncSel and FncExt, are declared in vc.h:
   see "The commands" there. */

/* Routines of the rest of the program that this file calls. Those that
   are declared below are still assembly, and are called by their entries
   for C; the others are C, of vc.h.

   OncZoom puts a box on the screen with its middle at `place`, zooming it
   only for the first box of a command: `box` is where the assembly
   describes it, an offset in its code segment, and `include` the text that
   ^I stands for in a line of the box, another such offset. Dialog, which
   is C, does that and takes the keys; the low byte of what it returns is
   the number of the answer, 0 for the first, and more than any for Esc:
   here always for a box that is not on the screen yet, with what it sets
   in the main data. ResWin puts back in the screen buffer what a box
   covers, and Window, which is in vc.h, shows it. ChgDisk asks for the
   disk of a drive that shares the floppy drive with another, and is not 0
   if the user would not change it.

   Intr21f is Intr21 with the carry flag set before DOS is called if
   `carry` is not 0, and clear if it is. HidCur puts the cursor below the
   screen. */

/* C routines of other files that vc.h does not have. */
const u8 __far *FndLet(const u8 __far *text);           /* string.c */
const u8 __far *TxtNum(const u8 __far *text, u16 STACK *number); /* digits.c */
void NormStr(u8 __far *text);                           /* string.c */
u8 __far *UnConv(u8 __far *name, const u8 __far *buffer);   /* mask.c */
u32 TotSel(u16 panel);                                  /* files.c */
void PutFls(u16 panel);                                 /* panel.c */
void PutTre(u16 panel);                                 /* tree.c */
u8 *FilDir(u8 *buffer, u16 directories, u16 files);     /* panel.c */
u32 GetVol(u8 *volume);                                 /* read.c */
struct DateText {                                       /* format.c */
    u16 century;
    u16 first, second, third;
};
struct TimeText {
    u16 hour, minute, second;
    u8  unused;
    u8  suffix;
};
void PutDate(u16 date, struct DateText STACK *text);
u8 *BufDate(const struct DateText STACK *text, u8 *buffer);
void PutTime(u16 time, struct TimeText STACK *text);
u8 *BufTime(const struct TimeText STACK *text, u8 *buffer);
#pragma aux FndLet  "*_c" __far parm [es si] value [es si] modify exact [si es]
#pragma aux TxtNum  "*_c" __far parm [es si] [bx] value [es si] modify exact [si es]
#pragma aux NormStr "*_c" __far parm [es si] modify exact [ax]
#pragma aux UnConv  "*_c" __far parm [es di] [dx si] value [es di] modify exact [di es]
#pragma aux TotSel  "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux PutFls  "*_c" __far parm [ax] modify exact [ax]
#pragma aux PutTre  "*_c" __far parm [ax] modify exact [ax]
#pragma aux FilDir  "*_c" __far parm [di] [bx] [dx] value [di] modify exact [di]
#pragma aux GetVol  "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux PutDate "*_c" __far parm [ax] [bx] modify exact [ax bx cx dx]
#pragma aux BufDate "*_c" __far parm [bx] [di] value [di] modify exact [ax bx di]
#pragma aux PutTime "*_c" __far parm [ax] [bx] modify exact [ax bx cx dx]
#pragma aux BufTime "*_c" __far parm [bx] [di] value [di] modify exact [ax bx di]

/* Boxes and texts, constants of port/dirfunc_t.c but for Empty_S, which
   OncZoom and Dialog take by their offsets in the segment that the
   constants are in: only those are used here. */
extern const u8 __far DelFl_P[];        /* "Deleting the file" */
extern const u8 __far DelDr_P[];        /* "Deleting the subdirectory" */
extern const u8 __far DelCo_B[];        /* "You are DELETING ... from" */
extern const u8 __far RdOnl_B[];        /* "... is marked read-only." */
extern const u8 __far NoEmp_B[];        /* "... is not empty." */
extern const u8 __far DelEr_B[];        /* "Can't delete the file" */
extern const u8 __far CntDi_B[];        /* "Can't delete the subdirectory" */
extern const u8 __far StAtr_P[];        /* "Setting attributes:" */
extern const u8 __far AtrEr_B[];        /* a line and the file's name */
extern const u8 __far NoDat_B[];        /* "Illegal date or time." */
extern const u8 __far PthEr_B[];        /* "Path too long" */
extern const u8 __far Del_S[];          /* " Delete " */
extern const u8 __far DelFs_S[];        /* "s" */
extern const u8 __far Empty_S[];        /* nothing */
extern const u8 __far NoFnd_S[];        /* "Can't find the file" */
extern const u8 __far NoAtr_S[];        /* "Can't set attributes" */
#pragma aux DelFl_P "*"
#pragma aux DelDr_P "*"
#pragma aux DelCo_B "*"
#pragma aux RdOnl_B "*"
#pragma aux NoEmp_B "*"
#pragma aux DelEr_B "*"
#pragma aux CntDi_B "*"
#pragma aux StAtr_P "*"
#pragma aux AtrEr_B "*"
#pragma aux NoDat_B "*"
#pragma aux PthEr_B "*"
#pragma aux Del_S "*"
#pragma aux DelFs_S "*"
#pragma aux Empty_S "*"
#pragma aux NoFnd_S "*"
#pragma aux NoAtr_S "*"
/* Those of the commands. DlWr4_S is copied with what follows it there,
   81 bytes in all. */
extern const u8 __far MkDi1_B[];        /* "Create the directory", a path */
extern const u8 __far MkDi2_B[];        /* the same, a name: in the tree */
extern const u8 __far MkErr_B[];        /* "Can't create the directory" */
extern const u8 __far CrDir_S[];        /* "Create the directory" */
extern const u8 __far MkDir_S[];        /* " Make directory " */
extern const u8 __far DelDi_B[];        /* "Do you wish to delete" a */
extern const u8 __far Subdi_S[];        /* " the directory" */
extern const u8 __far DelFi_B[];        /* "Do you wish to delete" a file */
extern const u8 __far DelSe_B[];        /* "You have selected" */
extern const u8 __far DlMas_B[];        /* "Delete the file:" */
extern const u8 __far DlWrn_B[];        /* "WARNING!" */
extern const u8 __far DlWr4_S[];        /* "in the current directory" */
extern const u8 __far StAt1_B[];        /* the attributes of a file */
extern const u8 __far StAt2_B[];        /* those to set and to clear */
extern const u8 __far Attr8_S[];        /* " for" */
extern const u8 __far Label_B[];        /* "Change volume label ... to" */
extern const u8 __far DelVo_B[];        /* "Do you wish to delete" it */
extern const u8 __far ErVol_B[];        /* "Could not set" it */
extern const u8 __far NoFnc_B[];        /* "Can't find file" */
#pragma aux MkDi1_B "*"
#pragma aux MkDi2_B "*"
#pragma aux MkErr_B "*"
#pragma aux CrDir_S "*"
#pragma aux MkDir_S "*"
#pragma aux DelDi_B "*"
#pragma aux Subdi_S "*"
#pragma aux DelFi_B "*"
#pragma aux DelSe_B "*"
#pragma aux DlMas_B "*"
#pragma aux DlWrn_B "*"
#pragma aux DlWr4_S "*"
#pragma aux StAt1_B "*"
#pragma aux StAt2_B "*"
#pragma aux Attr8_S "*"
#pragma aux Label_B "*"
#pragma aux DelVo_B "*"
#pragma aux ErVol_B "*"
#pragma aux NoFnc_B "*"

/* What FncAddr can name, by the numbers of vc.h: a routine that is called
   with the attributes of a file in AL and answers in AX. The original
   has the address of the routine in FncAddr and calls that; here the
   routines are a table of functions, all of one kind, and FncAddr has the
   place in it. Those of other files: C00_00 of copy.c, DiSi10 of read.c
   and FindFil of find.c. */
u16 C00_00(u16 attributes);
u16 DiSi10(u16 attributes);
u16 FindFil(u16 attributes);
#pragma aux C00_00  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux DiSi10  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux FindFil "*_c" __far parm [ax] value [ax] modify exact [ax]
typedef u16 __far FncFn(u16 attributes);
#pragma aux FncFn parm [ax] value [ax] modify exact [ax]
static FncFn * const CODE Fnc_T[] = {
    (FncFn *)C00_00, (FncFn *)DiSi10, (FncFn *)FindFil, (FncFn *)F38_40,
    (FncFn *)M30_50
};

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

/* Names in the assembly's code segment. Mask_S is \*.* and what follows
   it there, of which DirFunc copies a byte; Tree_F is \TREEINFO.NCD. */
extern const u8 __far Mask_S[];
extern const u8 __far Tree_F[];
#pragma aux Mask_S "*"
#pragma aux Tree_F "*"

#define LenCmd 124                      /* Length of command line */

/* What a command keeps in TmpBuf. */
#define DEL_ALL    md.TmpBuf[90]
#define FNC_CHANGE md.TmpBuf[91]
#define DEL_TREE   md.TmpBuf[92]
#define ATTR_KEEP  md.TmpBuf[94]
#define ATTR_SET   md.TmpBuf[95]
#define AttrDat    (*(u16 *)(md.TmpBuf + 96))
#define AttrTim    (*(u16 *)(md.TmpBuf + 98))

#define DONE 1                          /* what a routine of FncAddr */
#define NOT_DONE 0                      /* answers with */
#define STOP 0x100

#define BOX_PLACE 0x528                 /* where the boxes have their middle */

/* Errors of DOS, and the one that Intr21 makes of a drive that is not
   ready. */
#define ERR_NO_PATH     0x03
#define ERR_NO_ACCESS   0x05
#define ERR_CURRENT_DIR 0x10
#define ERR_NOT_READY   0x53

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))
#define PAIR(high, low) ((u32)(high) << 16 | (low))

/* DX of a call of DOS, where it is not the offset of a name. */
#define DX_OF(regs) (*(u16 STACK *)&(regs).name)

/* The offset in the main data of something there. */
#define NEAR(pointer) ((u8 *)(u16)(u32)(pointer))

/* Take a box off the screen. */
/* Have DOS make an FCB of a name: its function 29h with the low byte of
   `flags` in AL, called without Intr21, as the assembly calls it. Both
   are in the main data. Returns AX as DOS leaves it: the low byte is 1
   if the name is a mask. */
static u16 Fcb29(const u8 *name, u8 *fcb, u16 flags)
{
    struct DosRegs regs;

    regs.ax = 0x2900 | flags & 0xFF;
    regs.name = (const u8 __far *)&md;
    regs.si = (u16)name;
    regs.di = (u16)fcb;
    Int21d((struct DosRegs STACK *)&regs);
    return regs.ax;
}

static void Close(struct Box STACK *box)
{
    ResWin(box->begin, box->size, box->saved);
    Window(box->begin, box->size);
}

/* What the box for selected files, "You have selected ...", has Dialog
   call when it is answered: with Delete, 0, it asks again, "You are
   DELETING ... from", with an answer more, All, which is kept for the
   directories that are not empty. Returns what the first box is to be
   answered with: 0 to delete. */
u16 F38_10(u16 answer)
{
    if ((u8)answer != 0)
        return answer;
    /* None of its lines has ^I: no include line. */
    answer = Dialog(OFFSET(DelCo_B), 0, 0, 0, MAIN_SEG);
    if ((u8)answer > 1)
        return answer;
    DEL_ALL = (u8)answer;
    return answer & 0xFF00;
}

/* Delete the file or the directory at TmpBuf, by the attributes it has.

   A read-only file is asked about, and deleted if the answer is Delete.
   A directory that is not empty is asked about, unless All was the
   answer once, and then its files are deleted, by F38_70 those that go
   at one call of DOS and by DirFunc, which calls this routine for each,
   what is left and the directories. The first directory that a command
   deletes has the tree's file marked as out of date.

   Ported as it is: Cancel for a directory that is not empty leaves that
   one and goes on with the next, where for a read-only file it stops
   the command. */
u16 F38_40(u16 attributes)
{
    struct DosRegs regs;
    struct Box box;
    u8 *end;
    u32 removed;
    u16 answer, error, report, count;

    if (!(attributes & ATTR_DIRECTORY)) {
        OncZoom(BOX_PLACE, OFFSET(DelFl_P), OFFSET(Empty_S),
                (struct Box STACK *)&box);
        if (ChgDisk(md.TmpBuf))
            goto stop;
        regs.name = md.TmpBuf;
        regs.ax = 0x4100;
        if (!Intr21((struct DosRegs STACK *)&regs))
            goto done;
        if (regs.ax == ERR_NO_ACCESS) {
            regs.ax = 0x4300;
            if (!Intr21((struct DosRegs STACK *)&regs)) {
                /* What is reported if it is not read-only: not the error
                   of a drive that is not ready, whatever DOS left. */
                regs.ax &= 0xFF00;
                if (regs.cx & 1) {
                    answer = (u8)Dialog(OFFSET(RdOnl_B), OFFSET(Del_S),
                                        0, 0, MAIN_SEG);
                    if (answer == 1)
                        goto not_done;
                    if (answer > 1)
                        goto stop;
                    regs.cx = 0x20;
                    regs.ax = 0x4301;
                    if (!Intr21((struct DosRegs STACK *)&regs)) {
                        FNC_CHANGE = 1;
                        regs.ax = 0x4100;
                        if (!Intr21((struct DosRegs STACK *)&regs))
                            goto done;
                    }
                }
            }
        }
        /* F38_42 */
        for (count = 0; count < LenPath + 13; count++)
            md.TmpBuf[130 + count] = md.TmpBuf[count];
        error = regs.ax;
        report = OFFSET(DelEr_B);
        goto failed;
    }

    /* F38_50 */
    OncZoom(BOX_PLACE, OFFSET(DelDr_P), OFFSET(Del_S),
            (struct Box STACK *)&box);
    if (ChgDisk(md.TmpBuf))
        goto stop;
    removed = F38_60();
    if (HIGH(removed) != 0) {
        error = LOW(removed);
        if (error < 100 && (u8)error != ERR_NO_ACCESS) {
            report = OFFSET(CntDi_B);
            goto failed;
        }
        /* F38_51: it is not empty */
        if (DEL_ALL == 0) {
            answer = (u8)Dialog(OFFSET(NoEmp_B), OFFSET(Del_S), 0, 0,
                                MAIN_SEG);
            if (answer > 1)
                goto not_done;
            if (answer == 1)
                DEL_ALL = 1;
        }
        /* F38_52 */
        Close((struct Box STACK *)&box);
        end = md.TmpBuf;
        for (count = LenPath + 13; count != 0; count--)
            if (*end++ == 0)
                break;
        end--;
        for (count = 0; count < 5; count++)
            end[count] = Mask_S[count];
        answer = F38_70();
        *end = 0;
        if (answer >> 8)
            goto out;
        answer = DirFunc();
        if (answer >> 8)
            goto out;
        OncZoom(BOX_PLACE, OFFSET(DelDr_P), OFFSET(Del_S),
                (struct Box STACK *)&box);
        removed = F38_60();
        if (HIGH(removed) != 0) {
            if (LOW(removed) == ERR_NOT_READY)
                goto stop;
            goto not_done;
        }
    }
    /* F38_54 */
    FNC_CHANGE = 1;
    if (DEL_TREE == 0) {
        DEL_TREE = 1;
        /* DelTree */
        if (md.ChgTree != 0) {
            removed = DelTr0(md.TmpBuf);
            if (HIGH(removed) != 0 && LOW(removed) == ERR_NOT_READY)
                goto stop;
        }
    }
done:
    answer = DONE;
    goto close;
failed:
    /* F38_43 */
    if (error != ERR_NOT_READY
        && (u8)Dialog(report, OFFSET(Del_S), 0, 0, MAIN_SEG) == 0) {
not_done:
        answer = NOT_DONE;
        goto close;
    }
stop:
    answer = STOP;
close:
    Close((struct Box STACK *)&box);
out:
    FNC_CHANGE |= (u8)answer;
    return answer;
}

/* Remove the directory at TmpBuf. If it is the current one, its path is
   made a full one, in TmpBuf too, the directory above is made the
   current one of the current drive, and it is tried again. Returns what
   DOS answered in the low word, and something that is not 0 in the high
   one if it was an error. */
static u32 F38_60(void)
{
    struct DosRegs regs;
    u16 count;

    regs.name = md.TmpBuf;
    regs.ax = 0x3A00;
    if (!Intr21((struct DosRegs STACK *)&regs))
        return regs.ax;
    if (regs.ax != ERR_CURRENT_DIR)
        return PAIR(1, regs.ax);
    regs.si = (u16)md.TmpBuf;
    regs.di = (u16)md.TmpErr;
    regs.ax = 0x6000;
    if (Intr21((struct DosRegs STACK *)&regs))
        return PAIR(1, regs.ax);
    for (count = 0; count < LenPath + 13; count++)
        md.TmpBuf[count] = md.TmpErr[count];
    md.TmpErr[0] = '.';
    md.TmpErr[1] = '.';
    md.TmpErr[2] = 0;
    regs.name = md.TmpErr;
    regs.ax = 0x3B00;
    if (Intr21((struct DosRegs STACK *)&regs))
        return PAIR(1, regs.ax);
    regs.name = md.TmpBuf;
    regs.ax = 0x3A00;
    if (Intr21((struct DosRegs STACK *)&regs))
        return PAIR(1, regs.ax);
    return regs.ax;
}

/* Delete the files that the mask at TmpBuf names, at one call of DOS:
   by an FCB, which is made at TmpBuf + 200 and takes hidden and system
   files too. An FCB has no path, so the directory of the mask is made
   the current one for the call, and the one that was put back. Returns
   0 if the name is no mask, or if DOS has deleted no file or not all of
   them; 1 if it has deleted them; 100h to stop the command.

   Ported as it is: a mask with a drive and no directory, "c:*.txt",
   always counts as deleted. The call of DOS for an FCB leaves the carry
   flag as it finds it, the assembly has it set by then, of comparing the
   colon with a backslash, and Intr21 takes that for an error of DOS and
   asks which, so the answer of the call is lost. What Intr21 hands back
   instead, one more than which is returned here, is 0 where nothing has
   failed before. So nothing says "Can't find files" for such a mask. */
static u16 F38_70(void)
{
    struct DosRegs regs;
    struct Box box;
    u8 *fcb = md.TmpBuf + 200;
    u8 *back = md.TmpBuf + 100;
    u8 *name, *end;
    u16 answer, drive, carry = 0;
    u8 kept;

    fcb[0] = 0xFF;                      /* an extended one, */
    fcb[6] = 0x26;                      /* for these attributes */
    drive = 'A' - 1;
    if (md.TmpBuf[1] == ':')
        drive = md.TmpBuf[0];
    fcb[7] = (u8)(drive - ('A' - 1));
    name = NEAR(CutPath(md.TmpBuf));
    if ((u8)Fcb29(name, fcb + 7, 2) != 1)
        return NOT_DONE;
    OncZoom(BOX_PLACE, OFFSET(DelFl_P), OFFSET(DelFs_S),
            (struct Box STACK *)&box);
    if (ChgDisk(md.TmpBuf))
        goto stop;
    *back = 0;
    if (name > md.TmpBuf) {
        if (name[-1] != '\\') {
            carry = name[-1] < '\\';
        } else {
            drive = fcb[7];
            if (drive != 0) {
                *back++ = (u8)(drive + 'A' - 1);
                *back++ = ':';
            }
            *back++ = '\\';
            regs.name = md.TmpBuf;
            DX_OF(regs) = drive;
            regs.si = (u16)back;
            regs.ax = 0x4700;
            if (Intr21((struct DosRegs STACK *)&regs))
                goto failed;
            end = name;
            if (end > md.TmpBuf + 1 && end[-2] != ':')
                end--;
            kept = *end;
            *end = 0;
            regs.name = md.TmpBuf;
            regs.ax = 0x3B00;
            answer = Intr21((struct DosRegs STACK *)&regs);
            *end = kept;
            if (answer) {
failed:
                /* F38_75 */
                if (regs.ax == ERR_NOT_READY)
                    goto stop;
                answer = NOT_DONE;
                goto close;
            }
        }
    }
    /* F38_77 */
    FNC_CHANGE = 1;
    regs.name = fcb;
    regs.ax = 0x1300;
    Intr21f((struct DosRegs STACK *)&regs, carry);
    answer = regs.ax;
    if (md.TmpBuf[100] != 0) {
        regs.name = md.TmpBuf + 100;
        regs.ax = 0x3B00;
        if (Intr21((struct DosRegs STACK *)&regs))
            goto stop;
    }
    /* What DOS left in AL: 0 for deleted, 0FFh for not. One more, and
       that with its sign. */
    answer = (u16)(signed char)(u8)(answer + 1);
    goto close;
stop:
    answer = STOP;
close:
    Close((struct Box STACK *)&box);
    return answer;
}

/* Set the attributes of the file at TmpBuf, and its date and time if
   either is to change, for which the file is opened; read-only is taken
   off first. For a directory, do it to what is in it.

   Ported as it is: DOS is not asked whether it could read the date and
   the time of the file, or set them. */
u16 M30_50(u16 attributes)
{
    struct DosRegs regs;
    struct Box box;
    u32 opened;
    u16 answer, old, include;

    if (attributes & ATTR_DIRECTORY)
        return DirFunc();
    /* None of its lines has ^I: no include line. */
    OncZoom(BOX_PLACE, OFFSET(StAtr_P), 0, (struct Box STACK *)&box);
    if (ChgDisk(md.TmpBuf))
        goto stop;
    regs.name = md.TmpBuf;
    regs.ax = 0x4300;
    if (Intr21((struct DosRegs STACK *)&regs))
        goto failed;
    old = regs.cx;
    if ((AttrDat & AttrTim) != 0xFFFF) {
        regs.cx = old & 0xFFFE;
        regs.ax = 0x4301;
        if (Intr21((struct DosRegs STACK *)&regs))
            goto failed;
        FNC_CHANGE = 1;
        opened = OpenFil(md.TmpBuf, 0x41);
        regs.ax = LOW(opened);
        if (HIGH(opened) != 0)
            goto failed;
        regs.bx = LOW(opened);
        regs.ax = 0x5700;
        Intr21((struct DosRegs STACK *)&regs);
        if (AttrDat != 0xFFFF)
            DX_OF(regs) = AttrDat;
        if (AttrTim != 0xFFFF)
            regs.cx = AttrTim;
        regs.ax = 0x5701;
        Intr21((struct DosRegs STACK *)&regs);
        regs.name = md.TmpBuf;
        regs.ax = 0x3E00;
        if (Intr21((struct DosRegs STACK *)&regs))
            goto failed;
    }
    /* M30_54 */
    regs.cx = old & 0xFF00 | (u8)old & ATTR_KEEP | ATTR_SET;
    regs.ax = 0x4301;
    if (!Intr21((struct DosRegs STACK *)&regs)) {
        answer = DONE;
        goto close;
    }
failed:
    /* M30_56 */
    if (regs.ax != ERR_NOT_READY) {
        include = OFFSET(NoAtr_S);
        if (regs.ax <= ERR_NO_PATH)
            include = OFFSET(NoFnd_S);
        if ((u8)Dialog(OFFSET(AtrEr_B), include, 0, 0, MAIN_SEG) == 0) {
            answer = NOT_DONE;
            goto close;
        }
    }
stop:
    answer = STOP;
close:
    FNC_CHANGE |= (u8)answer;
    Close((struct Box STACK *)&box);
    return answer;
}

/* A number in a date or a time, and what follows the spaces behind it.
   Returns 0 if there is no digit. */
static u16 Figure(const u8 * STACK *text, u16 STACK *number)
{
    const u8 *from = *text;

    *text = NEAR(TxtNum(from, number));
    if (*text == from)
        return 0;
    *text = NEAR(FndLet(*text));
    return 1;
}

/* What the dialog of attributes has Dialog call when it is answered:
   with Set, 0, the date and the time are read from the two lines that
   were typed, and put where M30_50 takes them, AttrDat and AttrTim. A
   line that is empty leaves 0FFFFh there: not to be changed. Returns
   `answer` in the low word; and if a line is no date or no time, which a
   box says, what that was answered with there and something that is not
   0 in the high word, for the dialog to go on.

   A date is three numbers with the country's separator between them, in
   the country's order. A year below 100 is one from 1980 to 2079. A time
   is hours, and minutes and seconds if there are any, with the country's
   separator, and a letter behind it or not: A or P, for which the hours
   go up to 12, and 12 is 0. Without one they go up to 24, and 24 is 0.

   Ported as it is: a time that has only hours and one separator, "5:",
   is refused, and so is anything behind the letter, but spaces. */
u32 M30_60(u16 answer)
{
    const u8 *text;
    u16 month, day, year, days, swap;
    u16 hours, minutes = 0, seconds = 0, add = 0, limit;
    u8 letter = 0;

    if ((u8)answer != 0)
        return answer;
    AttrDat = 0xFFFF;
    AttrTim = 0xFFFF;
    text = NEAR(FndLet(md.DosBuf + LenCmd + 10 + 3));
    if (*text != 0) {
        if (!Figure((const u8 * STACK *)&text, (u16 STACK *)&month))
            goto refused;
        if (*text != md.Country.DateSep[0])
            goto refused;
        text = NEAR(FndLet(text + 1));
        if (!Figure((const u8 * STACK *)&text, (u16 STACK *)&day))
            goto refused;
        if (*text != md.Country.DateSep[0])
            goto refused;
        text = NEAR(FndLet(text + 1));
        if (!Figure((const u8 * STACK *)&text, (u16 STACK *)&year))
            goto refused;
        if (*text != 0)
            goto refused;
        if (md.Country.DateFmt != 0) {
            swap = month;
            month = day;
            day = swap;
            if (md.Country.DateFmt != 1) {
                swap = year;
                year = day;
                day = swap;
            }
        }
        /* M30_63 */
        if (year >= 1980) {
            year -= 1980;
        } else {
            if (year >= 100)
                goto refused;
            if (year >= 80)
                year -= 80;
            else
                year += 20;
        }
        if (year >= 128 || month > 12 || month < 1)
            goto refused;
        days = month;
        if (days > 7)
            days++;
        days = (days & 1) + 30;
        if (month == 2) {
            days = 28;
            if ((year & 3) == 0)
                days++;
        }
        if (day > days || day == 0)
            goto refused;
        AttrDat = (year << 9) + (month << 5) + day;
    }

    /* M30_70 */
    text = NEAR(FndLet(md.DosBuf + LenCmd + 10 + 100 + 3));
    if (*text == 0)
        return 0;
    if (!Figure((const u8 * STACK *)&text, (u16 STACK *)&hours))
        goto refused;
    if (*text == md.Country.TimeSep[0]) {
        text = NEAR(FndLet(text + 1));
        if (!Figure((const u8 * STACK *)&text, (u16 STACK *)&minutes))
            goto refused;
        if (minutes >= 60)
            goto refused;
        if (*text == md.Country.TimeSep[0]) {
            text = NEAR(FndLet(text + 1));
            if (!Figure((const u8 * STACK *)&text, (u16 STACK *)&seconds))
                goto refused;
            if (seconds >= 60)
                goto refused;
        }
    }
    /* M30_71 */
    if (*text != 0) {
        letter = (u8)UpCase(*text);
        text = NEAR(FndLet(text + 1));
        if (*text != 0)
            goto refused;
    }
    limit = 24;
    if (letter != 0) {
        limit = 12;
        if (letter != 'A') {
            add = 12;
            if (letter != 'P')
                goto refused;
        }
    }
    if (hours > limit)
        goto refused;
    if (hours == limit)
        hours = 0;
    AttrTim = ((hours + add) << 11) + (minutes << 5) + (seconds >> 1);
    return 0;

refused:
    /* None of its lines has ^I: no include line. */
    return PAIR(1, Dialog(OFFSET(NoDat_B), 0, 0, 0, MAIN_SEG));
}

/* Call what FncAddr names, for a file with these attributes at TmpBuf. */
u16 FncCall(u16 attributes)
{
    return Fnc_T[md.FncAddr](attributes);
}

/* Call what FncAddr names for every file and directory of the directory
   at TmpBuf, but "." and "..", with its name behind the path there.
   Returns 1 if every call answered with something in the low byte, 0 if
   one did not, and 100h to stop: if a call said so, if Esc was pressed,
   or if a drive is not ready.

   The routine that is called may call DirFunc again, for a directory,
   and DOS then no longer knows where it was in this one: there is one
   DTA. So a search that has ended looks, in the directory above, for the
   directory it has gone through, by its name, which leaves DOS where the
   search above goes on from. Not for the root of a drive.

   A path of 68 characters or more is not gone through, which a box
   says, but for the sizes of directories, which are then too small.

   Ported as they are: the path "\" answers with 100h when its files
   have been gone through, and so does any path whose directory is not
   found in the one above. With the mask five bytes are copied, one more
   than it has. */
u16 DirFunc(void)
{
    struct DosRegs regs;
    u8 *end, *name;
    u16 count, length, answer, skip;
    u16 all = 1;
    u8 c;

    end = md.TmpBuf;
    for (count = LenPath; ; count--) {
        if (count == 0) {
            if (md.FncAddr != FNC_DISI10)
                /* None of its lines has ^I: no include line. */
                Dialog(OFFSET(PthEr_B), 0, 0, 0, MAIN_SEG);
            return 0;
        }
        if (*end++ == 0)
            break;
    }
    end--;
    skip = 1;                           /* "*.*", */
    if (end != md.TmpBuf && end[-1] != ':' && end[-1] != '\\')
        skip = 0;                       /* or with the backslash */
    for (count = 0; count < 5; count++)
        end[count] = Mask_S[skip + count];
    SetDTA();
    regs.cx = 0x37;
    regs.ax = 0x4E00;
    for (;;) {
        /* DiFu05 */
        if (ChgDisk(md.TmpBuf))
            return STOP;
        regs.name = md.TmpBuf;
        if (Intr21((struct DosRegs STACK *)&regs)) {
            if (regs.ax == ERR_NOT_READY)
                return STOP;
            break;
        }
        /* DiFu03 */
        if (md.DTA.FilName[0] != '.') {
            name = NEAR(CutPath(md.TmpBuf));
            for (count = 0; count < 13; count++)
                name[count] = md.DTA.FilName[count];
            if (TstBrk())
                return STOP;
            /* The high byte goes as DOS left it: C00_00 keeps all of AX. */
            answer = FncCall(regs.ax & 0xFF00 | md.DTA.FilAttr);
            if (answer >> 8)
                return STOP;
            if ((u8)answer == 0)
                all = 0;
        }
        regs.ax = 0x4F00;
    }

    /* DiFu07: the directory itself, in the one above */
    name = NEAR(CutPath(md.TmpBuf));
    if (name <= md.TmpBuf)
        return STOP;
    c = name[-1];
    if (c == '\\') {
        name--;
        if (name <= md.TmpBuf)
            return STOP;
        *name = 0;
        c = name[-1];
    }
    if (c == ':')
        return all;
    name = NEAR(CutPath(md.TmpBuf));
    for (count = 0; count < 13; count++)
        md.MenuBuf[count] = name[count];
    for (count = 0; count < 4; count++)
        name[count] = Mask_S[1 + count];
    if (ChgDisk(md.TmpBuf))
        return STOP;
    regs.cx = 0x37;
    regs.ax = 0x4E00;
    for (;;) {
        regs.name = md.TmpBuf;
        if (Intr21((struct DosRegs STACK *)&regs))
            return STOP;
        if (md.DTA.FilAttr & ATTR_DIRECTORY) {
            /* The name with the zero that ends it, 13 bytes at the
               most. */
            for (length = 0; length < 13; )
                if (md.MenuBuf[length++] == 0)
                    break;
            for (count = 0; count < length; count++)
                if (md.MenuBuf[count] != md.DTA.FilName[count])
                    break;
            if (count == length)
                break;
        }
        regs.ax = 0x4F00;
    }
    /* DiFu12 */
    name = NEAR(CutPath(md.TmpBuf));
    for (count = 0; count < 13; count++)
        name[count] = md.MenuBuf[count];
    return all;
}

/* Mark the tree's file of a drive as out of date: its first byte is made
   0. The drive is that of `path`, or the current one if it names none.
   Returns what DOS answered in the low word, and something that is not 0
   in the high one if the file could not be opened, written or closed. A
   file that may not be written, and a disk that is full, are a beep and
   no error.

   This is DelTree of the assembly from its label DelTr0 on, which ^R in
   a tree and the tree in a box call, the box from C. DelTree itself does nothing unless
   the tree is read again by itself, ChgTree: its stub sees to that, and
   F38_40 here.

   Ported as they are. When the file cannot be closed, what is returned
   for the error is what writing answered, the 1 byte written. And ^R in
   a tree takes what is returned without an error for the kind of the
   panel, 1 for a tree: that is the 1 byte written, and with a file that
   may not be written it is DOS's 5. */
u32 DelTr0(const u8 *path)
{
    struct DosRegs regs;
    u32 opened;
    u16 count, written, failed, carry;

    md.TmpErr[0] = path[1] == ':' ? path[0] : md.DosPth[0];
    md.TmpErr[1] = ':';
    for (count = 0; count < 13 + 1; count++)
        md.TmpErr[2 + count] = Tree_F[count];
    md.BlocEr = 1;
    opened = OpenFil(md.TmpErr, 0x11);
    written = LOW(opened);
    if (HIGH(opened) != 0) {
        carry = 1;
        if (written == ERR_NO_ACCESS) {
            Beep();
            carry = 0;
        }
    } else {
        md.TmpErr[0] = 0;
        regs.bx = written;
        regs.cx = 1;
        regs.name = md.TmpErr;
        regs.ax = 0x4000;
        failed = Intr21((struct DosRegs STACK *)&regs);
        written = regs.ax;
        regs.ax = 0x3E00;
        carry = 1;
        if (!Intr21((struct DosRegs STACK *)&regs) && !failed) {
            carry = 0;
            if (written != regs.cx)
                Beep();
        }
    }
    md.BlocEr = 0;
    return PAIR(carry, written);
}

/* Shift-F7, and F7 where no tree is under the cursor: ask for a path and
   make the directory. A name without a path is added to the list of the
   current panel, as a directory, with the cursor on it, and the panels
   are read again. `box` asks: MkDi1_B for a path, MkDi2_B for a name, of
   12 characters, which is then made in the current directory of the
   tree, and that directory becomes the current one there.

   The name is cut to eight characters and three and put in capitals
   before DOS gets it. What cannot be made is said in a box, but for a
   drive that is not ready, and a name with a path in the tree is not
   tried.

   Ported as it is: the path of the tree's new directory is copied into
   CrnPath from its third byte on, 66 bytes, and the zero behind them
   goes into the byte after CrnPath, the first of ViewFil. And the entry
   is added to the list of the current panel whatever that shows, and
   without asking whether the list has room. */
static u16 F47_01(u16 box)
{
    struct DosRegs regs;
    struct WCBdef __far *wcb;
    struct FILENT __far *entry;
    u8 *path = md.TmpBuf;
    u8 *name, *end;
    u16 i;

    md.TmpBuf[0] = 0;
    if ((u8)Dialog(box, OFFSET(CrDir_S), 0, 0, MAIN_SEG) != 0)
        return TO_FUNC04;
    if (md.TmpBuf[0] == 0)
        return TO_FUNC04;
    name = NEAR(CutPath(md.TmpBuf));
    Convert(name, md.TmpBuf + 100);
    UnConv(name, md.TmpBuf + 100);
    if (box == OFFSET(MkDi2_B)) {
        if (NEAR(CutPath(md.TmpBuf)) != md.TmpBuf)
            goto failed;
        path = md.TmpBuf + 100;
        end = NEAR(MvzLine(path, md.CrnPath + 2, LenPath));
        end = NEAR(AddFile(end));
        for (i = 0; i < 13; i++)
            end[i] = md.TmpBuf[i];
    }
    /* F47_02 */
    if (ChgDisk(path))
        return TO_FUNC04;
    regs.name = path;
    regs.ax = 0x3900;
    if (Intr21((struct DosRegs STACK *)&regs)) {
        if (regs.ax == ERR_NOT_READY)
            return TO_FUNC04;
failed:
        /* F47_03 */
        for (i = 0; i < LenPath + 13; i++)
            md.TmpBuf[130 + i] = md.TmpBuf[i];
        Dialog(OFFSET(MkErr_B), OFFSET(MkDir_S), 0, 0, MAIN_SEG);
        return TO_FUNC04;
    }
    /* F47_05 */
    if (box == OFFSET(MkDi2_B)) {
        for (i = 0; i < LenPath - 2; i++)
            md.CrnPath[2 + i] = path[i];
        md.ViewFil[0] = 0;              /* the byte behind CrnPath */
        PathLin(md.CrnPath + 2);
    } else if (NEAR(CutPath(path)) == path) {
        /* F47_06 */
        PathLin(path);
        wcb = PANEL(md.WCBcrn);
        entry = ENTRY(wcb, wcb->EndBuf);
        wcb->CurAdr = wcb->EndBuf;
        wcb->EndBuf += sizeof(struct FILENT);
        for (i = 0; i < 12; i++)
            entry->NameF[i] = path[i];
        entry->NameF[12] = 0;
        entry->AttrF = ATTR_DIRECTORY;
    }
    /* F47_07: DelTree */
    if (md.ChgTree != 0)
        DelTr0(path);
    return TO_FNCEX1;
}

/* Shift-F7: make a directory, by a path. */
u16 F47_00(void)
{
    return F47_01(OFFSET(MkDi1_B));
}

/* F7: make a directory, in the current directory of the tree if the
   panel shows that. */
u16 F37_00(u16 panel)
{
    const struct WCBdef __far *wcb = PANEL(panel);

    if (wcb->WinTyp == 1 && wcb->Visible != 0)
        return F47_01(OFFSET(MkDi2_B));
    return F47_01(OFFSET(MkDi1_B));
}

/* F8: delete what is selected in the panel, or what the cursor is on if
   nothing is, or if OneFile says so; in a tree the current directory,
   which is then no longer the current one of the tree. It is asked
   about first; FncSel then has F38_40 called for each. For a panel that
   is off or shows neither files nor the tree, this is Shift-F8. */
u16 F38_00(u16 panel)
{
    const struct WCBdef __far *wcb = PANEL(panel);
    const struct FILENT __far *file;
    u8 *end;
    u32 selected;
    u16 box, include, i;

    DEL_ALL = 0;
    FNC_CHANGE = 0;
    DEL_TREE = 0;
    md.FncAddr = FNC_F38_40;
    if (wcb->Visible == 0 || wcb->WinTyp > 1)
        return F48_00();
    if (wcb->WinTyp == 1) {
        if (md.CrnPath[3] == 0)         /* the root */
            return TO_FNCEXT;
        for (i = 0; i < LenPath; i++)
            md.TmpBuf[i] = md.CrnPath[i];
        if ((u8)Dialog(OFFSET(DelDi_B), OFFSET(Subdi_S), 0, 0, MAIN_SEG)
            != 0)
            return TO_FUNC04;
        md.ZoomOne = 0;
        if ((u8)F38_40(ATTR_DIRECTORY) != 0) {
            end = NEAR(CutPath(md.CrnPath));
            if (end > md.CrnPath + 3)
                end--;
            *end = 0;
        }
        return TO_FNCEXT;
    }
    /* F38_04 */
    selected = TotSel(panel);
    if (selected != 0 && md.OneFile == 0) {
        FilDir(md.TmpBuf, LOW(selected), HIGH(selected));
        for (i = 0; i < LenPath; i++)
            md.TmpBuf[100 + i] = wcb->WinPath[i];
        box = OFFSET(DelSe_B);
        /* None of its lines has ^I: the assembly hands over what it
           has there, the number of files selected. */
        include = HIGH(selected);
    } else {
        /* F38_06 */
        if (wcb->CurAdr >= wcb->EndBuf)
            return TO_FNCEXT;
        file = ENTRY(wcb, wcb->CurAdr);
        if (file->NameF[0] == '.')
            return TO_FNCEXT;
        box = OFFSET(DelFi_B);
        include = OFFSET(Empty_S);
        if (file->AttrF & ATTR_DIRECTORY) {
            box = OFFSET(DelDi_B);
            include = OFFSET(Subdi_S);
        }
        for (i = 0; i < 13; i++)
            md.TmpBuf[i] = file->NameF[i];
    }
    /* F38_08 */
    if ((u8)Dialog(box, include, 0, 0, MAIN_SEG) != 0)
        return TO_FUNC04;
    return TO_FNCSEL;
}

/* Shift-F8: ask for a name or a mask, and delete the files it names. A
   mask for every file, "*.*" or a full stop, with a path or without, is
   asked about once more. F38_70 has DOS delete what it deletes at one
   call, and FncMs1 goes through the files that are left, or says that
   there was none. */
u16 F48_00(void)
{
    const u8 __far *from;
    u8 *name;
    u16 i, answer, include;

    /* None of its lines has ^I: no include line. */
    if ((u8)Dialog(OFFSET(DlMas_B), 0, 0, 0, MAIN_SEG) != 0)
        return TO_FUNC04;
    for (i = 0; i < LenStr + 1; i++)
        md.TmpBuf[i] = md.ViewFil[i];
    NormStr(md.TmpBuf);
    LetLin(md.TmpBuf);
    if (md.TmpBuf[0] == 0)
        return TO_FUNC04;
    name = NEAR(CutPath(md.TmpBuf));
    if (*(u16 *)name != '.') {
        /* Every file, if DOS makes eleven question marks of it. */
        if ((u8)Fcb29(name, md.TmpBuf + 100, 0) != 1)
            goto delete;
        for (i = 1; i <= 11; i++)
            if (md.TmpBuf[100 + i] != '?')
                goto delete;
    }
    /* F48_01 */
    for (i = 0; i < 4; i++)
        name[i] = Mask_S[1 + i];
    from = md.TmpBuf;
    if (NEAR(CutPath(md.TmpBuf)) == md.TmpBuf)
        from = DlWr4_S;
    for (i = 0; i < LenPath + 13; i++)
        md.TmpBuf[100 + i] = from[i];
    /* No ^I here either. */
    if ((u8)Dialog(OFFSET(DlWrn_B), 0, 0, 0, MAIN_SEG) != 0)
        return TO_FUNC04;
delete:
    /* F48_04 */
    md.ZoomOne = 0;
    DEL_ALL = 0;
    FNC_CHANGE = 0;
    DEL_TREE = 0;
    md.FncAddr = FNC_F38_40;
    answer = F38_70();
    if (answer >> 8)
        return TO_FNCEXT;
    include = 0;
    if ((u8)answer == 0)
        include = OFFSET(Del_S);
    return FncMs1(include);
}

/* Ctrl-F8: delete the file under the cursor, whatever is selected. */
u16 F58_00(u16 panel)
{
    md.OneFile = 1;
    return F38_00(panel);
}

/* The bits of the attributes that the four boxes at `ticks` have a
   tick in: read only, archive, hidden and system. */
static u16 M30_82(const u8 *ticks)
{
    u16 bits = 0;

    if (ticks[0] != 0)
        bits |= 0x01;
    if (ticks[1] != 0)
        bits |= 0x20;
    if (ticks[2] != 0)
        bits |= 0x02;
    if (ticks[3] != 0)
        bits |= 0x04;
    return bits;
}

/* The attributes: ask for those of the file under the cursor, with its
   date and time, or, for a directory and for what is selected, for those
   to set and those to clear; FncSel then has M30_50 called for each.
   Nothing for a panel that shows no files, and for "..".

   The dialog has its ticks at the start of TmpBuf, those to clear at
   10, the date at 15 and the time at 30, each without the space that it
   begins with where a number has one digit, and the name at 100. */
u16 M30_00(u16 panel)
{
    const struct WCBdef __far *wcb = PANEL(panel);
    const struct FILENT __far *file;
    struct DateText date;
    struct TimeText time;
    u8 *end;
    u16 i, bits, attributes;

    if (!FilePan(panel))
        return TO_FUNC04;
    if (TotSel(panel) == 0) {
        /* M30_03 */
        if (wcb->CurAdr >= wcb->EndBuf)
            return TO_FUNC04;
        file = ENTRY(wcb, wcb->CurAdr);
        if (file->NameF[0] == '.')
            return TO_FUNC04;
        attributes = file->AttrF;
        if (!(attributes & ATTR_DIRECTORY)) {
            md.TmpBuf[0] = (attributes & 0x01) != 0;
            md.TmpBuf[1] = (attributes & 0x20) != 0;
            md.TmpBuf[2] = (attributes & 0x02) != 0;
            md.TmpBuf[3] = (attributes & 0x04) != 0;
            PutDate(file->DateF, (struct DateText STACK *)&date);
            end = md.TmpBuf + 15;
            if ((u8)date.first == ' ')
                end--;
            end = BufDate((struct DateText STACK *)&date, end);
            *end = 0;
            PutTime(file->TimeF, (struct TimeText STACK *)&time);
            end = md.TmpBuf + 30;
            if ((u8)time.hour == ' ')
                end--;
            end = BufTime((struct TimeText STACK *)&time, end);
            if (time.suffix == ' ')
                end--;
            *end = 0;
            for (i = 0; i < 13; i++)
                md.TmpBuf[100 + i] = file->NameF[i];
            if ((u8)Dialog(OFFSET(StAt1_B), OFFSET(Attr8_S), 0, 0, MAIN_SEG)
                != 0)
                return TO_FUNC04;
            bits = M30_82(md.TmpBuf);
            ATTR_KEEP = (u8)bits;
            ATTR_SET = (u8)bits;
            goto set;
        }
    }
    /* M30_01 */
    for (i = 0; i < 100; i++)
        md.TmpBuf[i] = 0;
    if ((u8)Dialog(OFFSET(StAt2_B), OFFSET(Empty_S), 0, 0, MAIN_SEG) != 0)
        return TO_FUNC04;
    ATTR_SET = (u8)M30_82(md.TmpBuf);
    ATTR_KEEP = (u8)~M30_82(md.TmpBuf + 10);
set:
    /* M30_07 */
    md.FncAddr = FNC_M30_50;
    return TO_FNCSEL;
}

/* Ctrl-F4: ask for the volume label of the current drive, and set it.
   Nothing for a drive of a network. A label is an entry of the root with
   an attribute of its own: the one that is there is deleted, through an
   FCB that asks for that attribute, and the new one is made like a
   file, the label as a file name with a full stop behind its eighth
   character. With no label typed the old one is deleted, after asking.

   Ported as it is: the old label is deleted before it is known whether
   the new one can be made. The calls for the FCB leave the carry flag
   as they find it, and the assembly has it clear there, of comparing
   with 0. None of the boxes has ^I in a line, and they get no include
   line here, where the assembly left them what DX held. */
u16 F54_00(void)
{
    struct DosRegs regs;
    u8 *end;
    u16 i;

    HidCur();
    ChgDisk(md.DosPth);
    if (HIGH(GetVol(md.Volume)) == 0)
        return TO_FUNC04;
    md.TmpBuf[0] = '\\';
    for (i = 0; i < 12; i++)
        md.TmpBuf[1 + i] = md.Volume[i];
    md.TmpBuf[20] = 0;
    if (md.Volume[0] != 0) {
        md.TmpBuf[20] = ' ';
        md.TmpBuf[21] = '"';
        end = NEAR(MvzLine(md.TmpBuf + 22, md.Volume, 12));
        end[0] = '"';
        end[1] = 0;
    }
    /* F54_01 */
    if ((u8)Dialog(OFFSET(Label_B), 0, 0, 0, MAIN_SEG) != 0)
        return TO_FUNC04;
    md.TmpBuf[20] = 0xFF;               /* an extended FCB, */
    md.TmpBuf[26] = 8;                  /* for a label, */
    md.TmpBuf[27] = 0;                  /* on the current drive */
    end = NEAR(MvzLine(md.TmpBuf + 28, md.Volume, 12));
    for (i = 0; i < 12; i++)
        end[i] = ' ';
    regs.name = md.TmpBuf + 20;
    if (md.TmpBuf[1] == 0) {
        if (md.Volume[0] == 0)
            return TO_FUNC04;
        if ((u8)Dialog(OFFSET(DelVo_B), 0, 0, 0, MAIN_SEG) != 0)
            return TO_FUNC04;
        regs.ax = 0x1300;
        Intr21f((struct DosRegs STACK *)&regs, 0);
        return TO_FNCEXT;
    }
    /* F54_03 */
    regs.ax = 0x1300 | ' ';
    Intr21f((struct DosRegs STACK *)&regs, 0);
    *(u16 *)(md.TmpBuf + 12) = *(u16 *)(md.TmpBuf + 11);
    *(u16 *)(md.TmpBuf + 10) = *(u16 *)(md.TmpBuf + 9);
    md.TmpBuf[9] = '.';
    regs.name = md.TmpBuf;
    regs.cx = 8;
    regs.ax = 0x3C00;
    if (!Intr21((struct DosRegs STACK *)&regs)) {
        regs.bx = regs.ax;
        regs.ax = 0x3E00;
        Intr21((struct DosRegs STACK *)&regs);
        return TO_FNCEXT;
    }
    /* F54_04 */
    *(u16 *)(md.TmpBuf + 9) = *(u16 *)(md.TmpBuf + 10);
    *(u16 *)(md.TmpBuf + 11) = *(u16 *)(md.TmpBuf + 12);
    Dialog(OFFSET(ErVol_B), 0, 0, 0, MAIN_SEG);
    return TO_FNCEXT;
}

/* Go through the files that the mask at TmpBuf names, but for
   directories and hidden and system files, and call what FncAddr names
   for each, with its name in place of the mask's; a key that TstBrk
   takes, and an answer with something in its high byte, stop that. If
   DOS finds no file at all and `include` is not 0, a box says so,
   "Can't find file", or "files" for a mask, with `include`, the name of
   the command, for its title. Returns where to go on: FncExt. */
static u16 FncMs1(u16 include)
{
    struct DosRegs regs;
    const u8 *text;
    u8 *name;
    u16 i, answer;

    SetDTA();
    regs.cx = 0x37;
    regs.ax = 0x4E00;
    for (;;) {
        /* FncMs4 */
        if (ChgDisk(md.TmpBuf))
            return TO_FNCEXT;
        regs.name = md.TmpBuf;
        if (Intr21((struct DosRegs STACK *)&regs))
            break;
        /* FncMs2 */
        if (!(md.DTA.FilAttr & (ATTR_DIRECTORY | ATTR_SYSTEM | ATTR_HIDDEN))) {
            include = 0;
            name = NEAR(CutPath(md.TmpBuf));
            for (i = 0; i < 13; i++)
                name[i] = md.DTA.FilName[i];
            if (TstBrk())
                return TO_FNCEXT;
            answer = FncCall(0);
            FNC_CHANGE |= (u8)answer;
            if (answer >> 8)
                return TO_FNCEXT;
        }
        regs.ax = 0x4F00;
    }
    if (regs.ax != 0x12 && regs.ax > 3)
        return TO_FNCEXT;
    /* FncMs5 */
    if (include == 0)
        return TO_FNCEXT;
    *(u16 *)(md.TmpBuf + 100) = 0;
    for (text = md.TmpBuf; *text != 0; text++)
        if (*text == '*' || *text == '?') {
            *(u16 *)(md.TmpBuf + 100) = 's';
            break;
        }
    Dialog(OFFSET(NoFnc_B), include, 0, 0, MAIN_SEG);
    return TO_FNCEXT;
}

/* FncMas of the assembly, which a command jumps to: the files of the
   mask at TmpBuf, each by what FncAddr names. */
u16 FncMas(u16 include)
{
    md.ZoomOne = 0;
    FNC_CHANGE = 0;
    return FncMs1(include);
}

/* Where a command ends that may have changed what is on the disk:
   FncExt. Returns 1 if the panels are to be read again, 0 if they are as
   they were; the main loop goes on at Init50 or at Func04 by that.
   FncEx1, of vc.h, has them read again whatever. */
u16 FncEx1(void)
{
    u16 panel = md.WCBcrn;

    PANEL(panel)->ChgFlg = 1;
    PANEL(InvWCB(panel))->ChgFlg = 1;
    return 1;
}

u16 FncExt(void)
{
    if (FNC_CHANGE == 0)
        return 0;
    return FncEx1();
}

/* FncSel of the assembly, which a command jumps to: the selected files
   and directories of the current panel, each by what FncAddr names, or
   the one under the cursor if none is selected, or if the command is for
   one file only, OneFile. A key that TstBrk takes stops it, and so does
   an answer with something in its high byte.

   A file that has been dealt with loses its mark, bit 6 of its
   attributes, and gets bit 7 instead, by which ^M selects it again; the
   marks that the command before has left are taken off first. If the
   setup has it so, UnSlct, the panel is written anew for every such file
   and its line put on the screen, if that is among those shown, with the
   status line. */
u16 FncSel(void)
{
    u16 panel = md.WCBcrn;
    struct WCBdef __far *wcb = PANEL(panel);
    struct FILENT __far *file;
    u16 address, answer, lines, number, begin, i;

    md.ZoomOne = 0;
    FNC_CHANGE = 0;
    address = wcb->CurAdr;
    if (md.OneFile != 0)
        goto one;
    for (address = DSKBUF; address < wcb->EndBuf;
         address += sizeof(struct FILENT))
        ENTRY(wcb, address)->AttrF &= 0x7F;
    address = wcb->CurAdr;
    if (TotSel(panel) == 0)
        goto one;
    for (address = DSKBUF; address < wcb->EndBuf;
         address += sizeof(struct FILENT)) {
        if (!(ENTRY(wcb, address)->AttrF & ATTR_SELECTED))
            continue;
one:
        file = ENTRY(wcb, address);
        for (i = 0; i < 13; i++)
            md.TmpBuf[i] = file->NameF[i];
        answer = FncCall(file->AttrF);
        FNC_CHANGE |= (u8)answer;
        panel = md.WCBcrn;
        wcb = PANEL(panel);
        if (md.OneFile != 0)
            break;
        file = ENTRY(wcb, address);
        if ((u8)answer != 0 && file->AttrF & ATTR_SELECTED) {
            file->AttrF = file->AttrF & 0x3F | 0x80;
            if (md.UnSlct != 0) {
                if (wcb->WinTyp == 0)
                    PutFls(panel);
                else if (wcb->WinTyp == 1)
                    PutTre(panel);
                else
                    PutInf(panel);
                lines = LnsAX();
                number = (u16)(address - wcb->First) / sizeof(struct FILENT);
                i = number % lines;
                number /= lines;
                if (number <= (wcb->BrfFul == 0 ? 2 : 0)) {
                    begin = BegBX(panel) + 0x201;
                    Window(begin + (i << 8), 0x126);
                }
                if (md.MiniSta != 0) {
                    begin = BegBX(panel);
                    Window(begin + (SizDX() & 0xFF00) - 0x200, 0x128);
                }
            }
        }
        if (TstBrk())
            break;
        if (answer >> 8)
            break;
    }
    return FncExt();
}
