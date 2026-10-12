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
 * A command is run by the part of the program that stays in the memory,
 * which is still assembly: F21_31 saves there what the program needs to
 * start over afterwards, the setup, the command and what the panels show,
 * gives the main data back to DOS, and returns NEXT_EXEC, or NEXT_INIT11 if
 * there is nothing to run after all. The stub jumps on from that (CmdExe and
 * CmdIni in port/asm/VCFUNC.INC): the program does not come back here,
 * it starts over.
 *
 * These are commands, as in keys.c: functions that get the panel which
 * the assembly has in DS and return where the program goes on.
 */
#include "vc.h"

u32 F21_00(u16 panel);
u32 F21_10(u16 panel);
u32 F21_25(void);
u32 F21_31(void);
u32 F22_00(u16 panel);
u32 F23_00(u16 panel);
u32 F24_00(u16 panel);
u32 F24_01(u16 panel, u16 path);
u32 F24_02(u16 panel);
u32 F25_00(void);
u32 F26_00(void);
u32 F27_00(u16 panel);
u32 F30_00(u16 panel);
u32 F75_00(u16 panel);

/* How the assembly passes arguments to these, and what it expects back
   (see CmdC in port/asm/VCFUNC.INC). F21_30, which other files call too,
   is in vc.h. */
#pragma aux F21_00 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux F21_10 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux F21_25 "*_c" __far value [dx ax] modify exact [ax dx]
#pragma aux F21_31 "*_c" __far value [dx ax] modify exact [ax dx]
#pragma aux F22_00 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux F23_00 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux F24_00 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux F24_01 "*_c" __far parm [ax] [dx] value [dx ax] modify exact [ax dx]
#pragma aux F24_02 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux F25_00 "*_c" __far value [dx ax] modify exact [ax dx]
#pragma aux F26_00 "*_c" __far value [dx ax] modify exact [ax dx]
#pragma aux F27_00 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux F30_00 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux F75_00 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]

/* ResSeg is the segment of the part of the program that stays in the
   memory while a command runs: an entry of the assembly, which has that
   part. What else is called of the rest of the program is C, of vc.h:
   HidCur and Cursor1 for the cursor, InvVar, which makes of a byte 1 if
   it is 0 and 0 otherwise, Int16 and Int10, the BIOS for the keyboard
   and for the screen, Int21d, DOS as it is where the assembly did not go
   through Intr21, and Fantom, SetDTA, Intr21, SetPth, UpCase, RestVct,
   SavMous, CurMous, Quit and Window. */
u16 ResSeg(void);
#pragma aux ResSeg  "*_a" __far value [ax] modify exact [ax]

/* C routines of other files that vc.h does not have. TstExe is 1 for a
   name that DOS can run: com, exe or bat. TrCr10 finds the current
   directory in the tree again. Run_00 makes the commands of a line of an
   extension file, which begin at `text` in MenuBuf, and is 1 if there is
   one to run. */
u16 TstExe(const u8 __far *name);                       /* files.c */
void TrCr10(void);                                      /* tree.c */
u16 Run_00(u16 text, u16 view);                         /* usermenu.c */
#pragma aux TstExe "*_c" __far parm [dx si] value [ax] modify exact [ax]
#pragma aux TrCr10 "*_c" __far modify exact [ax]
#pragma aux Run_00 "*_c" __far parm [ax] [dx] value [ax] modify exact [ax]

/* The dialog that asks whether to quit, and the name of the program for
   it: in the assembly, where Dialog reads them. And where the part of the
   program that stays in the memory keeps the main data and the two
   panels while a command runs. */
extern const u8 __far Quit_B[];
extern const u8 __far Name_S[];
extern const u8 __far ComnIni[];
extern const u8 __far WCB1sav[];
extern const u8 __far WCB2sav[];
#pragma aux Quit_B "*"
#pragma aux Name_S "*"
#pragma aux ComnIni "*"
#pragma aux WCB1sav "*"
#pragma aux WCB2sav "*"

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))
#define AT(segment, offset) ((u8 __far *)((u32)(segment) << 16 | (offset)))

#define LenCmd 124                      /* Length of command line */

/* How much of the main data and of a panel is kept while a command runs:
   CmnSave and WCBsave of the assembly. Of a file in a panel's list its
   name and its attributes are kept, 13 bytes. */
#define CmnSave ((u16)&((struct MainDat *)0)->DosLin)
#define WCBsave ((u16)&((struct WCBdef *)0)->ExtBuf)

