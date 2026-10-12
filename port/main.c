/*
 * The core of the program: the loop that takes a key and goes to its
 * command, with what draws the panels before it and what the mouse does
 * at the panels; the box that says a drive is not ready; and leaving.
 *
 * Translated from VC.ASM.
 *
 * The loop is no routine. The start-up code, which is still assembly,
 * falls into it; commands jump back into it at one of several places, and
 * it jumps to the command of a key, which is a label of the assembly. So
 * here it is one function, MainLoop, that is entered at a place, the
 * numbers below, and returns where the program goes on, the numbers of
 * Main_T in port/asm/VC.ASM, once a key or the mouse has said which
 * command it is: Func12 has the keys.
 *
 * What a command leaves in the registers reaches the next command through
 * the loop, and one command is known to read some of it: Shift-F6, which
 * renames, goes by BX and DX as they come. So the stub of the loop hands
 * the registers over as a structure on the stack, and gets them back as
 * the loop of the assembly would have left them at the jump: where the
 * code below sets a field of `r` that nothing here reads, that is why.
 */
#include "vc.h"

/* The registers that go round the loop, as the stub pushes them. */
struct Regs {
    u16 ax, bx, cx, dx, si, di, bp;
    u16 ds;                             /* the panel that commands take */
};

u16 MainLoop(u16 entry, struct Regs STACK *r);
u16 ErrRdy(u16 letter);
void Exit(void);
/* Quit, RestVct and HookVct are in vc.h. */
void RstVc1(void);

#pragma aux MainLoop "*_c" __far parm [ax] [bx] value [ax] modify exact [ax]
#pragma aux ErrRdy   "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux Exit     "*_c" __far modify exact [ax]
#pragma aux RstVc1   "*_c" __far modify exact [ax dx]

/* Where the loop is entered: what MainC of port/asm/VC.ASM hands over. */
enum {
    INIT40,                             /* find the command line on the
                                           screen, keep what DOS shows,
                                           and draw everything */
    INIT50,                             /* draw the panels, reading those
                                           that have changed */
    FUNC01,                             /* draw the panel in DS */
    FUNC02,                             /* put the screen buffer on the
                                           screen */
    FUNC04,                             /* draw the menu bar */
    FUNC05,                             /* save the setup if it has
                                           changed and is saved by itself */
    FUNC07,                             /* wait for a key */
    FUNC12                              /* go to the command of the key
                                           in AX */
};

/* Where the program goes on: the places of Main_T. */
enum {
    TO_F73_00,                          /* the key goes to the command
                                           line */
    TO_F77_00,                          /* the speed search */
    TO_F11_00,                          /* the panels off or on */
    TO_F39_00,                          /* the menu bar */
    TO_F20_00,                          /* the other panel */
    TO_F34_10,                          /* the editor on dirinfo */
    TO_F00_60,                          /* the cursor to an entry */
    TO_F00_05,                          /* a line up */
    TO_F01_05,                          /* a line down */
    TO_F21_10,                          /* Enter on the entry */
    TO_F21_31,                          /* run the command in DosBuf */
    /* The other commands of the keys, in the order of Main_T: see
       Func12. */
    TO_F00_00, TO_F01_00, TO_F02_00, TO_F03_00, TO_F04_00, TO_F05_00,
    TO_F06_00, TO_F07_00, TO_F08_00, TO_F09_00, TO_F10_00, TO_F76_00,
    TO_F21_00, TO_F27_00, TO_F74_00, TO_F78_00, TO_F12_00, TO_F13_00,
    TO_F14_00, TO_F15_00, TO_F16_00, TO_F17_00, TO_M16_00, TO_M16_10,
    TO_M30_00, TO_M49_00, TO_F18_00, TO_F79_00, TO_F22_00, TO_F23_00,
    TO_F24_00, TO_F25_00, TO_F26_00, TO_F80_00, TO_F81_00, TO_F82_00,
    TO_F30_00, TO_F32_00, TO_F33_00, TO_F34_00, TO_F35_00, TO_F36_00,
    TO_F37_00, TO_F38_00, TO_F40_00, TO_F42_00, TO_F43_00, TO_F44_00,
    TO_F45_00, TO_F46_00, TO_F47_00, TO_F48_00, TO_M75_00, TO_F54_00,
    TO_F55_00, TO_F56_00, TO_F58_00, TO_F61_00, TO_F62_00, TO_F63_00,
    TO_F64_00, TO_F65_00, TO_M42_00, TO_F67_00, TO_F68_00, TO_F69_00,
    TO_M41_00, TO_F75_00,
    AGAIN = 0x100                       /* not a place: go round, from the
                                           entry in the low byte */
};

/* What is at an offset in a segment, and the offset of something that
   the assembly defines. */
#define AT(segment, offset) \
    ((u8 __far *)((u32)(segment) << 16 | (u16)(offset)))
#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

/* Routines of the rest of the program that this file calls. Those that
   are declared below are still assembly, and are called by their entries
   for C; the others are C, of vc.h.

   HidCur puts the cursor below the screen, Cursor at a row and a column,
   the row in the high byte, and Cursor1 on the command line. ClrKbd empties the
   keyboard buffer if the setup has it so. ResWin puts back what a box
   covered, ResUser what DOS has shown, for a window given like Window's,
   and SavScr takes the screen into the screen buffer.

   GetMous returns the buttons of the mouse that are down, and in the high
   word where the mouse is, or `place` as it was if there is no mouse.
   TypMous gives the mouse pointer the look of a character, or its own for
   0. InWind is not 0 if a place is outside a window. DoublMo answers 2 if
   a click on the line `line` is the first one there, 1 if it is the
   second, in time, and 0 if the button went up somewhere else.

   TstDrv is 1 if there is a drive of the letter. SelDrv is the call of
   DOS that makes a drive the current one, 0 for A:, and Int21d any call
   of DOS that the assembly made without Intr21; DosTime asks DOS for the
   time of day, the hours and minutes in the low word. Int10 calls the
   BIOS for the screen.

   MakeIni writes into MenuBuf what the setup file holds, and SaveSet
   writes that file: both of vc.h. ReDir reads what a panel shows,
   keeping the cursor and the marks if `keep` is not 0: not 0 if it could
   not be read.

   AsmSeg returns the code segment of the assembly and ResSeg the segment
   of the part of the program that stays in the memory while a command
   runs, where the stack is. */
