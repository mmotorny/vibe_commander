/*
 * Where the two panels are on the screen and how large, and where in a
 * panel the cursor and the first line shown are.
 *
 * Translated from VCSUB2.INC and VCFUNC.INC.
 *
 * A panel keeps the cursor and the first line it shows as addresses: of
 * file entries in its own segment, CurAdr and First, or, if it shows the
 * tree, of directory entries in the main data, TreAdr and TreFrst. To move
 * them the program turns the addresses into numbers, counted from the
 * start of the list, and back.
 *
 * The arithmetic that several of these share is written as macros and
 * with shifts: CorPar is called from where the panels are drawn, deep
 * down, and a function called or a multiplication costs bytes of the
 * program's short stack.
 */
#include "vc.h"

u16 NumAdr(struct WCBdef *panel, u16 number);

/* struct Place, BegBX, SizDX, LnsAX, InvWCB, FilePan, AdrNum and CorPar
   are in vc.h, and so is ChgAdr, with struct Shown. */

/* The rows a panel takes: half the screen and three, one more on a screen
   of more than 25 lines; or, with the panels full screen, all lines but
   those of the command line and the key bar, and of the menu bar where
   that is always visible. A byte, as the assembly has it in DH. */
#define ROWS() \
    ((u8)(md.FulScr != 0 ? md.Lines - 2 - (md.MenuVs != 0 ? 1 : 0) \
                         : (md.Lines >> 1) + 3 + (md.Lines > 25 ? 1 : 0)))

/* How many of those rows show files: all but the frame and the titles,
   and but the two of the mini status where that is on. `rows` is a byte
   made a word with its sign, and the mini status is held against the
   high byte of that, which is 0 on any screen there is. */
#define LINES(rows) \
    (((rows) & 0xFF00) \
     | (u8)((rows) - 3 - ((u8)((rows) >> 8) != md.MiniSta ? 2 : 0)))

/* The number of the entry at an address, and the address of the entry
   with a number: in a list of files, and in the tree. */
#define FILE_NUMBER(address) \
    ((u16)((address) - DSKBUF) / sizeof(struct FILENT))
#define TREE_NUMBER(address) \
    ((u16)((address) - TRELIST) / sizeof(struct DIRENT))
#define FILE_ADDRESS(number) (((number) << 4) + ((number) << 3) + DSKBUF)
#define TREE_ADDRESS(number) (((number) << 4) + TRELIST)

/* Where on the screen a panel begins: the row in the high byte, below the
   menu bar if that is always visible, and the column in the low one. */
u16 BegBX(struct WCBdef *panel)
{
    return (panel == LeftPanel ? 0 : 40) | (md.MenuVs != 0 ? 0x100 : 0);
}

/* The size of a panel: its rows in the high byte, its 40 columns in the
   low one. */
u16 SizDX(void)
{
    return (u16)ROWS() << 8 | 40;
}

/* How many lines of a panel show files. */
u16 LnsAX(void)
{
    u16 rows = (u16)(signed char)ROWS();

    return LINES(rows);
}

/* The other panel. */
struct WCBdef *InvWCB(struct WCBdef *panel)
{
    return panel == LeftPanel ? RightPanel : LeftPanel;
}

/* Whether a panel shows files and is not switched off: the original
   answers in the carry flag. */
u16 FilePan(struct WCBdef *panel)
{
    struct WCBdef *wcb = panel;

    return wcb->Visible != 0 && wcb->WinTyp == 0;
}

/* Where the cursor of a panel is, as numbers: see struct Place. The last
   entry is the number of them less one, or 0 if there is none. A panel
   that shows no files is taken to show the tree, which has four rows
   that show no directory. */
#define PLACE(wcb, cursor, first, last, lines) \
    do { \
        if ((wcb)->WinTyp == 0) { \
            cursor = FILE_NUMBER((wcb)->CurAdr); \
            first = FILE_NUMBER((wcb)->First); \
            last = FILE_NUMBER((wcb)->EndBuf); \
            lines = (u16)(signed char)ROWS(); \
            lines = LINES(lines); \
        } else { \
            cursor = TREE_NUMBER(md.TreAdr); \
            first = TREE_NUMBER(md.TreFrst); \
            last = ((struct DIRTREE *)md.TreeBuf)->TreeLen; \
            lines = (u16)(signed char)(u8)(ROWS() - 4); \
        } \
        if (last != 0) \
            last--; \
    } while (0)

void AdrNum(struct WCBdef *panel, struct Place *place)
{
    u16 cursor, first, last, lines;

    PLACE(panel, cursor, first, last, lines);
    place->cursor = cursor;
    place->first = first;
    place->last = last;
    place->lines = lines;
}

/* The address of the entry with a number, in the list of a panel. */
u16 NumAdr(struct WCBdef *panel, u16 number)
{
    if (panel->WinTyp == 0)
        return FILE_ADDRESS(number);
    return TREE_ADDRESS(number);
}

/* Give a panel another cursor and another first line, as addresses, and
   return those it had. */
struct Shown ChgAdr(struct WCBdef *panel, u16 cursor, u16 first)
{
    struct Shown was;

    if (panel->WinTyp == 0) {
        was.cursor = panel->CurAdr;
        was.first = panel->First;
        panel->CurAdr = cursor;
        panel->First = first;
    } else {
        was.cursor = md.TreAdr;
        was.first = md.TreFrst;
        md.TreAdr = cursor;
        md.TreFrst = first;
    }
    return was;
}

/* Set right the first line that a panel of files or of the tree shows,
   so that the cursor is among the lines shown, and the last of them is
   not empty while there are entries above the first. A panel of files in
   brief has three columns of lines. */
void CorPar(struct WCBdef *panel)
{
    u16 cursor, first, last, lines, below;

    if (panel->WinTyp > 1)
        return;
    PLACE(panel, cursor, first, last, lines);
    if (cursor < first)
        first = cursor;
    if ((panel->WinTyp | panel->BrfFul) == 0)
        lines += lines + lines;
    lines--;
    below = cursor - first;
    if (below > lines)
        first += below - lines;
    last -= first;
    if (lines > last) {
        lines -= last;
        first = first >= lines ? first - lines : 0;
    }
    if (panel->WinTyp == 0) {
        panel->CurAdr = FILE_ADDRESS(cursor);
        panel->First = FILE_ADDRESS(first);
    } else {
        md.TreAdr = TREE_ADDRESS(cursor);
        md.TreFrst = TREE_ADDRESS(first);
    }
}
