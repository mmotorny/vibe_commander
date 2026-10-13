/*
 * The core of the program: the loop that takes a key and goes to its
 * command, with what draws the panels before it and what the mouse does
 * at the panels; the box that says a drive is not ready; and leaving.
 *
 * Translated from VC.ASM.
 *
 * The loop is no routine in the original. The code that starts the
 * program falls into it; it jumps to the command of a key, which is no
 * routine either, and the command jumps back into it at one of several
 * places, or on into another command. Here the loop is two functions and
 * a third that has them take turns, MainLoop, which Main calls, at
 * the end of this file, when it has started the program: that is the
 * program from when DOS has loaded it, and where the loop comes back
 * to when the program ends, or starts over for a command of DOS. Loop
 * is entered at a place,
 * the numbers below, and returns the command that a key or the mouse
 * asks for: Func12 has the keys. Command calls the function of that
 * command (see "The commands" in vc.h), and returns the place of the loop
 * where the program goes on, which the function says, by way of what the
 * original has at the end of each family of commands: CmdEnd, MnuEnd,
 * F1Next and F35_90.
 *
 * In the original a command finds the registers as the loop leaves them,
 * and leaves them to the loop in its turn, and so to the next command.
 * Some of that is read: the mouse hands its commands where the cursor is
 * to go; a key that goes on to the command line is taken there from AX;
 * and Rename, with Shift or without a panel of files, goes by BX and DX
 * as they come (F35_50 of copy.c). So the registers are a structure
 * here, `r`, which Loop and Command keep as the original has them
 * wherever a command reads one: where the code below sets a field of `r`
 * that nothing here reads, that is why. Func05 loads BX, with a number
 * that is not 0, so a command that goes back to the loop there or before
 * leaves nothing in BX and DX that Rename could tell; the cursor's keys
 * go back behind it, at Func07.
 */
#include "vc.h"

/* The registers that go round the loop are struct Regs of vc.h. */
static u16 CmdEnd(struct Onward next, struct Regs STACK *r);
static u16 Command(u16 command, u16 panel, struct Regs STACK *r);
u16 ErrRdy(u16 letter);
static void Exit(void);
/* Quit, RestVct and HookVct are in vc.h. */
void RstVc1(void);


/* Where the program goes on after a command, which is what Command
   returns: a place of the loop, where Loop is entered, or the loop is
   left. */
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
    FUNC12,                             /* go to the command of the key
                                           in AX */
    /* The rest are no places: the loop is left, for what each says, and
       MainLoop returns it. They come last, END_INIT11 the first of them,
       which MainLoop goes by. */
    END_INIT11,                         /* to start the program over */
    END_EXEC,                           /* to have the commands of DosBuf
                                           run first */
    END_EXIT                            /* to leave the program */
};

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   HidCur puts the cursor below the screen, Cursor at a row and a column,
   the row in the high byte, and Cursor1 on the command line. ClrKbd empties the
   keyboard buffer if the setup has it so. ResWin puts back what a box
   covered, ResUser what DOS has shown, for a window given like Window's,
   and SavScr takes the screen into the screen buffer.

   GetMous returns the buttons of the mouse that are down and where the
   mouse is, or `place` as it was if there is no mouse.
   TypMous gives the mouse pointer the look of a character, or its own for
   0. InWind is not 0 if a place is outside a window. DoublMo answers 2 if
   a click on the line `line` is the first one there, 1 if it is the
   second, in time, and 0 if the button went up somewhere else.

   TstDrv is 1 if there is a drive of the letter. What the file asks of
   the system it asks by the functions of sys.h.

   MakeIni writes into MenuBuf what the setup file holds, and SaveSet
   writes that file: both of vc.h. ReDir reads what a panel shows,
   keeping the cursor and the marks if `keep` is not 0: not 0 if it could
   not be read.

   AsmSeg returns the segment of the program's tables. */

/* Routines of other files that are C. PutFls writes a panel of files,
   PutTre the tree and PutInf, of vc.h, the Info. */
void Panel(u16 panel);                                  /* panel.c */
void PutPath(u16 panel);
void PutFls(u16 panel);
void PutTre(u16 panel);                                 /* tree.c */
u16 StrLen(const u8 __far *text);                       /* string.c */
const u8 __far *FndLet(const u8 __far *text);

/* What this reads or names of other files: the scan codes of Alt with a
   digit or a letter, which begin the speed search; of main_t.c the box
   that says a drive is not ready, which DialBox takes by its offset;
   and of setup.c the setup as it was last saved or found. Menu_T, of
   commands_t.c, is a word for each table of a menu's commands that
   names it, a pointer there: see NAMED of vc.h. */
extern const u8 __far AltSr_T[];
extern const u16 * const __far Menu_T[];
extern const u8 __far NoRd_P[];
extern u8 IniSave[];

/* The main data: one structure, which is all that the original has in
   the block of memory that it asks DOS for and has DS on. Here it is an
   object of the program, in the segment that the compiler has for the
   program's data and that DS is on, and the first thing of that
   segment, at offset 0, as the original has it in its block: the line
   in front of it names a segment of the compiler's group for it alone,
   of a class of its own, and the linker's command has that class first
   (see CMakeLists.txt). The first is the compiler's and the second the
   linker's, not the language's; the tests stop where it is not at 0
   (harness/coverage.py).

   It is first in the program's file, too, 62 KB of nothing: the file
   is the program as it lies in the memory. */
#pragma data_seg("MAINDATA", "MAIN")
struct MainDat md;
#pragma data_seg()

#define ALT_KEYS 38                     /* the length of AltSr_T */

