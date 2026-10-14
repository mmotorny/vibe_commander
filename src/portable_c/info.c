/*
 * The Info panel: the disk, the files of the other panel, and what the
 * directory says about itself in its file dirinfo.
 *
 * Translated from VCSUB3.INC. The original has two lines on the memory
 * in front of these, how much the machine has and how much of it a
 * command gets, which mean nothing off DOS and are gone from here
 * (DEVIATIONS.md, 11): PtIn20 of the assembly, with the texts PtIn1_S,
 * PtIn2_S and PtIn3_S. What the program is based on stands where the
 * line under the title was, that line a row further down, and the row
 * below it is empty: Panel of panel.c writes all three.
 *
 * What is shown has been read before, when the directory was: the sizes
 * of the disk in clusters, its label, and the first bytes of dirinfo, in
 * InfBuf. Only the files are counted here.
 */
#include "vc.h"

/* PutInf is in vc.h. */

static const u8 PtIn4_S[] = "Volume in drive ^r^W:^r ^I^r^V";
static const u8 PtInA_S[] = "is ";
static const u8 PtInB_S[] = "has no label";
static const u8 PtIn5_S[] = "^r^V^r ^I on drive ^r^W:";
static const u8 PtIn6_S[] = "total bytes";
static const u8 PtIn7_S[] = "bytes free";
static const u8 PtIn8_S[] = "^r^V^r files uses ^r^W^r bytes in";
static const u8 PtIn9_S[] = " No `dirinfo' file in this directory";
static const u8 PtIn_P[] = {Back_C, Title_C};

#define TAB 9
#define CR_LF 0x0A0D                    /* the two that end a line, as a */
#define LF_CR 0x0D0A                    /* word, and the other way round */

/* Write the Info into its panel: nothing, if the panel is of another
   kind. Each line has its middle in the middle of the panel.

   From the sixth row of the panel, which is where the original has
   them, below its two lines on the memory: the label of the current
   drive; the size of the disk and what is free on it. If the other panel
   shows files and is not switched off: how many files it has, without
   the directories, and what they take on the disk, every file counted in
   whole clusters; and its path with its mask, as it stands over that
   panel, in at most 37 characters.

   Four rows below the line of the files, if dirinfo could not be read,
   a line that says so. If it could, its text from one row higher and one
   column in: as many lines as the panel has rows less 12, of each the
   first 36 columns, with tabs to every eighth. A line ends with a
   carriage return and a line feed, in either order. The characters are
   written, not their colours.

   The text has no end of its own: the bytes read are gone through until
   one is looked at that lies behind them, and the last line is looked at
   two bytes at a time like the others, so its end is whatever follows
   the text in InfBuf.

   One byte is "1 byte" only where the files take a single byte, which a
   disk with clusters of more than a byte does not have. */
void PutInf(struct WCBdef *panel)
{
    struct LineArgs line;
    const struct FILENT *file;
    u8 *text, *cell;
    u16 *screen;
    u32 size, total;
    u16 place;
    u16 cluster;                        /* bytes of a cluster */
    u16 two;                            /* two characters of dirinfo */
    i32 count, lines, room, skip, i;
    struct WCBdef *other;

    if (panel->WinTyp != 2)
        return;
    place = BegBX(panel) + 0x514;
    line.colors = PtIn_P;

/* PtIn23 in the assembly: a line, and on to the next row. */
#define PUT(markup) \
    do { \
        line.value = place; \
        line.line = (markup); \
        CntLine(&line); \
        place += 0x100; \
    } while (0)

    md.TmpErr[101] = md.DosPth[0];
    md.TmpErr[102] = 0;
    for (i = 0; i < 12; i++)
        md.TmpErr[1 + i] = md.Volume[i];
    if (md.Volume[0] == 0)
        line.include = PtInB_S;
    else
        line.include = PtInA_S;
    PUT(PtIn4_S);

    /* PtIn21: a number of clusters as bytes. */
    size = (u32)md.Total * md.ClstSiz;
    Number(md.TmpErr + 1, (u16)(size >> 16), (u16)size, NUM_SEPARATED);
    line.include = PtIn6_S;
    PUT(PtIn5_S);
    size = (u32)md.FreeDsk * md.ClstSiz;
    Number(md.TmpErr + 1, (u16)(size >> 16), (u16)size, NUM_SEPARATED);
    line.include = PtIn7_S;
    PUT(PtIn5_S);

    other = InvWCB(panel);
    if (FilePan(other)) {
        total = 0;
        count = 0;
        cluster = md.ClstSiz;
        for (file = ENTRY(other, DSKBUF);
             (const u8 *)file - (const u8 *)other < other->EndBuf;
             file++) {
            if (file->AttrF & ATTR_DIRECTORY)
                continue;
            count++;
            size = file->SizeF;
            /* The size in whole clusters, in 32 bits: wrapping at 4 GB
               as the sum does. The original counts the clusters of what
               lies below a multiple of 65,536 clusters in 16 bits, and
               that count wraps round to 0 where it is 65,535 and one
               more is filled in part, so that such a file counts 65,536
               clusters less: a bug of the original, fixed on the user's
               word of 14 October 2026 (DEVIATIONS.md, 19). */
            if (cluster > 1 && size % cluster != 0)
                size += cluster - size % cluster;
            total += size;
        }
        /* The count fits: a panel holds 2,703 entries, ReDr00. */
        Number(md.TmpErr + 1, 0, (u16)count, NUM_SEPARATED);
        Number(md.TmpErr + 101, (u16)(total >> 16), (u16)total,
               NUM_SEPARATED);
        /* "files uses" without the one s or the other, and "bytes"
           without its own for a single byte. */
        text = md.TmpBuf;
        for (i = 0; i < 12; i++)
            *text++ = PtIn8_S[i];
        if (count == 1)
            text--;
        for (; i < 17; i++)
            *text++ = PtIn8_S[i];
        if (count != 1)
            text--;
        for (; i < 30; i++)
            *text++ = PtIn8_S[i];
        if (total == 1)
            text--;
        for (; i < 34; i++)
            *text++ = PtIn8_S[i];
        line.value = place;
        line.line = md.TmpBuf;
        CntLine(&line);
        PtPt10(other);
        ShrtLin(md.TmpErr + 2, 38 - 1);
        line.value = place + 0x100;
        line.line = Var_S;
        CntLine(&line);
    }
    place += 0x400;
    lines = (u8)((SizDX() >> 8) - 12);  /* above 0: a panel has more
                                           than 12 rows */
    if (md.InfoLen == 0xFFFF) {
        PUT(PtIn9_S);
        return;
    }
#undef PUT

    screen = BasAdr(place - 0x112);
    text = md.InfBuf;
    do {
        cell = (u8 *)screen;
        room = 36;
        for (;;) {
            two = *(u16 *)text;
            text += 2;
            if (two == LF_CR || two == CR_LF)
                break;
            text--;
            if (text > md.InfBuf + md.InfoLen)
                break;
            if (room == 0)
                continue;
            two &= 0xFF;
            if (two == TAB) {
                skip = 8 - (36 - room & 7);
                cell += skip + skip;
                room = room < skip ? 0 : room - skip;
                continue;
            }
            *cell = (u8)two;
            cell += 2;
            room--;
        }
        if (text > md.InfBuf + md.InfoLen)
            break;
        screen += 80;
    } while (--lines);
}
