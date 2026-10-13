/*
 * The keys of the panels: the cursor moved by lines, columns and pages,
 * files selected one by one and by a mask, the panels switched off and
 * on, swapped, made of another kind and read again, the key bar, and Tab.
 *
 * Translated from VCFUNC.INC.
 *
 * These are commands, not routines: see "The commands" in vc.h, which
 * declares them. The main loop, MainLoop of main.c, calls one for its key
 * with the current panel, and what it returns says where the program
 * goes on: see GO below, and CmdEnd in main.c.
 *
 * The pull-down menus have some of these too, and the mouse, which the
 * loop reads, enters three of them past their start: F00_05, F01_05 and
 * F00_60.
 */
#include "vc.h"

void F20_02(u16 panel);

/* F20_02 is a routine, which other files call too, as F00_50 of vc.h. */

/* What a command returns: where the program goes on, a NEXT_ of vc.h.
   The menu's routines take a panel, in the high word, and a byte, in the
   high byte of the low one: for M03_00 five less the kind of panel, for
   M06_00 the place in the menu bar that is lit meanwhile. M01_01 takes
   neither byte. */
#define GO(where)                    ((u32)(where))

#define GO_PANEL(where, panel, byte) \
    ((u32)(panel) << 16 | (u16)((u16)(u8)(byte) << 8 | (where)))

/* Routines of other files that are C. PutFls writes a panel of files,
   PutTre the tree and PutInf, of vc.h, the Info. */
u16 NumAdr(u16 panel, u16 number);                      /* place.c */
void PutPath(u16 panel);                                /* panel.c */
void PutFls(u16 panel);
void SetCur(u16 panel, u16 set);
void PutTre(u16 panel);                                 /* tree.c */
u16 StrLen(const u8 __far *text);                       /* string.c */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   HidCur puts the cursor below the screen and Cursor1 on the command line;
   Int10m calls the BIOS with the mouse cursor off, and hands back what
   that leaves in DX and in CX, the corners of the window; ResUser writes what DOS
   has shown, which the panels cover, to the screen buffer, for a window
   given like Window's; InvVar makes of a byte of the main data 1 if it is
   0, and 0 otherwise. SelDrv is the call of DOS that makes a drive the
   current one, 0 for A:, which the assembly makes without Intr21. SetPth,
   which asks DOS for the current directory, is in vc.h, and so are SavMous
   and CurMous, of the mouse, and the bars and the prompt, which are C:
   PutMnu, ClsLine, PutPrm. */

/* The dialog that asks for a mask, the box that says no file is like it,
   and what ^I stands for in both: constants of keys_t.c, which
   Dialog takes by their offsets. */
extern const u8 __far Sel_B[];
extern const u8 __far ErSel_B[];
extern const u8 __far Sel_S[];          /* "Select" */
extern const u8 __far Unsl_S[];         /* "Unselect" */
extern const u8 __far Inv_S[];          /* "Invert" */

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

/* Bits of a file's attributes that are the program's own: selected, and
   selected before a command went through the selected files. */
#define ATTR_WAS_SELECTED 0x80

/* Bits of WinMous, which says what the mouse is doing in a panel: it
   holds a button there, it has passed over a file with the right button
   down, and it selects the files it passes over, where otherwise it
   unselects them. */
#define MOUSE_HELD    1
#define MOUSE_PASSED  2
#define MOUSE_SELECTS 4

/* The byte of the BIOS that says which keys are held down: its highest
   bit is the Ins key. */
#define KEYS_DOWN ((u8 __far *)0x00000418UL)

#define DOS_CHDIR 0x3B00                /* DOS: make a directory current */
#define ERR_PATH  3                     /* its error: there is no such */

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

/* What a routine leaves in BX and DX, for the keys of the cursor: they go
   back to the loop where it does not load the two again, and Rename may
   read them (see main.c). */
static struct Left Leaves(u16 bx, u16 dx)
{
    struct Left left;