/* Where ComnIni is in the original, 25E5h. The assembly has that in BX
   when it has the BIOS write the end of a line, where the BIOS takes the
   page of the screen and a colour. */
#define COMNINI_WAS 0x25E5

#define DOS_FIND_FIRST 0x4E00           /* DOS: the first file of a name */
#define DOS_CHDIR      0x3B00           /* make a directory current */
#define DOS_FREE       0x4900           /* give a block of memory back */
#define DOS_DRIVE      0x1900           /* which drive is current */

static const u8 CODE CDir_S[] = "\\";

static u32 F23_02(u16 panel, u16 address);

/* Save a panel for the time that a command runs, at `to` in the part of
   the program that stays in the memory: the start of its segment, with
   its kind, its path and where its cursor is, and of each file in its
   list the name and the attributes.

   No more than MinFls files are kept. Of a longer list the last ones are
   dropped, and if the cursor is on one of those, its file is put into the
   last place that is kept; the first file shown, if it is not kept either,
   into the place before. Both lose the marks of being selected. The list
   is read again when the command is done, and these two are looked for
   in it by their names. */
static void F21_40(u16 panel, u8 __far *to)
{
    struct WCBdef __far *wcb = PANEL(panel);
    const u8 __far *from;
    struct FILENT __far *file;
    u16 end = wcb->EndBuf;
    u16 limit = DSKBUF + MinFls * sizeof(struct FILENT);
    u16 place, first, count, i;

    if (end > limit) {
        end = limit;
        wcb->EndBuf = end;
        if (end <= wcb->CurAdr) {
            place = end - sizeof(struct FILENT);
            first = wcb->First;
            if (first >= place) {
                place -= sizeof(struct FILENT);
                wcb->First = place;
                file = ENTRY(wcb, first);
                file->AttrF &= 0x3F;
                *ENTRY(wcb, place) = *file;
                place += sizeof(struct FILENT);
            }
            file = ENTRY(wcb, wcb->CurAdr);
            wcb->CurAdr = place;
            file->AttrF &= 0x3F;
            *ENTRY(wcb, place) = *file;
        }
    }
    from = (const u8 __far *)wcb;
    for (i = 0; i < WCBsave; i++)
        *to++ = *from++;
    count = (end - DSKBUF) / sizeof(struct FILENT);
    for (place = DSKBUF; count != 0; count--) {
        file = ENTRY(wcb, place);
        for (i = 0; i < 12; i++)
            *to++ = file->NameF[i];
        *to++ = file->AttrF;
        place += sizeof(struct FILENT);
    }
}

/* Hand over to the part of the program that stays in the memory, to run
   what the command line holds: F21_31, where the program also comes when
   it has started and read its setup, to start over from what is saved
   here.

   The vectors go back to their owners, the setup, the command and the
   panels are saved in the part that stays, the screen is as DOS left it
   but for the prompt, and the main data is given back to DOS. What is
   read afterwards is the copy in the part that stays: with no command in
   it the program only starts over, NEXT_INIT11; else that part is told the
   drive, to make it the current one first if it is not the one the
   program began on, and runs the commands, NEXT_EXEC. */
u32 F21_31(void)
{
    struct MainDat __far *saved;
    struct DosRegs regs;
    struct BiosRegs bios;
    u8 __far *to;
    const u8 *from;
    u16 stays = ResSeg();
    u16 i, drive;

    RestVct();
    SavMous();
    to = AT(stays, OFFSET(ComnIni));
    from = (const u8 *)&md;
    for (i = 0; i < CmnSave; i++)
        *to++ = *from++;
    F21_40(md.WCB1seg, AT(stays, OFFSET(WCB1sav)));
    F21_40(md.WCB2seg, AT(stays, OFFSET(WCB2sav)));
    Quit();
    if (!(md.LoadFl & 0x80))
        PutPrm(MAIN_SEG);               /* Quit leaves DS on the main data */
    Window(0, (u16)md.Lines << 8 | 80);
    Cursor1();
    regs.ax = DOS_FREE;                 /* ES is on the main data */
    regs.bx = regs.cx = regs.si = regs.di = 0;
    regs.name = (const u8 *)&md;
    Int21d((struct DosRegs STACK *)&regs);
    CurMous(0);

    saved = (struct MainDat __far *)AT(stays, OFFSET(ComnIni));
    if (saved->DosBuf[0] == 0) {
        saved->LoadFl = saved->LoadRes;
        return NEXT_INIT11;
    }
    bios.ax = 0x0E0D;
    bios.bx = COMNINI_WAS;
    bios.cx = bios.dx = 0;
    Int10((struct BiosRegs STACK *)&bios);
    if (!(saved->LoadFl & 0x80)) {
        bios.ax = 0x0E0A;
        bios.bx = COMNINI_WAS;
        Int10((struct BiosRegs STACK *)&bios);
    }
    regs.ax = DOS_DRIVE;
    Int21d((struct DosRegs STACK *)&regs);
    drive = regs.ax & 0xFF;
    if (drive != saved->DosChg[0])
        drive = (u8)(drive + 'A');
    else
        drive = 0;
    saved->DosChg[0] = 2;               /* the length of "A:" */
    saved->DosChg[1] = (u8)drive;
    saved->DosChg[2] = ':';
    saved->DosChg[3] = 0x0D;
    return NEXT_EXEC;
}