#define KEY_MOUSE 0xFC                  /* the scan code that Input has
                                           for the mouse */

/* Where the setup file has a field of a panel and one of the main data
   (see WCBini and CmnIni in vc.h). The program keeps the setup as the
   loop last found it, IniSave of setup.c; the original keeps that where
   it has the copy of the first panel while a command runs. */
#define INI_WCB(number, field) \
    (3 + (number) * WCBini + OFFSET_IN(struct WCBdef, field))
#define INI_CMN(field) \
    (3 + 2 * WCBini + OFFSET_IN(struct MainDat, field))

/* What a panel shows, written to the screen buffer: PutFls of the
   original, which goes on in what writes the tree or the Info for a
   panel of that kind. */
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
    const u8 __far *tail;
    u8 *to;
    u16 page, row, line, bottom, count, i, c;

    page = ScreenState().page;
    row = CursorWhere(page).place >> 8;
    if (md.KeyBar != 0) {
        line = (u8)(md.Lines - 1);
        if (row >= line) {
            row = (u8)(line - 1);
            ScreenType('\n', page, 0);  /* a line feed: the screen scrolls */
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
    tail = FndLet(ProgramTail());
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
        root = (struct DIRENT *)MD_POINTER(TRELIST);
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
                DriveSelect(letter);
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
            md.BlocEr = 1;
            failed = DirCurrent(letter, wcb->WinPath + 3).failed;
            md.BlocEr = 0;
            if (!failed)
                drive = 0;
        }
        keep = 0;                       /* Init78 */
    }
}

/* What the mouse asks for at the panels, which is what Func21 returns:
   a command, or nothing, and then the place where the loop goes round
   from. */
struct Asked {
    u8  nothing;                        /* not 0 if no command is asked
                                           for: the loop goes round, */
    u8  entry;                          /* from this place of it */
    u16 command;                        /* else the command, a CMD_ of
                                           vc.h */
};

/* It asks for a command, */
static struct Asked Asks(u16 command)
{
    struct Asked asked;

    asked.nothing = 0;
    asked.entry = 0;
    asked.command = command;
    return asked;
}

/* or for nothing, and the loop goes round from `entry`. */
static struct Asked Nothing(u16 entry)
{
    struct Asked asked;

    asked.nothing = 1;
    asked.entry = (u8)entry;
    asked.command = 0;
    return asked;
}

/* What the mouse does at the panels, with a button down: `r->ax` is the
   buttons, `r->cx` where the mouse is, the row in the high byte, and
   `r->ds` the current panel. Returns what the mouse asks for, struct
   Asked above: a command, a CMD_ of vc.h, or nothing, with the place
   where the loop goes round from.

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
static struct Asked Func21(struct Regs STACK *r)
{
    struct WCBdef __far *wcb;
    struct Place place;
    struct Mouse mouse;
    u16 look, lines, rows, column, kind, answer;

    ClrKbd();
    if (md.WinMous != 0)
        goto Func30;
    if (r->cx < 2) {                    /* Panels on/off */
        look = 0;
        for (;;) {                      /* Func22 */
            mouse = GetMous(r->cx);
            r->ax = mouse.buttons;
            r->cx = mouse.place;
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
            return Nothing(FUNC07);
        return Asks(CMD_F11_00);
    }
    if ((r->cx >> 8) == 0)              /* Func25: pull down menu */
        return Asks(CMD_F39_00);
    r->bx = BegBX(r->ds);               /* Func26 */
    r->dx = SizDX();
    r->dx = r->dx & 0xFF | (u16)(u8)((r->dx >> 8) + (r->bx >> 8)) << 8;
    if ((r->cx >> 8) >= (r->dx >> 8))
        return Nothing(FUNC07);
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
    return Asks(CMD_F20_00);

Func30:
    wcb = PANEL(r->ds);
    if ((r->ax & 3) == 0 || wcb->Visible == 0)
        return Nothing(FUNC07);
    if (wcb->WinTyp < 2)
        goto Func40;
    if (wcb->WinTyp != 2)
        return Nothing(FUNC07);
    r->bx = BegBX(r->ds) + 0x0B01;      /* Edit 'dirinfo' */
    r->dx = SizDX() - 0x0C02;
    if (InWind(r->cx, r->bx, r->dx))
        return Nothing(FUNC07);
    return Asks(CMD_F34_10);

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
        return Nothing(FUNC07);
    r->bx += 0x100;
    if (wcb->WinTyp == 0)
        r->bx += 0x100;
    rows = r->cx >> 8;                  /* Func41 */
    r->cx = r->cx & 0xFF | (u16)(u8)(rows - (r->bx >> 8)) << 8;
    if (rows < r->bx >> 8) {            /* Func50: scroll up */
        r->si = r->di;
        return Asks(CMD_F00_05);
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
        return Asks(CMD_F01_05);
    }
    column = (u8)(column - (r->bx & 0xFF));     /* Set cursor position */
    r->cx = r->cx & 0xFF00 | column;
    if (column >= 39)
        return Nothing(FUNC07);
    r->bx = r->bx & 0xFF00 | kind;
    if (kind == 0) {                    /* three columns of names */
        r->bx = r->bx & 0xFF00 | 13;
        r->ax = (column / 13) * (lines & 0xFF);
        column %= 13;
        rows = (u8)(rows + r->ax);
        r->cx = rows << 8 | column;
    }
    if (column == 0)                    /* Func42 */
        return Nothing(FUNC07);
    r->ax = (u16)(signed char)rows + r->di;
    r->si = r->ax;
    if (r->si > r->dx) {
        r->si = r->dx;
        return Asks(CMD_F00_60);
    }
    if (!(r->bp & 1))                   /* Func43 */
        return Asks(CMD_F00_60);

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
        return Asks(CMD_F00_60);
    if (answer == 0)
        return Nothing(FUNC04);
    r->ax &= 0xFF00;                    /* Func62 */
    TypMous(0);
    return Asks(CMD_F21_10);
}

