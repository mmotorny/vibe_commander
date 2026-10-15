/*
 * The tree of directories: what a panel shows of it, and where in it a
 * directory is.
 *
 * Translated from VCSUB3.INC.
 *
 * The tree is a list in the main data, TreeBuf: one entry for every
 * directory of the disk, the root first and each directory before those
 * in it, with the level it is on. A path is not kept: it is made, entry
 * by entry, by putting each name behind the backslash of its level in a
 * path that is carried along the list. The program keeps the cursor and
 * the first line shown as pointers to entries, TreeCursor and TreeFirst,
 * and the entry of the current directory in TreeCurrent: TreAdr, TreFrst
 * and TreCrnt of the original, which are addresses in the main data.
 *
 * The assembly keeps what it carries along in MenuBuf, and so does this:
 * the buffer is free while no menu is open, and what is left in it goes
 * with the rest of the main data. What is done to a path in more than one
 * place is written as macros: these routines are called from deep down,
 * and a function called costs bytes of the program's short stack.
 */
#include "vc.h"

/* PutTree, PutTre, TreeCur, TrCr10 and TrCr20 are in vc.h. */

/* The colours of the tree in a panel: of a directory, of the one under
   the cursor, and of the current directory likewise. */
static const u8 PutTr_P[] = {Back_C, Cursr_C, Slct_C, CurSl_C};

#define TREE ((struct DIRTREE *)md.TreeBuf)

/* What PutTree keeps in MenuBuf. */
#define GOES_ON (md.MenuBuf)            /* for each level, whether a line
                                           runs down the screen there: 40
                                           bytes */
#define STATUS  (*(u16 *)(md.MenuBuf + 46))     /* where the path of the
                                                   cursor is written */
/* The word at 48 is the panel in the original, its segment, kept there
   by PutTree for itself. Here that is PutTree's `panel`, and the word is
   not written. */
#define PATH    (md.MenuBuf + 50)       /* the path of the entry at hand */
#define SHOWN   (md.MenuBuf + 140)      /* that of the cursor, as shown */

/* The characters a tree is drawn with. */
#define RUNS_DOWN    0xB3               /* a line past a directory */
#define BRANCH       0xC3               /* a line to one, and on */
#define BRANCH_LAST  0xC0               /* a line to the last one */
#define LINE         0xC4
#define MARK_CURRENT 0x11               /* the current directory */
#define BRACKETS     0xDDDE             /* around the cursor of a panel
                                           that is not the current one */

/* Put the name of the directory `entry` into `path` behind the backslash
   of its level, with a backslash and a zero byte behind it. A backslash
   is looked for in 20 bytes; if none is there the next one is looked for
   from behind them. 13 bytes are copied for the name, and one that has
   no end among them is taken to end with the last. `text`, `i` and `n`
   are variables to use. */
#define DESCEND(path, entry, text, i, n) \
    do { \
        (text) = (path); \
        for ((i) = (entry)->DirLevel; (i); (i)--) \
            for ((n) = 20; (n); (n)--) \
                if (*(text)++ == '\\') \
                    break; \
        for ((i) = 0; (i) < 13; (i)++) \
            (text)[(i)] = (entry)->DirName[(i)]; \
        for ((i) = 0; (i) < 12 && (text)[(i)] != 0; (i)++) \
            ; \
        (text)[(i)] = '\\'; \
        (text)[(i) + 1] = 0; \
    } while (0)

/* Whether the path `made`, which ends as DESCEND leaves it or is that of
   the root, is the directory `path`: the same up to where `path` ends,
   and ending there itself or one character later, behind its backslash.
   The end of `path` is looked for in 100 bytes; one that has none is
   taken to be 99 long. `i` and `n` are variables to use. */
#define IS_PATH(result, made, path, i, n) \
    do { \
        for ((n) = 0; (n) < 99 && (path)[(n)] != 0; (n)++) \
            ; \
        for ((i) = 0; (i) < (n) && (made)[(i)] == (path)[(i)]; (i)++) \
            ; \
        (result) = (i) == (n) && ((made)[(n)] == 0 || (made)[(n) + 1] == 0); \
    } while (0)

/* TrCr11 in the assembly: put the cursor of the tree on the directory
   CrnPath, if that is on the current drive and in the tree. `found` says
   whether it was. The paths are made at the start of MenuBuf. */
