/*
 * The commands of the pull-down menus: the kind of a panel, its order,
 * its filter, a panel switched off and on; the tree in a box and the
 * sizes of directories; EGA lines; two directories compared; the
 * configuration and the advanced options; the menu file and the extension
 * files, to edit; the switches of the Options menu; and the setup saved.
 *
 * Translated from VCMENU.INC.
 *
 * These are commands, not routines: the stub of the menu bar jumps to one
 * by its tables, the main loop by its table of keys, and the keys of the
 * panels and the box of the drives go on in some of them. A command is a
 * function here that the stub at its label calls, with the panel that the
 * assembly has in DS and with CX as the menu or the key left it, and what
 * it returns says where the program goes on: see FUNC04 below, and MnuEnd
 * in port/asm/VCMENU.INC.
 *
 * A command of the Left or the Right menu gets that menu's panel, and
 * finds the current one on the stack, where the bar's stub has pushed it;
 * so do M01_01, M03_00, M06_00 and M06_10 from wherever they are entered.
 * The assembly takes it off before it jumps on, and once to have Tab
 * pressed for it. Here the stub takes it off, and the functions go by
 * WCBcrn instead: every way into a command has DS on the current panel,
 * and nothing changes WCBcrn before the place that popped it.
 */
#include "vc.h"

/* A command: `panel` is what the assembly has in DS, `item` what it has
   in CX, from the menu bar the menu in the low byte and the item in the
   high one. Each takes of the two what it reads. */
u16 M01_00(u16 panel, u16 item);
u16 M01_01(u16 panel, u16 item);
u16 M03_00(u16 panel, u16 item);
u16 M06_00(u16 panel, u16 item);
u16 M06_10(u16 panel, u16 item);
u16 M08_00(u16 panel, u16 item);
u16 M16_00(u16 panel);
u16 M16_01(u16 panel);
u16 M16_10(u16 panel);
u16 M21_00(void);
u16 M41_00(void);
u16 M42_00(u16 panel);
u16 F69_00(u16 item);
u16 M49_00(u16 panel);
u16 M60_00(void);
u16 M61_00(void);
u16 M53_00(u16 item);
u16 M62_00(u16 item);
u16 M63_00(u16 item);
u16 M64_00(u16 item);
u16 M66_00(u16 panel);
u16 M67_00(u16 panel);
u16 M69_00(u16 panel);
u16 M70_00(u16 panel);
u16 M71_00(u16 panel);
u16 M72_00(u16 panel);
u16 M73_00(u16 panel);
u16 M75_00(u16 panel);

/* How the assembly passes arguments to these, and what it expects back:
   the stubs go on with CX and DS as they were. */
#pragma aux M01_00 "*_c" __far parm [ax] [cx] value [ax] modify exact [ax]
#pragma aux M01_01 "*_c" __far parm [ax] [cx] value [ax] modify exact [ax]
#pragma aux M03_00 "*_c" __far parm [ax] [cx] value [ax] modify exact [ax]
#pragma aux M06_00 "*_c" __far parm [ax] [cx] value [ax] modify exact [ax]
#pragma aux M06_10 "*_c" __far parm [ax] [cx] value [ax] modify exact [ax]
#pragma aux M08_00 "*_c" __far parm [ax] [cx] value [ax] modify exact [ax]
#pragma aux M16_00 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M16_01 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M16_10 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M21_00 "*_c" __far value [ax] modify exact [ax]
#pragma aux M41_00 "*_c" __far value [ax] modify exact [ax]
#pragma aux M42_00 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux F69_00 "*_c" __far parm [cx] value [ax] modify exact [ax]
#pragma aux M49_00 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M60_00 "*_c" __far value [ax] modify exact [ax]
#pragma aux M61_00 "*_c" __far value [ax] modify exact [ax]
#pragma aux M53_00 "*_c" __far parm [cx] value [ax] modify exact [ax]
#pragma aux M62_00 "*_c" __far parm [cx] value [ax] modify exact [ax]
#pragma aux M63_00 "*_c" __far parm [cx] value [ax] modify exact [ax]
#pragma aux M64_00 "*_c" __far parm [cx] value [ax] modify exact [ax]
#pragma aux M66_00 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M67_00 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M69_00 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M70_00 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M71_00 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M72_00 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M73_00 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux M75_00 "*_c" __far parm [ax] value [ax] modify exact [ax]

