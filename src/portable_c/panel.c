/*
 * A panel of files on the screen: its frame, the path over it, the files
 * in it and the cursor on one of them.
 *
 * Translated from VCSUB2.INC and VCSUB3.INC.
 *
 * All of this is written to the screen buffer, and but for SetCur it is
 * left to the caller to put that on the screen. A panel that shows the
 * tree or the Info has its frame, its title and its cursor from here; what
 * is in it is written by PutTre of tree.c and PutInf of info.c, which
 * who wants a panel written calls for a panel of that kind.
 *
 * What PutName and Panel8 are in the assembly is written out where it is
 * used: these routines are called from deep down, and a function called
 * costs bytes of the program's short stack.
 */
#include "vc.h"

void Panel(u16 panel);
void PutPath(u16 panel);
void PutFls(u16 panel);
void SetCur(u16 panel, u16 set);
u8 *FilDir(u8 *buffer, u16 directories, u16 files);

/* How the assembly passes arguments to these, and what it expects back;
   PtPt10 is in vc.h. */
#pragma aux Panel   "*_c" __far parm [ax] modify exact [ax]
#pragma aux PutPath "*_c" __far parm [ax] modify exact [ax]
#pragma aux PutFls  "*_c" __far parm [ax] modify exact [ax]
#pragma aux SetCur  "*_c" __far parm [dx] [ax] modify exact [ax]
#pragma aux FilDir  "*_c" __far parm [di] [bx] [dx] value [di] modify exact [di]

/* The titles of the columns, in brief and in full. */
static const u8 CODE Brief_S[] = {
    '^', 'r', 'N', 'a', 'm', 'e', 0xFF, 9, 'N', 'a', 'm', 'e', 0xFF, 9,
    'N', 'a', 'm', 'e', 0
};
static const u8 CODE Full_S[] = {
    '^', 'r', 'N', 'a', 'm', 'e', 0xFF, 8, 'S', 'i', 'z', 'e', 0xFF, 5,
    'D', 'a', 't', 'e', 0xFF, 4, 'T', 'i', 'm', 'e', 0
};
static const u8 CODE Panel_P[] = {Back_C, Title_C};
/* The columns of a panel in which a line separates two of its own: in
   brief, Brief_T in the assembly, and in full, Full_T. A 0 ends them. */
static const u8 CODE Panel_T[2][4] = {{13, 26, 0, 0}, {13, 23, 32, 0}};

/* What stands for the path over a panel of another kind, and what follows
   the path of one that shows only programs. Tree_S is a constant of
   src/c/treebox_t.c.

   PtPt10 copies 25 bytes for Progs, which has 7. Behind it here is what
   the original has behind it and so copies too, the two markers of
   directories in the size column. */
static const u8 CODE Info_S[] = {' ', 'I', 'n', 'f', 'o', ' ', 0};
static const u8 CODE Progs_S[25] = {
    ' ', 'P', 'r', 'o', 'g', 's', 0,
    0x10, 'U', 'P', '-', '-', 'D', 'I', 'R', 0x11,
    0x10, 'S', 'U', 'B', '-', 'D', 'I', 'R', 0x11
};
static const u8 CODE PutPt_P[] = {Back_C, Cursr_C};

static const u8 CODE BytIn_S[25] = {
    ' ', 'b', 'y', 't', 'e', 's', ' ', 'i', 'n', ' ', ' ', 's', 'e', 'l',
    'e', 'c', 't', 'e', 'd', ' ', 'f', 'i', 'l', 'e', 's'
};
static const u8 CODE PtFs_P[] = {Slct_C};

static const u8 CODE FilDi_S[24] = {
    ' ', 'f', 'i', 'l', 'e', 's', ' ', 'a', 'n', 'd', ' ', ' ', 'd', 'i',
    'r', 'e', 'c', 't', 'o', 'r', 'i', 'e', 's', 'y'
};

/* Characters of the frame, and those that mark a file between its name
   and its extension. */
#define LINE        0xC4                /* a single line */
#define LEFT_RIGHT  0xC7B6              /* where one meets the frame, on
                                           the left and on the right */
#define COLUMN_TOP  0xD1                /* where a column line begins, */
#define COLUMN      0xB3                /* runs */
#define COLUMN_END  0xCF                /* and ends in the frame, */
#define COLUMN_LINE 0xC1                /* or in a single line */
#define MARK_HIDDEN   0xB0              /* hidden or a system file */
#define MARK_SELECTED 0xDE              /* selected, where colours do not
                                           show it */
#define MARK_BOTH     0xDB

#define LAPTOP 2                        /* ColTyp of the palette without
                                           colours to tell files by */