u16 ReDir(u16 panel, u16 keep);
u16 AsmSeg(void);
u16 ResSeg(void);
#pragma aux ReDir   "*_a" __far parm [dx] [ax] value [ax] modify exact [ax es]
#pragma aux AsmSeg  "*_a" __far value [ax] modify exact [ax]
#pragma aux ResSeg  "*_a" __far value [ax] modify exact [ax]

/* Routines of other files that are C. PutFls writes a panel of files,
   PutTre the tree and PutInf, of vc.h, the Info. */
void Panel(u16 panel);                                  /* panel.c */
void PutPath(u16 panel);
void PutFls(u16 panel);
void PutTre(u16 panel);                                 /* tree.c */
u16 StrLen(const u8 __far *text);                       /* string.c */
const u8 __far *FndLet(const u8 __far *text);
#pragma aux Panel   "*_c" __far parm [ax] modify exact [ax]
#pragma aux PutPath "*_c" __far parm [ax] modify exact [ax]
#pragma aux PutFls  "*_c" __far parm [ax] modify exact [ax]
#pragma aux PutTre  "*_c" __far parm [ax] modify exact [ax]
#pragma aux StrLen  "*_c" __far parm [es di] value [cx] modify exact [cx]
#pragma aux FndLet  "*_c" __far parm [es si] value [es si] modify exact [si es]

/* What the assembly has that this reads or names: the scan codes of Alt
   with a digit or a letter, which begin the speed search; the routine
   that DOS calls for a critical error; and, in the part that stays in
   the memory, the routine for Ctrl-Break, which only returns, and the
   copy of the first panel that is kept while a command runs. And of
   port/main_t.c the box that says a drive is not ready, which DialBox
   takes by its offset. */
extern const u8 __far AltSr_T[];
extern const u8 __far NoRd_P[];
extern const u8 __far CtrlC[];
extern const u8 __far WCB1sav[];
#pragma aux AltSr_T "*"
#pragma aux NoRd_P "*"
#pragma aux CtrlC "*"
#pragma aux WCB1sav "*"

#define ALT_KEYS 38                     /* the length of AltSr_T */

#define KEY_MOUSE 0xFC                  /* the scan code that Input has
                                           for the mouse */

/* Where the setup file has a field of a panel and one of the main data
   (see WCBini and CmnIni in vc.h). The program keeps the file that it
   has written last, or found when it started, where the copy of the
   first panel is kept while a command runs: IniSave. */
#define INI_WCB(number, field) \
    (3 + (number) * WCBini + (u16)(u32)&((struct WCBdef __far *)0)->field)
#define INI_CMN(field) \
    (3 + 2 * WCBini + (u16)&((struct MainDat *)0)->field)

#define DOS_SET_VECTOR 0x2500           /* DOS: an interrupt to DS:DX */
#define DOS_PATH       0x4700           /* the current directory of a
                                           drive, to DS:SI */
#define DOS_UNIT       0x440F           /* which drive a unit is that
                                           stands for several */

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

/* Whether a character ends the command that the program was started
   with: 1 or 0. */
static u16 Init46(u16 c)
{
    return c == 0 || c == '\r' || c == '\n';
}

/* Find the row of the command line and keep what DOS has on the screen.
   Returns 1 if the program has been started with a command to run, which
   is in DosBuf then.

   Ported as it is: where the panels are, their row and size are asked
   for with DS on the main data, not on a panel. BegBX then answers for
   the right panel, and only the row is used. */
static u16 Init40(void)
{
    struct BiosRegs regs;
    const u8 __far *tail;
    u8 *to;
    u16 row, line, bottom, count, i, c;

    regs.bx = 0;
    regs.cx = 0;
    regs.dx = 0;
    regs.ax = 0x0F00;                   /* which page is shown */
    Int10((struct BiosRegs STACK *)&regs);
    regs.ax = 0x0300 | regs.ax & 0xFF;  /* where the cursor is */
    Int10((struct BiosRegs STACK *)&regs);
    row = regs.dx >> 8;
    if (md.KeyBar != 0) {
        line = (u8)(md.Lines - 1);
        if (row >= line) {
            row = (u8)(line - 1);
            regs.dx = regs.dx & 0xFF | row << 8;
            regs.ax = 0x0E0A;           /* a line feed: the screen scrolls */
            Int10((struct BiosRegs STACK *)&regs);
        }
    }
    /* Init41 */
    md.MenuBuf[0] = (u8)row;
    line = row;
    if (PANEL(md.WCBcrn)->Visible != 0) {
        bottom = (u8)((SizDX() >> 8) + (BegBX(MAIN_SEG) >> 8));
        if (line < bottom)
            line = bottom;
    }
    md.DosLin = (u8)line;               /* Init42 */
    HidCur();
    SavScr();
    IniMous();
    count = (u16)(u8)(md.Lines - 1) * 80;
    for (i = 0; i < count; i++)
        md.UserScr[i] = md.ScrBuf[i];

    /* The command the program was started with, once. */
    if (md.LoadRes != 0xFF)
        return 0;
    md.LoadRes = 0;
    tail = FndLet(AT(ResSeg(), 0x81));
    if (Init46(*tail))
        return 0;
    to = md.DosBuf;
    for (;;) {                          /* Init43 */
        c = *tail++;
        if (Init46(c))
            break;
        if (c == '\t')
            c = ' ';
        *to++ = (u8)c;
    }
    to[0] = 0;                          /* Init45 */
    to[1] = 0;
    md.DosLin = (u8)(md.MenuBuf[0] - 1);
    md.LoadRes = md.LoadFl;
    md.LoadFl = 0x81;
    return 1;
}

