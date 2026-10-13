/*
 * Reading what a panel shows: the files of its directory, and for the
 * Info panel the sizes of the disk, its label and the text of dirinfo.
 *
 * Translated from VCSUB2.INC and VCSUB3.INC.
 *
 * The panels lie in the memory one behind the other, each as long as its
 * list of files, and behind them begins the room that the viewer and the
 * editor take, at ViewSeg. Before a directory is read the panel behind
 * the one that reads is moved up as far as it goes, so that the list may
 * grow, and afterwards down again to where the list ends. While the
 * directory is read the list that the panel had is kept behind the
 * panels: what was selected in it and where the cursor was is looked up
 * in the new one by name.
 *
 * ReDir chooses by which of the three kinds a panel is of: RdInfo for
 * the Info, for the tree ReadTre of treebox.c, and for files ReDr00,
 * ReDr45 and ReDr50 here, one after the other. It answers by ChgFlg: 0
 * there says that what the panel shows has been read.
 *
 * Reading the files is three functions for the sake of the stack, from
 * when that was 256 bytes. Filtering the list is the deepest that most
 * of what the program does ever gets, by way of the capital letters of
 * a mask, which DOS makes on the stack of whoever asks, in 62 bytes: so
 * that is called with nothing of what reading the directory needed left
 * on the stack. For the same reason GetVol shares the registers of
 * RdInfo.
 */
#include "vc.h"

/* What ReDr00 keeps of the list that a panel had, for ReDr50: ReDir has
   it on the stack between the two. The list itself is at ViewSeg. */
struct OldList {
    u16 keep;                           /* not 0: the cursor and what is
                                           selected are to stay */
    u16 end;                            /* how long the list is, in bytes */
    u16 cursor;                         /* where in it the cursor was */
    u16 first;                          /* and the first entry shown */
};

static void ReDr00(u16 panel, u16 keep, struct OldList STACK *old);
static void ReDr45(u16 panel);
static void ReDr50(u16 panel, struct OldList STACK *old);
static void RdInfo(u16 panel);
u16 ReadTre(void);                                      /* treebox.c */
u32 GetVol(u8 *volume);
void DirSize(u16 panel);
u16 DiSi10(u16 attributes);
void TstInfo(u16 panel);
static u32 GetVol0(u8 *volume, struct DosRegs STACK *regs);

/* ReDir is in vc.h. */

/* The names of files: Mask_S is \*.* and Ext_F is VC.EXT. They are
   copied with MvzLine, which takes what follows a text along, so they
   are constants of system_t.c, written there with what follows
   them in the original, and not texts of this file. */
extern const u8 __far Mask_S[];
extern const u8 __far Ext_F[];

static const u8 CODE DrIn_F[] = "DIRINFO";

#define SIZE_FILENT sizeof(struct FILENT)
#define MinView 8000                    /* Minimum view buffer size */

/* Errors of DOS, and the one that Intr21 makes of a drive that is not
   ready. */
#define ERR_NO_FILE   0x02
#define ERR_NO_PATH   0x03
#define ERR_NO_DRIVE  0x0F
#define ERR_NO_MORE   0x12
#define ERR_NOT_READY 0x53

/* The segment that a pointer into the main data is in. */
#define SEGMENT(pointer) ((u16)((u32)(const void __far *)(pointer) >> 16))

/* Read the directory of a panel of files, and its extension file: ReDir
   of the original from ReDr00 on. What
   the panel had before is left for ReDr50 at ViewSeg and in `old`, with
   `keep`, of which the low byte counts; that is made 0 if the directory
   was not there.

   The list is the entry "..", unless the path is that of a root, and
   then the entries as DOS has them, but for "." and "..", with the names
   of files in small letters and numbered in that order. A directory that
   is not there is tried again as the current directory of the drive, and
   then as the root of the current drive. When the list is full, the
   program beeps and shows what it has.

   Then the extension file of the directory is read, VC.EXT, or the main
   one taken where it has none.

   ChgFlg is left 0 if the directory was read to its end, or as far as
   there was room, and 1 otherwise. The list is as long as what was read
   in either case.

   Ported as it is: of "." and ".." the attributes, time and date go
   into the first entry of the list, which is ".." where the path is not
   a root. Whether they do is decided by the byte of the main data that
   lies where the fourth character of the path lies in a panel, which is
   the 24th colour of the first palette: the assembly has DS on the main
   data there. */