/* Have the part of the program that stays in the memory run what the
   command line holds, DosBuf, which may be several commands, each with
   its zero byte: F21_30. With Shift held down the program is, for this
   once, kept in the memory whole if the setup has it otherwise, and the
   other way round. */
u32 F21_30(void)
{
    u8 small;

    md.LoadRes = md.LoadFl & 0x7F;
    if (Int16(0x200 | md.LoadRes) & 3) {
        small = md.LoadFl;
        md.LoadFl &= 0x7F;
        InvVar(&md.LoadFl);
        md.LoadFl |= small & 0x80;
    }
    return F21_31();
}

/* Run what the command line holds, and note it in the history: F21_25.
   The history is the commands one behind the other, each with its zero
   byte, the last one run first, behind a zero byte at its start.

   A command that is the same as the last one is not noted again. One
   that is noted pushes the others back, and what then lies across the
   end of the history is cut off. The command is looked for in 256 bytes
   and gets a second zero byte behind it. */
u32 F21_25(void)
{
    u8 *line = md.DosBuf;
    u8 *history = md.HstBuf + 1;
    u8 *entry;
    u16 length = 0;
    u16 i, count;
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
   and any other file is run by the command that the extension file of
   the directory has for its kind, if it has one. The extension file is in
   the panel's segment; LenMenu bytes are taken from there, though it has
   LenExt. */
u32 F21_10(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    const struct FILENT __far *file;
    const u8 __far *from;
    struct DosRegs regs;
    u32 pair;
    u16 address, command, cursor, i;
    u8 c;

    if (wcb->WinTyp == 1) {
        if (((struct DIRTREE *)md.TreeBuf)->TreeLen == 0)
            return NEXT_FUNC04;
        HidCur();
        if (Fantom((u8)(md.DosPth[0] - ('A' - 1))))
            return NEXT_FUNC04;
        regs.bx = regs.si = regs.di = 0;
        regs.name = md.CrnPath + 2;
        if (md.CrnPath[3] != 0) {
            SetDTA();
            regs.ax = DOS_FIND_FIRST;
            regs.cx = 0x37;
            if (Intr21((struct DosRegs STACK *)&regs))
                return NEXT_FUNC04;
            if (!(md.DTA.FilAttr & ATTR_DIRECTORY))
                return NEXT_FUNC04;
            regs.name = md.CrnPath + 2;
        }
        regs.ax = DOS_CHDIR;
        if (Intr21((struct DosRegs STACK *)&regs))
            return NEXT_FUNC04;
        return F24_02(panel);
    }
    if (wcb->WinTyp != 0)
        return NEXT_FUNC04;
    address = wcb->CurAdr;
    if (address >= wcb->EndBuf)
        return NEXT_FUNC04;
    file = ENTRY(wcb, address);
    if (file->AttrF & ATTR_DIRECTORY)
        return F23_02(panel, address);
    if (!TstExe(file->NameF)) {
        from = wcb->ExtBuf;
        for (i = 0; i < LenMenu - 1; i++)
            md.MenuBuf[i] = from[i];
        md.MenuBuf[LenMenu - 1] = 0x1A;
        command = FindExt(ENTRY(wcb, wcb->CurAdr)->NameF);
        if (command == 0)
            return NEXT_FUNC04;
        command--;
        *(u8 *)command = ' ';
        if (Run_00(command, 0))
            return F21_30();
        return NEXT_FUNC04;
    }
    /* The name is typed into the command line, as far as that takes it. */
    file = ENTRY(wcb, wcb->CurAdr);
    cursor = 0;
    md.DosBuf[0] = 0;
    for (i = 0; (c = file->NameF[i]) != 0; i++) {
        pair = EdLin(c, md.DosBuf, cursor, LenCmd - 1);
        cursor = (u16)pair;
        if (pair >> 16 == 0)
            break;
    }
    md.DosCol = (u8)cursor;
    DosCom();
    Cursor1();
    return F21_25();
}

/* Enter. */
u32 F21_00(u16 panel)
{
    if (md.DosBuf[0] == 0 && PANEL(panel)->Visible != 0)
        return F21_10(panel);
    return F21_25();
}

/* The panels after DOS has gone to another directory: F24_02. A panel of
   files is read again, and so is the other panel unless both show files:
   the Info is of the current directory. A tree that is not to be read
   again has the directory looked for in it. */
u32 F24_02(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u16 kind, find = 0;

    SetPth();
    kind = wcb->WinTyp;
    if (kind != 1)
        wcb->ChgFlg = 1;
    else if (wcb->ChgFlg == 0)
        find = 1;
    wcb = PANEL(InvWCB(panel));
    if ((kind | wcb->WinTyp) == 0)
        return NEXT_INIT50;
    if (wcb->WinTyp != 1)
        wcb->ChgFlg = 1;
    else if (wcb->ChgFlg == 0)
        find = 1;
    if (find)
        TrCr10();
    return NEXT_INIT50;
}

/* Go into the directory at `address` in the list of a panel: F23_02. For
   the one above, "..", the list is emptied but for one entry, with the
   name of the directory that is left: the cursor is put on that name when
   the list has been read. The name is taken off the path of the panel
   with it, 12 bytes of it whatever its length. */
static u32 F23_02(u16 panel, u16 address)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct FILENT __far *file = ENTRY(wcb, address);
    struct DosRegs regs;
    u8 __far *name;
    u16 i;

    HidCur();
    if (Fantom((u8)(wcb->WinPath[0] - ('A' - 1))))
        return NEXT_FUNC04;
    regs.ax = DOS_CHDIR;
    regs.bx = regs.cx = regs.si = regs.di = 0;
    regs.name = file->NameF;
    if (Intr21((struct DosRegs STACK *)&regs))
        return NEXT_FUNC04;
    if (file->NameF[0] == '.') {
        wcb->CurAdr = DSKBUF;
        wcb->First = DSKBUF + sizeof(struct FILENT);
        wcb->EndBuf = DSKBUF + sizeof(struct FILENT);
        ENTRY(wcb, DSKBUF + sizeof(struct FILENT))->NameF[0] = 0;
        name = (u8 __far *)CutPath(wcb->WinPath);
        file = ENTRY(wcb, DSKBUF);
        for (i = 0; i < 12; i++)
            file->NameF[i] = name[i];
        file->NameF[12] = 0;
        file->AttrF = ATTR_DIRECTORY;
        if ((u16)(u32)name > (u16)(u32)(wcb->WinPath + 3))
            name--;
        *name = 0;
    }
    return F24_02(panel);
}

