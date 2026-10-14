/*
 * The commands of the pull-down menus: the kind of a panel, its order,
 * its filter, a panel switched off and on; the tree in a box and the
 * sizes of directories; two directories compared; the configuration and
 * the advanced options; the menu file and the extension files, to edit;
 * the switches of the Options menu; and the setup saved.
 *
 * Translated from VCMENU.INC. The original has one more here, EGA lines,
 * F69_00, Alt+F9, which the portable C has not (see DEVIATIONS.md).
 *
 * These are commands, not routines: in the original the menu bar jumps
 * to one by its tables, the main loop by its table of keys, and the keys
 * of the panels and the box of the drives go on in some of them. A
 * command is a function here that the main loop calls, Command of
 * main.c, with the panel that the original has in DS and with CX as the
 * menu or the key left it, and what it returns says where the program
 * goes on: see MNU_FUNC04 in vc.h, and MnuEnd in main.c.
 *
 * A command of the Left or the Right menu gets that menu's panel. In the
 * original it finds the current one on the stack, where the menu bar has
 * pushed it, and so do M01_01, M03_00, M06_00 and M06_10 from wherever
 * they are entered: the original takes it off before it jumps on, and
 * once to have Tab pressed for it. The functions go by CurrentPanel instead:
 * every way into a command has DS on the current panel, and nothing
 * changes CurrentPanel before the place that popped it.
 */
#include "vc.h"

/* The commands are declared in vc.h, see "The commands" there: `panel`
   is what the assembly has in DS, `item` what it has in CX, from the menu
   bar the menu in the low byte and the item in the high one, and each
   takes of the two what it reads. What one returns is where the program
   goes on, a MNU_ of vc.h. */

/* Routines of other files that are C. PutFls writes a panel of files,
   PutTre the tree and PutInf, of vc.h, the Info. F20_02 makes the other
   panel the current one and has DOS follow there. ChTree has a directory
   chosen from the tree in a box, whose left edge is at `column`: not 0 if
   none was, else its path is at TmpBuf. DirSize works out the sizes of
   the directories of a panel, and TstInfo has an Info next to a panel
   read again. Help00 is the help of the page HlpPage. MainFil writes the
   path of a file of the program's directory. */
void PutPath(struct WCBdef *panel);                     /* panel.c */
void PutFls(struct WCBdef *panel);
void PutTre(struct WCBdef *panel);                      /* tree.c */
void F20_02(struct WCBdef *panel);                      /* keys.c */
bool ChTree(i32 column);                                /* treebox.c */
void DirSize(struct WCBdef *panel);                     /* read.c */
void TstInfo(struct WCBdef *panel);
void Help00(void);                                      /* help.c */
u8 *MainFil(u8 *to, const u8 *name);                    /* string.c */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   HidCur puts the cursor below the screen. ResUser writes what DOS has shown, which
   the panels cover, to the screen buffer, for a window given like
   Window's, and ResWin what a box covers, from where it was kept.
   SaveSet writes the setup to its file. */

/* The boxes and the dialogs of these commands, the texts that ^I stands
   for in them, and texts and names of files that are copied with what
   follows them: constants, most of them of commands_t.c, which DialBox
   and Dialog take a pointer to. A box and a dialog are declared here
   as what each begins with, as vc.h has it at the two. */
extern const struct BOXHD Filt_B;       /* the filter of a panel */
extern const struct BOXTOP DiSiz_P;     /* "Scanning sizes of
                                           subdirectories" */
extern const struct BOXHD CmpDi_B;      /* "The two directories appear to
                                           be identical." */
extern const struct BOXHD Conf_B;       /* the configuration */
extern const u8 Chk1_S[];               /* what begins a box to tick in it, */
extern const struct BOXHD Sets_B;       /* the advanced options, */
extern const u8 Chk2_S[];               /* and what begins one there */
extern const struct BOXHD MnLoc_B;      /* "Main, Local, Cancel" */
extern const u8 User_S[];               /* " User Menu " */
extern const u8 ExFl_S[];               /* " Extension File " */
extern const u8 UsMnu_S[];              /* "user menu" */
extern const u8 ExtFl_S[];              /* "extension file" */
extern const u8 Menu_F[];               /* "VC.MNU" */
extern const u8 Ext_F[];                /* "VC.EXT" */
extern const u8 ViewE_F[];              /* "VCVIEW.EXT" */
extern const u8 EditE_F[];              /* "VCEDIT.EXT" */
extern const struct BOXHD SvSt_B;       /* "Do you wish to save the current
                                           setup?" */