/* Give a panel that could not be read something to show: no files, a
   tree of the root alone, or an Info without figures. */
static void Init90(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct DIRENT *root;
    u16 i;

    if (wcb->WinTyp == 0) {
        wcb->EndBuf = DSKBUF;
        wcb->EndFul = DSKBUF;
        wcb->CurAdr = DSKBUF;
        wcb->First = DSKBUF;
    } else if (wcb->WinTyp == 2) {
        md.Total = 0;
        md.FreeDsk = 0;
        md.ClstSiz = 1;
        md.Volume[0] = 0;
        md.InfoLen = 0xFFFF;
    } else if (wcb->WinTyp < 2) {       /* Init92 */
        root = (struct DIRENT *)TRELIST;
        for (i = 0; i < sizeof(struct DIRENT); i++)
            ((u8 *)root)[i] = 0;
        root->DirName[0] = '\\';
        root->DirShift = 1;
        ((struct DIRTREE *)md.TreeBuf)->TreeLen = 1;
        md.TreFrst = TRELIST;
        md.TreAdr = TRELIST;
    }
}

/* Draw the two panels, reading from the disk what each shows if it has
   changed. The one that shows files goes first, the current one if both
   do: the directory of a panel of files becomes the current directory of
   DOS, which the tree and the Info of the other panel go by.

   A panel of files whose drive cannot be read is drawn empty, and
   ErrRdy asks for another drive. For the first panel the drive is made
   the current one and everything begins again; for the second its
   current directory is asked for and the panel is read again. `drive` is
   the letter of the drive that failed first, until the first panel has
   been read from it after all: a second panel on that drive is not even
   tried.

   Ported as it is: the window that is put on the screen has, after
   another drive has been chosen, the number of that drive for a width.
   Window goes by the width for a single row only.

   What the assembly leaves in the registers here is not kept: the loop
   goes on at Func02, and all of them are set again from there on
   before a command gets them, but BP, which this does not change. */
static void Init50(void)
{
    struct DosRegs regs;
    struct WCBdef __far *wcb;
    u16 panel, size, drive, keep, failed, letter, length, same, i;

    HidCur();
    panel = md.WCBcrn;
    if (PANEL(md.WCBsec)->WinTyp < PANEL(panel)->WinTyp)
        panel = md.WCBsec;
    wcb = PANEL(panel);
    size = (u16)md.Lines << 8 | 80;     /* Init51 */
    drive = 0;
    if (wcb->ChgFlg == 0)
        goto Init53;
Init52:
    md.BlocEr = 1;
    SetPth();
    md.BlocEr = 0;
Init53:
    PutPrm(panel);
    keep = 0;
    failed = 0;
    if (wcb->Visible == 0)
        goto Init70;
    Panel(panel);                       /* Init54 */
    if (wcb->ChgFlg != 0) {
        Window(0, size);
        if (wcb->WinTyp == 0) {
            /* The directory of DOS is the panel's now. Whether it was
               before says whether the cursor and the marks are kept. */
            length = StrLen(wcb->WinPath);
            same = 1;
            for (i = 0; i < length; i++)
                if (md.DosPth[i] != wcb->WinPath[i]) {
                    same = 0;
                    break;
                }
            for (i = 0; i < LenPath; i++)
                wcb->WinPath[i] = md.DosPth[i];
            md.OldFlg = (u8)same;
        }
        if (md.PathEr != 0) {           /* Init55 */
            Init90(panel);
            failed = 1;
        } else {
            if (drive == 0 && md.OldFlg != 0)   /* Init56 */
                keep = 1;
            if (wcb->WinTyp == 0)       /* Init57 */
                md.BlocEr = 1;
            failed = ReDir(panel, keep);
            md.BlocEr = 0;
        }
        if (failed) {                   /* Init59 */
            failed = 1;
            if (drive == 0)
                drive = wcb->WinPath[0];
        }
    }
    PutPath(panel);                     /* Init60 */
    PtFs00(panel);
    if (failed) {
        Window(0, size);
        if (wcb->WinTyp == 0) {
            letter = ErrRdy(wcb->WinPath[0]) & 0xFF;
            if (letter != 0) {
                if (drive == 0)
                    drive = letter;
                letter = (u8)(letter - 'A');    /* Init61 */
                size = size & 0xFF00 | letter;
                SelDrv(size);
                goto Init52;
            }
        }
    } else if (drive == wcb->WinPath[0]) {      /* Init62 */
        drive = 0;
    }

Init70:
    panel = InvWCB(panel);
    wcb = PANEL(panel);
    keep = 1;
    if (wcb->Visible == 0)
        return;
    for (;;) {
        Panel(panel);                   /* Init71 */
        failed = 0;
        if (wcb->ChgFlg != 0) {
            Window(0, size);
            if (wcb->WinTyp == 0 && drive == wcb->WinPath[0]) {
                wcb->WinPath[3] = 0;
                Init90(panel);          /* Init73 */
                failed = 1;
            } else if (wcb->WinTyp != 0 && md.PathEr != 0) {   /* Init72 */
                Init90(panel);
                failed = 1;
            } else {
                if (wcb->WinTyp == 0)   /* Init74 */
                    md.BlocEr = 1;
                failed = ReDir(panel, keep);    /* Init75 */
                md.BlocEr = 0;
                if (failed)
                    failed = 1;
            }
        }
        PutPath(panel);                 /* Init77 */
        PtFs00(panel);
        if (wcb->WinTyp != 0 || !failed)
            return;
        Window(0, size);
        letter = ErrRdy(wcb->WinPath[0]) & 0xFF;
        if (letter == 0)
            return;
        wcb->WinPath[3] = 0;
        drive = letter;
        wcb->WinPath[0] = (u8)letter;
        letter = (u8)(letter - ('A' - 1));
        size = size & 0xFF00 | letter;
        if (!Fantom(letter)) {
            regs.ax = DOS_PATH | letter;
            regs.bx = 0;
            regs.cx = drive;
            regs.name = (const u8 __far *)wcb + size;   /* DL, and DS */
            regs.si = (u16)(u32)(wcb->WinPath + 3);
            regs.di = 0;
            md.BlocEr = 1;
            failed = Intr21((struct DosRegs STACK *)&regs);
            md.BlocEr = 0;
            if (!failed)
                drive = 0;
        }
        keep = 0;                       /* Init78 */
    }
}

