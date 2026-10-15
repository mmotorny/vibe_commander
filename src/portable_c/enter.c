/*
 * Enter, the keys that change the directory, the commands before and
 * after in the history, and F10.
 *
 * Translated from VCFUNC.INC.
 *
 * Enter runs what the command line holds. With nothing there it goes by
 * what the cursor of the panel is on: into a directory, a file that DOS
 * can run by its name, any other file by the command that the extension
 * file of its directory has for files of its kind.
 *
 * A command is run by Exec of exec.c: F21_31 makes the program ready
 * for that, the interrupts given back and the screen as DOS left it, and
 * returns NEXT_EXEC, or NEXT_INIT11 if there is nothing to run after
 * all. The main loop leaves for that: the program does not come back
 * here, it starts over.
 *
 * These are commands, as in keys.c: functions that get the panel which
 * the original has in DS and return where the program goes on, struct
 * Onward of vc.h. See To below.
 */
#include "vc.h"

/* The commands are declared in vc.h, see "The commands" there, and so is
   F21_30, which other files call too. */

/* What is called of the rest of the program is of vc.h: HidCur and
   Cursor1 for the cursor, and Fantom, SetDTA, SetPth, UpCase, RestVct,
   SavMous, CurMous, Quit and Window. What is asked of the system is
   asked by the functions of sys.h. */

/* Routines of other files, which vc.h has: TrCr10 finds the current
   directory in the tree again. Whether a file is a program, which the
   original asks of its name, com, exe or bat (TstExe), is asked of the
   system, ProgramOfFile of sys.h: on DOS by the same three, on Linux by
   whether the user may execute the file. */

/* The dialog that asks whether to quit, and the name of the program for
   it: constants of keys_t.c. */
extern const struct BOXHD Quit_B;
extern const u8 Name_S[];

#define LenCmd 124                      /* Length of command line */


/* Where the original has what it keeps of the main data while a command
   runs, ComnIni: 25E5h. The assembly has that in BX when it has the BIOS
   write the end of a line, where the BIOS takes the page of the screen
   and a colour: the high byte and the low one. */
#define COMNINI_WAS 0x25E5

static const u8 CDir_S[] = "\\";

static struct Onward F23_02(struct WCBdef *panel, i32 address);

/* What a command returns, struct Onward of vc.h: the place where the
   program goes on, a NEXT_ there. None of these places takes a panel. */
static struct Onward To(i32 place)
{
    struct Onward next;

    next.place = (u8)place;
    next.item = 0;
    next.panel = 0;
    return next;
}

/* Make the program ready for what the command line holds to be run:
   F21_31, where the program also comes when it has started and read its
   setup with a command to run.

   The vectors go back to their owners, and the screen is as DOS left it
   but for the prompt. With no command the program only starts over,
   NEXT_INIT11; else the commands are run, NEXT_EXEC: Exec of exec.c,
   which the loop's end in main.c calls.

   The original asks DOS for the current drive here and leaves a command
   in DosChg that goes to it, if it is not the drive that was the
   current one when the program last started: for Quick execute
   commands, where the shell that the program was started from runs the
   commands and is on that drive still. The portable C has no such way
   of running one (see DEVIATIONS.md), and neither asks nor leaves
   anything.

   The original saves the setup, the command and the panels here, in the
   part of it that stays in the memory, and gives the main data back to
   DOS; what it reads from then on is the copy. Of a panel it keeps 256
   files, F21_40: a longer list loses its end, the cursor's file and the
   first one shown are put last if they are beyond, and both lose the
   marks of being selected; the list is read again when the command is
   done, and what was selected beyond the 256th file is no longer. The
   portable C keeps its panels where they are, whole, and cuts nothing
   (see DEVIATIONS.md). */
