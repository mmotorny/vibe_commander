/*
 * Reading what a panel shows: the files of its directory, and for the
 * Info panel the sizes of the disk, its label and the text of dirinfo.
 *
 * Translated from VCSUB2.INC and VCSUB3.INC.
 *
 * Each panel has a block of the memory of its own, 64K, which is as long
 * as a list of files can get, and the room that the viewer and the
 * editor take is a third, at Room (see vc.h). In the original the panels
 * lie one behind the other, each as long as its list, and the room
 * begins behind them, at ViewSeg: before a directory is read the panel
 * behind the one that reads is moved up as far as it goes, so that the
 * list may grow, and afterwards down again to where the list ends.
 * Nothing is moved here. While the directory is read the list that the
 * panel had is kept in the room: what was selected in it and where the
 * cursor was is looked up in the new one by name.
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
 * on the stack.
 */
#include "vc.h"

/* What ReDr00 keeps of the list that a panel had, for ReDr50: ReDir has
   it on the stack between the two. The list itself is at Room. */
struct OldList {
    bool keep;                          /* the cursor and what is selected
                                           are to stay */
    i32 end;                            /* how long the list is, in bytes */
    u16 cursor;                         /* where in it the cursor was */
    u16 first;                          /* and the first entry shown */
};

static void ReDr00(struct WCBdef *panel, bool keep, struct OldList *old);
static void ReDr45(struct WCBdef *panel);
static void ReDr50(struct WCBdef *panel, struct OldList *old);
static void RdInfo(struct WCBdef *panel);
bool ReadTre(void);                                     /* treebox.c */
void DirSize(struct WCBdef *panel);
u16 DiSi10(u16 attributes);
void TstInfo(struct WCBdef *panel);

/* ReDir and GetVol are in vc.h. */

/* The names of files: Mask_S is \*.* and Ext_F is VC.EXT. They are
   copied with MvzLine, which takes what follows a text along, so they
   are constants of system_t.c, written there with what follows
   them in the original, and not texts of this file. */
extern const u8 Mask_S[];
extern const u8 Ext_F[];

static const u8 DrIn_F[] = "DIRINFO";

#define SIZE_FILENT ((i32)sizeof(struct FILENT))

/* Errors of DOS, and the one that a call makes of a drive that is not
   ready, where it is asked with the program's errors. */
#define ERR_NO_FILE   0x02
#define ERR_NO_PATH   0x03
#define ERR_NO_DRIVE  0x0F
#define ERR_NO_MORE   0x12
#define ERR_NOT_READY 0x53