static void ReDr00(u16 panel, u16 keep, struct OldList STACK *old)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct DosRegs regs;
    struct FILENT __far *entry;
    u8 __far *to;
    const u8 __far *from;
    u16 top, other, size, count, i;
    u16 limit, file, number, retries;
    u16 oldCursor, oldFirst;
    u16 drive, attributes, found, answer;
    u32 opened;

    /* Expand: the panel behind this one as far up as it goes, but no
       further than 64K from here. */
    top = SEGMENT(&md) + md.FreMem2 - (((MinView - 1) >> 4) + 1);
    limit = panel + 0x1000;
    other = md.WCB1seg;
    if (panel >= other)
        other = md.WCB2seg;
    if (panel < other) {
        size = md.ViewSeg - other;
        top -= size;
        if (top > limit)
            top = limit;
        md.ViewSeg = size + top;
        count = size << 4;
        from = (const u8 __far *)PANEL(other);
        to = (u8 __far *)PANEL(top);
        for (i = count; i != 0; ) {     /* from its end: they overlap */
            i--;
            to[i] = from[i];
        }
        if (other == md.WCB1seg)
            md.WCB1seg = top;
        if (other == md.WCB2seg)
            md.WCB2seg = top;
        if (other == md.WCBcrn)
            md.WCBcrn = top;
        if (other == md.WCBsec)
            md.WCBsec = top;
    } else {
        if (top > limit)
            top = limit;
        md.ViewSeg = top;
    }
    limit = top - panel;                /* where the list has to end */
    if (limit > 0xFFF)
        limit = 0xFFF;
    limit = (limit << 4) - 2 * SIZE_FILENT;

    /* Keep the old list behind the panels, as much of it as there is room
       for in whole entries. */
    size = md.FreMem2 - (md.ViewSeg - SEGMENT(&md));
    if (size > 0xFFF)
        size = 0xFFF;
    size <<= 4;
    size = size / SIZE_FILENT * SIZE_FILENT;
    count = wcb->EndBuf - DSKBUF;
    oldCursor = wcb->CurAdr;
    oldFirst = wcb->First;
    if (count > size) {
        count = size;
        size += DSKBUF - SIZE_FILENT;
        if (oldCursor > size) {
            oldCursor = size;
            if (oldFirst > size)
                oldFirst = size;
        }
    }
    from = (const u8 __far *)wcb + DSKBUF;
    to = (u8 __far *)PANEL(md.ViewSeg);
    for (i = 0; i < count; i++)
        to[i] = from[i];
    old->end = count;                   /* places in the old list from */
    old->cursor = oldCursor - DSKBUF;   /* here on, not addresses */
    old->first = oldFirst - DSKBUF;
    keep &= 0xFF;

    /* Read the directory. */
    retries = 0;
    for (;;) {
        /* ReDr20 */
        to = MvzLine(md.TmpBuf, wcb->WinPath, LenPath);
        to = AddFile(to);
        MvzLine(to, Mask_S + 1, LenPath);
        number = 0;
        file = DSKBUF;
        if (wcb->WinPath[3] != 0) {
            entry = ENTRY(wcb, file);
            entry->NameF[0] = '.';
            entry->NameF[1] = '.';
            for (i = 2; i < SIZE_FILENT; i++)
                ((u8 __far *)entry)[i] = 0;
            entry->AttrF = ATTR_DIRECTORY;
            entry->DateF = 0x182F;
            file += SIZE_FILENT;
            number = 1;
        }
        drive = (u8)(wcb->WinPath[0] - ('A' - 1));
        if (Fantom(drive))
            goto unread;
        SetDTA();
        regs.ax = 0x4E00 | drive;
        regs.cx = 0x37;
        regs.name = md.TmpBuf;
        if (!Intr21((struct DosRegs STACK *)&regs))
            break;
        if (regs.ax == ERR_NO_MORE)
            goto read;
        if (regs.ax != ERR_NO_PATH)
            goto unread;
        /* ReDr22: the directory is not there. */
        keep = 0;
        retries++;
        if (retries > 2)
            goto unread;
        if (retries == 2) {
            wcb->WinPath[0] = md.DosPth[0];
            wcb->WinPath[3] = 0;
        }
        for (;;) {
            /* ReDr24 */
            drive = (u8)(wcb->WinPath[0] - ('A' - 1));
            if (Fantom(drive))
                goto unread;
            regs.ax = 0x4700 | drive;
            regs.name = (const u8 __far *)wcb + drive;  /* DL, and DS */
            regs.si = (u16)(u32)(wcb->WinPath + 3);
            if (!Intr21((struct DosRegs STACK *)&regs))
                break;
            if (regs.ax != ERR_NO_DRIVE)
                goto unread;
            wcb->WinPath[0] = md.DosPth[0];
            wcb->WinPath[3] = 0;
        }
    }
    for (;;) {
        /* ReDr27 */
        md.BlocEr = 0;
        attributes = md.DTA.FilAttr;
        if (md.DTA.FilName[0] == '.' && (attributes & ATTR_DIRECTORY)) {
            /* What the assembly reads as WinPath[3], with DS on the main
               data. */
            if (md.ColTab[(u16)(u32)(wcb->WinPath + 3)] != 0) {
                md.DTA.FilAttr &= 0xF9;
                md.DTA.FilAttr &= 0x3F;
                to = &ENTRY(wcb, DSKBUF)->AttrF;
                for (i = 0; i < 9; i++)
                    to[i] = (&md.DTA.FilAttr)[i];
            }
        } else {
            entry = ENTRY(wcb, file);
            entry->NumbF = number;
            number++;
            for (i = 0; i < 13; i++) {
                found = md.DTA.FilName[i];
                if (!(attributes & ATTR_DIRECTORY))
                    found = LowCase(found);
                entry->NameF[i] = (u8)found;
            }
            md.DTA.FilAttr &= 0x3F;
            to = &entry->AttrF;         /* attributes, time, date, size */
            for (i = 0; i < 9; i++)
                to[i] = (&md.DTA.FilAttr)[i];
            file += SIZE_FILENT;
        }
        regs.ax = 0x4F00;
        if (Intr21((struct DosRegs STACK *)&regs)) {
            if (regs.ax != ERR_NO_MORE)
                goto ended;
            break;
        }
        if (file >= limit) {
            Beep();
            break;
        }
    }