/* Where the program goes on after a command: the numbers of MnuNx_T in
   port/asm/VCMENU.INC. */
enum {
    FUNC04,                             /* back to the panels as they are:
                                           the menu bar is drawn first */
    INIT50,                             /* draw the panels again, reading
                                           those that have changed */
    F24_01,                             /* have DOS go to the directory at
                                           TmpBuf, and read the panels */
    V00_01                              /* edit the file ViewFil, with the
                                           menu marked that the command
                                           came from */
};

/* Routines of other files that are C. PutFls writes a panel of files,
   PutTre the tree and PutInf, of vc.h, the Info. F20_02 makes the other
   panel the current one and has DOS follow there. ChTree has a directory
   chosen from the tree in a box, whose left edge is at `column`: not 0 if
   none was, else its path is at TmpBuf. DirSize works out the sizes of
   the directories of a panel, and TstInfo has an Info next to a panel
   read again. Help00 is the help of the page HlpPage. MainFil writes the
   path of a file of the program's directory. */
void PutPath(u16 panel);                                /* panel.c */
void PutFls(u16 panel);
void PutTre(u16 panel);                                 /* tree.c */
void F20_02(u16 panel);                                 /* keys.c */
u16 ChTree(u16 column);                                 /* treebox.c */
void DirSize(u16 panel);                                /* read.c */
void TstInfo(u16 panel);
void Help00(void);                                      /* help.c */
u8 __far *MainFil(u8 __far *to, const u8 __far *name);  /* string.c */
#pragma aux PutPath "*_c" __far parm [ax] modify exact [ax]
#pragma aux PutFls  "*_c" __far parm [ax] modify exact [ax]
#pragma aux PutTre  "*_c" __far parm [ax] modify exact [ax]
#pragma aux F20_02  "*_c" __far parm [ax] modify exact [ax]
#pragma aux ChTree  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux DirSize "*_c" __far parm [ax] modify exact [ax]
#pragma aux TstInfo "*_c" __far parm [ax] modify exact [ax]
#pragma aux Help00  "*_c" __far modify exact [ax]
#pragma aux MainFil "*_c" __far parm [es di] [dx si] value [es di] modify exact [di es]

/* Routines that are still assembly, by their entries for C. HidCur puts
   the cursor below the screen. ResUser writes what DOS has shown, which
   the panels cover, to the screen buffer, for a window given like
   Window's, and ResWin what a box covers, from where it was kept. SelDrv
   is the call of DOS that makes a drive the current one, 0 for A:, which
   the assembly makes without Intr21. AsmSeg returns the code segment of
   the assembly. ScrOffs asks the BIOS where in the memory of the screen
   the page begins that is shown. Int10, which calls the BIOS for the
   screen, and SaveSet, which writes the setup to its file, are in vc.h. */
void HidCur(void);
void ResUser(u16 begin, u16 size);
void ResWin(u16 begin, u16 size, u16 buffer);
void SelDrv(u16 drive);
u16 AsmSeg(void);
void ScrOffs(void);
#pragma aux HidCur  "*_a" __far modify exact [es]
#pragma aux ResUser "*_a" __far parm [bx] [dx] modify exact [es]
#pragma aux ResWin  "*_a" __far parm [bx] [dx] [si] modify exact [es]
#pragma aux SelDrv  "*_a" __far parm [dx] modify exact [ax]
#pragma aux AsmSeg  "*_a" __far value [ax] modify exact [ax]
#pragma aux ScrOffs "*_a" __far modify exact [ax bx di es]

/* The boxes and the dialogs of these commands, the texts that ^I stands
   for in them, and texts and names of files that are copied with what
   follows them: they are in the assembly's code segment, where DialBox
   and Dialog read them. */
extern const u8 __far Filt_B[];         /* the filter of a panel */
extern const u8 __far DiSiz_P[];        /* "Scanning sizes of
                                           subdirectories" */
extern const u8 __far CmpDi_B[];        /* "The two directories appear to
                                           be identical." */