/* The command of a key, a CMD_ of vc.h: the command line for a key that
   has none, and for one without a scan code.

   The assembly had a table of keys and commands, FncTab, which Case went
   through from its start: an entry was a key, or a character that counts
   with any scan code. Every character of the second kind that a key of
   the first kind has too, Tab, Enter and Esc, stood behind that key, so
   the keys are looked for first here. Case left SI behind the entry it
   had found, or on the table: no command reads it, and here it stays as
   it is.

   The original's table has two keys more, Alt+F5, the map of the
   memory, and Alt+F9, EGA lines, which the portable C has not (see
   DEVIATIONS.md): here each is a key that has no command. */
static u16 Func12(u16 key)
{
    if (key >> 8 == 0)
        return CMD_F73_00;
    switch (key) {
    case 0x4800: return CMD_F00_00;  /* Up */
    case 0x5000: return CMD_F01_00;  /* Down */
    case 0x4B00: return CMD_F02_00;  /* Left */
    case 0x4D00: return CMD_F03_00;  /* Right */
    case 0x4900: return CMD_F04_00;  /* PgUp */
    case 0x5100: return CMD_F05_00;  /* PgDn */
    case 0x4700: return CMD_F06_00;  /* Home */
    case 0x4F00: return CMD_F07_00;  /* End */
    case 0x5200: return CMD_F08_00;  /* Ins: select / unselect */
    case 0x4E2B: return CMD_F09_00;  /* Grey +: select group */
    case 0x4A2D: return CMD_F10_00;  /* Grey -: unselect group */
    case 0x372A: return CMD_F76_00;  /* Grey *: invert group */
    case 0x0F09: return CMD_F20_00;  /* Tab: change active panel */
    case 0x1C0D: return CMD_F21_00;  /* Enter: run / change directory */
    case 0x011B: return CMD_F74_00;  /* Esc: clear command line / on/off */
    case 0x5E00: return CMD_F13_00;  /* ^F1: on/off left panel */
    case 0x5F00: return CMD_F14_00;  /* ^F2: on/off right panel */
    case 0x2308: return CMD_M16_10;  /* ^H: hidden files */
    case 0x8400: return CMD_F22_00;  /* ^PgUp: CD .. */
    case 0x7600: return CMD_F23_00;  /* ^PgDn: change directory */
    case 0x4400: return CMD_F30_00;  /* F10: quit */
    case 0x3C00: return CMD_F32_00;  /* F2: user menu */
    case 0x3D00: return CMD_F33_00;  /* F3: view */
    case 0x3E00: return CMD_F34_00;  /* F4: edit */
    case 0x3F00: return CMD_F35_00;  /* F5: copy */
    case 0x4000: return CMD_F36_00;  /* F6: rename / move */
    case 0x4100: return CMD_F37_00;  /* F7: make directory */
    case 0x4200: return CMD_F38_00;  /* F8: delete */
    case 0x4300: return CMD_F39_00;  /* F9: pull down menu */
    case 0x5D00: return CMD_F40_00;  /* Shift F10: pull down menu */
    case 0x5500: return CMD_F42_00;  /* Shift F2: main menu... */
    case 0x5600: return CMD_F43_00;  /* Shift F3: view... */
    case 0x5700: return CMD_F44_00;  /* Shift F4: edit... */
    case 0x5800: return CMD_F45_00;  /* Shift F5: copy... */
    case 0x5900: return CMD_F46_00;  /* Shift F6: rename/move... */
    case 0x5A00: return CMD_F47_00;  /* Shift F7: make directory... */
    case 0x5B00: return CMD_F48_00;  /* Shift F8: delete... */
    case 0x5C00: return CMD_M75_00;  /* Shift F9: save setup */
    case 0x6100: return CMD_F54_00;  /* ^F4: change label */
    case 0x6200: return CMD_F55_00;  /* ^F5: copy file */
    case 0x6300: return CMD_F56_00;  /* ^F6: rename/move file */
    case 0x6500: return CMD_F58_00;  /* ^F8: delete file */
    case 0x6800: return CMD_F61_00;  /* Alt F1: change left drive */
    case 0x6900: return CMD_F62_00;  /* Alt F2: change right drive */
    case 0x6A00: return CMD_F63_00;  /* Alt F3: internal viewer */
    case 0x6B00: return CMD_F64_00;  /* Alt F4: internal editor */
    case 0x6D00: return CMD_M42_00;  /* Alt F6: directory sizes */
    case 0x6E00: return CMD_F67_00;  /* Alt F7: file find */
    case 0x6F00: return CMD_F68_00;  /* Alt F8: history */
    case 0x7100: return CMD_M41_00;  /* Alt F10: Tree */
    case 0xFD00: return CMD_F39_00;  /* Alt: pull down menu */
    case 0xFE00: return CMD_F75_00;  /* the time in AutoTim has come:
                                           auto CD / quick view */
    }
    switch (key & 0xFF) {
    case 0x0A: return CMD_F27_00;  /* ^Enter: history command */
    case 0x0D: return CMD_F78_00;  /* ^M: restore marks */
    case 0x0F: return CMD_F11_00;  /* ^O: on/off panels */
    case 0x10: return CMD_F12_00;  /* ^P: on/off unactive panel */
    case 0x15: return CMD_F15_00;  /* ^U: swap panels */
    case 0x02: return CMD_F16_00;  /* ^B: on/off key bar */
    case 0x12: return CMD_F17_00;  /* ^R: re-read */
    case 0x06: return CMD_M16_00;  /* ^F: filter */
    case 0x01: return CMD_M30_00;  /* ^A: attributes */
    case 0x03: return CMD_M49_00;  /* ^C: compare directories */
    case 0x0C: return CMD_F18_00;  /* ^L: info */
    case 0x1A: return CMD_F79_00;  /* ^Z: tree */
    case 0x1C: return CMD_F24_00;  /* ^\: CD \ */
    case 0x05: return CMD_F25_00;  /* ^E: previous history command */
    case 0x18: return CMD_F26_00;  /* ^X: next history command */
    case 0x09: return CMD_F80_00;  /* ^I: insert files */
    case 0x1B: return CMD_F81_00;  /* ^[: insert left path */
    case 0x1D: return CMD_F82_00;  /* ^]: insert right path */
    }
    return CMD_F73_00;
}