#define FIND(found, entry, count, text, i, n) \
    do { \
        (found) = false; \
        if (md.DosPth[0] != md.CrnPath[0]) \
            break; \
        md.MenuBuf[0] = md.DosPth[0]; \
        md.MenuBuf[1] = ':'; \
        md.MenuBuf[2] = '\\'; \
        md.MenuBuf[3] = 0; \
        (entry) = TRELIST; \
        for ((count) = TREE->TreeLen; (count); (count)--) { \
            if ((entry)->DirLevel != 0) \
                DESCEND(md.MenuBuf, entry, text, i, n); \
            IS_PATH(found, md.MenuBuf, md.CrnPath, i, n); \
            if (found) { \
                TreeCursor = (entry); \
                break; \
            } \
            (entry)++; \
        } \
    } while (0)

/* Write the tree into a panel that shows it, after setting right its
   first line.

   In the original this is where PutFls goes for a panel that does not
   show files, and it goes on to PutInf if the panel does not show the
   tree either. That choice is made by who wants a panel written, PtFs00
   of keys.c and its like, so that the Info is not written from inside
   another function. */
void PutTre(struct WCBdef *panel)
{
    CorPar(panel);
    PutTree(panel, BegBX(panel), SizDX(), PutTr_P);
}

/* Write the tree to the screen buffer, from its first line shown: into a
   box that begins at `place` and has the size `size`, rows in the high
   bytes, the lines one row and two columns in and four rows fewer than
   the box has. Each line is 38 cells from the column before, a directory
   at the column of its level with the lines that lead to it. `colors`
   are the numbers of four colours, as in PutTr_P.

   The current directory is in its own colours and has a mark in the last
   column but one; its entry is noted in TreeCurrent, which is 0 if it is
   not among the lines written. The directory under the cursor is in the
   colour of the cursor if `panel` is the current panel, and between
   brackets if not; its path is written into the row below the lines,
   which is the last but one of the box, the characters only, and is left
   in CrnPath.

   A tree with no directory deeper than 7 levels has three columns to a
   level, a deeper one two. The eleventh level has one column, and none
   has any from there on: such directories are drawn over each other's
   lines, without the piece that would lead to them from above.

   The entry behind the last one is looked at too, for whether it begins
   a level: that is whatever follows the list in TreeBuf. */
