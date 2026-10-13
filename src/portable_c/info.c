/*
 * The Info panel: the memory, the disk, the files of the other panel, and
 * what the directory says about itself in its file dirinfo.
 *
 * Translated from VCSUB3.INC.
 *
 * What is shown has been read before, when the directory was: the sizes
 * of the disk in clusters, its label, and the first bytes of dirinfo, in
 * InfBuf. Only the files are counted here.
 */
#include "vc.h"

/* PutInf is in vc.h. */

static const u8 CODE PtIn1_S[] = "^r^V^r Bytes ^I";
static const u8 CODE PtIn2_S[] = "Memory";
static const u8 CODE PtIn3_S[] = "Free";
static const u8 CODE PtIn4_S[] = "Volume in drive ^r^W:^r ^I^r^V";
static const u8 CODE PtInA_S[] = "is ";
static const u8 CODE PtInB_S[] = "has no label";
static const u8 CODE PtIn5_S[] = "^r^V^r ^I on drive ^r^W:";
static const u8 CODE PtIn6_S[] = "total bytes";
static const u8 CODE PtIn7_S[] = "bytes free";
static const u8 CODE PtIn8_S[] = "^r^V^r files uses ^r^W^r bytes in";
static const u8 CODE PtIn9_S[] = " No `dirinfo' file in this directory";
static const u8 CODE PtIn_P[] = {Back_C, Title_C};

#define TAB 9
#define CR_LF 0x0A0D                    /* the two that end a line, as a */
#define LF_CR 0x0D0A                    /* word, and the other way round */

/* Write the Info into its panel: nothing, if the panel is of another
   kind. Each line has its middle in the middle of the panel.

   From the fourth row of the panel: the memory there is and how much of
   it is free with the program loaded; the label of the current drive; the
   size of the disk and what is free on it. If the other panel shows files and is not switched
   off: how many files it has, without the directories, and what they
   take on the disk, every file counted in whole clusters; and its path
   with its mask, as it stands over that panel, in at most 37 characters.

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
void PutInf(u16 panel)
{
    struct LineArgs line;
    const struct FILENT __far *file;
    u8 *text, *cell;
    u16 *screen;
    u32 size, total;
    u16 place, other, count, cluster, high, lines, room, i;

    if (PANEL(panel)->WinTyp != 2)
        return;
    place = BegBX(panel) + 0x314;
    line.colors = PtIn_P;

/* PtIn23 in the assembly: a line, and on to the next row. */
#define PUT(markup) \
    do { \
        line.value = place; \
        line.line = (markup); \
        CntLine((struct LineArgs STACK *)&line); \
        place += 0x100; \
    } while (0)

    /* PtIn20: a number of paragraphs as bytes. */
    i = md.Memorys;
    Number(md.TmpErr + 1, i >> 12, i << 4, NUM_SEPARATED);
    line.include = PtIn2_S;
    PUT(PtIn1_S);
    /* The memory that a command has: all that the program has for its
       panels and the viewer but the panels, and the paragraph of the
       header that DOS puts where a block begins. The original gives
       its main data back for a command too, and shows the block that
       held all three; and another number, FreMem1, where it is set up
       to give most of itself up. */
    i = MAIN_SEG + md.FreMem2 - md.MemSeg - 2 * PANEL_PARAS - 1;
    Number(md.TmpErr + 1, i >> 12, i << 4, NUM_SEPARATED);
    line.include = PtIn3_S;
    PUT(PtIn1_S);

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
        for (file = ENTRY(PANEL(other), DSKBUF);
             OFFSET_IN_SEG(other, file) < PANEL(other)->EndBuf; file++) {
            if (file->AttrF & ATTR_DIRECTORY)
                continue;
            count++;
            size = file->SizeF;
            if (cluster > 1) {
                /* The clusters that the high half of the size fills, and
                   then those of the rest, of which one may be filled in
                   part. They are counted in 16 bits. */
                high = (u16)(size >> 16);
                total += (u32)(u16)(high - high % cluster) << 16;
                size = (u32)(high % cluster) << 16 | (u16)size;
                i = (u16)(size / cluster);
                if ((u16)((u16)size - i * cluster) != 0)
                    i++;
                size = (u32)i * cluster;
            }
            total += size;
        }
        Number(md.TmpErr + 1, 0, count, NUM_SEPARATED);
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
        CntLine((struct LineArgs STACK *)&line);
        PtPt10(other);
        ShrtLin(md.TmpErr + 2, 38 - 1);
        line.value = place + 0x100;
        line.line = Var_S;
        CntLine((struct LineArgs STACK *)&line);
    }
    place += 0x400;
    lines = (u8)((SizDX() >> 8) - 12);
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
            i = *(u16 *)text;
            text += 2;
            if (i == LF_CR || i == CR_LF)
                break;
            text--;
            if ((u16)text > (u16)md.InfBuf + md.InfoLen)
                break;
            if (room == 0)
                continue;
            i &= 0xFF;
            if (i == TAB) {
                i = 8 - (36 - room & 7);
                cell += i + i;
                room = room < i ? 0 : room - i;
                continue;
            }
            *cell = (u8)i;
            cell += 2;
            room--;
        }
        if ((u16)text > (u16)md.InfBuf + md.InfoLen)
            break;
        screen += 80;
    } while (--lines);
}