extern const u8 __far Conf_B[];         /* the configuration */
extern const u8 __far Chk1_S[];         /* what begins a box to tick in it, */
extern const u8 __far Sets_B[];         /* the advanced options, */
extern const u8 __far Chk2_S[];         /* and what begins one there */
extern const u8 __far MnLoc_B[];        /* "Main, Local, Cancel" */
extern const u8 __far User_S[];         /* " User Menu " */
extern const u8 __far ExFl_S[];         /* " Extension File " */
extern const u8 __far UsMnu_S[];        /* "user menu" */
extern const u8 __far ExtFl_S[];        /* "extension file" */
extern const u8 __far Menu_F[];         /* "VC.MNU" */
extern const u8 __far Ext_F[];          /* "VC.EXT" */
extern const u8 __far ViewE_F[];        /* "VCVIEW.EXT" */
extern const u8 __far EditE_F[];        /* "VCEDIT.EXT" */
extern const u8 __far SvSt_B[];         /* "Do you wish to save the current
                                           setup?" */
extern const u8 __far SetEr_B[];        /* "There was an error while saving
                                           the setup file" */
#pragma aux Filt_B "*"
#pragma aux DiSiz_P "*"
#pragma aux CmpDi_B "*"
#pragma aux Conf_B "*"
#pragma aux Chk1_S "*"
#pragma aux Sets_B "*"
#pragma aux Chk2_S "*"
#pragma aux MnLoc_B "*"
#pragma aux User_S "*"
#pragma aux ExFl_S "*"
#pragma aux UsMnu_S "*"
#pragma aux ExtFl_S "*"
#pragma aux Menu_F "*"
#pragma aux Ext_F "*"
#pragma aux ViewE_F "*"
#pragma aux EditE_F "*"
#pragma aux SvSt_B "*"
#pragma aux SetEr_B "*"

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

#define AT(segment, offset) \
    ((const u8 __far *)((u32)(segment) << 16 | (u16)(offset)))

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

#define DOS_CHDIR     0x3B00            /* DOS: make a directory current */
#define ERR_PATH      3                 /* its error: there is no such */
#define ERR_NOT_READY 0x53              /* Intr21's: a drive is not ready */

/* The minutes after which the screen is blanked, as the configuration
   offers them, from the first line of the group down. */
static const u8 CODE Delay_T[] = { 40, 20, 5, 3, 1, 0 };

/* What a panel shows, written to the screen buffer: PutFls of the
   assembly, whose stub there chooses between these three by the kind of
   the panel. */
static void PtFs00(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);

    if (wcb->WinTyp == 0)
        PutFls(panel);
    else if (wcb->WinTyp == 1)
        PutTre(panel);
    else
        PutInf(panel);
}

/* Have DOS go to the drive and the directory of a panel, and write the
   prompt for it. A directory that is not there any more is no error: DOS
   then stays in the one it has on that drive. Returns something that is
   not 0 if the diskette of the drive was not put in, if DOS will not go
   to the directory for another reason, or if it will not tell where it
   is afterwards. */
static u16 M02_10(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct DosRegs regs;
    u16 drive = (u8)(wcb->WinPath[0] - ('A' - 1));
    u16 failed;

    if (Fantom(drive))
        return 1;
    regs.ax = DOS_CHDIR | drive;
    regs.bx = regs.cx = regs.si = regs.di = 0;
    regs.name = wcb->WinPath;
    if (Intr21((struct DosRegs STACK *)&regs) && regs.ax != ERR_PATH)
        return 1;
    SelDrv((u8)(wcb->WinPath[0] - 'A'));
    failed = SetPth();
    PutPrm(panel);
    return failed;
}

/* A panel's list of files made anew from what was read of the directory:
   those that its filter lets through, in its order, with the cursor on
   the file that it was on and the first one shown the same as before, as
   far as they are still in the list. */
static void M16_20(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    const u8 __far *from;
    u16 was = wcb->EndBuf;
    u16 cursor, first, found, i;

    from = (const u8 __far *)wcb + wcb->CurAdr;
    for (i = 0; i < 13; i++)
        md.MenuBuf[i] = from[i];
    from = (const u8 __far *)wcb + wcb->First;
    for (i = 0; i < 13; i++)
        md.MenuBuf[13 + i] = from[i];
    SetMask(panel);
    Sort(panel);
    cursor = DSKBUF;
    first = DSKBUF;
    if (was > DSKBUF) {
        found = FnFile(panel, md.MenuBuf);
        if (found < wcb->EndBuf)
            cursor = found;
        found = FnFile(panel, md.MenuBuf + 13);
        if (found < wcb->EndBuf)
            first = found;
    }
    wcb->CurAdr = cursor;
    wcb->First = first;
}