/* The main loop as far as a command: see the top of this file. `entry`
   is where it is entered and `r` the registers as they are there; it
   returns the command, with the registers as the command gets them. */
static u16 Loop(u16 entry, struct Regs STACK *r)
{
    struct WCBdef __far *wcb;
    u8 __far *saved;
    const u8 __far *keys;
    const u8 *image;
    struct Time moment;
    struct Mouse mouse;
    struct Asked asked;
    u16 i, length, same, time, seconds, hundredths, menu, key;

    for (;;) {
        switch (entry) {
        case INIT40:
            if (Init40())
                return CMD_F21_31;
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
            saved = IniSave;
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
            /* What the registers hold behind the comparison, places as
               numbers, which a command may go by: the second is of a
               variable that stands behind the main data, IniSave, and
               its number is how far behind, which is what the word
               makes of a pointer that is no place of the main data. */
            r->si = MD_PLACE(md.MenuBuf) + INI_LEN;
            r->di = MD_PLACE(IniSave) + INI_LEN;
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
                r->si = MD_PLACE(md.CrnPath) + i;
                r->di = MD_PLACE(md.DosPth) + i;
                if (!same && md.FlagCD != 0) {
                    moment = TimeOfDay();
                    time = moment.clock;
                    hundredths = (u8)(moment.seconds + md.DelayTm[0]);
                    seconds = moment.seconds >> 8;
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
                mouse = GetMous(r->cx);
                r->ax = mouse.buttons;
                r->cx = mouse.place;
                if (r->ax != 0)
                    goto Mouse;
                md.WinMous = 0;
                md.MousEnt = 0;
                key = Input(0);         /* Func11: the original has
                                           another bar here for a
                                           display without EGA
                                           lines */
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
                        return CMD_F77_00;
                }
            }
            /* fall through */
        case FUNC12:
            if (r->ax >> 8 == KEY_MOUSE)
                goto Func20;
            if (md.Quote != 0)
                return CMD_F73_00;
            return Func12(r->ax);
        }

    Func20:                             /* Mouse support */
        mouse = GetMous(r->cx);
        r->ax = mouse.buttons;
        r->cx = mouse.place;
    Mouse:
        asked = Func21(r);
        if (!asked.nothing)
            return asked.command;
        entry = asked.entry;
    }
}

/*---------------- Where a command goes on ----------------
   What the function of a command returns says where the program goes
   on. Each of these makes of it what the original has at the end of that
   family of commands, and returns the place of the loop, an entry of
   Loop, or what the loop is left for, an END_. Where the original goes
   on in another command, so does this. */

/* Open the file ViewFil and show it in the viewer, or in the editor if
   `edit` is not 0: V00_00, V00_01 and V00_02 of the original, which are
   one piece of code entered with this or that in its registers. `menu`
   is the item of the menu bar to mark meanwhile, and `whole` is 0 for the
   window that an Info panel has for its dirinfo. The editor may have the
   file viewed instead, where it is too large to edit. */
static u16 V00_03(u16 menu, u16 edit, u16 whole)
{
    struct Open open;
    struct EditEnd end;
    u16 what;

    what = V00_02(menu, edit, whole, (struct Open STACK *)&open);
    if (what == 0)
        return FUNC04;
    if (what == 1) {
        end = T00_00(open.handle, open.box.begin, open.box.size,
                     open.box.saved);
        if (end.how == EDIT_CLOSE) {    /* T00_10: not read */
            V13_00();
            return INIT50;
        }
        if (end.how == EDIT_DONE) {     /* T35_02: edited */
            V13_01(end.written);
            return INIT50;
        }
    }
    V00_10(open.handle);
    return INIT50;
}

/* After a command of the menus, of commands.c: MnuEnd. */
static u16 MnuEnd(u16 answer, struct Regs STACK *r)
{
    switch (answer) {
    case MNU_FUNC04:
        return FUNC04;
    case MNU_INIT50:
        return INIT50;
    case MNU_F24_01:                    /* MnuF24 */
        return CmdEnd(F24_01(r->ds, MD_PLACE(md.TmpBuf)), r);
    default:                            /* MnuV00, MNU_V00_01: the menu of
                                           the command is in CL */
        return V00_03(r->cx & 0xFF, 1, 1);
    }
}

/* After a command of the keys of the panels, of keys.c, enter.c and
   view.c: CmdEnd. `next` is what the command returned, with the panel
   and the item for a routine of a panel's menu; `r->ax` is the key
   still, for the command line.

   Func07 is the one place of the loop where the next command gets BX and
   DX as this one leaves them. Only the keys of the cursor go there, and
   they have put into `r` what the original leaves in the two. */