extern const struct BOXHD SetEr_B;      /* "There was an error while saving
                                           the setup file" */

#define ERR_PATH      3                 /* DOS's error where a directory is
                                           made current: there is no such */
#define ERR_NOT_READY 0x53              /* a drive is not ready, and the
                                           user gave up */

/* The minutes after which the screen is blanked, as the configuration
   offers them, from the first line of the group down. */
static const u8 Delay_T[] = { 40, 20, 5, 3, 1, 0 };

/* What a panel shows, written to the screen buffer: PutFls of the
   original, which goes on in what writes the tree or the Info for a
   panel of that kind. */
static void PtFs00(struct WCBdef *panel)
{
    if (panel->WinTyp == 0)
        PutFls(panel);
    else if (panel->WinTyp == 1)
        PutTre(panel);
    else
        PutInf(panel);
}

/* Have DOS go to the drive and the directory of a panel, and write the
   prompt for it. A directory that is not there any more is no error: DOS
   then stays in the one it has on that drive. Returns true if the
   diskette of the drive was not put in, if DOS will not go to the
   directory for another reason, or if it will not tell where it is
   afterwards. */
static bool M02_10(struct WCBdef *panel)
{
    struct Answer changed;
    i32 drive = panel->WinPath[0] - ('A' - 1);
    bool failed;

    if (Fantom((u16)drive))
        return true;
    changed = DirChange(panel->WinPath);
    if (changed.failed && changed.value != ERR_PATH)
        return true;
    DriveSelect((u8)(panel->WinPath[0] - 'A'));
    failed = SetPth() != 0;
    PutPrm(panel);
    return failed;
}

/* A panel's list of files made anew from what was read of the directory:
   those that its filter lets through, in its order, with the cursor on
   the file that it was on and the first one shown the same as before, as
   far as they are still in the list. */
static void M16_20(struct WCBdef *panel)
{
    const u8 *from;
    u16 was = panel->EndBuf;
    u16 cursor, first, found;
    i32 i;

    from = (const u8 *)panel + panel->CurAdr;
    for (i = 0; i < 13; i++)
        md.MenuBuf[i] = from[i];
    from = (const u8 *)panel + panel->First;
    for (i = 0; i < 13; i++)
        md.MenuBuf[13 + i] = from[i];
    SetMask(panel);
    Sort(panel);
    cursor = DSKBUF;
    first = DSKBUF;
    if (was > DSKBUF) {
        found = FnFile(panel, md.MenuBuf);
        if (found < panel->EndBuf)
            cursor = found;
        found = FnFile(panel, md.MenuBuf + 13);
        if (found < panel->EndBuf)
            first = found;
    }
    panel->CurAdr = cursor;
    panel->First = first;
}

/* M06_10: a panel switched on. If the current panel is off, this one
   becomes the current one. Where the panel is to be read again, an Info
   next to it is too. `item` is the menu to mark meanwhile, of which the
   low byte counts. */
i32 M06_10(struct WCBdef *panel, u16 item)
{
    HidCur();
    panel->Visible = 1;
    if (CurrentPanel != panel && CurrentPanel->Visible == 0) {
        PutMnu(item);
        F20_02(CurrentPanel);
        panel = CurrentPanel;
    }
    if (panel->ChgFlg != 0) {
        PutMnu(item);
        TstInfo(panel);
    }
    return MNU_INIT50;
}

/* M03_05: the panel is to be on, with its menu marked meanwhile. */
static i32 M03_05(struct WCBdef *panel)
{
    return M06_10(panel, panel == LeftPanel ? 1 : 5);
}

/* M16_11: the list of a panel made anew, and the panel drawn. */
static i32 M16_11(struct WCBdef *panel)
{
    M16_20(panel);
    return M03_05(panel);
}

/* M01_01: a panel is to be one of files, in the order in the low byte of
   `item` and brief or full by its high byte. If it shows the tree or the
   Info and is the current one, DOS first goes to the directory it had
   before, and nothing changes if that fails. The list is put in order
   here if only the order has changed; a panel that is to be read is put
   in order then. */