/* ^PgUp: the directory above, if the list begins with it. */
u32 F22_00(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);

    if (ENTRY(wcb, DSKBUF)->NameF[0] != '.')
        return NEXT_FUNC05;
    if (!FilePan(panel))
        return NEXT_FUNC05;
    if (DSKBUF >= wcb->EndBuf)
        return NEXT_FUNC05;
    if (!(ENTRY(wcb, DSKBUF)->AttrF & ATTR_DIRECTORY))
        return NEXT_FUNC05;
    return F23_02(panel, DSKBUF);
}

/* ^PgDn: the directory under the cursor. */
u32 F23_00(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u16 address = wcb->CurAdr;

    if (!FilePan(panel))
        return NEXT_FUNC05;
    if (address >= wcb->EndBuf)
        return NEXT_FUNC05;
    if (!(ENTRY(wcb, address)->AttrF & ATTR_DIRECTORY))
        return NEXT_FUNC05;
    return F23_02(panel, address);
}

/* Have DOS go to the directory `path`, and the panel show it from its
   start: F24_01, where the tree in its box comes in with the directory
   that was chosen, at TmpBuf. */
static u32 ChDir(u16 panel, const u8 __far *path)
{
    struct DosRegs regs;

    regs.ax = DOS_CHDIR;
    regs.bx = regs.cx = regs.si = regs.di = 0;
    regs.name = path;
    if (Intr21((struct DosRegs STACK *)&regs))
        return NEXT_FUNC04;
    PANEL(panel)->EndBuf = DSKBUF;
    return F24_02(panel);
}

u32 F24_01(u16 panel, u16 path)
{
    return ChDir(panel, (const u8 *)path);
}

