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

/* The name of the file is a constant of system_t.c, and the code that
   starts the program reads it there too. */
extern const u8 Init_F[];

/* C routines of other files that vc.h does not have. */
void FilZero(u8 *text, i32 length);                     /* string.c */
u8 *MainFil(u8 *to, const u8 *name);

#define ERR_NO_FILE 2
#define ARCHIVE     0x20                /* the attribute of a file that
                                           has changed */

/* The setup as the main loop last found it, the bytes that the file
   would hold: by it the loop tells whether the setup has changed and is
   to be saved (Func05 in main.c). Without the letters V at its start it
   is none, as when the program has just started or come back from a
   command. The original keeps it in the part of it that stays in the
   memory while a command runs, where the copy of a panel is then. */
u8 IniSave[INI_LEN];

/* What the file holds of a panel, to `to`. The mask of the panel and its
   path are filled with zeros behind their ends first, in the panel
   itself. Returns where the next bytes go. */
static u8 *MkIni1(u8 *to, struct WCBdef *panel)
{
    const u8 *from = (const u8 *)panel;
    u16 i;

    FilZero(panel->DskMas, 13);
    FilZero(panel->WinPath, LenPath);
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
    to = MkIni1(to, LeftPanel);
    to = MkIni1(to, RightPanel);
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
    struct FileAttr had;
    struct Answer opened, wrote, closed;
    u16 length;

    length = MakeIni();
    MainFil(md.TmpBuf, Init_F);
    if (ChgDisk(md.TmpBuf))
        return Failed(0);
    had = FileAttributes(md.TmpBuf);
    if (had.failed) {
        if (had.value != ERR_NO_FILE)
            return Failed(had.value);
        had.attributes = 0;
    }
    opened = FileCreate(md.TmpBuf, had.attributes | ARCHIVE);   /* SavSt1 */
    if (opened.failed)
        return opened;
    wrote = FileWrite(opened.value, md.MenuBuf, length);
    closed = FileClose(opened.value);
    if (closed.failed)
        return closed;
    if (wrote.failed)
        return wrote;
    if (wrote.value != length)
        return Failed(wrote.value & 0xFF00);    /* SavSt2 */
    return wrote;
}