read:
    wcb->ChgFlg = 0;
    goto ended;
unread:
    keep = 0;
ended:
    /* ReDr35 */
    wcb->EndFul = file;
    wcb->EndBuf = file;
    wcb->CurAdr = DSKBUF;
    wcb->First = DSKBUF;
    md.BlocEr = 0;

    /* The extension file of the directory, or else the main one. */
    if (wcb->ChgFlg == 0) {
        to = (u8 __far *)CutPath(md.TmpBuf);
        MvzLine(to, Ext_F, LenPath);
        opened = OpenFil(md.TmpBuf, 0x40);
        answer = (u16)opened;
        if ((u16)(opened >> 16) == 0) {
            regs.bx = answer;
            regs.name = wcb->ExtBuf;
            regs.cx = LenExt - 1;
            regs.ax = 0x3F00 | (answer & 0xFF);
            found = Intr21((struct DosRegs STACK *)&regs);
            count = regs.ax;
            regs.ax = 0x3E00 | (count & 0xFF);
            if (!Intr21((struct DosRegs STACK *)&regs) && !found) {
                wcb->ExtBuf[count] = 0x1A;
                goto done;
            }
            answer = regs.ax;
        }
        if ((answer & 0xFF) == ERR_NOT_READY)
            wcb->ChgFlg = 1;
    }
    for (i = 0; i < LenExt; i++)
        wcb->ExtBuf[i] = md.ExtMain[i];
done:
    old->keep = keep;
}

/* When the directory has been read: work out the sizes of the
   directories, if that is switched on and the directory was read, and
   filter and sort the list. */
static void ReDr45(u16 panel)
{
    PANEL(panel)->SizDir = 0;
    if (PANEL(panel)->ChgFlg == 0 && md.AutoSiz != 0)
        DirSize(panel);
    SetMask(panel);
    Sort(panel);
}

/* Give the new list of a panel what the old one had, and let go of the
   memory that the list does not need.

   With `keep` the cursor and the first line stay on the files they were
   on, or on the nearest that are left, and what was selected stays
   selected. Without, only what Ctrl-M would select again is taken over,
   and by name alone: a file of that name in another directory gets it. */
