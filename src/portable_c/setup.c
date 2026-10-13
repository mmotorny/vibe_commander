/*
 * The setup file, VC.INI: what it holds, and writing it.
 *
 * Translated from VCSUB3.INC.
 *
 * The file is in the program's own directory. It holds three letters V,
 * of each panel what a WCBdef has before CurAdr, of the main data what
 * it has before LoadRes, and the sum of all these bytes as a word:
 * INI_LEN bytes, of vc.h. The main loop makes the same bytes every time
 * it comes round, to see whether the setup has changed since it was
 * saved (Func05 in main.c); the code that starts the program reads the
 * file (start.c).
 */
#include "vc.h"

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

/* What is at an offset in a segment, and the offset of something that
   the assembly defines. */
#define AT(segment, offset) \
    ((const u8 __far *)((u32)(segment) << 16 | (u16)(offset)))
#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

/* AsmSeg, of vc.h, returns the segment of the program's constants: the
   name of the file is one of system_t.c, and the code that starts
   the program reads it there too. */
extern const u8 __far Init_F[];

/* C routines of other files that vc.h does not have. */
void FilZero(u8 __far *text, u16 length);               /* string.c */
u8 __far *MainFil(u8 __far *to, const u8 __far *name);

#define DOS_ATTRIBUTES 0x4300           /* of the file at DS:DX, into CX */
#define DOS_CREATE     0x3C00           /* that file, with those of CX */
#define DOS_WRITE      0x4000           /* CX bytes at DS:DX to BX */
#define DOS_CLOSE      0x3E00

#define ERR_NO_FILE 2
#define ARCHIVE     0x20                /* the attribute of a file that
                                           has changed */

/* What the file holds of a panel, to `to`. The mask of the panel and its
   path are filled with zeros behind their ends first, in the panel
   itself. Returns where the next bytes go. */
static u8 *MkIni1(u8 *to, u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    const u8 __far *from = (const u8 __far *)wcb;
    u16 i;

    FilZero(wcb->DskMas, 13);
    FilZero(wcb->WinPath, LenPath);
    for (i = 0; i < WCBini; i++)
        *to++ = from[i];
    return to;
}

/* Write into MenuBuf what the setup file holds, as the setup is now.
   Returns how many bytes that is. */
u16 MakeIni(void)
{
    const u8 *from = (const u8 *)&md;
    u8 *to = md.MenuBuf;
    u16 length, i;

    *to++ = 'V';
    *to++ = 'V';
    *to++ = 'V';
    to = MkIni1(to, md.WCB1seg);
    to = MkIni1(to, md.WCB2seg);
    for (i = 0; i < CmnIni; i++)
        *to++ = from[i];
    length = to - md.MenuBuf;
    *(u16 *)to = ChkSum(md.MenuBuf, length);
    return length + 2;
}

/* What SaveSet returns when it has failed, with what DOS said. */
static struct Answer Failed(u16 error)
{
    struct Answer answer;

    answer.value = error;
    answer.failed = 1;
    return answer;
}

/* Write the setup to its file, which keeps the attributes it has and is
   marked as changed; one that is not there is made. Returns the number
   of bytes written, or what DOS said was wrong if it failed, 53h for a
   drive that is not ready.

   Ported as they are: a file that could not be written whole is left as
   it is, and counts as an error whose low byte is 0, and so does a disk
   that the user would not put in. The file is made anew before anything
   is written: what it held is gone when the writing fails. */
struct Answer SaveSet(void)
{
    struct DosRegs regs;
    struct Answer answer;
    u16 length, written, failed;

    length = MakeIni();
    MainFil(md.TmpBuf, AT(AsmSeg(), OFFSET(Init_F)));
    if (ChgDisk(md.TmpBuf))
        return Failed(0);
    regs.bx = length;
    regs.name = md.TmpBuf;
    regs.ax = DOS_ATTRIBUTES;
    if (Intr21((struct DosRegs STACK *)&regs)) {
        if (regs.ax != ERR_NO_FILE)
            return Failed(regs.ax);
        regs.cx = 0;
    }
    regs.cx |= ARCHIVE;                 /* SavSt1 */
    regs.ax = DOS_CREATE | regs.ax & 0xFF;
    if (Intr21((struct DosRegs STACK *)&regs))
        return Failed(regs.ax);
    regs.bx = regs.ax;
    regs.cx = length;
    regs.name = md.MenuBuf;
    regs.ax = DOS_WRITE | regs.ax & 0xFF;
    failed = Intr21((struct DosRegs STACK *)&regs);
    written = regs.ax;
    regs.ax = DOS_CLOSE | regs.ax & 0xFF;
    if (Intr21((struct DosRegs STACK *)&regs))
        return Failed(regs.ax);
    if (failed)
        return Failed(written);
    if (written != length)
        return Failed(written & 0xFF00);        /* SavSt2 */
    answer.value = written;
    answer.failed = 0;
    return answer;
}