struct Onward F21_31(void)
{
    RestVct();
    SavMous();
    Quit();
    if (!(md.LoadFl & 0x80))
        PutPrm(MAIN_AS_PANEL);          /* Quit leaves DS on the main data */
    Window(0, (u16)(md.Lines << 8 | 80));
    Cursor1();
    CurMous(0);

    if (md.DosBuf[0] == 0) {
        md.LoadFl = md.LoadRes;
        return To(NEXT_INIT11);
    }
    ScreenType('\r', COMNINI_WAS >> 8, COMNINI_WAS & 0xFF);
    if (!(md.LoadFl & 0x80))
        ScreenType('\n', COMNINI_WAS >> 8, COMNINI_WAS & 0xFF);
    return To(NEXT_EXEC);
}

/* Have what the command line holds run, DosBuf, which may be several
   commands, each with its zero byte: F21_30. LoadRes keeps the byte of
   the setup that LoadFl is while the command runs, without the bit that
   says not to show the command.

   The original looks at Shift here: held down, the program is for this
   once kept in the memory whole if the setup has it otherwise, and the
   other way round. The portable C always stays whole, and Shift with
   Enter is Enter (see DEVIATIONS.md). */
struct Onward F21_30(void)
{
    md.LoadRes = md.LoadFl & 0x7F;
    return F21_31();
}

/* Run what the command line holds, and note it in the history: F21_25.
   The history is the commands one behind the other, each with its zero
   byte, the last one run first, behind a zero byte at its start.

   A command that is the same as the last one is not noted again. One
   that is noted pushes the others back, and what then lies across the
   end of the history is cut off. The command is looked for in 256 bytes
   and gets a second zero byte behind it. */
struct Onward F21_25(void)
{
    u8 *line = md.DosBuf;
    u8 *history = md.HstBuf + 1;
    u8 *entry;
    i32 length = 0;
    i32 i, count;
    u8 c;

    do {
        c = *line++;
        length++;
    } while (c != 0 && length != 256);
    *line = 0;
    if (length == 1)
        return F21_30();
    for (i = 0; i != length; i++)
        if (md.DosBuf[i] != history[i])
            break;
    if (i == length)
        return F21_30();
    for (i = LenHist - 1; i >= length; i--)
        md.HstBuf[i] = md.HstBuf[i - length];
    for (i = 0; i != length; i++)
        history[i] = md.DosBuf[i];
    entry = history + length;
    for (;;) {
        if (*entry == 0)
            break;
        line = entry;
        count = 256;
        do
            c = *entry++;
        while (--count && c != 0);
        if (entry >= md.HstBuf + LenHist) {
            *line = 0;
            break;
        }
    }
    return F21_30();
}

/* Enter with nothing in the command line, a second click of the mouse on
   an entry, and, in the tree, the cursor left on a directory for a while
   with Auto change directory on: F21_10.

   In the tree DOS goes to the directory under the cursor, CrnPath, if it
   is still there. In a panel of files a directory is gone into, a file
   that DOS can run has its name put into the command line and is run,
   behind what the system has typed in front of it, and any other file
   is run by the command that the extension file of the directory has
   for its kind, if it has one, or else handed to what the system opens
   files with, if it has anything of the kind, which DOS has not. The
   extension file is in the panel's block; LenMenu bytes are taken from
   there, though it has LenExt. */
struct Onward F21_10(struct WCBdef *panel)
{
    const struct FILENT *file;
    const u8 *from;
    const u8 *before;
    u8 *command;
    struct Edited edited;
    i32 address, cursor, i;
    u8 c;