/* M06_10: a panel switched on. If the current panel is off, this one
   becomes the current one. Where the panel is to be read again, an Info
   next to it is too. `item` is the menu to mark meanwhile, of which the
   low byte counts. */
u16 M06_10(u16 panel, u16 item)
{
    HidCur();
    PANEL(panel)->Visible = 1;
    if (md.WCBcrn != panel && PANEL(md.WCBcrn)->Visible == 0) {
        PutMnu(item);
        F20_02(md.WCBcrn);
        panel = md.WCBcrn;
    }
    if (PANEL(panel)->ChgFlg != 0) {
        PutMnu(item);
        TstInfo(panel);
    }
    return INIT50;
}

/* M03_05: the panel is to be on, with its menu marked meanwhile. */
static u16 M03_05(u16 panel)
{
    return M06_10(panel, panel == md.WCB1seg ? 1 : 5);
}

/* M16_11: the list of a panel made anew, and the panel drawn. */
static u16 M16_11(u16 panel)
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
u16 M01_01(u16 panel, u16 item)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u8 order = (u8)item;
    u8 was;

    HidCur();
    if (wcb->WinTyp != 0) {
        if (panel == md.WCBcrn && M02_10(panel))
            return INIT50;
        wcb->WinTyp = 0;
        wcb->ChgFlg = 1;
    }
    was = wcb->SortTyp;
    wcb->SortTyp = order;
    wcb->BrfFul = (u8)(item >> 8);
    if (was != order && wcb->ChgFlg == 0)
        return M16_11(panel);
    return M03_05(panel);
}

/* Brief and Full, the first two items of a panel's menu. */
u16 M01_00(u16 panel, u16 item)
{
    return M01_01(panel, (u16)(u8)((item >> 8) - 1) << 8
                         | PANEL(panel)->SortTyp);
}

/* The order of a panel, items 7 to 11 of its menu: by name, extension,
   time, size, or as the disk has the files. */
u16 M08_00(u16 panel, u16 item)
{
    return M01_01(panel, (u16)PANEL(panel)->BrfFul << 8
                         | (u8)((item >> 8) - 7));
}

/* Info and Tree, the third and the fourth item of a panel's menu, and
   where ^L, ^Z and ^R go on: the kind of panel is five less the high
   byte of `item`, and 3, which is none, if that makes 0. If the panel is
   the current one and the other one shows files, DOS first goes to that
   one's directory, and nothing changes if that fails. Ctrl_L keeps what
   the panel showed before. */
u16 M03_00(u16 panel, u16 item)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u8 kind = (u8)(5 - (item >> 8));
    u16 other;

    HidCur();
    if (kind == 0)
        kind = 3;
    if (wcb->Visible == 0 || kind != wcb->WinTyp) {
        if (panel == md.WCBcrn) {
            other = InvWCB(panel);
            if (FilePan(other) && M02_10(other))
                return FUNC04;
        }
        if (wcb->WinTyp != kind)
            wcb->Ctrl_L = wcb->WinTyp;
    }
    wcb->WinTyp = kind;
    wcb->ChgFlg = 1;
    if (kind == 1)
        md.CrnPath[0] = 0;
    return M03_05(panel);
}

/* On/Off, the fifth item of a panel's menu, and ^P, ^F1 and ^F2. A panel
   that is switched off leaves what DOS has shown in its place; if it was
   the current one and the other one is on, that one becomes the current
   one. The low byte of `item` is the menu to mark meanwhile. Func02 of
   the loop is Window for all of the screen and then Func04. */
u16 M06_00(u16 panel, u16 item)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u16 current;

    if (wcb->Visible == 0)
        return M06_10(panel, item);
    wcb->Visible = 0;
    ResUser(BegBX(panel), SizDX());
    md.Ctrl_O = 0;
    current = md.WCBcrn;
    if (current == panel && PANEL(md.WCBsec)->Visible != 0) {
        PutMnu(item);
        F20_02(current);
        current = md.WCBcrn;
        PutPath(current);
        PtFs00(current);
    }
    Window(0, (u16)md.Lines << 8 | 80);
    return FUNC04;
}

/* M16_01: the dialog of a panel's filter, whose fields are in the panel's
   segment. The lines of its box have no ^I, so the text for that, which
   the assembly leaves as DX happens to be, is not read. */