i32 M01_01(struct WCBdef *panel, u16 item)
{
    struct WCBdef *wcb = panel;
    i32 order = (u8)item;
    i32 was;

    HidCur();
    if (wcb->WinTyp != 0) {
        if (panel == CurrentPanel && M02_10(panel))
            return MNU_INIT50;
        wcb->WinTyp = 0;
        wcb->ChgFlg = 1;
    }
    was = wcb->SortTyp;
    wcb->SortTyp = (u8)order;
    wcb->BrfFul = (u8)(item >> 8);
    if (was != order && wcb->ChgFlg == 0)
        return M16_11(panel);
    return M03_05(panel);
}

/* Brief and Full, the first two items of a panel's menu. */
i32 M01_00(struct WCBdef *panel, u16 item)
{
    return M01_01(panel, (u16)((u8)((item >> 8) - 1) << 8
                               | panel->SortTyp));
}

/* The order of a panel, items 7 to 11 of its menu: by name, extension,
   time, size, or as the disk has the files. */
i32 M08_00(struct WCBdef *panel, u16 item)
{
    return M01_01(panel, (u16)(panel->BrfFul << 8
                               | (u8)((item >> 8) - 7)));
}

/* Info and Tree, the third and the fourth item of a panel's menu, and
   where ^L, ^Z and ^R go on: the kind of panel is five less the high
   byte of `item`, and 3, which is none, if that makes 0. If the panel is
   the current one and the other one shows files, DOS first goes to that
   one's directory, and nothing changes if that fails. Ctrl_L keeps what
   the panel showed before. */
i32 M03_00(struct WCBdef *panel, u16 item)
{
    /* The byte wraps, as the original's SUB in CL does: the keys of the
       panels hand on five less a kind that DOS may have answered, which
       wraps alike in struct Onward, and this gives the kind back. */
    i32 kind = (u8)(5 - (item >> 8));
    struct WCBdef *other;

    HidCur();
    if (kind == 0)
        kind = 3;
    if (panel->Visible == 0 || kind != panel->WinTyp) {
        if (panel == CurrentPanel) {
            other = InvWCB(panel);
            if (FilePan(other) && M02_10(other))
                return MNU_FUNC04;
        }
        if (panel->WinTyp != kind)
            panel->Ctrl_L = panel->WinTyp;
    }
    panel->WinTyp = (u8)kind;
    panel->ChgFlg = 1;
    if (kind == 1)
        md.CrnPath[0] = 0;
    return M03_05(panel);
}

/* On/Off, the fifth item of a panel's menu, and ^P, ^F1 and ^F2. A panel
   that is switched off leaves what DOS has shown in its place; if it was
   the current one and the other one is on, that one becomes the current
   one. The low byte of `item` is the menu to mark meanwhile. Func02 of
   the loop is Window for all of the screen and then Func04. */
i32 M06_00(struct WCBdef *panel, u16 item)
{
    struct WCBdef *current;

    if (panel->Visible == 0)
        return M06_10(panel, item);
    panel->Visible = 0;
    ResUser(BegBX(panel), SizDX());
    md.Ctrl_O = 0;
    current = CurrentPanel;
    if (current == panel && OtherPanel->Visible != 0) {
        PutMnu(item);
        F20_02(current);
        current = CurrentPanel;
        PutPath(current);
        PtFs00(current);
    }
    Window(0, (u16)(md.Lines << 8 | 80));
    return MNU_FUNC04;
}

/* M16_01: the dialog of a panel's filter, whose fields are in the
   panel. The lines of its box have no ^I, so the text for that, which
   the assembly leaves as DX happens to be, is not read. */
i32 M16_01(struct WCBdef *panel)
{
    if ((u8)Dialog(&Filt_B, 0, 0, 0, panel) != 0)
        return MNU_FUNC04;
    return M16_11(panel);
}

/* ^F: the filter of the current panel, if that shows files. */
i32 M16_00(struct WCBdef *panel)
{
    if (!FilePan(panel))
        return MNU_FUNC04;
    return M16_01(panel);
}

/* ^H: hidden files shown or not, in the current panel if that shows
   files. */
i32 M16_10(struct WCBdef *panel)
{
    if (!FilePan(panel))
        return MNU_FUNC04;
    panel->Hidden = panel->Hidden == 0;     /* InvVar */
    return M16_11(panel);
}

/* Help, the first item of the Files menu: the page that the menu bar has
   set. */
i32 M21_00(void)
{
    Help00();
    return MNU_FUNC04;
}

/* Alt-F10: the tree in a box, to go to the directory that is chosen
   there. */