    left.bx = bx;
    left.dx = dx;
    return left;
}

/* Move the cursor of a panel of files or of the tree to the entry at the
   address `cursor`, with the entry at `first` the first one shown, if
   that leaves the cursor among those shown, and put on the screen what
   has changed: the two entries, the status line below them, and the
   entries shown if they are others. Where they have moved by one, the
   BIOS scrolls them and only the new one is written.

   In the tree a cursor that has moved has the other panel follow it,
   after a while: FlagCD.

   Returns what the original leaves in BX and DX: the first entry shown
   and the cursor's, as addresses, if nothing more was written than
   those two; where the panel is and its size if all of it was; else the
   place and the size of the line written last, which is one that the
   BIOS has scrolled in, or the status line. DX is never 0. */
struct Left F00_50(u16 panel, u16 cursor, u16 first)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct Shown was = ChgAdr(panel, cursor, first);
    struct Shown now;
    struct Left left;
    struct Corners window;
    u16 step, moved, how, begin, corner, bottom;

    CorPar(panel);
    if (wcb->WinTyp == 0) {
        step = sizeof(struct FILENT);
        first = wcb->First;
        cursor = wcb->CurAdr;
    } else {
        step = sizeof(struct DIRENT);
        first = md.TreFrst;
        cursor = md.TreAdr;
        if (was.cursor != cursor)
            md.FlagCD = 1;
    }
    moved = was.first - first;
    how = 0;
    if (moved != 0) {
        if (moved == step)
            how = 7;                    /* the BIOS scrolls down */
        else if (moved == (u16)-step)
            how = 6;                    /* and up */
        else {
            PtFs00(panel);
            left = Leaves(BegBX(panel), SizDX());
            Window(left.bx, left.dx);
            return left;
        }
    } else if (was.cursor == cursor)
        return Leaves(first, cursor);
    left = Leaves(first, cursor);

    /* The cursor goes from where it was, on the screen as it is. */
    now = ChgAdr(panel, was.cursor, was.first);
    SetCur(panel, 0);
    ChgAdr(panel, now.cursor, now.first);
    PtFs00(panel);
    if (moved != 0) {
        /* The entries, without the frame and the titles: the tree has no
           titles, and no line between it and its status line. */
        begin = BegBX(panel);
        corner = begin + 0x201;
        bottom = SizDX() - 0x400;
        if (wcb->WinTyp != 0) {
            corner -= 0x100;
            bottom -= 0x100;
        } else if (md.MiniSta != 0)
            bottom -= 0x200;
        bottom = (bottom & 0xFF00 | 38 - 1) + corner;
        window = Int10m(how << 8 | 1, Color(Back_C) << 8 | begin & 0xFF,
                        corner, bottom);
        /* The line that has come in, the first or the last of the
           window, with the corners as the BIOS has left them. */
        corner = window.upper;
        if (how != 7)
            corner = window.lower & 0xFF00 | corner & 0xFF;
        Window(corner, 0x126);
        left = Leaves(corner, 0x126);
    }
    SetCur(panel, 1);
    if ((wcb->WinTyp | md.MiniSta) == 0)
        return left;
    /* The status line. */
    begin = BegBX(panel) + (SizDX() & 0xFF00) - 0x200 + 1;
    Window(begin, 0x126);
    return Leaves(begin, 0x126);
}

/* Show a file selected or not, and the status line, which counts what is
   selected. Where the status line is written, its place and its size are
   what the original has in BX and DX afterwards, and go into `r`. */
static void F08_10(u16 panel, struct Regs STACK *r)
{
    SetCur(panel, 1);
    if (md.MiniSta == 0)
        return;
    PtFs00(panel);
    r->bx = BegBX(panel) + (SizDX() & 0xFF00) - 0x200 + 1;
    r->dx = 0x126;
    Window(r->bx, r->dx);
}

/* The cursor to the entry with the number `cursor`, and `first` the first
   one shown: with BX and DX as F00_50 leaves them. */
