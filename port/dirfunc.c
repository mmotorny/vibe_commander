/*
 * What is done to files one after the other: deleting them, and setting
 * their attributes, date and time; and DirFunc, which goes through the
 * files of a directory for these and for the other commands that do.
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
 * What the commands ask before they begin, and FncSel and FncMas, are
 * still assembly.
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
u16 F38_70(void);
u16 M30_50(u16 attributes);
u32 M30_60(u16 answer);
static u32 F38_60(void);

/* How the assembly passes arguments to these, and what it expects back
   (see the stubs in port/asm/VCF1.INC). DirFunc and DelTr0 are in vc.h. */
#pragma aux F38_10  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux F38_40  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux F38_70  "*_c" __far value [ax] modify exact [ax]
#pragma aux M30_50  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M30_60  "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]

/* Routines of the rest of the program that are still assembly, by their
   entries for C. OncZoom puts a box on the screen with its middle at
   `place`, zooming it only for the first box of a command: `box` is where
   the assembly describes it, an offset in its code segment, and `include`
   the text that ^I stands for in a line of the box, another such offset.
   Dialog, which is C, does that and takes the keys; the low byte of what
   it returns is the number of the answer, 0 for the first, and more than
   any for Esc: here always for a box that is not on the screen yet, with
   what it sets in the main data. ResWin puts back in the screen buffer what a box covers, and Window,
   which is in vc.h, shows it. ChgDisk asks for the disk of a drive that
   shares the floppy drive with another, and is not 0 if the user would
   not change it.

   Intr21f is Intr21 with the carry flag set before DOS is called if
   `carry` is not 0, and clear if it is. Fcb29 has DOS make an FCB of a
   name: function 29h with 2 in AL, not through Intr21. */
void OncZoom(u16 place, u16 box, u16 include, struct Box STACK *where);
void ResWin(u16 begin, u16 size, u16 saved);
u16 ChgDisk(const u8 *path);
u16 Intr21f(struct DosRegs STACK *regs, u16 carry);
u16 Fcb29(const u8 *name, u8 *fcb);
#pragma aux OncZoom "*_a" __far parm [bx] [ax] [dx] [di] modify exact [bx dx si es]
#pragma aux ResWin  "*_a" __far parm [bx] [dx] [si] modify exact [es]
#pragma aux ChgDisk "*_a" __far parm [dx] value [ax] modify exact [ax es]
#pragma aux Intr21f "*_a" __far parm [bx] [ax] value [ax] modify exact [ax bx cx dx si di es]
#pragma aux Fcb29   "*_a" __far parm [si] [di] value [ax] modify exact [ax si es]

/* C routines of other files that vc.h does not have. */
const u8 __far *FndLet(const u8 __far *text);           /* string.c */
const u8 __far *TxtNum(const u8 __far *text, u16 STACK *number); /* digits.c */
#pragma aux FndLet  "*_c" __far parm [es si] value [es si] modify exact [si es]
#pragma aux TxtNum  "*_c" __far parm [es si] [bx] value [es si] modify exact [si es]

/* Boxes and texts of the assembly, which OncZoom and Dialog take by their
   offsets in its code segment: only those are used here. */
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
u16 F38_70(void)
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
    if ((u8)Fcb29(name, fcb + 7) != 1)
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
            if (md.FncAddr != DiSi10())
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