static u16 CmdEnd(struct Onward next, struct Regs STACK *r)
{
    u16 item = next.item;
    u16 panel = next.panel;

    switch (next.place) {
    case NEXT_FUNC07:
        return FUNC07;
    case NEXT_FUNC05:
    case NEXT_POP_FUNC05:
        return FUNC05;
    case NEXT_FUNC04:
    case NEXT_POP_FUNC04:
        return FUNC04;
    case NEXT_FUNC01:                   /* CmdF01: Tab has made another
                                           panel the current one */
        r->ds = md.WCBcrn;
        return FUNC01;
    case NEXT_INPUT:
        F73_00(r->ax);
        return FUNC05;
    case NEXT_EARLIER:
        return CmdEnd(F25_00(), r);
    case NEXT_LATER:
        return CmdEnd(F26_00(), r);
    case NEXT_INIT40:
        return INIT40;
    case NEXT_INIT50:
        return INIT50;
    case NEXT_M01_01:                   /* CmdM01: in the order and the
                                           form that the panel has */
        return MnuEnd(M01_01(panel, (u16)PANEL(panel)->BrfFul << 8
                                    | PANEL(panel)->SortTyp), r);
    case NEXT_M03_00:                   /* CmdM03 */
        return MnuEnd(M03_00(panel, item << 8), r);
    case NEXT_M06_00:                   /* CmdM06 */
        return MnuEnd(M06_00(panel, item << 8 | item), r);
    case NEXT_INIT11:                   /* CmdIni */
        return END_INIT11;
    case NEXT_EXEC:                     /* CmdExe */
        return END_EXEC;
    case NEXT_VIEW:                     /* V00_00: with the Files menu
                                           marked, and in DL whether to
                                           edit */
        return V00_03(2, 0, 1);
    case NEXT_EDIT:
        return V00_03(2, 1, 1);
    case NEXT_DIRINFO:                  /* CmdV02 */
        return V00_03(2, 1, 0);
    default:                            /* NEXT_EXIT */
        return END_EXIT;
    }
}

/* After a command of dirfunc.c, and after the file search: F1Next. The
   files that are selected, or the panels read again if something has
   been written, or whatever has. */
static u16 F1Next(u16 answer)
{
    u16 read;

    if (answer == TO_FUNC04)
        return FUNC04;
    if (answer == TO_FNCSEL)
        read = FncSel();
    else if (answer == TO_FNCEXT)
        read = FncExt();
    else
        read = FncEx1();
    return read ? INIT50 : FUNC04;      /* FncEx0 */
}

/* After Copy and Rename, of copy.c: F35_90. */
static u16 F35_90(struct Copied copied)
{
    switch (copied.place) {
    case GO_KEYS:
        return FUNC04;
    case GO_PANELS:
        return INIT50;
    case GO_SELECTED:
        return FncSel() ? INIT50 : FUNC04;
    case GO_MASKED:
        FncMas(copied.title);
        /* fall through */
    default:                            /* GO_DONE */
        return FncExt() ? INIT50 : FUNC04;
    }
}

/* The box of the drives for a panel, and the panel switched on if a
   drive was chosen: F62_01 of the original, where Alt-F1 and Alt-F2 go
   on. `item` is the menu to mark meanwhile. */
static u16 F62_02(u16 panel, u16 item, struct Regs STACK *r)
{
    if (F62_01(panel, item) == 0)
        return FUNC04;                  /* F62_46 */
    return MnuEnd(M06_10(panel, item), r);
}

/* The menu bar, F9, and with `again` Shift-F10, which goes back to the
   item chosen last: an item is chosen there or the bar is left, and the
   command of the item is done. The original jumps to it by the tables of
   the menus, of commands_t.c, which have the commands by their
   numbers here: one for each menu, and two more for the first eight
   items of the Files menu with Shift and with Ctrl held down.

   The command gets the registers as the original has them at that jump:
   in AX the item and the table, in BX twice the place of the item in its
   table, in CX the item and the menu, in DX 1 and what the bar left in
   DL, in SI the table, in DI the panel of the Left menu, or of the Right
   one if that is the menu. Rename goes by BX and DX, the editors of the
   menu file and the extension files by CX, and a key that an item hands
   on to the command line is what AX holds. A command of the Left or the
   Right menu is for the panel of that menu. */
static u16 F39_01(u16 again, struct Regs STACK *r)
{
    struct Picked picked = F39_00(again, r->ds);
    u16 menu = picked.menu;
    u16 item = picked.item;
    u16 table = picked.table;
    u16 code = AsmSeg();
    u16 panel = r->ds;

    r->cx = item << 8 | menu;
    if (table == 0)
        return FUNC04;
    r->ax = item << 8 | table;
    r->bx = (item - 1) & 0xFF00 | (item - 1) << 1 & 0xFF;
    r->dx = 0x100 | picked.held;
    r->di = menu == 5 ? md.WCB2seg : md.WCB1seg;
    if (table == 1)
        panel = r->di;
    r->si = NAMED(*(const u16 * const __far *)
                  AT(code, OFFSET(Menu_T) + (table - 1) * sizeof *Menu_T));
    return Command(*(const u16 __far *)AT(code, r->si + r->bx), panel, r);
}

/* Do a command: call its function, with what it reads of the registers,
   and go on by what that returns. Returns the place of the loop where the
   program goes on, with the registers as that place and the next command
   get them, or what the loop is left for, an END_.

   `panel` is the panel that the command is for, which the original has
   in DS when it jumps to the command: the current panel, `r->ds`, but for
   a command of the Left or the Right menu. */