static void F00_70(u16 panel, u16 cursor, u16 first, struct Regs STACK *r)
{
    struct Left left = F00_50(panel, NumAdr(panel, cursor),
                              NumAdr(panel, first));

    r->bx = left.bx;
    r->dx = left.dx;
}

/* The same for Up and Down, and for the mouse, whose buttons are in
   `buttons`: with the right one held it selects the files it passes
   over, or unselects them if the first of them was selected. */
u32 F00_60(u16 panel, u16 cursor, u16 first, u16 buttons, struct Regs STACK *r)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct FILENT __far *file;
    u16 address;
    u8 attributes;

    if (buttons & 2) {
        if (wcb->WinTyp == 0
            && (address = NumAdr(panel, cursor)) < wcb->EndBuf
            && (file = ENTRY(wcb, address))->NameF[0] != '.') {
            if (!(md.WinMous & MOUSE_PASSED)) {
                md.WinMous |= MOUSE_PASSED;
                if (!(file->AttrF & ATTR_SELECTED))
                    md.WinMous |= MOUSE_SELECTS;
            }
            attributes = file->AttrF & ~ATTR_SELECTED;
            if (md.WinMous & MOUSE_SELECTS)
                attributes |= ATTR_SELECTED;
            if (attributes != file->AttrF) {
                file->AttrF = attributes;
                F08_10(panel, r);
            }
        }
    } else
        md.WinMous &= MOUSE_HELD;
    F00_70(panel, cursor, first, r);
    return GO(NEXT_FUNC07);
}

/* The place of the cursor for the keys that go by pages: F04_30. Returns
   one less than a panel shows, which in brief is three columns. */
static u16 F04_30(u16 panel, struct Place STACK *place)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u16 page;

    AdrNum(panel, place);
    page = place->lines;
    if ((wcb->WinTyp | wcb->BrfFul) == 0)
        page += place->lines + place->lines;
    return page - 1;
}

/* A number `more` further on, and no further than `last`: F03_30. */
static u16 F03_30(u16 number, u16 more, u16 last)
{
    number += more;
    if (number > last)
        number = last;
    return number;
}

/* Up, and the mouse held above the entries: F00_05, which has the place
   of the cursor already. With the panels off Up is the command before. */
u32 F00_05(u16 panel, u16 cursor, u16 first, u16 buttons, struct Regs STACK *r)
{
    if (cursor != 0)
        cursor--;
    return F00_60(panel, cursor, first, buttons, r);
}

u32 F00_00(u16 panel, struct Regs STACK *r)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct Place place;

    if (wcb->Visible == 0)
        return GO(NEXT_EARLIER);
    if (wcb->WinTyp > 1)
        return GO(NEXT_FUNC07);
    AdrNum(panel, (struct Place STACK *)&place);
    return F00_05(panel, place.cursor, place.first, 0, r);
}

/* Down, and the mouse held below the entries: F01_05. With the panels
   off Down is the command after. */
u32 F01_05(u16 panel, u16 cursor, u16 first, u16 last, u16 buttons,
           struct Regs STACK *r)
{
    if (cursor < last)
        cursor++;
    return F00_60(panel, cursor, first, buttons, r);
}

/* F01_01, where Ins goes on to. */
static u32 F01_01(u16 panel, struct Regs STACK *r)
{
    struct Place place;

    if (PANEL(panel)->WinTyp > 1)
        return GO(NEXT_FUNC07);
    AdrNum(panel, (struct Place STACK *)&place);
    return F01_05(panel, place.cursor, place.first, place.last, 0, r);
}

u32 F01_00(u16 panel, struct Regs STACK *r)
{
    if (PANEL(panel)->Visible == 0)
        return GO(NEXT_LATER);
    return F01_01(panel, r);
}

/* Left and Right, in a panel of files in brief: a column back and on. In
   any other panel, and with the panels off, they are keys of the command
   line. */