/* What the mouse does at the panels, with a button down: `r->ax` is the
   buttons, `r->cx` where the mouse is, the row in the high byte, and
   `r->ds` the current panel. Returns where the program goes on, or AGAIN
   with the place of the loop.

   A button in the top left corner switches the panels off and on when it
   comes up there; one on the top row is the menu bar's; one below the
   panels does nothing. A button in the other panel makes that the
   current one. In the current one, the lower part of an Info edits
   dirinfo, and in a list the cursor goes where the mouse is, the list
   moves when the mouse is above or below it, and a second click on an
   entry is Enter there. WinMous is not 0 while a button is held that
   went down in the list.

   Ported as they are: what DoublMo leaves in AX and CX is not handed on;
   where the program goes on then, at F00_60, at Func04 and at F21_10,
   both are set before they are read. */
static u16 Func21(struct Regs STACK *r)
{
    struct WCBdef __far *wcb;
    struct Place place;
    u32 pair;
    u16 look, lines, rows, column, kind, answer;

    ClrKbd();
    if (md.WinMous != 0)
        goto Func30;
    if (r->cx < 2) {                    /* Panels on/off */
        look = 0;
        for (;;) {                      /* Func22 */
            pair = GetMous(r->cx);
            r->ax = (u16)pair;
            r->cx = (u16)(pair >> 16);
            if (r->ax == 0)
                break;
            r->ax = r->cx < 2 ? 0xFE : 0;
            if (r->ax == look)
                continue;
            look = r->ax;
            r->bx = r->bx & 0xFF00 | look;
            TypMous(look);
        }
        r->bx = r->bx & 0xFF00 | look;  /* Func24 */
        TypMous(0);
        if (look == 0)
            return AGAIN | FUNC07;
        return TO_F11_00;
    }
    if ((r->cx >> 8) == 0)              /* Func25: pull down menu */
        return TO_F39_00;
    r->bx = BegBX(r->ds);               /* Func26 */
    r->dx = SizDX();
    r->dx = r->dx & 0xFF | (u16)(u8)((r->dx >> 8) + (r->bx >> 8)) << 8;
    if ((r->cx >> 8) >= (r->dx >> 8))
        return AGAIN | FUNC07;
    if ((r->cx & 0xFF) < 40) {          /* Change panel */
        r->ds = md.WCB1seg;
        if (md.Active == 0)
            goto Func30;
    } else {                            /* Func27 */
        r->ds = md.WCB2seg;
        if (md.Active != 0)
            goto Func30;
    }
    r->ds = md.WCBcrn;                  /* Func28 */
    return TO_F20_00;

Func30:
    wcb = PANEL(r->ds);
    if ((r->ax & 3) == 0 || wcb->Visible == 0)
        return AGAIN | FUNC07;
    if (wcb->WinTyp < 2)
        goto Func40;
    if (wcb->WinTyp != 2)
        return AGAIN | FUNC07;
    r->bx = BegBX(r->ds) + 0x0B01;      /* Edit 'dirinfo' */
    r->dx = SizDX() - 0x0C02;
    if (InWind(r->cx, r->bx, r->dx))
        return AGAIN | FUNC07;
    return TO_F34_10;

Func40:                                 /* Move the cursor */
    md.WinMous |= 1;
    *(u16 *)md.TmpBuf = r->cx;          /* TmpMous */
    r->bp = r->ax;
    AdrNum(r->ds, (struct Place STACK *)&place);
    r->si = place.cursor;
    r->di = place.first;
    r->dx = place.last;
    lines = place.lines;
    r->ax = lines;
    r->bx = BegBX(r->ds);
    column = r->cx & 0xFF;
    if (column < (r->bx & 0xFF) || column >= (u8)((r->bx & 0xFF) + 40))
        return AGAIN | FUNC07;
    r->bx += 0x100;
    if (wcb->WinTyp == 0)
        r->bx += 0x100;
    rows = r->cx >> 8;                  /* Func41 */
    r->cx = r->cx & 0xFF | (u16)(u8)(rows - (r->bx >> 8)) << 8;
    if (rows < r->bx >> 8) {            /* Func50: scroll up */
        r->si = r->di;
        return TO_F00_05;
    }
    rows = r->cx >> 8;
    kind = wcb->WinTyp | wcb->BrfFul;
    if (rows >= (lines & 0xFF)) {       /* Func55: scroll down */
        r->bx = r->bx & 0xFF00 | kind;
        if (kind == 0)
            r->ax = (lines & 0xFF) * 3;
        r->ax--;                        /* Func56 */
        r->si = r->di + r->ax;
        if (r->si > r->dx)
            r->si = r->dx;
        return TO_F01_05;
    }
    column = (u8)(column - (r->bx & 0xFF));     /* Set cursor position */
    r->cx = r->cx & 0xFF00 | column;
    if (column >= 39)
        return AGAIN | FUNC07;
    r->bx = r->bx & 0xFF00 | kind;
    if (kind == 0) {                    /* three columns of names */
        r->bx = r->bx & 0xFF00 | 13;
        r->ax = (column / 13) * (lines & 0xFF);
        column %= 13;
        rows = (u8)(rows + r->ax);
        r->cx = rows << 8 | column;
    }
    if (column == 0)                    /* Func42 */
        return AGAIN | FUNC07;
    r->ax = (u16)(signed char)rows + r->di;
    r->si = r->ax;
    if (r->si > r->dx) {
        r->si = r->dx;
        return TO_F00_60;
    }
    if (!(r->bp & 1))                   /* Func43 */
        return TO_F00_60;

    r->cx = *(u16 *)md.TmpBuf;          /* Func60: double click */
    r->bx = BegBX(r->ds) + 1;
    r->bx = r->bx & 0xFF | r->cx & 0xFF00;
    r->dx = 0x126;
    r->cx &= 0xFF;
    r->ax = r->ax & 0xFF00 | kind;
    if (kind == 0) {
        column = (u8)(r->cx - (r->bx & 0xFF));
        column = (u16)(signed char)column / 13 * 13;
        r->ax = column;
        r->cx = 13;
        r->bx = r->bx & 0xFF00 | (u8)(r->bx + column);
        r->dx = 0x10C;
    }
    answer = DoublMo(r->bx, r->dx, r->si);      /* Func61 */
    if (answer == 2)
        return TO_F00_60;
    if (answer == 0)
        return AGAIN | FUNC04;
    r->ax &= 0xFF00;                    /* Func62 */
    TypMous(0);
    return TO_F21_10;
}

