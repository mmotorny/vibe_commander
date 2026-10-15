/*
 * What the program asks of DOS and of the BIOS at every turn: the time,
 * the keys that wait, the bell, capitals, DOS itself with its errors, a
 * file opened, the current directory, and the disk of a drive that
 * shares the floppy drive with another.
 *
 * Translated from VCSUB1.INC and VCSUB3.INC.
 *
 * What the system is asked is in sys.h, and how DOS and the BIOS are
 * asked for it in dos32.c; this is what the program makes of the answers.
 * Intr21 of the original, DOS called with the program's errors looked
 * for, is no routine here: it is in every function of sys.h that is
 * asked so, in dos32.c.
 */
#include "vc.h"

/* The dialog that asks for the disk of a drive, a constant of
   system_t.c: Dialog takes it by its offset. */
extern const struct BOXHD Fant_B;

#define KEY_CTRL_C 0x2E03
#define KEY_ESC    0x011B
#define KEY_F10    0x4400

#define ERR_SHARED 0x20                 /* DOS's error: others have the file
                                           open in a way that forbids it */

/* The timer's count of ticks since midnight. */
u32 Timer(void)
{
    return Ticks();
}

/* Take the keys that wait from the keyboard, if the setup says that the
   buffer is to be cleared. */
void ClrKbd(void)
{
    if (md.ClrBuf == 0)
        return;
    while (!KeyWaiting().none)
        KeyRead();
}

/* Look whether the user wants to stop what is going on: take Esc, Ctrl-C
   and F10 from the keyboard, and the 0 that Ctrl-Break leaves there, as
   long as one of them is the next key. True if one was. Another key
   stays, and whatever is behind it. */
bool TstBrk(void)
{
    bool pressed = false;
    struct Keys keys;
    u16 key;

    for (;;) {
        keys = KeyWaiting();
        if (keys.none)
            break;
        key = keys.key;
        if (key != 0 && key != KEY_CTRL_C && key != KEY_ESC
            && key != KEY_F10)
            break;
        KeyRead();
        pressed = true;
    }
    return pressed;
}

/* Sound the bell, if errors are to sound. The assembly had the BIOS
   write the character 7 with whatever BX held, the page and the colour
   for a character that is shown; a bell is not. */
void Beep(void)
{
    if (md.ErrSnd != 0)
        ScreenType(7, 0, 0);
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
    FindRecord(&md.DTA);
}

/* Is there a drive of this letter, the low byte of `letter`? DOS is asked
   to make it the current one, then which the current one is, and the one
   that was is made current again. */
bool TstDrv(u16 letter)
{
    i32 drive = (u8)(letter - 'A');
    i32 was, now;

    /* A drive is 0 to 255 here, as DOS takes it in a byte. */
    was = DriveCurrent();
    DriveSelect((u16)drive);
    now = DriveCurrent();
    DriveSelect((u16)was);
    return now == drive;
}

/* Open a file, `mode` as DOS takes it in AL. Where DOS says that others
   have the file open in a way that does not go with the mode, it is
   asked once more with what the mode says of sharing left out. Returns
   the handle, or the error. */
struct Answer OpenFil(const u8 *name, u16 mode)
{
    struct Answer opened = FileOpen(name, mode);

    if (opened.failed && opened.value == ERR_SHARED)
        opened = FileOpen(name, mode & 0x0F);
    return opened;
}

/* Ask DOS for the current drive and its current directory, into DosPth,
   with the disk of the drive asked for first. PathEr is 1 if that
   failed, and DosPth is the root of the drive then; what is returned is
   true. */
bool SetPth(void)
{
    i32 drive;

    md.PathEr = 1;
    drive = DriveCurrent();
    md.DosPth[0] = (u8)(drive + 'A');
    md.DosPth[1] = ':';
    md.DosPth[2] = '\\';
    md.DosPth[3] = 0;
    drive = (u8)(drive + 1);
    if (Fantom(drive))
        return true;
    if (DirCurrent((u16)drive, md.DosPth + 3).failed)
        return true;
    md.PathEr = 0;
    return false;
}

/* The disk of a drive that shares a floppy drive with another: `drive`
   is 1 for A:, in its low byte. DOS is asked which of the letters the
   drive goes by at the moment; if that is another, and DOS takes the
   new one, the old one is put back, the user is asked to put the disk
   in, and with Ok the new one is set. True if the user would not. The
   page of the help is the same afterwards as before.

   The dialog names the drive by its letter, a text of one letter at
   TmpErr+1: the letter and, behind it, the end of the text. */
bool Fantom(i32 drive)
{
    struct Answer unit;
    i32 goes_by, page;
    u16 answer;

    /* The drive is a byte's, 0 to 255, below. */
    drive &= 0xFF;
    unit = DriveLetter((u16)drive);
    if (unit.failed)
        return false;
    goes_by = unit.value & 0xFF;
    if (goes_by == 0 || goes_by == drive)
        return false;
    unit = DriveSetLetter((u16)drive);
    if (unit.failed)
        return false;
    if ((unit.value & 0xFF) == goes_by)
        return false;
    if (DriveSetLetter((u16)goes_by).failed)
        return false;
    *(u16 *)(md.TmpErr + 1) = (u8)(drive + 'A' - 1);
    page = md.HlpPage;
    answer = Dialog(&Fant_B, 0, 0, 0, &md);
    md.HlpPage = (u8)page;
    if ((answer & 0xFF) != 0)
        return true;
    DriveSetLetter((u16)drive);
    return false;
}

/* The disk of the drive that a path names, or of the current one for a
   path without a drive: `path` is in the main data. True if the user
   would not put it in. */
bool ChgDisk(const u8 *path)
{
    u16 letter = path[0];

    if (path[1] != ':')
        letter = md.DosPth[0];
    return Fantom((u8)(UpCase(letter) - ('A' - 1)));
}