static void ReDr50(u16 panel, struct OldList STACK *old)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u8 __far *to;
    const u8 __far *from;
    u16 top, other, size, count, i;
    u16 attributes, found;

    from = (const u8 __far *)PANEL(md.ViewSeg);
    for (i = 0; i < old->end; i += SIZE_FILENT) {
        attributes = ((const struct FILENT __far *)(from + i))->AttrF & 0xC0;
        if (attributes == 0)
            continue;
        if (old->keep == 0) {
            attributes &= 0x80;
            if (attributes == 0)
                continue;
        }
        found = FnFile(panel, from + i);
        if (found < wcb->EndBuf)
            ENTRY(wcb, found)->AttrF |= attributes;
    }
    if (old->keep != 0) {
        found = NearFl(panel, from + old->cursor, old->end);
        if (found != 0)
            wcb->CurAdr = found;
        found = NearFl(panel, from + old->first, old->end);
        if (found != 0)
            wcb->First = found;
    }

    /* Compress: the panel behind this one down to where the list ends, or
       to where a panel with the fewest files would. */
    top = wcb->EndFul + SIZE_FILENT - 1;
    if (top < sizeof(struct WCBdef) - 1)
        top = sizeof(struct WCBdef) - 1;
    top = (top >> 4) + 1 + panel;
    other = md.WCB1seg;
    if (panel >= other)
        other = md.WCB2seg;
    if (panel < other) {
        size = md.ViewSeg - other;
        count = size << 4;
        from = (const u8 __far *)PANEL(other);
        to = (u8 __far *)PANEL(top);
        for (i = 0; i < count; i++)
            to[i] = from[i];
        if (other == md.WCB1seg)
            md.WCB1seg = top;
        if (other == md.WCB2seg)
            md.WCB2seg = top;
        if (other == md.WCBcrn)
            md.WCBcrn = top;
        if (other == md.WCBsec)
            md.WCBsec = top;
        top += size;
    }
    md.ViewSeg = top;
    CorPar(panel);
}

/* Read what the Info panel shows of the current drive: the size of a
   cluster, how many the disk has and how many are free, the first bytes
   of the file dirinfo of the current directory, and the label. Nothing,
   if the panel is of another kind.

   ChgFlg of the panel, which is 1 when this is called, is made 0 if all
   of it could be read. The sizes stay 0 and the cluster 1 if the drive
   gives none; the label stays empty unless the sizes and dirinfo were
   read, or there is no such file. InfoLen is the length of what was read
   of it, or 0FFFFh for none.

   Ported as it is: a label is looked for and found with the carry flag
   set, and what is in AX then counts as the error of a drive that is not
   ready if it is 53h: what DOS left in AH, and the eleventh character of
   the label. */
static void RdInfo(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct DosRegs regs;
    u16 drive, failed, handle, length;
    u32 volume;

    if (wcb->WinTyp != 2)
        return;
    md.ClstSiz = 1;
    md.Total = 0;
    md.FreeDsk = 0;
    md.InfoLen = 0xFFFF;
    md.Volume[0] = 0;
    drive = (u8)(md.DosPth[0] - ('A' - 1));
    if (Fantom(drive))
        return;
    regs.ax = 0x3600 | drive;
    regs.name = (const u8 __far *)(u32)drive;   /* DL */
    if (Intr21((struct DosRegs STACK *)&regs))
        return;
    if (regs.ax == 0xFFFF)
        return;
    md.Total = (u16)(u32)regs.name;             /* DX */
    md.FreeDsk = regs.bx;
    md.ClstSiz = regs.ax * regs.cx;
    volume = OpenFil(DrIn_F, 0x40);
    handle = (u16)volume;
    if ((u16)(volume >> 16) != 0) {
        if (handle != ERR_NO_FILE)
            return;
    } else {
        regs.bx = handle;
        regs.name = md.InfBuf;
        regs.cx = sizeof md.InfBuf;
        regs.ax = 0x3F00 | (handle & 0xFF);
        failed = Intr21((struct DosRegs STACK *)&regs);
        length = regs.ax;
        regs.ax = 0x3E00 | (length & 0xFF);
        if (Intr21((struct DosRegs STACK *)&regs) || failed)
            return;
        md.InfoLen = length;
    }
    volume = GetVol0(md.Volume, (struct DosRegs STACK *)&regs);
    if ((u16)(volume >> 16) != 0 && (u16)volume == ERR_NOT_READY)
        return;
    wcb->ChgFlg = 0;
}

/* Read what a panel shows: the files of its directory, the tree of the
   disk or what the Info says, by the kind of the panel. With `keep` the
   cursor of a list of files and what is selected in it stay where they
   are. Returns 0 if it has been read, and else something that is not 0:
   the original answers in the carry flag, which it makes of ChgFlg. */
u16 ReDir(u16 panel, u16 keep)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct OldList old;

    wcb->ChgFlg = 1;
    if (wcb->WinTyp == 0) {
        ReDr00(panel, keep, (struct OldList STACK *)&old);
        ReDr45(panel);
        ReDr50(panel, (struct OldList STACK *)&old);
    } else if (wcb->WinTyp == 1) {      /* ReDr01: the tree */
        if (!ReadTre())
            wcb->ChgFlg = 0;
    } else                              /* ReDr02: the Info */
        RdInfo(panel);
    return wcb->ChgFlg;                 /* ReDr70 */
}