static u16 Command(u16 command, u16 panel, struct Regs STACK *r)
{
    struct SpeedKey ended;
    struct Searched searched;
    u16 answer;

    switch (command) {
    case CMD_F39_00: return F39_01(0, r);
    case CMD_F40_00: return F39_01(1, r);

    /* The command line, cmdline.c. */
    case CMD_F73_00:
        F73_00(r->ax);
        return FUNC05;
    case CMD_F74_00:                     /* Esc: the panels off or on, if
                                           the command line was empty */
        answer = F74_00();
        if (answer == 0)
            return FUNC05;
        return CmdEnd(F11_00(panel), r);
    case CMD_F77_00:
        /* The speed search ends with a key that is not its own, and the
           command of that key gets the registers as the search leaves
           them. */
        ended = F77_00(r->ax, panel);
        r->ax = ended.key;
        r->bx = ended.box;
        r->dx = 0x318;
        r->si = MD_PLACE(md.WinBuf);
        r->cx = md.KeyBar;
        return FUNC12;
    case CMD_F80_00: return F80_00(panel) ? FUNC01 : FUNC05;
    case CMD_F81_00:
        F82_01(md.WCB1seg, r->ax);
        return FUNC04;
    case CMD_F82_00:
        F82_01(md.WCB2seg, r->ax);
        return FUNC04;
    case CMD_F68_00:                     /* the history: a command to run? */
        if (F68_00() == 0)
            return FUNC04;
        return CmdEnd(F21_25(), r);

    /* The keys of the panels, keys.c. The mouse has its buttons in BP. */
    case CMD_F00_60:
        return CmdEnd(F00_60(panel, r->si, r->di, r->bp, r), r);
    case CMD_F00_05:
        return CmdEnd(F00_05(panel, r->si, r->di, r->bp, r), r);
    case CMD_F01_05:
        return CmdEnd(F01_05(panel, r->si, r->di, r->dx, r->bp, r), r);
    case CMD_F00_00: return CmdEnd(F00_00(panel, r), r);
    case CMD_F01_00: return CmdEnd(F01_00(panel, r), r);
    case CMD_F02_00: return CmdEnd(F02_00(panel, r), r);
    case CMD_F03_00: return CmdEnd(F03_00(panel, r), r);
    case CMD_F04_00: return CmdEnd(F04_00(panel, r), r);
    case CMD_F05_00: return CmdEnd(F05_00(panel, r), r);
    case CMD_F06_00: return CmdEnd(F06_00(panel, r), r);
    case CMD_F07_00: return CmdEnd(F07_00(panel, r), r);
    case CMD_F08_00: return CmdEnd(F08_00(panel, r), r);
    case CMD_F09_00: return CmdEnd(F09_00(panel), r);
    case CMD_F10_00: return CmdEnd(F10_00(panel), r);
    case CMD_F76_00: return CmdEnd(F76_00(panel), r);
    case CMD_F76_01: return CmdEnd(F76_01(panel), r);
    case CMD_F78_00: return CmdEnd(F78_00(panel), r);
    case CMD_F11_00: return CmdEnd(F11_00(panel), r);
    case CMD_F12_00: return CmdEnd(F12_00(panel), r);
    case CMD_F13_00: return CmdEnd(F13_00(), r);
    case CMD_F14_00: return CmdEnd(F14_00(), r);
    case CMD_F15_00: return CmdEnd(F15_00(panel), r);
    case CMD_F16_00: return CmdEnd(F16_00(), r);
    case CMD_F17_00:
    case CMD_F17_01: return CmdEnd(F17_01(panel), r);
    case CMD_F18_00: return CmdEnd(F18_00(panel), r);
    case CMD_F79_00: return CmdEnd(F79_00(panel), r);
    case CMD_F20_00: return CmdEnd(F20_00(panel), r);

    /* Enter, the directory changed and the history, enter.c. */
    case CMD_F21_00: return CmdEnd(F21_00(panel), r);
    case CMD_F21_10: return CmdEnd(F21_10(panel), r);
    case CMD_F21_31: return CmdEnd(F21_31(), r);
    case CMD_F22_00: return CmdEnd(F22_00(panel), r);
    case CMD_F23_00: return CmdEnd(F23_00(panel), r);
    case CMD_F24_00: return CmdEnd(F24_00(panel), r);
    case CMD_F25_00: return CmdEnd(F25_00(), r);
    case CMD_F26_00: return CmdEnd(F26_00(), r);
    case CMD_F27_00: return CmdEnd(F27_00(panel), r);
    case CMD_F30_00: return CmdEnd(F30_00(panel), r);
    case CMD_F75_00: return CmdEnd(F75_00(panel), r);

    /* The user menu, usermenu.c: a line with a command to run? */
    case CMD_F32_00:
    case CMD_F42_00:
        if (F32_00(command == CMD_F42_00) == 0)
            return FUNC04;              /* Run_0A */
        return CmdEnd(F21_30(), r);

    /* The viewer and the editor, view.c. */
    case CMD_F33_00: return CmdEnd(F33_00(panel), r);
    case CMD_F34_00: return CmdEnd(F34_00(panel), r);
    case CMD_F34_10: return CmdEnd(F34_10(panel), r);
    case CMD_F43_00: return CmdEnd(F43_00(), r);
    case CMD_F44_00: return CmdEnd(F44_00(), r);
    case CMD_F63_00: return CmdEnd(F63_00(panel), r);
    case CMD_F64_00: return CmdEnd(F64_00(panel), r);

    /* Copy and Rename, copy.c: with BX and DX as they come. */
    case CMD_F35_00: return F35_90(F35_01(panel, 0, r->bx, r->dx));
    case CMD_F36_00: return F35_90(F35_01(panel, 1, r->bx, r->dx));
    case CMD_F45_00: return F35_90(F45_01(panel, 0, r->bx, r->dx));
    case CMD_F46_00: return F35_90(F45_01(panel, 1, r->bx, r->dx));
    case CMD_F55_00:
    case CMD_F56_00:                     /* F56_01: of the one file */
        md.OneFile = 1;
        return F35_90(F35_01(panel, command == CMD_F56_00, r->bx, r->dx));

    /* Directories, deleting, attributes and the label, dirfunc.c. */
    case CMD_F37_00: return F1Next(F37_00(panel));
    case CMD_F38_00: return F1Next(F38_00(panel));
    case CMD_F47_00: return F1Next(F47_00());
    case CMD_F48_00: return F1Next(F48_00());
    case CMD_F54_00: return F1Next(F54_00());
    case CMD_F58_00: return F1Next(F58_00(panel));
    case CMD_M30_00: return F1Next(M30_00(panel));

    /* The box of the drives, drive.c: by the keys, and from the menu of
       a panel. */
    case CMD_F61_00: return F62_02(md.WCB1seg, 1, r);
    case CMD_F62_00: return F62_02(md.WCB2seg, 5, r);
    case CMD_F62_01: return F62_02(panel, r->cx & 0xFF, r);

    /* The file search, find.c: Go to has the current directory be
       another, and the panel that it hands back show it. */
    case CMD_F67_00:
        searched = F67_00();
        if (searched.place == TO_F24_02) {      /* F67_01 */
            F24_02(searched.panel);
            return INIT50;
        }
        return F1Next(searched.place);

    /* The commands of the menus, commands.c. Those of a panel's menu
       take the item, and so do the editors of the menu file and the
       extension files, to mark their menu. */
    case CMD_M01_00: return MnuEnd(M01_00(panel, r->cx), r);
    case CMD_M03_00: return MnuEnd(M03_00(panel, r->cx), r);
    case CMD_M06_00: return MnuEnd(M06_00(panel, r->cx), r);
    case CMD_M08_00: return MnuEnd(M08_00(panel, r->cx), r);
    case CMD_M16_00: return MnuEnd(M16_00(panel), r);
    case CMD_M16_01: return MnuEnd(M16_01(panel), r);
    case CMD_M16_10: return MnuEnd(M16_10(panel), r);
    case CMD_M21_00: return MnuEnd(M21_00(), r);
    case CMD_M41_00: return MnuEnd(M41_00(), r);
    case CMD_M42_00: return MnuEnd(M42_00(panel), r);
    case CMD_M49_00: return MnuEnd(M49_00(panel), r);
    case CMD_M53_00: return MnuEnd(M53_00(r->cx), r);
    case CMD_M60_00: return MnuEnd(M60_00(), r);
    case CMD_M61_00: return MnuEnd(M61_00(), r);
    case CMD_M62_00: return MnuEnd(M62_00(r->cx), r);
    case CMD_M63_00: return MnuEnd(M63_00(r->cx), r);
    case CMD_M64_00: return MnuEnd(M64_00(r->cx), r);
    case CMD_M66_00: return MnuEnd(M66_00(panel), r);
    case CMD_M67_00: return MnuEnd(M67_00(panel), r);
    case CMD_M69_00: return MnuEnd(M69_00(panel), r);
    case CMD_M70_00: return MnuEnd(M70_00(panel), r);
    case CMD_M71_00: return MnuEnd(M71_00(panel), r);
    case CMD_M72_00: return MnuEnd(M72_00(panel), r);
    case CMD_M75_00: return MnuEnd(M75_00(panel), r);

    /* The setup, stmain.c: when it has been left, the program goes on
       as when a command of DOS has ended. */
    default:                            /* CMD_SETUP */
        StSetup();
        return END_INIT11;
    }
}