u32 F02_00(u16 panel, struct Regs STACK *r)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct Place place;
    u16 cursor, first;

    if (wcb->Visible == 0 || wcb->WinTyp != 0 || wcb->BrfFul != 0)
        return GO(NEXT_INPUT);
    AdrNum(panel, (struct Place STACK *)&place);
    cursor = place.cursor;
    first = place.first;
    if (cursor - first >= place.lines)
        cursor -= place.lines;
    else {
        /* From the first column the list moves by one, to its start if
           it has less than one to move by. */
        cursor = cursor >= place.lines ? cursor - place.lines : 0;
        if (first >= place.lines)
            first -= place.lines;
        else
            cursor = first = 0;
    }
    F00_70(panel, cursor, first, r);
    return GO(NEXT_FUNC07);
}

u32 F03_00(u16 panel, struct Regs STACK *r)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct Place place;
    u16 cursor, first, column;

    if (wcb->Visible == 0 || wcb->WinTyp != 0 || wcb->BrfFul != 0)
        return GO(NEXT_INPUT);
    AdrNum(panel, (struct Place STACK *)&place);
    first = place.first;
    column = place.cursor - first;
    cursor = F03_30(place.cursor, place.lines, place.last);
    if (column >= place.lines + place.lines) {
        /* From the last column the list moves by one, and to its end if
           that leaves less than three columns to show. */
        first = F03_30(first, place.lines, place.last);
        if (place.last - first + 1
                < (u16)(place.lines + place.lines + place.lines)
            && first != 0)
            cursor = first = place.last;
    }
    F00_70(panel, cursor, first, r);
    return GO(NEXT_FUNC07);
}

/* PgUp and PgDn. */
u32 F04_00(u16 panel, struct Regs STACK *r)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct Place place;
    u16 cursor, first, page;

    if (wcb->Visible == 0 || wcb->WinTyp > 1)
        return GO(NEXT_FUNC07);
    page = F04_30(panel, (struct Place STACK *)&place);
    cursor = place.cursor;
    first = place.first;
    if (first == 0)
        cursor = 0;
    else {
        first = first >= page ? first - page : 0;
        if ((u16)(page + first) < cursor)
            cursor = page + first;
    }
    F00_70(panel, cursor, first, r);
    return GO(NEXT_FUNC07);
}

u32 F05_00(u16 panel, struct Regs STACK *r)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct Place place;
    u16 cursor, first, page, limit;

    if (wcb->Visible == 0 || wcb->WinTyp > 1)
        return GO(NEXT_FUNC07);
    page = F04_30(panel, (struct Place STACK *)&place);
    cursor = place.cursor;
    first = F03_30(place.first, page, place.last);
    limit = place.last >= page ? place.last - page : 0;
    if (first > limit)
        first = limit;
    if (first <= place.first)
        cursor = first = place.last;    /* nowhere to move to: the end */
    else if (first > cursor)
        cursor = first;
    F00_70(panel, cursor, first, r);
    return GO(NEXT_FUNC07);
}

/* Home and End. With the panels off, and in the Info, they are keys of
   the command line. */
u32 F06_00(u16 panel, struct Regs STACK *r)
{
    struct WCBdef __far *wcb = PANEL(panel);

    if (wcb->Visible == 0 || wcb->WinTyp > 1)
        return GO(NEXT_INPUT);
    F00_70(panel, 0, 0, r);
    return GO(NEXT_FUNC07);
}

u32 F07_00(u16 panel, struct Regs STACK *r)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct Place place;

    if (wcb->Visible == 0 || wcb->WinTyp > 1)
        return GO(NEXT_INPUT);
    AdrNum(panel, (struct Place STACK *)&place);
    F00_70(panel, place.last, place.last, r);
    return GO(NEXT_FUNC07);
}

/* Ins: select the file under the cursor, or unselect it, and go down a
   line if Ins is set up to. The BIOS is told that the key is up again
   then, unless the cursor is on the last file. */