u16 M16_01(u16 panel)
{
    if ((u8)Dialog(OFFSET(Filt_B), 0, 0, 0, panel) != 0)
        return FUNC04;
    return M16_11(panel);
}

/* ^F: the filter of the current panel, if that shows files. */
u16 M16_00(u16 panel)
{
    if (!FilePan(panel))
        return FUNC04;
    return M16_01(panel);
}

/* ^H: hidden files shown or not, in the current panel if that shows
   files. */
u16 M16_10(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);

    if (!FilePan(panel))
        return FUNC04;
    wcb->Hidden = wcb->Hidden == 0;     /* InvVar */
    return M16_11(panel);
}

/* Help, the first item of the Files menu: the page that the menu bar has
   set. */
u16 M21_00(void)
{
    Help00();
    return FUNC04;
}

/* Alt-F10: the tree in a box, to go to the directory that is chosen
   there. */
u16 M41_00(void)
{
    PutMnu(3);
    md.HlpPage = 28;
    if (ChTree(16))
        return INIT50;
    return F24_01;
}

/* Alt-F6: the sizes of the directories of the current panel, if that
   shows files, worked out with a box on the screen meanwhile; the list
   is put in order again, which may be by size. The lines of the box have
   no ^I. */
u16 M42_00(u16 panel)
{
    struct Box where;

    if (FilePan(panel)) {
        *(u16 *)md.TmpBuf = PANEL(panel)->WinPath[0];
        PutMnu(3);
        DialBox(0x528, OFFSET(DiSiz_P), 0, MAIN_SEG,
                (struct Box STACK *)&where);
        Window(where.begin, where.size);
        DirSize(panel);
        M16_20(panel);
        ResWin(where.begin, where.size, where.saved);
        Window(where.begin, where.size);
    }
    return INIT50;
}

/* M60_02: the prompt written again, and the panels drawn again if the
   panel `panel` is on. The configuration and EGA lines come here with DS
   on the main data instead: PutPrm and the test then read there what
   they would read of a panel. PutPrm takes of a panel only whether it is
   the left one, for a column that it does not use. The test finds the
   third byte of the main data, the colour of selected files in the
   first palette, which is 0Fh as the program comes: the panels are
   drawn again. */
static u16 M60_02(u16 panel)
{
    PutPrm(panel);
    if (PANEL(panel)->Visible == 0)
        return FUNC04;
    return INIT50;
}

/* Alt-F9: the screen with as many lines as an EGA or a VGA has for the
   small font, or with 25 again. The BIOS is asked how high the font is,
   the mode is set anew, which makes it 25 lines, and if the font was 14
   lines or more the small one is loaded; then the BIOS is asked how many
   lines that makes. What is on the screen moves up if it gets shorter,
   so that its last lines stay, and the command line with it; a screen
   that gets longer has empty lines below.

   The registers go from one call of the BIOS to the next as it leaves
   them, as in the assembly: BL, which says where the font is loaded, is
   0 from the first call and is not set again. CX and DX begin as the
   assembly has them, with the high byte of `item` and with the size of
   the window that Quit has put back; the BIOS takes neither. */