    if (panel->WinTyp == 1) {
        if (((struct DIRTREE *)md.TreeBuf)->TreeLen == 0)
            return To(NEXT_FUNC04);
        HidCur();
        if (Fantom((u8)(md.DosPth[0] - ('A' - 1))))
            return To(NEXT_FUNC04);
        if (md.CrnPath[3] != 0) {
            SetDTA();
            if (FindFirst(md.CrnPath + 2, 0x37).failed)
                return To(NEXT_FUNC04);
            if (!(md.DTA.FilAttr & ATTR_DIRECTORY))
                return To(NEXT_FUNC04);
        }
        if (DirChange(md.CrnPath + 2).failed)
            return To(NEXT_FUNC04);
        F24_02(panel);
        return To(NEXT_INIT50);
    }
    if (panel->WinTyp != 0)
        return To(NEXT_FUNC04);
    address = PLACES(panel)->CurAdr;
    if (address >= PLACES(panel)->EndBuf)
        return To(NEXT_FUNC04);
    file = ENTRY(panel, address);
    if (file->AttrF & ATTR_DIRECTORY)
        return F23_02(panel, address);
    before = ProgramOfFile(file->NameF);
    if (before == 0) {
        from = panel->ExtBuf;
        for (i = 0; i < LenMenu - 1; i++)
            md.MenuBuf[i] = from[i];
        md.MenuBuf[LenMenu - 1] = 0x1A;
        command = FindExt(ENTRY(panel, PLACES(panel)->CurAdr)->NameF);
        if (command == 0) {
            ProgramOpenFile(file->NameF);
            return To(NEXT_FUNC04);
        }
        command--;
        *command = ' ';
        if (Run_00(command, 0))
            return F21_30();
        return To(NEXT_FUNC04);
    }
    /* The name is typed into the command line, as far as that takes it. */
    file = ENTRY(panel, PLACES(panel)->CurAdr);
    cursor = 0;
    md.DosBuf[0] = 0;
    for (; (c = *before) != 0; before++) {
        edited = EdLin(c, md.DosBuf, cursor, LenCmd - 1);
        cursor = edited.cursor;
    }
    for (i = 0; (c = file->NameF[i]) != 0; i++) {
        edited = EdLin(c, md.DosBuf, cursor, LenCmd - 1);
        cursor = edited.cursor;
        if (edited.changed == 0)
            break;
    }
    md.DosCol = (u8)cursor;
    DosCom();
    Cursor1();
    return F21_25();
}

/* Enter. */
struct Onward F21_00(struct WCBdef *panel)
{
    if (md.DosBuf[0] == 0 && panel->Visible != 0)
        return F21_10(panel);
    return F21_25();
}

/* The panels after DOS has gone to another directory: F24_02. A panel of
   files is read again, and so is the other panel unless both show files:
   the Info is of the current directory. A tree that is not to be read
   again has the directory looked for in it.

   The original goes on from here at Init50, which draws the panels and
   reads those that have changed. That is for what calls this to say: a
   command returns NEXT_INIT50. */
void F24_02(struct WCBdef *panel)
{
    struct WCBdef *wcb = panel;
    i32 kind;
    bool find = false;

    SetPth();
    kind = wcb->WinTyp;
    if (kind != 1)
        wcb->ChgFlg = 1;
    else if (wcb->ChgFlg == 0)
        find = true;
    wcb = InvWCB(panel);
    if ((kind | wcb->WinTyp) == 0)
        return;
    if (wcb->WinTyp != 1)
        wcb->ChgFlg = 1;
    else if (wcb->ChgFlg == 0)
        find = true;
    if (find)
        TrCr10();
}

/* Go into the directory at `address` in the list of a panel: F23_02. For
   the one above, "..", the list is emptied but for one entry, with the
   name of the directory that is left: the cursor is put on that name when
   the list has been read. The name is taken off the path of the panel
   with it, 12 bytes of it whatever its length. */
static struct Onward F23_02(struct WCBdef *panel, i32 address)
{
    struct FILENT *file = ENTRY(panel, address);
    u8 *name;
    i32 i;