u32 F08_00(u16 panel, struct Regs STACK *r)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct FILENT __far *file;
    u16 address;

    if (!FilePan(panel))
        return GO(NEXT_FUNC07);
    address = wcb->CurAdr;
    if (address >= wcb->EndBuf)
        return GO(NEXT_FUNC07);
    if (md.InsDwn != 0
        && address + sizeof(struct FILENT) < wcb->EndBuf)
        *KEYS_DOWN &= 0x7F;
    file = ENTRY(wcb, address);
    if (file->NameF[0] != '.')
        file->AttrF ^= ATTR_SELECTED;
    F08_10(panel, r);
    if (md.InsDwn == 0)
        return GO(NEXT_FUNC07);
    return F01_01(panel, r);
}

/* Ask for a mask and select, unselect or turn round the files like it,
   as `include` says, the word that the dialog has for ^I. Directories
   are unselected by a mask and nothing else. In a panel that shows no
   files the key goes to the command line. */
static u32 F10_01(u16 panel, u16 include)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct FILENT __far *file;
    u16 address, found;

    if (!FilePan(panel))
        return GO(NEXT_INPUT);
    if ((u8)Dialog(OFFSET(Sel_B), include, 0, 0, MAIN_SEG) != 0
        || md.SelMas[0] == 0)
        return GO(NEXT_FUNC04);
    Convert(md.SelMas, md.TmpBuf);
    found = 0;
    for (address = DSKBUF; address < wcb->EndBuf;
         address += sizeof(struct FILENT)) {
        file = ENTRY(wcb, address);
        if (file->NameF[0] == '.' || !TstMas(file->NameF, md.TmpBuf))
            continue;
        if (include == OFFSET(Unsl_S)) {
            found = 1;
            file->AttrF &= ~ATTR_SELECTED;
        } else if (!(file->AttrF & ATTR_DIRECTORY)) {
            found = 1;
            if (include == OFFSET(Sel_S))
                file->AttrF |= ATTR_SELECTED;
            else
                file->AttrF ^= ATTR_SELECTED;
        }
    }
    if (found)
        return GO(NEXT_FUNC01);
    Dialog(OFFSET(ErSel_B), include, 0, 0, MAIN_SEG);
    return GO(NEXT_FUNC04);
}

/* Grey * turns the selection round, when nothing has been typed: it is a
   character of a command too. F76_01 is where the menu comes in. */
u32 F76_01(u16 panel)
{
    return F10_01(panel, OFFSET(Inv_S));
}

u32 F76_00(u16 panel)
{
    if (md.DosBuf[0] != 0)
        return GO(NEXT_INPUT);
    return F76_01(panel);
}

/* Grey + and Grey -. */
u32 F09_00(u16 panel)
{
    return F10_01(panel, OFFSET(Sel_S));
}

u32 F10_00(u16 panel)
{
    return F10_01(panel, OFFSET(Unsl_S));
}

/* ^M: select again what was selected before a command went through the
   selected files. */
u32 F78_00(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct FILENT __far *file;
    u16 address;

    if (!FilePan(panel))
        return GO(NEXT_FUNC04);
    for (address = DSKBUF; address < wcb->EndBuf;
         address += sizeof(struct FILENT)) {
        file = ENTRY(wcb, address);
        if (file->NameF[0] != '.' && file->AttrF & ATTR_WAS_SELECTED)
            file->AttrF |= ATTR_SELECTED;
    }
    return GO(NEXT_FUNC01);
}

/* ^O: both panels off, with what DOS has shown in their place, or on
   again: the other one if it was on, which Ctrl_O keeps. The menu bar is
   drawn first if a panel is to be read again. */