/* Draw the frame of a panel, empty: around a panel of files with the
   titles of its columns and the lines between them, and with the line
   over the mini status if that is on; around the Info with the program's
   name and the two lines that divide it; around the tree, or a panel of
   yet another kind, with the line over its last row.

   Nothing here has a line to include, and the assembly names none. */
void Panel(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct LineArgs line;
    u16 place = BegBX(panel);
    u16 *screen = BasAdr(place);
    u16 color = Color(Panel_P[0]) << 8;
    u16 full, count, i;

    screen = BoxDubl(screen, color, (SizDX() >> 8) - 2, 0, 38) - 3 * 80;
    if (wcb->WinTyp == 1 || wcb->WinTyp == 3) {
        BoxLine(screen, color | LINE, LEFT_RIGHT, 0, 38);
    } else if (wcb->WinTyp == 2) {
        place += 0x114;
        line.value = place;
        line.line = Title_S + 2;
        line.colors = Panel_P;
        CntLine((struct LineArgs STACK *)&line);
        screen = BoxLine(BasAdr(place) + (80 - 20), color | LINE, LEFT_RIGHT,
                         0, 38);
        BoxLine(screen + 7 * 80, color | LINE, LEFT_RIGHT, 0, 38);
    }
    if (wcb->WinTyp != 0)
        return;
    full = wcb->BrfFul != 0 ? 1 : 0;
    screen = BasAdr(place);
    line.value = color | 33;
    line.screen = screen + 85;
    if (full)
        line.line = Full_S;
    else
        line.line = Brief_S;
    line.colors = Panel_P;
    NgLine((struct LineArgs STACK *)&line);

    /* Panel8 in the assembly: a cell in each of the columns, and on to
       the next row. */
#define COLUMNS(cell) \
    do { \
        for (i = 0; Panel_T[full][i] != 0; i++) \
            screen[Panel_T[full][i]] = (cell); \
        screen += 80; \
    } while (0)

    COLUMNS(color | COLUMN_TOP);
    count = LnsAX() + 1;
    do
        COLUMNS(color | COLUMN);
    while (--count);
    if (md.MiniSta != 0) {
        BoxLine(screen, color | LINE, LEFT_RIGHT, 0, 38);
        COLUMNS(color | COLUMN_LINE);
    } else {
        COLUMNS(color | COLUMN_END);
    }
#undef COLUMNS
}

/* Make the text that is shown for the path of a panel of files, at
   TmpErr+1: between two spaces its path, with its mask if it has one, the
   whole then written as a path is, and with Progs behind it if it shows
   only programs. Returns the address of the zero byte that ends it. */
u8 *PtPt10(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u8 __far *end = md.TmpErr + 1;

    *end++ = ' ';
    end = MvzLine(end, wcb->WinPath, LenPath);
    if (wcb->DskMas[0] != 0) {
        end = MvzLine(AddFile(end), wcb->DskMas, 13);
        LetLin(md.TmpErr + 1);
    }
    /* 25 bytes for a text of 7: the rest stays behind the end of this
       text, as far as 106 bytes into TmpErr if the path is long. */
    if (wcb->MasTyp != 0)
        end = MvzLine(end, Progs_S, sizeof Progs_S);
    end[0] = ' ';
    end[1] = 0;
    return (u8 *)(u16)(u32)end + 1;
}

/* Write the path of a panel into the top of its frame, in the middle: in
   the colour of the cursor if the panel is the current one. A panel of
   another kind has its title there, and one of a kind beyond the Info
   nothing.

   A path is shortened to 37 characters. On the right panel, in the first
   row of the screen and with the clock on, it is shortened to as many as
   the clock leaves, 31 or 32, and one that is longer than 28 or 30 with
   its spaces is moved to the left by half of what it has more, as it was
   before it was shortened and of no more than the 32 or 33. */
void PutPath(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct LineArgs line;
    u16 place = BegBX(panel);
    u16 limit, least, length;

    place = ADD_LOW(place, 20);
    if (wcb->WinTyp > 2)
        return;
    if (wcb->WinTyp == 2) {
        line.line = Info_S;
    } else if (wcb->WinTyp != 0) {
        line.line = Tree_S;
    } else {
        length = (u8)(PtPt10(panel) - (md.TmpErr + 1));
        limit = 38;
        if (place >> 8 == 0 && md.Clock != 0 && panel == md.WCB2seg) {
            limit = 0x20;
            least = 0x1C;
            if (md.Country.TimeFmt != 0) {
                limit = 0x21;
                least = 0x1E;
            }
            if (length >= least) {
                if (length > limit)
                    length = limit;
                place = ADD_LOW(place, -((length - least >> 1) + 1));
            }
        }
        ShrtLin(md.TmpErr + 2, limit - 1);
        line.line = Var_S;
    }
    line.value = place;
    if (panel == md.WCBcrn)
        line.colors = PutPt_P + 1;
    else
        line.colors = PutPt_P;
    CntLine((struct LineArgs STACK *)&line);
}