/* Read the directory of a panel of files, and its extension file: ReDir
   of the original from ReDr00 on. What
   the panel had before is left for ReDr50 at Room and in `old`, with
   `keep`, which is made false if the directory was not there.

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
static void ReDr00(struct WCBdef *panel, bool keep, struct OldList *old)
{
    struct FILENT *entry;
    u8 *to;
    const u8 *from;
    i32 size, count, i;
    i32 number, retries, drive;
    u16 limit, file;
    u16 oldCursor, oldFirst;
    u16 attributes, found, answer;
    struct Answer opened, asked, got, closed;

    /* Expand, in the original: it moves the panel behind this one as
       far up as it goes, but no further than 64K from here, and the
       room with it, so that the list has room to grow. Here a panel has
       its 64K from the start, and nothing is moved. Where the list has
       to end is what the original has with memory enough: a paragraph
       short of 64K, which is as far as it counts in 16 bits. */
    limit = PANEL_BYTES - 16 - 2 * SIZE_FILENT;

    /* Keep the old list in the room, as much of it as there is room for
       in whole entries, and no more than a paragraph short of 64K, which
       is as far as the original counts in 16 bits: that is more than a
       panel's list, which is shorter than its block. */
    size = RoomBytes();
    if (size > 0xFFF0)
        size = 0xFFF0;
    size = size / SIZE_FILENT * SIZE_FILENT;
    count = panel->EndBuf - (i32)DSKBUF;
    oldCursor = panel->CurAdr;
    oldFirst = panel->First;
    if (count > size) {
        count = size;
        size += (i32)DSKBUF - SIZE_FILENT;
        if (oldCursor > size) {
            oldCursor = (u16)size;
            if (oldFirst > size)
                oldFirst = (u16)size;
        }
    }
    from = (const u8 *)panel + DSKBUF;
    to = Room;
    for (i = 0; i < count; i++)
        to[i] = from[i];
    old->end = count;                   /* places in the old list from */
    old->cursor = oldCursor - DSKBUF;   /* here on, not addresses */
    old->first = oldFirst - DSKBUF;

    /* Read the directory. */
    retries = 0;
    for (;;) {
        /* ReDr20 */
        to = MvzLine(md.TmpBuf, panel->WinPath, LenPath);
        to = AddFile(to);
        MvzLine(to, Mask_S + 1, LenPath);
        number = 0;
        file = DSKBUF;
        if (panel->WinPath[3] != 0) {
            entry = ENTRY(panel, file);
            entry->NameF[0] = '.';
            entry->NameF[1] = '.';
            for (i = 2; i < SIZE_FILENT; i++)
                ((u8 *)entry)[i] = 0;
            entry->AttrF = ATTR_DIRECTORY;
            entry->DateF = 0x182F;
            file += SIZE_FILENT;
            number = 1;
        }
        drive = (u8)(panel->WinPath[0] - ('A' - 1));
        if (Fantom(drive))
            goto unread;
        SetDTA();
        asked = FindFirst(md.TmpBuf, 0x37);
        if (!asked.failed)
            break;
        if (asked.value == ERR_NO_MORE)
            goto read;
        if (asked.value != ERR_NO_PATH)
            goto unread;
        /* ReDr22: the directory is not there. */
        keep = false;
        retries++;
        if (retries > 2)
            goto unread;
        if (retries == 2) {
            panel->WinPath[0] = md.DosPth[0];
            panel->WinPath[3] = 0;
        }
        for (;;) {
            /* ReDr24 */
            drive = (u8)(panel->WinPath[0] - ('A' - 1));
            if (Fantom(drive))
                goto unread;
            asked = DirCurrent((u16)drive, panel->WinPath + 3);
            if (!asked.failed)
                break;
            if (asked.value != ERR_NO_DRIVE)
                goto unread;
            panel->WinPath[0] = md.DosPth[0];
            panel->WinPath[3] = 0;
        }
    }
    for (;;) {
        /* ReDr27 */
        md.BlocEr = 0;
        attributes = md.DTA.FilAttr;
        if (md.DTA.FilName[0] == '.' && (attributes & ATTR_DIRECTORY)) {
            /* What the assembly reads as WinPath[3], with DS on the main
               data. */
            if (MAIN_AS_PANEL->WinPath[3] != 0) {
                md.DTA.FilAttr &= 0xF9;
                md.DTA.FilAttr &= 0x3F;
                to = &ENTRY(panel, DSKBUF)->AttrF;
                for (i = 0; i < 9; i++)
                    to[i] = (&md.DTA.FilAttr)[i];
            }
        } else {
            entry = ENTRY(panel, file);
            entry->NumbF = (u16)number; /* fewer than the list holds */
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
        asked = FindNext();
        if (asked.failed) {
            if (asked.value != ERR_NO_MORE)
                goto ended;
            break;
        }
        if (file >= limit) {
            Beep();
            break;
        }
    }
read:
    panel->ChgFlg = 0;
    goto ended;
unread:
    keep = false;
ended:
    /* ReDr35 */
    panel->EndFul = file;
    panel->EndBuf = file;
    panel->CurAdr = DSKBUF;
    panel->First = DSKBUF;
    md.BlocEr = 0;

    /* The extension file of the directory, or else the main one. */
    if (panel->ChgFlg == 0) {
        to = (u8 *)CutPath(md.TmpBuf);
        MvzLine(to, Ext_F, LenPath);
        opened = OpenFil(md.TmpBuf, 0x40);
        answer = opened.value;
        if (opened.failed == 0) {
            got = FileRead(opened.value, panel->ExtBuf, LenExt - 1);
            closed = FileClose(opened.value);
            if (!closed.failed && !got.failed) {
                panel->ExtBuf[got.value] = 0x1A;
                goto done;
            }
            /* A drive that was not ready has the panel read again: for
               the file opened, for the file closed, and, where closing
               did not fail, for the file read.

               The assembly looks in AL behind the close whichever of
               the two failed, and DOS answers nothing in AX when it has
               closed a file. A DOS that leaves AX as it finds it there,
               as the bench's FreeDOS does, has in AL what the assembly
               handed it: the low byte of what reading answered, its
               error. So on such a DOS the original marks the panel
               exactly when reading failed for a drive that was not
               ready, and that is what is said here. On a DOS that
               leaves something else in AX the original goes by that
               (see DEVIATIONS.md). */
            answer = got.value;
            if (closed.failed)
                answer = closed.value;
        }
        if ((answer & 0xFF) == ERR_NOT_READY)
            panel->ChgFlg = 1;
    }
    for (i = 0; i < LenExt; i++)
        panel->ExtBuf[i] = md.ExtMain[i];
done:
    old->keep = keep;
}