u32 F11_00(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct WCBdef __far *second = PANEL(md.WCBsec);
    u16 begin, size, was, changed;

    if (wcb->Visible != 0) {
        md.Ctrl_O = second->Visible;
        second->Visible = 0;
        wcb->Visible = 0;
        begin = BegBX(panel) & 0xFF00;
        size = SizDX() & 0xFF00 | 80;
        ResUser(begin, size);
        Window(begin, size);            /* Func03 */
        return GO(NEXT_FUNC04);
    }
    was = md.Ctrl_O;
    second->Visible = (u8)was;
    changed = second->ChgFlg;
    wcb->Visible = 1;
    if (wcb->ChgFlg != 0 || was != 0 && changed != 0)
        PutMnu(3);                      /* PutMnu3 */
    return GO(NEXT_INIT50);
}

/* ^P, ^F1 and ^F2: the other panel, the left and the right one off or
   on, which is in the menu of a panel. */
u32 F12_00(u16 panel)
{
    if (PANEL(panel)->Visible == 0)
        return GO(NEXT_POP_FUNC05);
    return GO_PANEL(NEXT_M06_00, md.WCBsec, 0);
}

u32 F13_00(void)
{
    return GO_PANEL(NEXT_M06_00, md.WCB1seg, 1);
}

u32 F14_00(void)
{
    return GO_PANEL(NEXT_M06_00, md.WCB2seg, 5);
}

/* ^U: the panels change places. */
u32 F15_00(u16 panel)
{
    u16 left;

    if (PANEL(panel)->Visible == 0)
        return GO(NEXT_FUNC04);
    left = md.WCB1seg;
    md.WCB1seg = md.WCB2seg;
    md.WCB2seg = left;
    InvVar(&md.Active);
    ResUser(BegBX(panel) & 0xFF00, SizDX() & 0xFF00 | 80);
    return GO(NEXT_INIT50);
}

/* ^B: the key bar off or on. Where the command line is on the last line
   of the screen the bar has no room: what is shown there is written
   again, and the program looks for the command line anew, which moves
   everything up a line. The assembly asks where a panel begins with DS
   on the main data: only the row counts, which is that of any panel. */
u32 F16_00(void)
{
    u16 last = (u16)(u8)(md.Lines - 1) << 8;
    u16 begin, size;

    if (md.KeyBar != 0) {
        md.KeyBar = 0;
        md.BarPrev = 0xFF;
        ClsLine(last, MAIN_SEG);
        Window(last, 0x150);
        return GO(NEXT_FUNC04);
    }
    md.KeyBar = 1;
    if (last >> 8 != md.DosLin)
        return GO(NEXT_FUNC04);
    begin = BegBX(MAIN_SEG) & 0xFF00;
    size = SizDX() & 0xFF00 | 80;
    ResUser(begin, size);
    Window(begin, size);
    Cursor1();
    SavMous();
    CurMous(0);
    return GO(NEXT_INIT40);
}

/* F17_06: a panel is to be one of files, and read. */
static u32 F17_06(u16 panel)
{
    PANEL(panel)->ChgFlg = 1;
    return GO_PANEL(NEXT_M01_01, panel, 0);
}

/* ^R: read a panel again. Files: none is selected afterwards and the
   cursor is on the first. The tree: its file on the disk is marked as
   out of date first, and if that cannot be done, because there is none,
   nothing is read. The kind of panel for the menu's routine is then
   what DOS answered to the one byte written, 1, which is the tree's.

   F17_01 is where the menu of a panel comes in, with that panel. */
u32 F17_01(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u16 kind = wcb->WinTyp;
    u16 address;
    struct Answer marked;

    if (kind != 0) {
        if (kind > 2)
            return GO(NEXT_POP_FUNC04);
        if (kind != 2) {
            marked = DelTr0(md.DosPth);
            if (marked.failed != 0)
                return GO(NEXT_POP_FUNC04);
            kind = (u8)marked.value;
        }
        return GO_PANEL(NEXT_M03_00, panel, 5 - kind);       /* F17_02 */
    }
    address = wcb->EndBuf;
    wcb->CurAdr = address;
    ENTRY(wcb, address)->NameF[0] = 0;
    for (address = DSKBUF; address < wcb->EndBuf;
         address += sizeof(struct FILENT))
        ENTRY(wcb, address)->AttrF &= ~ATTR_SELECTED;
    return F17_06(panel);
}