/* The command of a key, a place of Main_T: the command line for a key
   that has none, and for one without a scan code.

   The assembly had a table of keys and commands, FncTab, which Case went
   through from its start: an entry was a key, or a character that counts
   with any scan code. Every character of the second kind that a key of
   the first kind has too, Tab, Enter and Esc, stood behind that key, so
   the keys are looked for first here. Case left SI behind the entry it
   had found, or on the table: no command reads it, and here it stays as
   it is. */
static u16 Func12(u16 key)
{
    if (key >> 8 == 0)
        return TO_F73_00;
    switch (key) {
    case 0x4800: return TO_F00_00;  /* Up */
    case 0x5000: return TO_F01_00;  /* Down */
    case 0x4B00: return TO_F02_00;  /* Left */
    case 0x4D00: return TO_F03_00;  /* Right */
    case 0x4900: return TO_F04_00;  /* PgUp */
    case 0x5100: return TO_F05_00;  /* PgDn */
    case 0x4700: return TO_F06_00;  /* Home */
    case 0x4F00: return TO_F07_00;  /* End */
    case 0x5200: return TO_F08_00;  /* Ins: select / unselect */
    case 0x4E2B: return TO_F09_00;  /* Grey +: select group */
    case 0x4A2D: return TO_F10_00;  /* Grey -: unselect group */
    case 0x372A: return TO_F76_00;  /* Grey *: invert group */
    case 0x0F09: return TO_F20_00;  /* Tab: change active panel */
    case 0x1C0D: return TO_F21_00;  /* Enter: run / change directory */
    case 0x011B: return TO_F74_00;  /* Esc: clear command line / on/off */
    case 0x5E00: return TO_F13_00;  /* ^F1: on/off left panel */
    case 0x5F00: return TO_F14_00;  /* ^F2: on/off right panel */
    case 0x2308: return TO_M16_10;  /* ^H: hidden files */
    case 0x8400: return TO_F22_00;  /* ^PgUp: CD .. */
    case 0x7600: return TO_F23_00;  /* ^PgDn: change directory */
    case 0x4400: return TO_F30_00;  /* F10: quit */
    case 0x3C00: return TO_F32_00;  /* F2: user menu */
    case 0x3D00: return TO_F33_00;  /* F3: view */
    case 0x3E00: return TO_F34_00;  /* F4: edit */
    case 0x3F00: return TO_F35_00;  /* F5: copy */
    case 0x4000: return TO_F36_00;  /* F6: rename / move */
    case 0x4100: return TO_F37_00;  /* F7: make directory */
    case 0x4200: return TO_F38_00;  /* F8: delete */
    case 0x4300: return TO_F39_00;  /* F9: pull down menu */
    case 0x5D00: return TO_F40_00;  /* Shift F10: pull down menu */
    case 0x5500: return TO_F42_00;  /* Shift F2: main menu... */
    case 0x5600: return TO_F43_00;  /* Shift F3: view... */
    case 0x5700: return TO_F44_00;  /* Shift F4: edit... */
    case 0x5800: return TO_F45_00;  /* Shift F5: copy... */
    case 0x5900: return TO_F46_00;  /* Shift F6: rename/move... */
    case 0x5A00: return TO_F47_00;  /* Shift F7: make directory... */
    case 0x5B00: return TO_F48_00;  /* Shift F8: delete... */
    case 0x5C00: return TO_M75_00;  /* Shift F9: save setup */
    case 0x6100: return TO_F54_00;  /* ^F4: change label */
    case 0x6200: return TO_F55_00;  /* ^F5: copy file */
    case 0x6300: return TO_F56_00;  /* ^F6: rename/move file */
    case 0x6500: return TO_F58_00;  /* ^F8: delete file */
    case 0x6800: return TO_F61_00;  /* Alt F1: change left drive */
    case 0x6900: return TO_F62_00;  /* Alt F2: change right drive */
    case 0x6A00: return TO_F63_00;  /* Alt F3: internal viewer */
    case 0x6B00: return TO_F64_00;  /* Alt F4: internal editor */
    case 0x6C00: return TO_F65_00;  /* Alt F5: memory info */
    case 0x6D00: return TO_M42_00;  /* Alt F6: directory sizes */
    case 0x6E00: return TO_F67_00;  /* Alt F7: file find */
    case 0x6F00: return TO_F68_00;  /* Alt F8: history */
    case 0x7000: return TO_F69_00;  /* Alt F9: EGA lines */
    case 0x7100: return TO_M41_00;  /* Alt F10: Tree */
    case 0xFD00: return TO_F39_00;  /* Alt: pull down menu */
    case 0xFE00: return TO_F75_00;  /* the time in AutoTim has come:
                                           auto CD / quick view */
    }
    switch (key & 0xFF) {
    case 0x0A: return TO_F27_00;  /* ^Enter: history command */
    case 0x0D: return TO_F78_00;  /* ^M: restore marks */
    case 0x0F: return TO_F11_00;  /* ^O: on/off panels */
    case 0x10: return TO_F12_00;  /* ^P: on/off unactive panel */
    case 0x15: return TO_F15_00;  /* ^U: swap panels */
    case 0x02: return TO_F16_00;  /* ^B: on/off key bar */
    case 0x12: return TO_F17_00;  /* ^R: re-read */
    case 0x06: return TO_M16_00;  /* ^F: filter */
    case 0x01: return TO_M30_00;  /* ^A: attributes */
    case 0x03: return TO_M49_00;  /* ^C: compare directories */
    case 0x0C: return TO_F18_00;  /* ^L: info */
    case 0x1A: return TO_F79_00;  /* ^Z: tree */
    case 0x1C: return TO_F24_00;  /* ^\: CD \ */
    case 0x05: return TO_F25_00;  /* ^E: previous history command */
    case 0x18: return TO_F26_00;  /* ^X: next history command */
    case 0x09: return TO_F80_00;  /* ^I: insert files */
    case 0x1B: return TO_F81_00;  /* ^[: insert left path */
    case 0x1D: return TO_F82_00;  /* ^]: insert right path */
    }
    return TO_F73_00;
}

