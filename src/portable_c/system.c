/*
 * What the program asks of DOS and of the BIOS at every turn: the time,
 * the keys that wait, the bell, capitals, DOS itself with its errors, a
 * file opened, the current directory, and the disk of a drive that
 * shares the floppy drive with another.
 *
 * Translated from VCSUB1.INC and VCSUB3.INC.
 *
 * The calls themselves are in dos.c; this is what the program makes of
 * them.
 */
#include "vc.h"

/* The dialog that asks for the disk of a drive, a constant of
   src/c/system_t.c: Dialog takes it by its offset. */
extern const u8 __far Fant_B[];
#pragma aux Fant_B "*"
#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

#define BIOS_TICKS ((const u32 __far *)0x0040006CUL)    /* the BIOS's count
                                           of timer ticks since midnight */

#define KEY_CTRL_C 0x2E03
#define KEY_ESC    0x011B
#define KEY_F10    0x4400

/* DX of a call of DOS that is no pointer: a drive. */
#define DX_OF(regs) (*(u16 STACK *)&(regs).name)

/* The BIOS's count of timer ticks, read with the timer kept from counting
   between its two halves. */
u32 Timer(void)
{
    u32 ticks;

    IntOff();
    ticks = *BIOS_TICKS;
    IntOn();
    return ticks;
}

/* Take the keys that wait from the keyboard, if the setup says that the
   buffer is to be cleared. */
void ClrKbd(void)
{
    if (md.ClrBuf == 0)
        return;
    while ((u16)(Int16(0x100) >> 16) == 0)
        Int16(0);
}

/* Look whether the user wants to stop what is going on: take Esc, Ctrl-C
   and F10 from the keyboard, and the 0 that Ctrl-Break leaves there, as
   long as one of them is the next key. Not 0 if one was. Another key
   stays, and whatever is behind it. */
u16 TstBrk(void)
{
    u16 pressed = 0;
    u32 pair;
    u16 key;

    for (;;) {
        pair = Int16(0x100);
        if ((u16)(pair >> 16) != 0)
            break;
        key = (u16)pair;
        if (key != 0 && key != KEY_CTRL_C && key != KEY_ESC
            && key != KEY_F10)
            break;
        Int16(0);
        pressed = 1;
    }
    return pressed;
}

/* Sound the bell, if errors are to sound. The assembly had the BIOS
   write the character 7 with whatever BX held, the page and the colour
   for a character that is shown; a bell is not. */
void Beep(void)
{
    struct BiosRegs regs;

    if (md.ErrSnd == 0)
        return;
    regs.ax = 0x0E07;
    regs.bx = 0;
    regs.cx = 0;
    regs.dx = 0;
    Int10((struct BiosRegs STACK *)&regs);
}

/* The capital of a letter, the low byte of `letter`: of the 26 by
   arithmetic, and then DOS is asked, which knows the others of the
   country. The high byte comes back as it went. */
u16 UpCase(u16 letter)
{
    u16 al = letter & 0xFF;

    if (al >= 'a' && al <= 'z')
        letter -= 0x20;
    return CaseMap(letter);
}

/* Have DOS put what it finds, when it is asked for files, at DTA. */
void SetDTA(void)
{
    struct DosRegs regs;

    regs.ax = 0x1A00;
    regs.name = (const u8 __far *)&md.DTA;
    Int21d((struct DosRegs STACK *)&regs);
}

/* Is there a drive of this letter, the low byte of `letter`? DOS is asked
   to make it the current one, then which the current one is, and the one
   that was is made current again. */
u16 TstDrv(u16 letter)
{
    struct DosRegs regs;
    u16 drive = (u8)(letter - 'A');
    u16 was, now;

    regs.name = (const u8 __far *)&md;
    regs.ax = 0x1900;
    Int21d((struct DosRegs STACK *)&regs);
    was = regs.ax & 0xFF;
    regs.ax = 0x0E00;
    DX_OF(regs) = drive;
    Int21d((struct DosRegs STACK *)&regs);
    regs.ax = 0x1900;
    Int21d((struct DosRegs STACK *)&regs);
    now = regs.ax & 0xFF;
    regs.ax = 0x0E00;
    DX_OF(regs) = was;
    Int21d((struct DosRegs STACK *)&regs);
    return now == drive;
}

/* Make a drive the current one of DOS: 0 for A:, in the low byte of
   `drive`. */
void SelDrv(u16 drive)
{
    struct DosRegs regs;

    regs.name = (const u8 __far *)&md;
    regs.ax = 0x0E00;
    DX_OF(regs) = drive & 0xFF;
    Int21d((struct DosRegs STACK *)&regs);
}

/* Call DOS where a drive may not be ready, with the carry flag set first
   if `carry` is not 0. When DOS asks what to do about a drive, the
   program's routine for that, ErrRdy, shows a box and notes in ErrDat
   that it has; the call then counts as failed with the error 53h. An
   error that DOS reports itself is asked about once more, and its
   extended code is what comes back in AX: for that call AH is 59h and BX
   0, and the other registers are as the first call left them, which they
   are afterwards too. Not 0 for an error. */