/* Get the label of the current drive, as a text of at most 11 characters
   at `volume` in the main data: for a drive of a network the name it is
   shared by, which follows the name of the server in what DOS makes of a
   file name there, and otherwise the name of the entry of the root that
   is a label, without the full stop and the spaces of a file name. An
   empty text if there is none.

   Returns in the high word whether the original has the carry flag set,
   and in the low one AX. The flag is clear for a drive of a
   network, and set for any other, with a label or without. AX is the
   error of DOS where it could not say what the drive is or found no
   label; for a label it is what DOS left in AH with the last of the 11
   characters, a space unless the label is that long. */
u32 GetVol(u8 *volume)
{
    struct DosRegs regs;

    return GetVol0(volume, (struct DosRegs STACK *)&regs);
}

/* The same, with room for the registers of DOS that the caller has. */
static u32 GetVol0(u8 *volume, struct DosRegs STACK *regs)
{
    u8 *scan, *end;
    u16 i;
    u8 c;

    volume[0] = 0;
    *(u16 *)md.TmpErr = 'A';
    regs->ax = 0x6000;
    regs->name = md.TmpErr;
    regs->si = (u16)md.TmpErr;
    regs->di = (u16)(md.TmpErr + 100);
    if (Intr21(regs))
        return 0x10000UL | regs->ax;
    scan = md.TmpErr + 100;
    if (*(u16 *)scan == 0x5C5C) {               /* \\server\name\... */
        scan += 2;
        for (i = 100; i != 0; i--)
            if (*scan++ == '\\')
                break;
        for (i = 11; i != 0; i--) {
            c = *scan++;
            if (c == '\\')
                break;
            *volume++ = c;
        }
        *volume = 0;
        return 0;
    }
    SetDTA();
    regs->ax = 0x4E5C;
    regs->cx = 8;
    regs->name = Mask_S;
    if (Intr21(regs)) {
        *volume = 0;
        return 0x10000UL | regs->ax;
    }
    Convert(md.DTA.FilName, volume);
    end = volume;
    for (i = 0; i < 11; i++) {
        c = volume[i];
        if (c != ' ')
            end = volume + i + 1;
    }
    *end = 0;
    return 0x10000UL | (regs->ax & 0xFF00) | c;
}

/* Work out the sizes of the directories in a panel's list, of all files
   in them and in the directories below, and put each into its entry.
   SizDir is made 1 when all are known, and left alone when the work was
   broken off, by a key or by an error.

   The files are gone through by DirFunc, which takes the path at TmpBuf
   and for every file calls what FncAddr names, here DiSi10. */
void DirSize(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct FILENT __far *entry;
    u8 __far *to;
    u16 file, i;

    md.FncAddr = FNC_DISI10;
    to = MvzLine(md.TmpBuf, wcb->WinPath, LenPath);
    if (to[-1] != '\\')
        *(u16 __far *)to = '\\';
    for (file = DSKBUF; file < wcb->EndFul; file += SIZE_FILENT) {
        entry = ENTRY(wcb, file);
        if (!(entry->AttrF & ATTR_DIRECTORY) || entry->NameF[0] == '.')
            continue;
        *(u16 *)(md.TmpBuf + 90) = 0;
        *(u16 *)(md.TmpBuf + 92) = 0;
        to = (u8 __far *)CutPath(md.TmpBuf);
        for (i = 0; i < 13; i++)
            to[i] = entry->NameF[i];
        if (FncCall(ATTR_DIRECTORY) >> 8)
            return;
        entry->SizeF = *(u32 *)(md.TmpBuf + 90);
    }
    wcb->SizDir = 1;
}

/* What DirSize has done with every file that DirFunc finds, whose
   attributes are the low byte of `attributes`: the size of a file is
   added to the sum at TmpBuf+90, which stays at 0FFFFFFFFh once it has
   got beyond that, and a directory is gone through in its turn, by
   DirFunc, whose answer is returned then: 1 if all of it was done, 0 if
   not, 100h to stop. For a file it is 0. */
u16 DiSi10(u16 attributes)
{
    u32 *sum = (u32 *)(md.TmpBuf + 90);
    u32 size;

    if (attributes & ATTR_DIRECTORY)
        return DirFunc();
    size = md.DTA.FilSize;
    *sum += size;
    if (*sum < size)
        *sum = 0xFFFFFFFFUL;
    return 0;
}

/* Have the other panel read again if it is of a kind that shows what
   this one has: the tree next to files, or the Info next to either. */
void TstInfo(u16 panel)
{
    u16 other = InvWCB(panel);

    if (PANEL(panel)->WinTyp < PANEL(other)->WinTyp)
        PANEL(other)->ChgFlg = 1;
}