/* When the directory has been read: work out the sizes of the
   directories, if that is switched on and the directory was read, and
   filter and sort the list. */
static void ReDr45(struct WCBdef *panel)
{
    panel->SizDir = 0;
    if (panel->ChgFlg == 0 && md.AutoSiz != 0)
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
static void ReDr50(struct WCBdef *panel, struct OldList *old)
{
    const u8 *from;
    i32 i;
    u16 attributes, found;

    from = Room;
    for (i = 0; i < old->end; i += SIZE_FILENT) {
        attributes = ((const struct FILENT *)(from + i))->AttrF & 0xC0;
        if (attributes == 0)
            continue;
        if (!old->keep) {
            attributes &= 0x80;
            if (attributes == 0)
                continue;
        }
        found = FnFile(panel, from + i);
        if (found < panel->EndBuf)
            ENTRY(panel, found)->AttrF |= attributes;
    }
    if (old->keep) {
        found = NearFl(panel, from + old->cursor, old->end);
        if (found != 0)
            panel->CurAdr = found;
        found = NearFl(panel, from + old->first, old->end);
        if (found != 0)
            panel->First = found;
    }

    /* Compress, in the original: it moves the panel behind this one
       down to where the list ends, or to where a panel with the fewest
       files would, and has the room begin behind it. Nothing is moved
       here: see ReDr00. */
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

   The panel counts as not read where looking for the label failed for
   a drive that is not ready. The original goes by the carry flag and AX
   of GetVol, and that flag is set for a label that it has found too,
   with AX what DOS left in AH and the eleventh character of the label:
   a label of eleven characters that ends in S, on a DOS that leaves 0
   in AH there, is 53h, and the panel was taken as not read. Here it
   goes by whether the label could not be read (DEVIATIONS.md, 22). */
static void RdInfo(struct WCBdef *panel)
{
    struct Space space;
    i32 drive;
    u16 handle;
    struct Label label;
    struct Answer opened, read;

    if (panel->WinTyp != 2)
        return;
    ClusterBytes = 1;
    DiskClusters = 0;
    FreeClusters = 0;
    md.InfoLen = 0xFFFF;
    md.Volume[0] = 0;
    drive = (u8)(md.DosPth[0] - ('A' - 1));
    if (Fantom(drive))
        return;
    space = DriveSpace((u16)drive, 0);
    if (space.failed)
        return;
    if (space.value == 0xFFFF)
        return;
    DiskClusters = space.total;
    FreeClusters = space.free;
    /* In 32 bits: a cluster of 64K is its bytes, where the original
       keeps the product in 16 bits and has 0 (DEVIATIONS.md, 26). */
    ClusterBytes = (u32)space.value * space.sector;
    opened = OpenFil(DrIn_F, 0x40);
    handle = opened.value;
    if (opened.failed != 0) {
        if (handle != ERR_NO_FILE)
            return;
    } else {
        read = FileRead(handle, md.InfBuf, sizeof md.InfBuf);
        if (FileClose(handle).failed || read.failed)
            return;
        md.InfoLen = read.value;
    }
    label = GetVol(md.Volume);
    if (label.failed && label.answer == ERR_NOT_READY)
        return;
    panel->ChgFlg = 0;
}

/* Read what a panel shows: the files of its directory, the tree of the
   disk or what the Info says, by the kind of the panel. With `keep` the
   cursor of a list of files and what is selected in it stay where they
   are. Returns whether it could not be read: the original answers in
   the carry flag, which it makes of ChgFlg. */
bool ReDir(struct WCBdef *panel, bool keep)
{
    struct OldList old;

    panel->ChgFlg = 1;
    if (panel->WinTyp == 0) {
        ReDr00(panel, keep, &old);
        ReDr45(panel);
        ReDr50(panel, &old);
    } else if (panel->WinTyp == 1) {      /* ReDr01: the tree */
        if (!ReadTre())
            panel->ChgFlg = 0;
    } else                              /* ReDr02: the Info */
        RdInfo(panel);
    return panel->ChgFlg != 0;          /* ReDr70 */
}

/* What GetVol returns: whether the label could not be read and DOS's
   error then, and whether the drive is a local one. */
static struct Label Labelled(bool failed, u16 answer, bool local)
{
    struct Label label;

    label.failed = failed;
    label.answer = answer;
    label.local = local;
    return label;
}

/* Get the label of the current drive, as a text of at most 11 characters
   at `volume` in the main data: for a drive of a network the name it is
   shared by, which follows the name of the server in what DOS makes of a
   file name there, and otherwise the name of the entry of the root that
   is a label, without the full stop and the spaces of a file name. An
   empty text if there is none.

   Returns whether the drive is a local one, where the original has the
   carry flag, clear for a drive of a network and set for any other,
   with a label or without; and whether DOS could not say what the drive
   is or found no label, with its error. The original has that error in
   AX, and for a label that it has found it has what DOS left in AH with
   the last of the 11 characters there, a space unless the label is that
   long, which RdInfo took for an error (DEVIATIONS.md, 22): that is not
   kept. */
struct Label GetVol(u8 *volume)
{
    struct Answer asked;
    u8 *scan, *end;
    i32 i;
    u8 c;

    volume[0] = 0;
    *(u16 *)md.TmpErr = 'A';
    asked = FileTrueName(md.TmpErr, md.TmpErr + 100);
    if (asked.failed)
        return Labelled(true, asked.value, true);
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
        return Labelled(false, 0, false);
    }
    SetDTA();
    asked = FindFirst(Mask_S, 8);
    if (asked.failed) {
        *volume = 0;
        return Labelled(true, asked.value, true);
    }
    Convert(md.DTA.FilName, volume);
    end = volume;
    for (i = 0; i < 11; i++) {
        c = volume[i];
        if (c != ' ')
            end = volume + i + 1;
    }
    *end = 0;
    return Labelled(false, 0, true);
}

/* Work out the sizes of the directories in a panel's list, of all files
   in them and in the directories below, and put each into its entry.
   SizDir is made 1 when all are known, and left alone when the work was
   broken off, by a key or by an error.

   The files are gone through by DirFunc, which takes the path at TmpBuf
   and for every file calls what FncAddr names, here DiSi10. */
void DirSize(struct WCBdef *panel)
{
    struct FILENT *entry;
    u8 *to;
    u16 file;
    i32 i;

    md.FncAddr = FNC_DISI10;
    to = MvzLine(md.TmpBuf, panel->WinPath, LenPath);
    if (to[-1] != '\\')
        *(u16 *)to = '\\';
    for (file = DSKBUF; file < panel->EndFul; file += SIZE_FILENT) {
        entry = ENTRY(panel, file);
        if (!(entry->AttrF & ATTR_DIRECTORY) || entry->NameF[0] == '.')
            continue;
        *(u16 *)(md.TmpBuf + 90) = 0;
        *(u16 *)(md.TmpBuf + 92) = 0;
        to = (u8 *)CutPath(md.TmpBuf);
        for (i = 0; i < 13; i++)
            to[i] = entry->NameF[i];
        if (FncCall(ATTR_DIRECTORY) >> 8)
            return;
        entry->SizeF = *(u32 *)(md.TmpBuf + 90);
    }
    panel->SizDir = 1;
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
void TstInfo(struct WCBdef *panel)
{
    struct WCBdef *other = InvWCB(panel);

    if (panel->WinTyp < other->WinTyp)
        other->ChgFlg = 1;
}