u16 Intr21f(struct DosRegs STACK *regs, u16 carry)
{
    u16 bx, cx, dx, es;

    md.ErrDat = 0;
    carry = Int21f(regs, carry);
    if (md.ErrDat != 0) {
        regs->ax = 0x53;
        return 1;
    }
    if (carry == 0)
        return 0;
    bx = regs->bx;
    cx = regs->cx;
    dx = DX_OF(*regs);
    es = regs->es;
    regs->ax = regs->ax & 0xFF | 0x5900;
    regs->bx = 0;
    Int21d(regs);
    regs->bx = bx;
    regs->cx = cx;
    DX_OF(*regs) = dx;
    regs->es = es;
    return 1;
}

/* The same for a call of DOS that sets the carry flag itself, which is
   every one but those for an FCB. */
u16 Intr21(struct DosRegs STACK *regs)
{
    return Intr21f(regs, 0);
}

/* The time of day as DOS has it: the hour in the highest byte of the low
   word and the minute below it, and in the high word the second and the
   hundredth of one. */
u32 DosTime(void)
{
    struct DosRegs regs;

    regs.name = (const u8 __far *)&md;
    regs.ax = 0x2C00;
    Int21d((struct DosRegs STACK *)&regs);
    return (u32)DX_OF(regs) << 16 | regs.cx;
}

/* Open a file, `mode` as DOS takes it in AL. Where DOS says that others
   have the file open in a way that does not go with the mode, it is
   asked once more with what the mode says of sharing left out. Returns
   the handle in the low word, or the error there and 1 in the high
   one. */
u32 OpenFil(const u8 __far *name, u16 mode)
{
    struct DosRegs regs;
    u16 failed;

    regs.ax = 0x3D00 | mode & 0xFF;
    regs.bx = regs.ax;
    regs.name = name;
    failed = Intr21((struct DosRegs STACK *)&regs);
    if (failed != 0 && regs.ax == 0x20) {
        regs.ax = regs.bx & 0xFF0F;
        failed = Intr21((struct DosRegs STACK *)&regs);
    }
    return (u32)failed << 16 | regs.ax;
}

/* Ask DOS for the current drive and its current directory, into DosPth,
   with the disk of the drive asked for first. PathEr is 1 if that
   failed, and DosPth is the root of the drive then; what is returned is
   not 0. */
u16 SetPth(void)
{
    struct DosRegs regs;
    u16 drive;

    md.PathEr = 1;
    regs.name = (const u8 __far *)&md;
    regs.ax = 0x1900;
    Int21d((struct DosRegs STACK *)&regs);
    drive = regs.ax & 0xFF;
    md.DosPth[0] = (u8)(drive + 'A');
    md.DosPth[1] = ':';
    md.DosPth[2] = '\\';
    md.DosPth[3] = 0;
    drive = (u8)(drive + 1);
    if (Fantom(drive))
        return 1;
    regs.ax = 0x4700 | drive;
    DX_OF(regs) = drive;
    regs.si = (u16)(md.DosPth + 3);
    if (Intr21((struct DosRegs STACK *)&regs))
        return 1;
    md.PathEr = 0;
    return 0;
}

/* The disk of a drive that shares a floppy drive with another: `drive`
   is 1 for A:, in its low byte. DOS is asked which of the letters the
   drive goes by at the moment; if that is another, and DOS takes the
   new one, the old one is put back, the user is asked to put the disk
   in, and with Ok the new one is set. Not 0 if the user would not. The
   page of the help is the same afterwards as before.

   The dialog names the drive by its letter, a text of one letter at
   TmpErr+1: the letter and, behind it, the drive made a word with its
   sign, which is 0. */
u16 Fantom(u16 drive)
{
    struct DosRegs regs;
    u16 goes_by, answer;
    u8 page;

    drive &= 0xFF;
    regs.name = (const u8 __far *)&md;
    regs.ax = 0x440E;
    regs.bx = drive;
    if (Int21d((struct DosRegs STACK *)&regs))
        return 0;
    goes_by = regs.ax & 0xFF;
    if (goes_by == 0 || goes_by == drive)
        return 0;
    regs.ax = 0x440F;
    regs.bx = goes_by << 8 | drive;
    if (Int21d((struct DosRegs STACK *)&regs))
        return 0;
    if ((regs.ax & 0xFF) == goes_by)
        return 0;
    regs.ax = 0x440F;
    regs.bx = drive << 8 | goes_by;
    if (Int21d((struct DosRegs STACK *)&regs))
        return 0;
    *(u16 *)(md.TmpErr + 1) = (u16)(signed char)drive & 0xFF00
                              | (u8)(drive + 'A' - 1);
    page = md.HlpPage;
    answer = Dialog(OFFSET(Fant_B), 0, 0, 0, MAIN_SEG);
    md.HlpPage = page;
    if ((answer & 0xFF) != 0)
        return 1;
    regs.ax = 0x440F;
    regs.bx = regs.bx >> 8 | regs.bx << 8;
    Int21d((struct DosRegs STACK *)&regs);
    return 0;
}

/* The disk of the drive that a path names, or of the current one for a
   path without a drive: `path` is in the main data. Not 0 if the user
   would not put it in. */
u16 ChgDisk(const u8 *path)
{
    u16 letter = path[0];

    if (path[1] != ':')
        letter = md.DosPth[0];
    return Fantom((u8)(UpCase(letter) - ('A' - 1)));
}