/* The main loop: see the top of this file. It is entered where the
   original falls into it when the program has started, Init40, and
   returns when the loop is left, with why: an END_ above. */
static u16 MainLoop(void)
{
    struct Regs regs;
    struct Regs STACK *r = (struct Regs STACK *)&regs;
    u16 entry = INIT40;
    u16 command;

    /* What the start of the program leaves in the registers: nothing of
       it is read. Loop sets each before it is. */
    r->ax = r->bx = r->cx = r->dx = r->si = r->di = r->bp = r->ds = 0;
    for (;;) {
        command = Loop(entry, r);
        entry = Command(command, r->ds, r);
        if (entry >= END_INIT11)        /* no place of the loop */
            return entry;
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
    struct Box where;
    struct Mouse mouse;
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
            mouse = GetMous(place);
            buttons = mouse.buttons;
            if (buttons == 0)
                break;
            place = mouse.place;
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
    DriveSetLetter(drive - ('A' - 1));
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

/* Give Ctrl-Break back to whoever had it. */
void RstVc1(void)
{
    CtrlBreakGive();
}

/* Give the critical error of DOS and Ctrl-Break back to whoever had
   them. */
void RestVct(void)
{
    DeviceErrorGive();
    RstVc1();
}

/*--------------------- Fatal error ---------------------
   What main_t.c has for it: the texts and the two boxes, with
   Retry and Abort or with Ok alone, which go to Dialog by their offsets. */
extern const u8 __far Err_S[];
extern const u8 __far OnDr_S[];
extern const u8 __far OnDv_S[];
extern const u8 __far Errs_S[];
extern const u8 __far FatEr_S[];
extern const u8 __far FtEr1_B[];
extern const u8 __far FtEr2_B[];

/* A critical error of DOS, which the system asks the program about once
   HookVct has handed it this function: `how`, `code` and `device` are
   what sys.h says that such a function takes, the drive and DOS's bits,
   DOS's number for what is wrong, and the name of a device that is no
   drive. Say which drive or which device and what is wrong, in a box,
   and ask whether to try again, where DOS allows that; else the box has
   Ok alone. Returns what DOS is to do: try again, or have its call
   fail.

   Once a call has been let fail, ErrDat is set, and every error after
   it fails without a box until a function of sys.h that is asked with
   the program's errors clears that, as Intr21 of the original does.
   BlocEr does the same for those who want no box at all.

   The clock and the screen blanker are off while the box is up, and
   nothing is to happen by itself then: AutoTim.

   The box is the main program's also while the setup is open, where
   the original's setup program has no box and leaves the error to DOS's
   own words (see DEVIATIONS.md): its dialog and its key bar are not the
   setup's, and the setup draws its own key bar anew afterwards. */
static u16 FtErr1(u16 how, u16 code, const u8 FAR *device)
{
    u16 assembly = AsmSeg();
    const u8 __far *from;
    const u8 __far *text;
    u8 *to = md.TmpErr + 1;
    u8 *name;
    u16 i, answer;
    u8 clock, delay, page, setup;

    if (md.ErrDat | md.BlocEr)
        return DEVICE_FAIL;
    from = AT(assembly, OFFSET(Err_S) + 1);
    for (i = 6; i; i--)
        *to++ = *from++;
    text = AT(assembly, OFFSET(Errs_S));
    if (device == 0) {                  /* a disk */
        from = AT(assembly, OFFSET(OnDr_S));
        for (i = 11; i; i--)
            *to++ = *from++;
        to[-2] += (u8)how;
        if (how & 0x8000) {
            text = AT(assembly, OFFSET(FatEr_S));
            goto FtErr5;
        }
    } else {                            /* FtErr2: a device, by its name */
        from = AT(assembly, OFFSET(OnDv_S));
        for (i = 10; i; i--)
            *to++ = *from++;
        name = to;
        from = device;
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
    setup = StOpen;
    StOpen = 0;
    *(u16 *)&md.AutoTim = 0xFFFF;
    if (how & 0x1000)
        answer = (u8)Dialog(OFFSET(FtEr1_B), OFFSET_IN_SEG(assembly, text),
                            0, 0, MAIN_SEG)
                 == 0 ? DEVICE_RETRY : DEVICE_FAIL;
    else {
        Dialog(OFFSET(FtEr2_B), OFFSET_IN_SEG(assembly, text), 0, 0,
               MAIN_SEG);
        answer = DEVICE_FAIL;
    }
    md.Clock = clock;                   /* FtErr8 */
    md.BlDelay = delay;
    md.HlpPage = page;
    if (setup != 0) {
        StOpen = setup;
        md.BarPrev = 0xFF;
    }
    if (answer != DEVICE_RETRY)
        md.ErrDat = 1;
    return answer;
}

/* Note who had the critical error of DOS and Ctrl-Break, and take them:
   the first for FtErr1, and the second so that nothing happens. Both
   are noted before either is taken, as the original asks DOS. Where
   they arrive, FatErr and CtrlC of the original, is the system's:
   dos.c. */
void HookVct(void)
{
    DeviceErrorNote();
    CtrlBreakNote();
    DeviceErrorTake(FtErr1);
    CtrlBreakTake();
}

/* Leave the program: the screen as DOS had it, with the cursor on the
   line above the command line. Exit00 of resident.c ends the program.

   Ported as it is: of the two interrupts that the program has taken,
   only Ctrl-Break is given back here. DOS itself puts the critical error
   back when a program ends. */
static void Exit(void)
{
    RstVc1();
    Quit();
    ClsLine((u16)md.DosLin << 8, MAIN_SEG);
    Window(0, (u16)md.Lines << 8 | 80);
    CurMous(0);
    Cursor((u16)(u8)(md.DosLin - 1) << 8);
}

/* The loop, time after time: the program finds out about the machine,
   IniData, as it does after every command, and the main loop runs,
   until the program is to end or has commands of DOS to run, which
   Enter has left in DosBuf; with nothing to run after all it starts
   over at once. For the commands DOS gets the memory behind the panels
   back, and the program takes it again when they are done: if there is
   too little then, it says so and leaves, as it does when it starts.

   In the original the part of it that stays in the memory while a
   command runs has this loop, Stay there: it jumps to the rest of the
   program, at Init11, and is jumped back to with something to do. */
static void Round(void)
{
    struct Answer memory;
    u16 end;

    for (;;) {
        IniData();                      /* IniOk */
        end = MainLoop();
        if (end == END_EXIT) {
            Exit();
            Exit00(0);
        }
        if (end == END_EXEC) {          /* CmdExe */
            GiveMem();
            Exec();
            memory = TakeMem();
            if (memory.failed)
                AlocEr(memory.value);
        }
    }                                   /* CmdIni */
}
#pragma aux Round aborts

/* The program, from when DOS has loaded it: the system calls this
   where it enters the program, Alloc0 of dos.c, and it does not return.
   The version of DOS is looked at and the memory that the program does
   not fill is let go, Alloc3; the setup is read, Init00, and the memory
   of the panels asked for, Init11; with too little of it the program
   says so and leaves. */
void Main(void)
{
    struct Answer memory;

    Alloc3();
    Init00();
    memory = Init11();
    if (memory.failed)                  /* AlocEr */
        AlocEr(memory.value);
    Round();
}