/* The main loop: see the top of this file. `entry` is where it is
   entered and `r` the registers as they are there; it returns where the
   program goes on, with the registers as that place gets them. */
u16 MainLoop(u16 entry, struct Regs STACK *r)
{
    struct WCBdef __far *wcb;
    u8 __far *saved;
    const u8 __far *keys;
    const u8 *image;
    u32 pair;
    u16 i, length, same, time, seconds, hundredths, menu, key, where;

    for (;;) {
        switch (entry) {
        case INIT40:
            if (Init40())
                return TO_F21_31;
            PutMnu(0);                  /* Init49 */
            PutKey(0xFF);
            /* fall through */
        case INIT50:
            Init50();
            r->ds = md.WCBcrn;          /* Init80 */
            goto Func02;
        case FUNC01:
            Panel(r->ds);
            PutPath(r->ds);
            PtFs00(r->ds);
            /* fall through */
        case FUNC02:
        Func02:
            r->bx = 0;
            r->dx = (u16)md.Lines << 8 | 80;
            Window(r->bx, r->dx);       /* Func03 */
            /* fall through */
        case FUNC04:
            PutMnu(0);
            md.OneFile = 0;
            md.FlagCD = 0;
            md.MousEnt = 0;
            md.MousTim |= 0xFFFF0000UL;
            /* fall through */
        case FUNC05:
            /* The setup as it is now, held against the one that was
               saved last. The directories of the panels, which panel is
               the current one and whether the other is switched off do
               not count: they are taken over first. */
            MakeIni();
            image = md.MenuBuf;
            saved = AT(ResSeg(), OFFSET(WCB1sav));
            if (*(u16 __far *)saved == 0x5656) {        /* 'VV' */
                for (i = 0; i < LenPath; i++) {
                    saved[INI_WCB(0, WinPath) + i] =
                        image[INI_WCB(0, WinPath) + i];
                    saved[INI_WCB(1, WinPath) + i] =
                        image[INI_WCB(1, WinPath) + i];
                }
                saved[INI_CMN(Active)] = image[INI_CMN(Active)];
                saved[INI_CMN(Ctrl_O)] = image[INI_CMN(Ctrl_O)];
                r->bx = INI_CMN(Ctrl_O);
                same = 1;
                for (i = 0; i < INI_LEN - 2; i++)
                    if (saved[i] != image[i]) {
                        same = 0;
                        break;
                    }
                if (!same && md.AutoSav != 0) {
                    HidCur();
                    md.BlocEr = 1;
                    SaveSet();
                    md.BlocEr = 0;
                }
            }
            for (i = 0; i < INI_LEN; i++)       /* Func06 */
                saved[i] = image[i];
            r->si = (u16)md.MenuBuf + INI_LEN;
            r->di = OFFSET(WCB1sav) + INI_LEN;
            /* fall through */
        case FUNC07:
            Cursor1();
            r->ds = md.WCBcrn;
            wcb = PANEL(r->ds);
            /* When the cursor of a tree has moved, the directory under
               it becomes the current one after a while: AutoTim is when,
               or has 0FFFFh in its low word for never. */
            time = 0xFFFF;
            if (md.AutoCD != 0 && wcb->WinTyp == 1 && wcb->Visible != 0) {
                length = StrLen(md.DosPth);
                same = 1;
                for (i = 0; i < length; ) {
                    i++;
                    if (md.CrnPath[i - 1] != md.DosPth[i - 1]) {
                        same = 0;
                        break;
                    }
                }
                r->si = (u16)md.CrnPath + i;
                r->di = (u16)md.DosPth + i;
                if (!same && md.FlagCD != 0) {
                    pair = DosTime();
                    time = (u16)pair;
                    hundredths = (u8)((u16)(pair >> 16) + md.DelayTm[0]);
                    seconds = (u16)(pair >> 24);
                    if (hundredths >= 100) {
                        hundredths -= 100;
                        seconds = (u8)(seconds + 1);
                    }
                    seconds = (u8)(seconds + md.DelayTm[1]);    /* Func08 */
                    if (seconds >= 60) {
                        seconds -= 60;
                        time++;
                        if ((time & 0xFF) >= 60) {
                            time -= 60;
                            time += 0x100;
                            if (time >> 8 >= 24)
                                time &= 0xFF;
                        }
                    }
                    r->dx = seconds << 8 | hundredths;
                }
            }
            md.AutoTim = (u32)r->dx << 16 | time;       /* Func10 */
            md.HlpPage = 0;
            menu = md.MenuFl;
            md.MenuFl = 0;
            r->cx = menu << 8;
            r->ax = 0x3C00;             /* F2, for the menu that comes up
                                           by itself */
            if (menu == 0) {
                pair = GetMous(r->cx);
                r->ax = (u16)pair;
                r->cx = (u16)(pair >> 16);
                if (r->ax != 0)
                    goto Mouse;
                md.WinMous = 0;
                md.MousEnt = 0;
                key = Input(md.TstEGA != 0 ? 0 : 9);    /* Func11 */
                r->ax = key;
                if (key >> 8 == KEY_MOUSE)
                    goto Func20;
                if (wcb->Visible != 0 && wcb->WinTyp <= 1
                    && (key & 0xFF) == 0) {
                    keys = AT(AsmSeg(), OFFSET(AltSr_T));
                    same = 0;
                    for (i = 0; i < ALT_KEYS; ) {
                        i++;
                        if (keys[i - 1] == key >> 8) {
                            same = 1;
                            break;
                        }
                    }
                    r->di = OFFSET(AltSr_T) + i;
                    r->cx = ALT_KEYS - i;
                    if (same)
                        return TO_F77_00;
                }
            }
            /* fall through */
        case FUNC12:
            if (r->ax >> 8 == KEY_MOUSE)
                goto Func20;
            if (md.Quote != 0)
                return TO_F73_00;
            return Func12(r->ax);
        }

    Func20:                             /* Mouse support */
        pair = GetMous(r->cx);
        r->ax = (u16)pair;
        r->cx = (u16)(pair >> 16);
    Mouse:
        where = Func21(r);
        if (!(where & AGAIN))
            return where;
        entry = where & 0xFF;
    }
}

