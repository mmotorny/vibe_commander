/*
 * Where the two panels are on the screen and how large, and where in a
 * panel the cursor and the first line shown are.
 *
 * Translated from VCSUB2.INC and VCFUNC.INC.
 *
 * A panel keeps the cursor and the first line it shows as addresses: of
 * file entries, as offsets in the panel, CurAdr and First, or, if it
 * shows the tree, of directory entries in the main data, as pointers,
 * TreeCursor and TreeFirst. To move them the program turns the addresses
 * into numbers, counted from the start of the list, and back. Where one
 * routine does that for either list, an entry of the tree is an offset
 * too, in the tree's list: TREE_OFFSET of vc.h.
 *
 * The arithmetic that several of these share is written as macros and
 * with shifts: CorPar is called from where the panels are drawn, deep
 * down, and a function called or a multiplication costs bytes of the
 * program's short stack.
 */
#include "vc.h"

u16 NumAdr(struct WCBdef *panel, i32 number);

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
   and but the two of the mini status where that is on. The original
   holds the mini status against the high byte of the rows made a word
   with their sign, which is 0: they are fewer than 50. */
#define LINES(rows) ((rows) - 3 - (md.MiniSta != 0 ? 2 : 0))

/* The number of the entry at an address, and the address of the entry
   with a number: in a list of files, and in the tree. An address in a
   panel is 16 bits by the panel's layout, which FILE_ADDRESS says. */
#define FILE_NUMBER(address) \
    ((i32)(u16)((address) - DSKBUF) / (i32)sizeof(struct FILENT))
#define TREE_NUMBER(entry) ((entry) - TRELIST)
#define FILE_ADDRESS(number) \
    ((u16)(((number) << 4) + ((number) << 3) + (i32)DSKBUF))
#define TREE_ADDRESS(number) (TRELIST + (number))

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
    return (u16)(ROWS() << 8 | 40);
}

/* How many lines of a panel show files. */
i32 LnsAX(void)
{
    return LINES(ROWS());
}

/* The other panel. */
struct WCBdef *InvWCB(struct WCBdef *panel)
{
    return panel == LeftPanel ? RightPanel : LeftPanel;
}

/* Whether a panel shows files and is not switched off: the original
   answers in the carry flag. */
bool FilePan(struct WCBdef *panel)
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
            lines = LINES(ROWS()); \
        } else { \
            cursor = TREE_NUMBER(TreeCursor); \
            first = TREE_NUMBER(TreeFirst); \
            last = ((struct DIRTREE *)md.TreeBuf)->TreeLen; \
            lines = ROWS() - 4; \
        } \
        if (last != 0) \
            last--; \
    } while (0)

void AdrNum(struct WCBdef *panel, struct Place *place)
{
    i32 cursor, first, last, lines;

    PLACE(panel, cursor, first, last, lines);
    place->cursor = cursor;
    place->first = first;
    place->last = last;
    place->lines = lines;
}

/* The address of the entry with a number, in the list of a panel: its
   offset in the panel, or in the tree's list. */
u16 NumAdr(struct WCBdef *panel, i32 number)
{
    if (panel->WinTyp == 0)
        return FILE_ADDRESS(number);
    return (u16)(number << 4);          /* TreeBuf holds 768 entries */
}

/* Give a panel another cursor and another first line, as such offsets,
   and return those it had. */
struct Shown ChgAdr(struct WCBdef *panel, u16 cursor, u16 first)
{
    struct Shown was;

    if (panel->WinTyp == 0) {
        was.cursor = panel->CurAdr;
        was.first = panel->First;
        panel->CurAdr = cursor;
        panel->First = first;
    } else {
        was.cursor = TREE_OFFSET(TreeCursor);
        was.first = TREE_OFFSET(TreeFirst);
        TreeCursor = TREE_ENTRY(cursor);
        TreeFirst = TREE_ENTRY(first);
    }
    return was;
}

/* Set right the first line that a panel of files or of the tree shows,
   so that the cursor is among the lines shown, and the last of them is
   not empty while there are entries above the first. A panel of files in
   brief has three columns of lines. */
void CorPar(struct WCBdef *panel)
{
    i32 cursor, first, last, lines, below;

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
    last -= first;                      /* not below 0: the cursor is on
                                           an entry, behind first */
    if (lines > last) {
        lines -= last;
        first = first >= lines ? first - lines : 0;
    }
    if (panel->WinTyp == 0) {
        panel->CurAdr = FILE_ADDRESS(cursor);
        panel->First = FILE_ADDRESS(first);
    } else {
        TreeCursor = TREE_ADDRESS(cursor);
        TreeFirst = TREE_ADDRESS(first);
    }
}