/* Write the files of a panel into it, from the first one it shows, each
   in its colour: 8 characters of its name, the mark of a hidden or a
   system file, or of a selected one in the laptop palette, and 3 of its
   extension; a directory that begins with a full stop as it is. In full
   there are its size, date and time behind that, and in brief three
   columns of files.

   Then, if the mini status is on, write that: the name, size, date and
   time of the file under the cursor, or how many files are selected and
   how many bytes they have. A directory counts only when the sizes of
   directories have been worked out. What is too long for the line has
   its bytes in thousands, with a K. */
void PutFls(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct LineArgs line;
    const struct FILENT __far *file;
    const u8 __far *name;
    u16 *screen;
    u8 *text;
    const u8 *from;
    u32 size;
    u16 lines, count, place, bottom, address, color, c, i;

    CorPar(panel);
    lines = LnsAX();
    count = lines;
    place = BegBX(panel) + 0x201;
    bottom = (u8)((place >> 8) + lines);
    address = wcb->First;
    if (wcb->BrfFul == 0)
        count += lines + lines;
    do {
        if (address >= wcb->EndBuf)
            break;
        file = ENTRY(wcb, address);
        color = Back_C;
        if (address == wcb->CurAdr && panel == md.WCBcrn)
            color = Cursr_C;
        if (file->AttrF & ATTR_SELECTED)
            color += Slct_C;
        color = Color(color) << 8;
        screen = BasAdr(place);

        /* PutName in the assembly. */
        name = file->NameF;
        i = 12;
        if (name[0] != '.') {
            for (i = 8; i; i--) {
                c = *name;
                if (c == 0 || c == '.')
                    c = ' ';
                else
                    name++;
                *screen++ = color | c;
            }
            c = ' ';
            if (file->AttrF & (ATTR_HIDDEN | ATTR_SYSTEM))
                c = MARK_HIDDEN;
            if (md.ColTyp == LAPTOP && (file->AttrF & ATTR_SELECTED)) {
                c = MARK_SELECTED;
                if (file->AttrF & (ATTR_HIDDEN | ATTR_SYSTEM))
                    c = MARK_BOTH;
            }
            *screen++ = color | c;
            if (*name != 0)
                name++;
            i = 3;
        }
        for (; i; i--) {
            c = *name;
            if (c == 0)
                c = ' ';
            else
                name++;
            *screen++ = color | c;
        }

        place += 0x100;
        if (wcb->BrfFul != 0) {
            SzDtTm(file, screen, color);
        } else if ((u8)(place >> 8) >= bottom) {
            place -= (lines & 0xFF) << 8;
            place = ADD_LOW(place, 13);
        }
        address += sizeof(struct FILENT);
    } while (--count);

    if (md.MiniSta == 0)
        return;
    place = BegBX(panel);
    place += (SizDX() & 0xFF00) - 0x200;
    screen = BasAdr(place + 1);
    color = Color(Back_C) << 8;
    for (i = 0; i < 38; i++)
        screen[i] = color | ' ';
    address = DSKBUF;
    if (address >= wcb->EndBuf)
        return;
    size = 0;
    count = 0;
    do {
        file = ENTRY(wcb, address);
        if ((file->AttrF & ATTR_SELECTED)
            && (wcb->SizDir != 0 || !(file->AttrF & ATTR_DIRECTORY))) {
            size += file->SizeF;
            count++;
        }
        address += sizeof(struct FILENT);
    } while (address < wcb->EndBuf);
    if (count == 0) {
        file = ENTRY(wcb, wcb->CurAdr);
        MovDz0(screen, file->NameF);
        SzDtTm(file, screen + 12, color);
        return;
    }

    text = Number(md.TmpBuf, (u16)(size >> 16), (u16)size, NUM_SEPARATED);
    for (i = 0; i < 6; i++)
        *text++ = BytIn_S[i];
    if (size == 1)
        text--;                         /* 1 byte */
    for (; i < 10; i++)
        *text++ = BytIn_S[i];
    text = Number(text, 0, count, NUM_SEPARATED);
    for (; i < 25; i++)
        *text++ = BytIn_S[i];
    if (count == 1)
        text--;                         /* 1 selected file */
    *text = 0;
    if ((u16)(text - md.TmpBuf) > 36) {
        /* The b of bytes is looked for in 25 characters. A K goes over
           the last three digits before it, and over the separator before
           those if the country has one, and the rest moves up: 40 bytes,
           whatever they hold. */
        text = md.TmpBuf;
        for (i = 25; i; i--)
            if (*text++ == BytIn_S[1])
                break;
        text -= 2;
        from = text;
        text -= 3;
        if (md.Country.Sep1000[0] != 0)
            text--;
        *text++ = 'K';
        for (i = 40; i; i--)
            *text++ = *from++;
    }
    line.value = ADD_LOW(place, 20);
    line.line = md.TmpBuf;
    line.colors = PtFs_P;
    CntLine((struct LineArgs STACK *)&line);
}