u16 F69_00(u16 item)
{
    struct BiosRegs regs;
    u16 height, rows, lines, was, less, count, cell, i;
    const u16 *from;
    u16 *to;

    if (md.TstEGA == 0)
        return FUNC04;
    Quit();
    ClsLine((u16)md.DosLin << 8, MAIN_SEG);
    SavMous();
    CurMous(0);
    md.BarPrev = 0xFF;
    md.ScrPage = 0;
    regs.ax = 0x1130;                   /* about the font */
    regs.bx = 0;
    regs.cx = item & 0xFF00 | 16;
    regs.dx = (u16)(u8)(md.Lines - 1) << 8 | 80;
    Int10((struct BiosRegs STACK *)&regs);
    height = regs.cx;
    regs.ax = md.ScrMod;                /* set the mode */
    Int10((struct BiosRegs STACK *)&regs);
    if ((u8)height >= 14) {
        regs.ax = 0x1112;               /* the font of 8 lines */
        Int10((struct BiosRegs STACK *)&regs);
        regs.ax = 0x1200 | regs.ax & 0xFF;      /* a routine to print the */
        regs.bx = regs.bx & 0xFF00 | 0x20;      /* screen that knows of it */
        Int10((struct BiosRegs STACK *)&regs);
    }
    /* F69_03 */
    regs.dx &= 0xFF00;
    regs.bx &= 0xFF;
    regs.ax = 0x1130;
    Int10((struct BiosRegs STACK *)&regs);
    lines = 25;
    rows = regs.dx & 0xFF;              /* the last row */
    if (rows != 0) {
        rows = (u8)(rows + 1);
        lines = MaxLins;
        if (lines > rows)
            lines = rows;
    }
    /* F69_04 */
    was = md.Lines;
    md.Lines = (u8)lines;
    if (was > lines) {
        less = was - lines;
        if (md.DosLin > less)
            md.DosLin -= (u8)less;
        else
            md.DosLin = 1;
        from = md.ScrBuf + less * 80;
        to = md.ScrBuf;
        for (count = lines * 80; count != 0; count--)
            *to++ = *from++;
    } else if (was < lines) {
        to = md.ScrBuf + was * 80;
        cell = (u16)md.CrntCol << 8 | ' ';
        for (count = (u8)(lines - was) * 80; count != 0; count--)
            *to++ = cell;
    }
    /* F69_07 */
    for (i = 0; i < (MaxLins - 1) * 80; i++)
        md.UserScr[i] = md.ScrBuf[i];
    ScrOffs();
    if (md.Mouse != 0) {
        count = ((u16)(signed char)md.Lines << 3) - 1;
        if (md.MouseY > count)
            md.MouseY = count;
        IniMous();
    }
    return M60_02(MAIN_SEG);
}

/* Mark in a panel every file that the other panel has not, has as a
   directory, or has with an earlier date and time. Returns 1 if there is
   one, and `marked` otherwise. */
static u16 M49_10(u16 panel, u16 marked)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u16 other = InvWCB(panel);
    struct FILENT __far *file;
    const struct FILENT __far *its;
    u16 address, found, i;

    for (address = DSKBUF; address < wcb->EndBuf;
         address += sizeof(struct FILENT)) {
        file = ENTRY(wcb, address);
        if (file->AttrF & ATTR_DIRECTORY)
            continue;
        for (i = 0; i < 13; i++)
            md.TmpBuf[i] = file->NameF[i];
        found = FnFile(other, md.TmpBuf);
        its = ENTRY(PANEL(other), found);
        if (found < PANEL(other)->EndBuf
            && !(its->AttrF & ATTR_DIRECTORY)
            && (its->DateF > file->DateF
                || its->DateF == file->DateF
                   && its->TimeF >= file->TimeF))
            continue;
        file->AttrF |= ATTR_SELECTED;   /* M49_12 */
        marked = 1;
    }
    return marked;
}

/* ^C: the directories of the two panels compared, if both show files:
   the files that are in one only, or newer in one, are selected there,
   in addition to what is selected. A box says so if there is none; its
   lines have no ^I. */