void PutTree(struct WCBdef *panel, u16 place, u16 size, const u8 *colors)
{
    struct DIRENT *entry;
    u16 *screen, *cell;
    u8 *goes_on, *text;
    u16 color;                          /* a cell, in the colour of its
                                           directory, which Color answers
                                           in a byte */
    u16 ends;                           /* the two characters around the
                                           cursor */
    i32 lines, width, which, level, i, n;
    bool current;

    STATUS = place + (size & 0xFF00) - 0x200 + 1;
    TreeCurrent = 0;
    entry = TRELIST;
    n = TREE->TreeLen;
    for (i = 0; i < n && entry[i].DirLevel <= 7; i++)
        ;
    md.TreeTyp = n != 0 && i == n;
    for (i = 0; i < 40; i++)
        GOES_ON[i] = 0;
    PATH[0] = md.DosPth[0];
    PATH[1] = md.DosPth[1];
    PATH[2] = '\\';
    PATH[3] = 0;
    screen = BasAdr(place + 0x102);
    n -= (i32)(TreeFirst - TRELIST);    /* fewer than 4,096 entries */
    lines = (size >> 8) - 4;
    if (lines > n) {
        lines = n;
        if (n == 0)
            return;
    }

    /* Every entry from the first of the list is gone through, for its
       path and for the lines that run down; those before the first line
       shown are not written, but for the spaces of their line, which go
       where the first line will be. */
    for (;; entry++) {
        color = (u16)(Color(colors[0]) << 8 | ' ');
        cell = screen - 1;
        for (i = 38; i; i--)
            *cell++ = color;
        cell = screen;
        goes_on = GOES_ON;
        level = entry->DirLevel;
        if (level != 0) {
            while (--level) {
                if (entry >= TreeFirst && goes_on <= GOES_ON + 10) {
                    if (goes_on < GOES_ON + 10) {
                        if (*goes_on != 0)
                            *(u8 *)cell = RUNS_DOWN;
                        cell += 2;
                    }
                    if (goes_on == GOES_ON + 10 || md.TreeTyp != 0)
                        cell++;
                }
                goes_on++;
            }
            if (entry->DirCont == 0)
                *goes_on = 0;
            goes_on++;
            if (entry >= TreeFirst) {
                if (goes_on < GOES_ON + 12)
                    *(u8 *)cell = entry->DirCont == 0 ? BRANCH_LAST : BRANCH;
                cell++;
                *(u8 *)cell = LINE;
                cell++;
                if (md.TreeTyp != 0) {
                    *(u8 *)cell = LINE;
                    cell++;
                }
            }
        }
        if (entry[1].DirShift != 0)
            *goes_on = 1;
        if (entry->DirLevel != 0)
            DESCEND(PATH, entry, text, i, n);
        if (entry < TreeFirst)
            continue;

        width = 12;
        if (entry == TRELIST)
            width = 1;
        IS_PATH(current, PATH, md.DosPth, i, n);
        /* `which` of the colours a directory has here, and the one after
           it is that of the cursor. */
        which = 0;
        if (current) {
            TreeCurrent = entry;
            screen[36] = (u16)(Color(colors[2]) << 8 | MARK_CURRENT);
            which = 2;
        }
        if (entry != TreeCursor) {
            color = (u16)(Color(colors[which]) << 8);
        } else {
            ends = BRACKETS;
            if (panel == CurrentPanel) {
                ends = ' ' << 8 | ' ';
                which++;
            }
            color = (u16)(Color(colors[which]) << 8);
            cell[-1] = color | ends >> 8;
            cell[width] = color | (ends & 0xFF);
            for (i = 0; i < LenPath + 13; i++)
                SHOWN[i] = PATH[i];
            /* Without the backslash at its end, unless it is the root:
               the end is looked for in what was copied, and if it is not
               there the last byte but one is taken for the backslash. */
            text = SHOWN;
            if (text[3] != 0) {
                for (i = LenPath + 13; i; i--)
                    if (*text++ == 0)
                        break;
                text[-2] = 0;
            }
            for (i = 0; i < LenPath; i++)
                md.CrnPath[i] = SHOWN[i];
            ShrtLin(SHOWN, 38);
            text = (u8 *)BasAdr(STATUS);
            n = 0;
            for (i = 38; i; i--) {
                if (SHOWN[n] != 0)
                    *text = SHOWN[n++];
                else
                    *text = ' ';
                text += 2;
            }
        }
        text = entry->DirName;
        for (i = width; i; i--) {
            if (*text != 0)
                *cell++ = color | *text++;
            else
                *cell++ = color | ' ';
        }
        screen += 80;
        if (--lines == 0)
            break;
    }
}

/* Put the cursor of the tree on the current directory, when the tree has
   been read: on where it was, CrnPath, if that is on the current drive
   and in the tree still, or else on the current directory, which CrnPath
   then is; and begin the lines shown half a panel above it. If the
   current directory is not in the tree either, both stay at the root. */
void TreeCur(void)
{
    struct DIRENT *entry;
    u8 *text;
    i32 count, i, n;
    bool found, again;

    TreeFirst = TRELIST;
    TreeCursor = TRELIST;
    for (again = false;; again = true) {
        FIND(found, entry, count, text, i, n);
        if (found)
            break;
        if (again)
            return;
        for (i = 0; i < LenPath; i++)
            md.CrnPath[i] = md.DosPth[i];
    }
    TrCr20((SizDX() >> 8) - 4);
}

/* Put the cursor of the tree on the current directory, which CrnPath is
   made. It stays where it is if that is not in the tree. */
void TrCr10(void)
{
    struct DIRENT *entry;
    u8 *text;
    i32 count, i, n;
    bool found;

    for (i = 0; i < LenPath; i++)
        md.CrnPath[i] = md.DosPth[i];
    FIND(found, entry, count, text, i, n);
}

/* Begin the lines of the tree half of `lines` above the cursor, the half
   of an even number, or with the root if there are not as many above it.
   Only the low byte of `lines` counts, and of that not the highest bit. */
void TrCr20(i32 lines)
{
    /* The original takes eight bytes for each of them off the address
       of the cursor's entry, which is half as many entries of 16 bytes,
       and takes the root where that address has wrapped round below 0
       or is in front of the list. It cannot wrap: the list is further
       into the main data than 1,008 bytes, the most that there are to
       take off. So the root is taken where the cursor has fewer entries
       above it than that. */
    i32 above = (lines & 0x7E) >> 1;

    if (TreeCursor < TRELIST + above)
        TreeFirst = TRELIST;
    else
        TreeFirst = TreeCursor - above;
}