    HidCur();
    if (Fantom((u8)(panel->WinPath[0] - ('A' - 1))))
        return To(NEXT_FUNC04);
    if (DirChange(file->NameF).failed)
        return To(NEXT_FUNC04);
    if (file->NameF[0] == '.') {
        PLACES(panel)->CurAdr = DSKBUF;
        PLACES(panel)->First = DSKBUF + sizeof(struct FILENT);
        PLACES(panel)->EndBuf = DSKBUF + sizeof(struct FILENT);
        ENTRY(panel, DSKBUF + sizeof(struct FILENT))->NameF[0] = 0;
        name = CUT_PATH(panel->WinPath);
        file = ENTRY(panel, DSKBUF);
        for (i = 0; i < 12; i++)
            file->NameF[i] = name[i];
        file->NameF[12] = 0;
        file->AttrF = ATTR_DIRECTORY;
        if (name > panel->WinPath + 3)
            name--;
        *name = 0;
    }
    F24_02(panel);
    return To(NEXT_INIT50);
}

/* ^PgUp: the directory above, if the list begins with it. */
struct Onward F22_00(struct WCBdef *panel)
{
    if (ENTRY(panel, DSKBUF)->NameF[0] != '.')
        return To(NEXT_FUNC05);
    if (!FilePan(panel))
        return To(NEXT_FUNC05);
    if (DSKBUF >= PLACES(panel)->EndBuf)
        return To(NEXT_FUNC05);
    if (!(ENTRY(panel, DSKBUF)->AttrF & ATTR_DIRECTORY))
        return To(NEXT_FUNC05);
    return F23_02(panel, DSKBUF);
}

/* ^PgDn: the directory under the cursor. */
struct Onward F23_00(struct WCBdef *panel)
{
    i32 address = PLACES(panel)->CurAdr;

    if (!FilePan(panel))
        return To(NEXT_FUNC05);
    if (address >= PLACES(panel)->EndBuf)
        return To(NEXT_FUNC05);
    if (!(ENTRY(panel, address)->AttrF & ATTR_DIRECTORY))
        return To(NEXT_FUNC05);
    return F23_02(panel, address);
}

/* Have DOS go to the directory `path`, and the panel show it from its
   start: F24_01, where the tree in its box comes in with the directory
   that was chosen, at TmpBuf. */
static struct Onward ChDir(struct WCBdef *panel, const u8 *path)
{
    if (DirChange(path).failed)
        return To(NEXT_FUNC04);
    PLACES(panel)->EndBuf = DSKBUF;
    F24_02(panel);
    return To(NEXT_INIT50);
}

struct Onward F24_01(struct WCBdef *panel, const u8 *path)
{
    return ChDir(panel, path);
}

/* ^\: the root of the current drive. */
struct Onward F24_00(struct WCBdef *panel)
{
    HidCur();
    if (Fantom((u8)(md.DosPth[0] - ('A' - 1))))
        return To(NEXT_FUNC04);
    return ChDir(panel, CDir_S);
}

/* Type a text into the command line, from the column `cursor` on, as far
   as the line takes it: F27_07. The name of a file, which is at TmpBuf,
   gets a space behind it. The original goes on from here at Func05, as
   every command does that comes here. */
static void F27_07(const u8 *text, i32 cursor)
{
    const u8 *from = text;
    struct Edited edited;
    u8 c;

    for (;;) {
        c = *from;
        if (c == 0) {
            if (text == md.TmpBuf)
                cursor = EdLin(' ', md.DosBuf, cursor, LenCmd - 1).cursor;
            break;
        }
        from++;
        edited = EdLin(c, md.DosBuf, cursor, LenCmd - 1);
        cursor = edited.cursor;
        if (edited.changed == 0)
            break;
    }
    md.DosCol = (u8)cursor;
    DosCom();
}

/* Put the command of the history at `entry` into the command line, in
   place of what is there, and note where in the history that is:
   F27_06. */
static void F27_06(const u8 *entry)
{
    HistoryPlace = entry;
    md.DosBuf[0] = 0;
    md.DosBuf[1] = 0;
    F27_07(entry, 0);
}