/* Say that the disk of a drive cannot be read, and take the letter of
   another drive, which is shown in the box; Enter tries the drive that
   the box shows, Esc and F10 give up. Only the low byte of `letter`
   counts. Returns the letter in the low byte, or 0 there for none, with
   the high byte as it came.

   With the mouse, the right button is Enter, the left one outside the
   box gives up, and so do both together.

   Ported as they are: a button that is up again when the mouse is asked
   gives up as well. And whatever the answer, DOS is told that the drive
   of the letter is the one that its unit stands for, where a unit stands
   for two, as one floppy drive does for A: and B:. */
u16 ErrRdy(u16 letter)
{
    struct DosRegs regs;
    struct Box where;
    u32 pair;
    u16 *cell;
    u16 drive, key, c, buttons, place, look, shown, last;

    *(u16 *)(md.TmpErr + 1) = (u16)(signed char)letter;
    drive = letter & 0xFF;
    PutKey(2);
    DialBox(0x528, OFFSET(NoRd_P), 0, MAIN_SEG, (struct Box STACK *)&where);
    Window(where.begin, where.size);
    Cursor(where.begin + 0x42C);
    ClrKbd();
    Beep();
    md.HlpPage = 0xFF;
    cell = BasAdr(where.begin) + 364;   /* the letter in the box */
    for (;;) {                          /* ErRdy1 */
        key = Input(2);
        if (key >> 8 != KEY_MOUSE) {
            if (key >> 8 != 0) {
                if ((key & 0xFF) == 0x1B || key == 0x4400)
                    goto ErRdy6;
                if ((key & 0xFF) == '\r')
                    goto ErRdy7;
            }
            c = key & 0xFF;
            if (c >= 0x80)
                continue;
            c = UpCase(c) & 0xFF;
            if (c < 'A' || c > 'Z' || !TstDrv(c))
                continue;
            *(u8 *)cell = (u8)c;
            drive = c;
            Window(where.begin + 0x42C, 0x101);
            continue;
        }
        place = 0;                      /* ErRdy2 */
        last = 0;
        shown = 0;
        for (;;) {                      /* ErRdy3 */
            pair = GetMous(place);
            buttons = (u16)pair;
            if (buttons == 0)
                break;
            place = (u16)(pair >> 16);
            look = buttons == 1 ? 0 : buttons == 2 ? 0xFB : 'x';
            last = buttons;             /* ErRdy4 */
            if (look == shown)
                continue;
            shown = look;
            TypMous(look);
        }
        TypMous(0);                     /* ErRdy5 */
        if ((last & 0xFF) == 2)
            goto ErRdy7;
        if ((last & 0xFF) == 1
            && !InWind(place, where.begin, where.size - 0x102))
            continue;
        goto ErRdy6;
    }
ErRdy6:
    drive = 0;
ErRdy7:
    HidCur();
    ResWin(where.begin, where.size, where.saved);
    Window(where.begin, where.size);
    regs.ax = DOS_UNIT;
    regs.bx = where.begin & 0xFF00 | (u8)(drive - ('A' - 1));
    regs.cx = drive;
    regs.name = (const u8 __far *)&md + where.size;
    regs.si = 0;
    regs.di = 0;
    Int21d((struct DosRegs STACK *)&regs);
    return letter & 0xFF00 | drive;
}

/* Take the key bar off the screen buffer, if there is one, and put there
   what DOS had on the screen above it. */
void Quit(void)
{
    u16 line = (u8)(md.Lines - 1);

    if (md.KeyBar != 0)
        ClsLine(line << 8, MAIN_SEG);
    ResUser(0, line << 8 | 80);
}

/* Have DOS send an interrupt somewhere. */
static void SetVect(u16 number, const u8 __far *to)
{
    struct DosRegs regs;

    regs.ax = DOS_SET_VECTOR | number;
    regs.bx = 0;
    regs.cx = 0;
    regs.name = to;
    regs.si = 0;
    regs.di = 0;
    Int21d((struct DosRegs STACK *)&regs);
}

/* Where an interrupt goes, as DOS says: the offset in the low word. The
   low byte of `call` is the number of the interrupt. */
static u32 Dos35(u16 call)
{
    struct DosRegs regs;

    regs.ax = 0x3500 | call & 0xFF;
    regs.bx = 0;
    regs.cx = 0;
    regs.name = (const u8 __far *)&md;
    regs.si = 0;
    regs.di = 0;
    Int21d((struct DosRegs STACK *)&regs);
    return (u32)regs.es << 16 | regs.bx;
}

/* Give Ctrl-Break back to whoever had it. */
void RstVc1(void)
{
    SetVect(0x1B, (const u8 __far *)md.BrkVct);
}

/* Give the critical error of DOS and Ctrl-Break back to whoever had
   them. */
void RestVct(void)
{
    SetVect(0x24, (const u8 __far *)md.ErrVct);
    RstVc1();
}