/* Show the cursor of a panel on the screen, if `set` is not 0, or take it
   off: the colours of the entry it is on, which is the width of a column
   in brief, of the panel in full, and in the tree 14 cells from the one
   before the name, or 3 for the root. Only the low byte of `set` counts.
   Nothing is done for a panel of another kind, or for one with no entry.

   The mark between the name and the extension of a file is written too.
   So is, in the tree, the cell before the name, which is a space under
   the cursor and without it a piece of the line that leads to the name;
   that piece is not given the colour of the rest. */
void SetCur(u16 panel, u16 set)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct Place at;
    u16 *screen;
    u8 *mark;
    u16 color, place, below, width, lines;
    u8 attributes;

    set &= 0xFF;
    color = set != 0 ? Cursr_C : Back_C;
    AdrNum(panel, (struct Place STACK *)&at);
    below = at.cursor - at.first;
    if (wcb->WinTyp > 1)
        return;
    if (wcb->WinTyp == 0) {
        if (wcb->CurAdr >= wcb->EndBuf)
            return;
        place = BegBX(panel) + 0x201;
        width = 38;
        if (wcb->BrfFul == 0) {
            /* The column, 13 cells wide, and the row in it: bytes, of a
               division by the low byte of the number of lines. */
            lines = at.lines & 0xFF;
            place = ADD_LOW(place, (below / lines & 0xFF) * 13);
            below %= lines;
            width = 12;
        }
        place += (below & 0xFF) << 8;
        screen = BasAdr(place);
        attributes = ENTRY(wcb, wcb->CurAdr)->AttrF;
        mark = (u8 *)(screen + 8);
        *mark = ' ';
        if (attributes & (ATTR_HIDDEN | ATTR_SYSTEM))
            *mark = MARK_HIDDEN;
        if (attributes & ATTR_SELECTED) {
            if (md.ColTyp == LAPTOP) {
                *mark = MARK_SELECTED;
                if (attributes & (ATTR_HIDDEN | ATTR_SYSTEM))
                    *mark = MARK_BOTH;
            }
            color += Slct_C;
        }
    } else {
        if (at.cursor >= ((struct DIRTREE *)md.TreeBuf)->TreeLen)
            return;
        place = BegBX(panel) + 0x118;
        place += (below & 0xFF) << 8;
        /* A directory up to 11 levels down has its name at its level,
           two or three columns to each; one further down in the last
           column. */
        lines = ((struct DIRENT *)md.TreAdr)->DirLevel;
        if (lines <= 11) {
            width = lines + lines - 0x17;
            if (md.TreeTyp != 0)
                width += lines;
            place = ADD_LOW(place, width);
        }
        screen = BasAdr(place);
        *(u8 *)screen = ' ';
        width = 3;
        if (md.TreAdr > TRELIST) {
            width = 14;
            if (set == 0) {
                *screen++ = Color(Back_C) << 8 | LINE;
                width--;
            }
        }
        if (md.TreAdr == md.TreCrnt)
            color += Slct_C;
    }
    MvsColr(screen, width, Color(color));
    Window(place, 0x100 | width + 1);
}

/* Write "3 files and 2 directories" to `buffer`, with a zero byte behind
   it, or of the two only what there is some of, and return the address
   behind the zero. With neither files nor directories that is
   "0 directories". */
u8 *FilDir(u8 *buffer, u16 directories, u16 files)
{
    u16 i;

    if (files != 0) {
        buffer = Number(buffer, 0, files, NUM_SEPARATED);
        for (i = 0; i < 6; i++)
            *buffer++ = FilDi_S[i];
        if (files == 1)
            buffer--;                   /* 1 file */
        if (directories == 0)
            goto end;
        for (; i < 11; i++)
            *buffer++ = FilDi_S[i];
    }
    buffer = Number(buffer, 0, directories, NUM_SEPARATED);
    for (i = 11; i < 23; i++)
        *buffer++ = FilDi_S[i];
    if (directories == 1) {
        buffer -= 3;
        *buffer++ = FilDi_S[23];        /* 1 directory */
    }
end:
    *buffer++ = 0;
    return buffer;
}