i32 M41_00(void)
{
    PutMnu(3);
    md.HlpPage = 28;
    if (ChTree(16))
        return MNU_INIT50;
    return MNU_F24_01;
}

/* Alt-F6: the sizes of the directories of the current panel, if that
   shows files, worked out with a box on the screen meanwhile; the list
   is put in order again, which may be by size. The lines of the box have
   no ^I. */
i32 M42_00(struct WCBdef *panel)
{
    struct Box where;

    if (FilePan(panel)) {
        *(u16 *)md.TmpBuf = panel->WinPath[0];
        PutMnu(3);
        DialBox(0x528, &DiSiz_P, 0, &md, &where);
        Window(where.begin, where.size);
        DirSize(panel);
        M16_20(panel);
        ResWin(where.begin, where.size, where.saved);
        Window(where.begin, where.size);
    }
    return MNU_INIT50;
}

/* M60_02: the prompt written again, and the panels drawn again if the
   panel `panel` is on. The configuration comes here with DS on the main
   data instead: PutPrm and the test then read there what they would
   read of a panel. PutPrm takes of a panel only whether it is the left
   one, for a column that it does not use. The test finds the third byte
   of the main data, the colour of selected files in the first palette,
   which is 0Fh as the program comes: the panels are drawn again. */
static i32 M60_02(struct WCBdef *panel)
{
    PutPrm(panel);
    if (panel->Visible == 0)
        return MNU_FUNC04;
    return MNU_INIT50;
}

/* Mark in a panel every file that the other panel has not, has as a
   directory, or has with an earlier date and time. Returns true if there
   is one, and `marked` otherwise. */
static bool M49_10(struct WCBdef *panel, bool marked)
{
    struct WCBdef *other = InvWCB(panel);
    struct FILENT *file;
    const struct FILENT *its;
    u16 address, found;
    i32 i;

    for (address = DSKBUF; address < panel->EndBuf;
         address += sizeof(struct FILENT)) {
        file = ENTRY(panel, address);
        if (file->AttrF & ATTR_DIRECTORY)
            continue;
        for (i = 0; i < 13; i++)
            md.TmpBuf[i] = file->NameF[i];
        found = FnFile(other, md.TmpBuf);
        its = ENTRY(other, found);
        if (found < other->EndBuf
            && !(its->AttrF & ATTR_DIRECTORY)
            && (its->DateF > file->DateF
                || its->DateF == file->DateF
                   && its->TimeF >= file->TimeF))
            continue;
        file->AttrF |= ATTR_SELECTED;   /* M49_12 */
        marked = true;
    }
    return marked;
}

/* ^C: the directories of the two panels compared, if both show files:
   the files that are in one only, or newer in one, are selected there,
   in addition to what is selected. A box says so if there is none; its
   lines have no ^I. */
i32 M49_00(struct WCBdef *panel)
{
    struct WCBdef *other;
    bool marked;

    if (!FilePan(panel))
        return MNU_FUNC04;
    other = InvWCB(panel);
    if (!FilePan(other))
        return MNU_FUNC04;
    marked = M49_10(other, false);
    marked = M49_10(panel, marked);
    if (!marked) {
        Dialog(&CmpDi_B, 0, 0, 0, panel);
        return MNU_FUNC04;
    }
    return M60_02(panel);
}

/* The configuration. The dialog sets what its boxes to tick are for; the
   palette and the minutes until the screen is blanked are chosen from
   groups, which it has at TmpBuf and TmpBuf+2, each with the line of its
   cursor behind it. The group of the minutes leaves 0 to 5. Afterwards
   everything but the last two lines is as DOS left it, and the panels
   are drawn anew: the menu bar may have come or gone. The assembly loads
   CL with MenuVs before the dialog, and nothing reads that. */
i32 M60_00(void)
{
    i32 delay;

    md.TmpBuf[0] = md.ColTyp;
    md.TmpBuf[1] = md.ColTyp;
    for (delay = 0; md.BlDelay < Delay_T[delay]; delay++)
        ;
    md.TmpBuf[2] = (u8)delay;
    md.TmpBuf[3] = (u8)delay;
    if ((u8)Dialog(&Conf_B, Chk1_S, 0, 0, &md) != 0)
        return MNU_FUNC04;
    md.ColTyp = md.TmpBuf[0];
    md.BlDelay = Delay_T[md.TmpBuf[2]];
    ResUser(0, (u16)((u8)(md.Lines - 2) << 8 | 80));
    PutMnu(0);
    return M60_02(MAIN_AS_PANEL);
}