/* ^\: the root of the current drive. */
u32 F24_00(u16 panel)
{
    HidCur();
    if (Fantom((u8)(md.DosPth[0] - ('A' - 1))))
        return NEXT_FUNC04;
    return ChDir(panel, CDir_S);
}

/* Type a text into the command line, from the column `cursor` on, as far
   as the line takes it: F27_07. The name of a file, which is at TmpBuf,
   gets a space behind it. */
static u32 F27_07(const u8 *text, u16 cursor)
{
    const u8 *from = text;
    u32 pair;
    u8 c;

    for (;;) {
        c = *from;
        if (c == 0) {
            if (text == md.TmpBuf)
                cursor = (u16)EdLin(' ', md.DosBuf, cursor, LenCmd - 1);
            break;
        }
        from++;
        pair = EdLin(c, md.DosBuf, cursor, LenCmd - 1);
        cursor = (u16)pair;
        if (pair >> 16 == 0)
            break;
    }
    md.DosCol = (u8)cursor;
    DosCom();
    return NEXT_FUNC05;
}

/* Put the command of the history at `entry` into the command line, in
   place of what is there, and note where in the history that is:
   F27_06. */
static u32 F27_06(const u8 *entry)
{
    md.HstAdr = (u16)entry;
    md.DosBuf[0] = 0;
    md.DosBuf[1] = 0;
    return F27_07(entry, 0);
}

/* ^E, and Up with the panels off: the command before the one that the
   command line has from the history, if there is one. */
u32 F25_00(void)
{
    const u8 *entry = (const u8 *)md.HstAdr;
    u16 count = 256;
    u8 c;

    do
        c = *entry++;
    while (--count && c != 0);
    if (*entry == 0)
        entry = (const u8 *)md.HstAdr;
    return F27_06(entry);
}

/* ^X, and Down with the panels off: the command after it, and after the
   last one run an empty line. */
u32 F26_00(void)
{
    u16 entry = md.HstAdr - 1;
    u16 count = 256;
    u8 c;

    if (entry >= (u16)md.HstBuf) {
        if (entry == (u16)md.HstBuf)
            return F27_06((const u8 *)entry);
        entry--;
        do
            c = *(const u8 *)entry--;
        while (--count && c != 0);
        entry++;
    }
    entry++;
    return F27_06((const u8 *)entry);
}

/* ^Enter: the name of the file under the cursor into the command line,
   if that is empty or its cursor is behind a character that words end at,
   or behind an "@". Else the history is gone through, from where it was
   left, for a command that begins with what the line holds, whatever
   capitals it has, and that command put there instead; and if none does
   and the cursor is at the start of the line, the name again. */
u32 F27_00(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    const struct FILENT __far *file;
    const u8 *line, *entry, *found;
    u16 cursor = md.DosCol;
    u16 count, i;
    u8 c, other;

    if (md.DosBuf[0] == 0)
        goto F27_05;
    entry = (const u8 *)md.HstAdr;
    if (cursor != 0) {
        c = md.DosBuf[cursor - 1];
        if (CmpSym(c) || c == '@')
            goto F27_05;
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
            if (c == 0)
                return F27_06(found);
            c = (u8)UpCase(c);
            other = (u8)UpCase(*entry++);
            if (other != c)
                break;
        }
    }
    if (cursor != 0)
        return NEXT_FUNC05;
F27_05:
    if (!FilePan(panel))
        return NEXT_FUNC05;
    if (wcb->CurAdr >= wcb->EndBuf)
        return NEXT_FUNC05;
    file = ENTRY(wcb, wcb->CurAdr);
    for (i = 0; i < 13; i++)
        md.TmpBuf[i] = file->NameF[i];
    return F27_07(md.TmpBuf, cursor);
}

/* F10: leave the program, once the dialog that asks about it has been
   answered with Yes, if the setup has it ask.

   The assembly has DS on the panel when it calls Dialog, which takes that
   for where the fields of a dialog have what they set. This one has only
   its two buttons, and those are kept in the main data whatever. */
u32 F30_00(u16 panel)
{
    if (md.QuitAck != 0
        && (u8)Dialog(OFFSET(Quit_B), OFFSET(Name_S), 0, 0, panel) != 0)
        return NEXT_FUNC04;
    return NEXT_EXIT;
}

/* What the main loop does when the cursor of the tree has been on a
   directory for a while, with Auto change directory on: go there. */
u32 F75_00(u16 panel)
{
    if (PANEL(panel)->WinTyp != 1)
        return NEXT_FUNC04;
    return F21_10(panel);
}