/* ^E, and Up with the panels off: the command before the one that the
   command line has from the history, if there is one. */
struct Onward F25_00(void)
{
    const u8 *entry = HistoryPlace;
    i32 count = 256;
    u8 c;

    do
        c = *entry++;
    while (--count && c != 0);
    if (*entry == 0)
        entry = HistoryPlace;
    F27_06(entry);
    return To(NEXT_FUNC05);
}

/* ^X, and Down with the panels off: the command after it, and after the
   last one run an empty line. */
struct Onward F26_00(void)
{
    const u8 *entry = HistoryPlace - 1;
    i32 count = 256;
    u8 c;

    if (entry >= md.HstBuf) {
        if (entry == md.HstBuf) {
            F27_06(entry);
            return To(NEXT_FUNC05);
        }
        entry--;
        do {
            c = *entry;
            entry--;
        } while (--count && c != 0);
        entry++;
    }
    entry++;
    F27_06(entry);
    return To(NEXT_FUNC05);
}

/* F27_05 of ^Enter: the name of the file under the cursor into the
   command line at `cursor`, if the current panel lists files and the
   cursor is on one. */
static void F27_05(struct WCBdef *panel, i32 cursor)
{
    const struct FILENT *file;
    i32 i;

    if (!FilePan(panel))
        return;
    if (PLACES(panel)->CurAdr >= PLACES(panel)->EndBuf)
        return;
    file = ENTRY(panel, PLACES(panel)->CurAdr);
    for (i = 0; i < 13; i++)
        md.TmpBuf[i] = file->NameF[i];
    F27_07(md.TmpBuf, cursor);
}

/* ^Enter: the name of the file under the cursor into the command line,
   if that is empty or its cursor is behind a character that words end at,
   or behind an "@". Else the history is gone through, from where it was
   left, for a command that begins with what the line holds, whatever
   capitals it has, and that command put there instead; and if none does
   and the cursor is at the start of the line, the name again. */
struct Onward F27_00(struct WCBdef *panel)
{
    const u8 *line, *entry, *found;
    i32 cursor = md.DosCol;
    i32 count;
    u8 c, other;

    if (md.DosBuf[0] == 0) {
        F27_05(panel, cursor);
        return To(NEXT_FUNC05);
    }
    entry = HistoryPlace;
    if (cursor != 0) {
        c = md.DosBuf[cursor - 1];
        if (CmpSym(c) || c == '@') {
            F27_05(panel, cursor);
            return To(NEXT_FUNC05);
        }
    }
    for (;;) {
        count = 256;                    /* the command behind this one */
        do
            c = *entry++;
        while (--count && c != 0);
        if (*entry == 0)
            break;
        found = entry;
        line = md.DosBuf;
        for (;;) {
            c = *line++;
            if (c == 0) {
                F27_06(found);
                return To(NEXT_FUNC05);
            }
            c = (u8)UpCase(c);
            other = (u8)UpCase(*entry++);
            if (other != c)
                break;
        }
    }
    if (cursor == 0)
        F27_05(panel, cursor);
    return To(NEXT_FUNC05);
}

/* F10: leave the program, once the dialog that asks about it has been
   answered with Yes, if the setup has it ask.

   The assembly has DS on the panel when it calls Dialog, which takes that
   for where the fields of a dialog have what they set. This one has only
   its two buttons, and those are kept in the main data whatever. */
struct Onward F30_00(struct WCBdef *panel)
{
    if (md.QuitAck != 0
        && (u8)Dialog(&Quit_B, Name_S, 0, 0, panel) != 0)
        return To(NEXT_FUNC04);
    return To(NEXT_EXIT);
}

/* What the main loop does when the cursor of the tree has been on a
   directory for a while, with Auto change directory on: go there. */
struct Onward F75_00(struct WCBdef *panel)
{
    if (panel->WinTyp != 1)
        return To(NEXT_FUNC04);
    return F21_10(panel);
}