/* The advanced options: all of them boxes to tick, which the dialog
   sets. */
i32 M61_00(void)
{
    Dialog(&Sets_B, Chk2_S, 0, 0, &md);
    return MNU_FUNC04;
}

/* M64_02: ask whether the main file of the name at `name` is meant, the
   one in the program's directory, or that of the current directory, and
   have the editor open it. `title` is the title of the box, `include`
   what it calls the file, `page` its page of the help.

   The original copies the title as 80 bytes and the name as 13, with
   what follows them there, and 64 of those bytes go on into ViewFil
   behind the name. So as many are copied here, and ExFl_S is written
   in commands_t.c with the 80 that the original has there. */
static i32 M64_02(u16 item, const u8 *name, const u8 *title,
                  const u8 *include, i32 page)
{
    i32 answer, i;

    md.HlpPage = (u8)page;
    PutMnu(item);
    for (i = 0; i < 80; i++)
        md.TmpBuf[i] = title[i];
    answer = Dialog(&MnLoc_B, include, 0, 0, &md) & 0xFF;
    if (answer >= 2)
        return MNU_FUNC04;
    for (i = 0; i < 13; i++)
        md.TmpBuf[i] = name[i];
    if (answer == 0)
        MainFil(md.TmpBuf, name);
    LetLin(md.TmpBuf);
    for (i = 0; i < LenStr; i++)
        md.ViewFil[i] = md.TmpBuf[i];
    md.ViewFil[LenStr] = 0;
    return MNU_V00_01;
}

/* Menu file edit, of the Commands menu. */
i32 M53_00(u16 item)
{
    return M64_02(item, Menu_F, User_S, UsMnu_S, 38);
}

/* Extension file edit, Viewers and Editors, of the Options menu. */
i32 M62_00(u16 item)
{
    return M64_02(item, Ext_F, ExFl_S, ExtFl_S, 42);
}

i32 M63_00(u16 item)
{
    return M64_02(item, ViewE_F, ExFl_S, ExtFl_S, 43);
}

i32 M64_00(u16 item)
{
    return M64_02(item, EditE_F, ExFl_S, ExtFl_S, 44);
}

/* M66_01: a switch of the Options menu, a byte of the setup that is 0 or
   1, made the other. */
static i32 M66_01(struct WCBdef *panel, u8 *option)
{
    *option = *option == 0;             /* InvVar */
    return M60_02(panel);
}

i32 M66_00(struct WCBdef *panel)        /* Auto menus */
{
    return M66_01(panel, &md.AutoMnu);
}

i32 M67_00(struct WCBdef *panel)        /* Path prompt */
{
    return M66_01(panel, &md.Prompt);
}

/* Full screen: the panels have another height, so what DOS has shown is
   put back where the two are. */
i32 M69_00(struct WCBdef *panel)
{
    ResUser(BegBX(panel) & 0xFF00, SizDX() & 0xFF00 | 80);
    return M66_01(panel, &md.FulScr);
}

/* Auto directory sizes: switched on, both panels are read again. */
i32 M70_00(struct WCBdef *panel)
{
    if (md.AutoSiz == 0) {
        panel->ChgFlg = 1;
        InvWCB(panel)->ChgFlg = 1;
    }
    return M66_01(panel, &md.AutoSiz);
}

i32 M71_00(struct WCBdef *panel)        /* Mini status */
{
    return M66_01(panel, &md.MiniSta);
}

i32 M72_00(struct WCBdef *panel)        /* Clock */
{
    return M66_01(panel, &md.Clock);
}

/* The original has one more switch here, Memory allocation, M73_00: the
   portable C stays in the memory whole while a command runs, and has no
   such option (see DEVIATIONS.md). */

/* Shift-F9: save the setup, if the answer is yes, and say so if it could
   not be written, unless the drive was not ready, which a box of its own
   has said. The boxes show nothing of what the caller has in DS, which
   is the current panel. */
i32 M75_00(struct WCBdef *panel)
{
    struct Answer saved;

    if ((u8)Dialog(&SvSt_B, 0, 0, 0, panel) == 0) {
        saved = SaveSet();
        if (saved.failed != 0 && saved.value != ERR_NOT_READY)
            Dialog(&SetEr_B, 0, 0, 0, panel);
    }
    return MNU_FUNC04;
}