u16 M49_00(u16 panel)
{
    u16 other, marked;

    if (!FilePan(panel))
        return FUNC04;
    other = InvWCB(panel);
    if (!FilePan(other))
        return FUNC04;
    marked = M49_10(other, 0);
    marked = M49_10(panel, marked);
    if (marked == 0) {
        Dialog(OFFSET(CmpDi_B), 0, 0, 0, panel);
        return FUNC04;
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
u16 M60_00(void)
{
    u8 delay;

    md.TmpBuf[0] = md.ColTyp;
    md.TmpBuf[1] = md.ColTyp;
    for (delay = 0; md.BlDelay < Delay_T[delay]; delay++)
        ;
    md.TmpBuf[2] = delay;
    md.TmpBuf[3] = delay;
    if ((u8)Dialog(OFFSET(Conf_B), OFFSET(Chk1_S), 0, 0, MAIN_SEG) != 0)
        return FUNC04;
    md.ColTyp = md.TmpBuf[0];
    md.BlDelay = Delay_T[md.TmpBuf[2]];
    ResUser(0, (u16)(u8)(md.Lines - 2) << 8 | 80);
    PutMnu(0);
    return M60_02(MAIN_SEG);
}

/* The advanced options: all of them boxes to tick, which the dialog
   sets. */
u16 M61_00(void)
{
    Dialog(OFFSET(Sets_B), OFFSET(Chk2_S), 0, 0, MAIN_SEG);
    return FUNC04;
}

/* M64_02: ask whether the main file of the name at `name` is meant, the
   one in the program's directory, or that of the current directory, and
   have the editor open it. `title` is the title of the box, `include`
   what it calls the file, `page` its page of the help; all three and
   the name are offsets in the assembly.

   The assembly copies the title as 80 bytes and the name as 13, with
   what follows them there, and 64 of those bytes go on into ViewFil
   behind the name. So they are copied from the assembly here too, as
   long as that has them. */
static u16 M64_02(u16 item, u16 name, u16 title, u16 include, u16 page)
{
    u16 code = AsmSeg();
    const u8 __far *from;
    u16 answer, i;

    md.HlpPage = (u8)page;
    PutMnu(item);
    from = AT(code, title);
    for (i = 0; i < 80; i++)
        md.TmpBuf[i] = from[i];
    answer = Dialog(OFFSET(MnLoc_B), include, 0, 0, MAIN_SEG) & 0xFF;
    if (answer >= 2)
        return FUNC04;
    from = AT(code, name);
    for (i = 0; i < 13; i++)
        md.TmpBuf[i] = from[i];
    if (answer == 0)
        MainFil(md.TmpBuf, from);
    LetLin(md.TmpBuf);
    for (i = 0; i < LenStr; i++)
        md.ViewFil[i] = md.TmpBuf[i];
    md.ViewFil[LenStr] = 0;
    return V00_01;
}

/* Menu file edit, of the Commands menu. */
u16 M53_00(u16 item)
{
    return M64_02(item, OFFSET(Menu_F), OFFSET(User_S), OFFSET(UsMnu_S), 38);
}

/* Extension file edit, Viewers and Editors, of the Options menu. */
u16 M62_00(u16 item)
{
    return M64_02(item, OFFSET(Ext_F), OFFSET(ExFl_S), OFFSET(ExtFl_S), 42);
}

u16 M63_00(u16 item)
{
    return M64_02(item, OFFSET(ViewE_F), OFFSET(ExFl_S), OFFSET(ExtFl_S),
                  43);
}

u16 M64_00(u16 item)
{
    return M64_02(item, OFFSET(EditE_F), OFFSET(ExFl_S), OFFSET(ExtFl_S),
                  44);
}

/* M66_01: a switch of the Options menu, a byte of the setup that is 0 or
   1, made the other. */
static u16 M66_01(u16 panel, u8 *option)
{
    *option = *option == 0;             /* InvVar */
    return M60_02(panel);
}

u16 M66_00(u16 panel)                   /* Auto menus */
{
    return M66_01(panel, &md.AutoMnu);
}

u16 M67_00(u16 panel)                   /* Path prompt */
{
    return M66_01(panel, &md.Prompt);
}

/* Full screen: the panels have another height, so what DOS has shown is
   put back where the two are. */
u16 M69_00(u16 panel)
{
    ResUser(BegBX(panel) & 0xFF00, SizDX() & 0xFF00 | 80);
    return M66_01(panel, &md.FulScr);
}

/* Auto directory sizes: switched on, both panels are read again. */
u16 M70_00(u16 panel)
{
    if (md.AutoSiz == 0) {
        PANEL(panel)->ChgFlg = 1;
        PANEL(InvWCB(panel))->ChgFlg = 1;
    }
    return M66_01(panel, &md.AutoSiz);
}

u16 M71_00(u16 panel)                   /* Mini status */
{
    return M66_01(panel, &md.MiniSta);
}

u16 M72_00(u16 panel)                   /* Clock */
{
    return M66_01(panel, &md.Clock);
}

u16 M73_00(u16 panel)                   /* Memory allocation */
{
    return M66_01(panel, &md.LoadFl);
}

/* Shift-F9: save the setup, if the answer is yes, and say so if it could
   not be written, unless the drive was not ready, which a box of its own
   has said. The boxes show nothing of what the caller has in DS, which
   is the current panel. */
u16 M75_00(u16 panel)
{
    u32 answer;

    if ((u8)Dialog(OFFSET(SvSt_B), 0, 0, 0, panel) == 0) {
        answer = SaveSet();
        if (HIGH(answer) != 0 && LOW(answer) != ERR_NOT_READY)
            Dialog(OFFSET(SetEr_B), 0, 0, 0, panel);
    }
    return FUNC04;
}