/* ^Z and ^L: the tree, or the Info, in the other panel, if neither panel
   shows it; and if one does, what that panel showed before, which Ctrl_L
   keeps. */
static u32 F18_01(u16 panel, u16 kind)
{
    struct WCBdef __far *wcb = PANEL(panel);

    if (wcb->Visible == 0)
        return GO(NEXT_POP_FUNC04);
    if (kind != wcb->WinTyp) {
        panel = InvWCB(panel);
        wcb = PANEL(panel);
        if (wcb->Visible == 0 || kind != wcb->WinTyp)
            return GO_PANEL(NEXT_M03_00, panel, 5 - kind);
    }
    kind = wcb->Ctrl_L;
    if (kind == 0)
        return F17_06(panel);
    return GO_PANEL(NEXT_M03_00, panel, 5 - kind);
}

u32 F79_00(u16 panel)
{
    return F18_01(panel, 1);
}

u32 F18_00(u16 panel)
{
    return F18_01(panel, 2);
}

/* Make the other panel the current one: have DOS go to its drive and
   directory, if both panels show files and not the same directory, write
   the prompt, and change the two over. If DOS cannot go there, for
   another reason than that the directory is gone, the current directory
   counts as unknown: its drive is all that is shown of it, with a
   question mark, and the panel is read again.

   `panel` is the current one. The menu's routines call this too, when
   the current panel is switched off. */
void F20_02(u16 panel)
{
    struct WCBdef __far *other = PANEL(md.WCBsec);
    const struct WCBdef __far *current;
    struct DosRegs regs;
    u16 whole = (u16)md.Lines << 8 | 80;
    u16 length, i;

    if (PANEL(panel)->WinTyp == 0 && other->WinTyp == 0) {
        if (md.PathEr == 0) {
            current = PANEL(md.WCBcrn);
            length = StrLen(current->WinPath);
            for (i = 0; i != length; i++)
                if (other->WinPath[i] != current->WinPath[i])
                    break;
            if (i == length)
                goto same;
        }
        HidCur();
        Window(0, whole);
        if (Fantom((u8)(other->WinPath[0] - ('A' - 1))) == 0) {
            regs.ax = DOS_CHDIR;
            regs.bx = regs.cx = regs.si = regs.di = 0;
            regs.name = other->WinPath;
            if (!Intr21((struct DosRegs STACK *)&regs)
                || regs.ax == ERR_PATH) {
                SelDrv((u8)(other->WinPath[0] - 'A'));
                SetPth();
                goto same;
            }
        }
        md.DosPth[0] = other->WinPath[0];
        md.DosPth[1] = other->WinPath[1];
        md.DosPth[2] = other->WinPath[2];
        md.DosPth[3] = 0;
        md.PathEr = 1;
        other->ChgFlg = 1;
        SelDrv((u8)(other->WinPath[0] - 'A'));
    } else if (md.PathEr != 0) {
        HidCur();
        Window(0, whole);
        SetPth();
    }
same:
    i = md.WCBsec;
    PutPrm(i);
    md.WCBsec = md.WCBcrn;
    md.WCBcrn = i;
    InvVar(&md.Active);
}

/* Tab: to the other panel, if it is on. The panel that is left is drawn
   as the one that is not current. */
u32 F20_00(u16 panel)
{
    u16 other = InvWCB(panel);
    u16 current;

    if (PANEL(other)->Visible == 0)
        return GO(NEXT_FUNC05);
    current = md.WCBcrn;
    md.WCBcrn = other;
    PutPath(panel);
    PtFs00(panel);
    md.WCBcrn = current;
    F20_02(panel);
    PutPath(md.WCBcrn);
    return GO(NEXT_FUNC01);
}