/*--------------------- Fatal error ---------------------
   What port/main_t.c has for it: the texts and the two boxes, with
   Retry and Abort or with Ok alone, which go to Dialog by their offsets.
   And the assembly: where DS will be on the main data, SecDat. */
extern const u8 __far Err_S[];
extern const u8 __far OnDr_S[];
extern const u8 __far OnDv_S[];
extern const u8 __far Errs_S[];
extern const u8 __far FatEr_S[];
extern const u8 __far FtEr1_B[];
extern const u8 __far FtEr2_B[];
extern u16 __near SecDat;
#pragma aux Err_S "*"
#pragma aux OnDr_S "*"
#pragma aux OnDv_S "*"
#pragma aux Errs_S "*"
#pragma aux FatEr_S "*"
#pragma aux FtEr1_B "*"
#pragma aux FtEr2_B "*"
#pragma aux SecDat "*"

/* What DOS is answered for a critical error. */
#define ERR_RETRY 1
#define ERR_FAIL  3

/* A critical error of DOS: `ax` and `code` are what DOS hands over in AX
   and DI, and `device` is the header of the device. Say which drive or
   which device and what is wrong, in a box, and ask whether to try
   again, where DOS allows that; else the box has Ok alone. Returns what
   DOS is to do: try again, or have its call fail.

   Once a call has been let fail, ErrDat is set, and every error after
   it fails without a box until whoever called DOS clears that: Intr21.
   BlocEr does the same for those who want no box at all.

   The clock and the screen blanker are off while the box is up, and
   nothing is to happen by itself then: AutoTim. */
static u16 FtErr1(u16 ax, u16 code, const u8 __far *device)
{
    u16 assembly = AsmSeg();
    const u8 __far *from;
    const u8 __far *text;
    u8 *to = md.TmpErr + 1;
    u8 *name;
    u16 i, answer;
    u8 clock, delay, page;

    if (md.ErrDat | md.BlocEr)
        return ERR_FAIL;
    from = AT(assembly, OFFSET(Err_S) + 1);
    for (i = 6; i; i--)
        *to++ = *from++;
    text = AT(assembly, OFFSET(Errs_S));
    if (!(device[5] & 0x80)) {          /* a disk */
        from = AT(assembly, OFFSET(OnDr_S));
        for (i = 11; i; i--)
            *to++ = *from++;
        to[-2] += (u8)ax;
        if (ax & 0x8000) {
            text = AT(assembly, OFFSET(FatEr_S));
            goto FtErr5;
        }
    } else {                            /* FtErr2: a device, by its name */
        from = AT(assembly, OFFSET(OnDv_S));
        for (i = 10; i; i--)
            *to++ = *from++;
        name = to;
        from = device + 10;
        for (i = 8; i; i--)
            *to++ = *from++;
        *to = ' ';
        for (to = name, i = 9; i; i--)
            if (*to++ == ' ')
                break;
        to--;
        *to++ = ':';
        *to = 0;
    }
    if (code != 0) {                    /* FtErr3 */
        if (code >= 0x15)
            code = 0x15;
        do
            for (i = 80; i; i--)
                if (*text++ == 0)
                    break;
        while (--code);
    }
FtErr5:
    clock = md.Clock;
    md.Clock = 0;
    delay = md.BlDelay;
    md.BlDelay = 0;
    page = md.HlpPage;
    *(u16 *)&md.AutoTim = 0xFFFF;
    if (ax & 0x1000)
        answer = (u8)Dialog(OFFSET(FtEr1_B), (u16)(u32)text, 0, 0, MAIN_SEG)
                 == 0 ? ERR_RETRY : ERR_FAIL;
    else {
        Dialog(OFFSET(FtEr2_B), (u16)(u32)text, 0, 0, MAIN_SEG);
        answer = ERR_FAIL;
    }
    md.Clock = clock;                   /* FtErr8 */
    md.BlDelay = delay;
    md.HlpPage = page;
    if (answer != ERR_RETRY)
        md.ErrDat = 1;
    return answer;
}

/* Put DS on the main data. An interrupt function is entered with DS
   anywhere, and the compiler puts it on its own group, which is the
   assembly's and where SecDat is: the segment of the main data, which
   the program asks DOS for when it starts and after every command. C
   has no words for loading a segment register, and the compiler takes
   none for an argument: this is the one instruction. */
static void DataSeg(u16 segment);
#pragma aux DataSeg = "mov ds,ax" parm [ax] modify exact []

/* INT 24h: what DOS calls for a critical error, with the error in AX and
   DI and the device at BP:SI, on the stack of whoever called DOS, which
   is the program's. It answers in AL, and every other register comes
   back as it was. */
void __interrupt __far FatErr(struct Saved r)
{
    DataSeg(SecDat);
    r.ax = FtErr1(r.ax, r.di, AT(r.bp, r.si));
}
#pragma aux FatErr "*"

/* Take the critical error of DOS, for FatErr, and
   Ctrl-Break, for a routine that only returns, and note who had them.
   Returns where that routine is in the part of the program that stays in
   the memory. */
u16 HookVct(void)
{
    md.ErrVct = Dos35(0x3524);
    md.BrkVct = Dos35(0x351B);
    SetVect(0x24, (const u8 __far *)(void __far *)FatErr);
    SetVect(0x1B, AT(ResSeg(), OFFSET(CtrlC)));
    return OFFSET(CtrlC);
}

/* Leave the program: the screen as DOS had it, with the cursor on the
   line above the command line. The stub goes on to the part of the
   program that stays in the memory, which ends the program.

   Ported as it is: of the two interrupts that the program has taken,
   only Ctrl-Break is given back here. DOS itself puts the critical error
   back when a program ends. */
void Exit(void)
{
    RstVc1();
    Quit();
    ClsLine((u16)md.DosLin << 8, MAIN_SEG);
    Window(0, (u16)md.Lines << 8 | 80);
    CurMous(0);
    Cursor((u16)(u8)(md.DosLin - 1) << 8);
}
