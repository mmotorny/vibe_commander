/*
 * Copying, renaming and moving: what is done with one file or one
 * directory, once the user has said where it is to go.
 *
 * Translated from VCCOPY.INC.
 *
 * The commands, at the end of this file, ask where to: they put the name
 * of the target into ToFile, as it was typed, and say in CopyMov what is
 * wanted. They then have C00_00 called for every file that is selected or
 * that fits a mask, by FncSel and FncMas of dirfunc.c, where the main
 * loop goes on by what they return (F35_90 of main.c), with its name in
 * CpSrc; for a directory C00_00 has DirFunc call it again
 * for everything in the directory, with the name behind that of the
 * directory. What it keeps from one file to the next, and from one call
 * of DOS to the next, is in TmpBuf.
 *
 * C00_00 first makes the name of the target, CpDest, then renames, copies
 * or moves. A file is read into the memory behind the program, from
 * ViewSeg on, as much of it as there is room for, and written from there.
 */
#include "vc.h"

/* What is kept in TmpBuf: the equates at the top of VCCOPY.INC. */
#define CpInpt  (md.TmpBuf + 0)         /* Input buffer */
#define CpVar1  (md.TmpBuf + 120)       /* Variable 1 */
#define CpVar2  (md.TmpBuf + 140)       /* Variable 2 */
#define CpSrc   (md.TmpBuf + 0)         /* Source filename */
#define CpDest  (md.TmpBuf + 130)       /* Destination filename */
#define CpDev1  md.TmpBuf[86]           /* Source is a device */
#define CpCase  md.TmpBuf[87]           /* Case: pathname, filename or none */
#define CpTree1 md.TmpBuf[88]           /* Source tree was changed */
#define CpTree2 md.TmpBuf[89]           /* Destination tree was changed */
#define CpAll   md.TmpBuf[90]           /* Copy / rename all */
#define FncChg  md.TmpBuf[91]           /* There was writing */
#define SplitFl md.TmpBuf[94]           /* Split flag */
#define CpCntSz md.TmpBuf[95]           /* Length of the per cent counter */
#define CpSize  (*(u32 *)(md.TmpBuf + 96))      /* File size */
#define CopyMov md.TmpBuf[100]          /* Copy / Move / Rename */
#define CpBgNam (*(u16 *)(md.TmpBuf + 102))     /* Start address of source filename */
#define CpDev2  md.TmpBuf[104]          /* Destination is a device */
#define CpRead  (*(u32 *)(md.TmpBuf + 106))     /* Was read */
#define CpWrite (*(u32 *)(md.TmpBuf + 110))     /* Was written */
#define CpHndl1 (*(u16 *)(md.TmpBuf + 116))     /* Source handle */
#define CpHndl2 (*(u16 *)(md.TmpBuf + 118))     /* Destination handle */
#define CpAttr  (*(u16 *)(md.TmpBuf + 120))     /* File attribute */
#define CpDate  (*(u16 *)(md.TmpBuf + 122))     /* File date */
#define CpTime  (*(u16 *)(md.TmpBuf + 124))     /* File time */
#define CpCntBg (*(u16 *)(md.TmpBuf + 126))     /* Base of the per cent counter */
#define CpCont  (*(u16 *)(md.TmpBuf + 128))     /* Start segment to continue copying */

#define LenName (LenPath + 13)          /* a path and a file name */

u16 C00_00(u16 attributes);
u16 F35_61(u16 answer, u16 place);

/* C00_00 is what FncAddr names, and F35_61 what the dialogs of the
   commands name for F10. The commands, F35_01 and F45_01, are declared in
   vc.h. */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   OncZoom puts a box on the screen with its middle at `place`, growing
   only if it is the first of a command: `box` is where the assembly
   describes it and `include` the text that ^I stands for in its lines,
   both offsets in the assembly's code segment; where the box is comes back
   as a struct Box of vc.h. ResWin puts back what a box covers. ChgDisk
   asks for the disk of a drive that shares the floppy drive with another,
   and is not 0 to stop.

   What the system is asked, it is asked by the functions of sys.h: for
   files, directories and the room of a drive with the program's errors,
   where the assembly calls DOS through Intr21, and bare for the current
   drive, where it calls DOS itself. */
/* AsmSeg returns the segment of the program's constants, where its
   texts are (see dialog.c). */

/* A dialog whose box is not on the screen yet, and which sets what it
   sets in the main data: Dialog of vc.h. The low byte of what it returns
   is the number of the answer, 0 for the first, and 0FFh for Esc. */
#define Ask(box, include) Dialog(box, include, 0, 0, MAIN_SEG)

/* C routines of other files that vc.h does not have. FilDir writes
   "2 files", or what else the numbers of TotSel make, and returns the
   address behind its zero byte; ChTree has a directory chosen from the
   tree, in a box that begins in a column, and returns 0 if one was, with
   its path in TmpBuf. */
const u8 *FndLet(const u8 *text);                       /* string.c */
void NormStr(u8 *text);
u8 *UnConv(u8 *name, const u8 *buffer);                         /* mask.c */
u8 *FilDir(u8 *buffer, u16 directories, u16 files);     /* panel.c */
u16 ChTree(u16 column);                                 /* treebox.c */

/* Boxes and texts, which OncZoom and Dialog take by their offsets in the
   segment that the constants are in: those of copy_t.c, and some of
   deleting. Only the offsets are used here, but for the texts that the
   commands copy from there into what their dialogs show. */
extern const u8 Copy0_S[];              /* " Copy " */
extern const u8 Move0_S[];              /* " Move " */
extern const u8 Ren0_S[];               /* " Rename " */
extern const u8 RnMv_S[];               /* "Rename or move " */
extern const u8 CopTo_S[];              /* "^V to" */
extern const u8 CopFi_S[];              /* "Copying the file" */
extern const u8 Copy1_B[];              /* the dialog of F5 and F6 */
extern const u8 Copy2_B[];              /* and that of Shift-F5 and -F6 */
extern const u8 Renam_B[];              /* "Rename ... to" */
extern const u8 RenFi_P[];              /* "Renaming or moving the file" */
extern const u8 CopFi_P[];              /* "Copying the file" */
extern const u8 MovFi_P[];              /* "Moving the file" */
extern const u8 CopDi_P[];              /* "Copying the directory" */
extern const u8 MovDi_P[];              /* "Renaming or moving the directory" */
extern const u8 DelDr_P[];              /* "Deleting the subdirectory" */
extern const u8 OvrCo_B[];              /* "The following file exists" */
extern const u8 Prot_B[];               /* "... is a read-only file" */
extern const u8 Cont_B[];               /* "There isn't enough room to copy" */
extern const u8 OpnEr_B[];              /* "Can't open the file" */
extern const u8 Itslf_B[];              /* "You can't copy a file to itself" */
extern const u8 RenEr_B[];              /* "Can't rename the file" */
extern const u8 MkErr_B[];              /* "Can't create the directory" */
extern const u8 DelEr_B[];              /* "Can't delete the file" */
extern const u8 RdOnl_B[];              /* "... is marked read-only" */
extern const u8 CntDi_B[];              /* "Can't delete the subdirectory" */

/* What is put behind a path to find everything in the directory. */
static const u8 AllIn_S[] = "\\*.*";

#define Box_C 16                        /* the number of the colour of a box */
#define BoxCu_C 18                      /* and of a line that is typed into */

/* Errors of DOS, and the one that a function of sys.h answers with when
   a drive was not ready and the user gave up. */
#define ERR_NO_PATH   0x03
#define ERR_NO_ACCESS 0x05
#define ERR_CURRENT   0x10
#define ERR_NOT_READY 0x53

/* What C00_00 returns: whether something was done, and whether to stop. */
#define DONE    0x0001
#define NOTHING 0x0000
#define STOP    0x0100
/* What C10_00 returns to have the file copied after all. */
#define COPY    0xFFFF

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))
#define PAIR(high, low) ((u32)(high) << 16 | (low))

/* The title of the boxes, as the offset of the text in the assembly:
   Copy, Move, or Rename once a file is being renamed. */
static u16 F35_40(void)
{
    if (CopyMov == 0)
        return OFFSET(Copy0_S);
    if (CopyMov <= 2)
        return OFFSET(Move0_S);
    return OFFSET(Ren0_S);
}

/* Where the text at `text` ends, looked for in `count` bytes: the address
   behind its zero byte, or behind the bytes if there is none. `count` is
   then what was left to look at. */
static const u8 *C00_End(const u8 *text, u16 *count)
{
    u16 left = *count;
    u8 c;

    do {
        c = *text++;
        left--;
    } while (left && c != 0);
    *count = left;
    return text;
}

/* The drive that a path is on, as a capital letter, or 0 for a path that
   begins with two backslashes. */
static u16 C00_30(const u8 *path)
{
    u16 pair = *(const u16 *)path;

    if (pair == 0x5C5C)
        return 0;
    if (pair >> 8 == ':')
        return (u8)UpCase(pair);
    return md.DosPth[0];
}

/* Make the whole path of the file at `path` in `buffer`, in capitals, for
   comparing: on the current drive and from the current directory, unless
   the path names the drive, or begins at the root. A full stop that the
   name ends with is left out.

   A path on another drive than the current one is taken as it is. */
static void C40_05(const u8 *path, u8 *buffer)
{
    u8 *to;
    u16 pair, count;

    to = NEAR(AddFile(MvzLine(buffer, md.DosPth, LenPath)));
    pair = *(const u16 *)path;
    if (pair >> 8 == ':') {
        if ((u8)UpCase(pair) != buffer[0]) {
            to = buffer;
            goto copy;
        }
        path += 2;
    }
    if (*path == '\\')
        to = buffer + 2;
copy:
    for (count = LenName - (to - buffer); count; count--)
        *to++ = *path++;
    *to = 0;
    NormStr(buffer);
    PathLin(buffer);
    count = LenName;
    to = (u8 *)C00_End(buffer, &count) - 2;
    if (*to == '.')
        *to = 0;
}

/* Whether the source and the target are the same file: 1 if so. `left`
   gets what the assembly has in CX afterwards, the bytes of the target's
   path that were not compared: C10_00 hands that on. */
static u16 C40_00(u16 *left)
{
    const u8 *source = md.TmpErr, *target = md.TmpErr + 100;
    u16 count;

    C40_05(CpSrc, md.TmpErr);
    C40_05(CpDest, md.TmpErr + 100);
    count = LenName;
    count = (u16)(C00_End(target, &count) - target);
    do {
        count--;
        if (*source++ != *target++) {
            *left = count;
            return 0;
        }
    } while (count);
    *left = 0;
    return 1;
}

/* Write the target as the boxes show it: a file with its name in small
   letters, a device in capitals. */
static void C50_00(void)
{
    if (CpDev2 == 0)
        LetLin(CpDest);
    else
        PathLin(CpDest);
}

/* Give the file of a handle the date and the time of the source, unless
   it is the first part of a file that did not fit. */
static void SetDate(u16 handle)
{
    if (SplitFl == 1)
        return;
    FileSetTime(handle, CpTime, CpDate);
}

/* Show how far the copying has got: of the row of cells that C20_00 has
   drawn, as many are filled as what has been read and written is of
   twice the size of the file. A file of no size has them all filled.

   The numbers are 48 bits long: what was read and written, counted once
   for every cell, less twice the size as often as that goes. */
static void PerCent(void)
{
    u32 sum, low = 0, twice, before;
    u16 carry, high = 0, twice_high, borrow, cells;
    u16 count;
    u16 *screen;
    u16 cell;

    sum = CpRead + CpWrite;
    carry = sum < CpRead;
    count = CpCntSz;
    do {
        before = low;
        low += sum;
        high += carry + (low < before);
    } while (--count);
    cells = CpCntSz;
    if (CpSize != 0) {
        twice = CpSize << 1;
        twice_high = HIGH(CpSize) >> 15;
        cells = 0xFFFF;
        do {
            cells++;
            borrow = twice_high + (low < twice);
            low -= twice;
            before = high;
            high -= borrow;
        } while (LOW(before) >= borrow);
        if (cells > CpCntSz)
            cells = CpCntSz;
    }
    screen = BasAdr(CpCntBg);
    cell = *screen & 0xFF00 | 0xDB;
    for (; cells; cells--)
        *screen++ = cell;
    Window(CpCntBg, 0x100 | CpCntSz);
}

/* Make the name of the target, CpDest, from what was typed, ToFile.
   Returns 0 to stop: for a drive that is not ready.

   In capitals. A name that ends with a backslash or a colon, the name
   "..", the directory of the other panel and any other directory that is
   there get the name of the source behind them, with the directories it
   was found in. A name with ? or * is a mask, and so is one that is
   nothing yet: what the mask leaves open is taken from the name of the
   source, or of the first directory it was found in, and the rest of the
   source's path follows. A file or a device that is there is taken as it
   is, and so is the name when a directory of the tree is renamed.

   What a name without a mask is, a directory, something new or a file,
   is found out for the first file and kept in CpCase for the others. */
static u16 C00_01(void)
{
    struct WCBdef *other;
    const u8 *from = md.ToFile;
    const u8 *end;
    u8 *name, *part, *to;
    struct Answer opened;
    struct Device device;
    struct FileAttr attr;
    u16 count, i, kind, info;
    u8 c;

    for (i = 0; i < LenName; i++)
        CpDest[i] = from[i];
    PathLin(CpDest);
    if (CopyMov == 3)
        return 1;
    name = NEAR(CutPath(CpDest));
    to = name;
    if (*name == 0)
        goto source;
    if (*(const u16 *)name == 0x2E2E && *FndLet(name + 2) == 0) {
        to = name + 3;
        goto directory;
    }
    end = name;
    do {
        c = *end++;
        if (c == '?' || c == '*')
            goto mask;
    } while (c != 0);
    count = LenName;
    end = C00_End(CpDest, &count);
    to = (u8 *)end;
    other = PANEL(md.WCBsec);
    count = end - CpDest;
    for (i = 0; other->WinPath[i] == CpDest[i]; )
        if (++i == count)
            goto directory;
    kind = CpCase;
    if (kind == 0) {
        if (ChgDisk(CpDest))
            return 0;
        opened = OpenFil(CpDest, 0x40);
        if (opened.failed == 0) {
            device = FileDevice(opened.value);
            info = device.kind;
            /* The assembly goes on with DX as it is, which DOS has not
               set if it would not say: the address of the name. */
            if (device.failed)
                info = MD_PLACE(CpDest);
            FileClose(opened.value);
            if (info & 0x80) {
                CpDev2 = 1;
                kind = 3;
                goto known;
            }
        } else if (opened.value == ERR_NOT_READY)
            return 0;
        attr = FileAttributes(CpDest);
        if (attr.failed) {
            if (attr.value == ERR_NOT_READY)
                return 0;
            kind = 1;
        } else {
            kind = 3;
            if (attr.attributes & ATTR_DIRECTORY)
                kind = 2;
        }
    }
known:
    CpCase = (u8)kind;
    if (kind == 2)
        goto directory;
    if (kind > 2)
        return 1;
mask:
    Convert(name, md.TmpErr);
    part = MD_POINTER(CpBgNam);
    do
        c = *part++;
    while (c != 0 && c != '\\');
    part--;
    *part = 0;
    Convert(MD_POINTER(CpBgNam), md.TmpErr + 20);
    *part = c;
    for (i = 0; i < 11; i++)
        if (md.TmpErr[i] == '?')
            md.TmpErr[i] = md.TmpErr[20 + i];
    to = NEAR(UnConv(name, md.TmpErr));
    from = part;
    count = to - CpDest;
    goto rest;
directory:
    to[-1] = '\\';
source:
    from = MD_POINTER(CpBgNam);
    count = from - CpSrc;
rest:
    if (count < LenName)
        for (count = LenName - count; count; count--)
            *to++ = *from++;
    *to = 0;
    return 1;
}

/* Mark the tree file of the drive of a path as out of date, if trees are
   read again by themselves: DelTree of the assembly. What it returns
   says whether that failed, and has the error then. */
static struct Answer DelTree(const u8 *path)
{
    struct Answer marked;

    if (md.ChgTree == 0) {
        marked.value = 0;
        marked.failed = 0;
        return marked;
    }
    return DelTr0(path);
}

/* Where the panel has the file that was renamed, give it its new name:
   if neither name has a path. */
static void C10_13(void)
{
    struct WCBdef *wcb = PANEL(md.WCBcrn);
    u8 *entry;
    u16 file, i;

    if (NEAR(CutPath(CpSrc)) != CpSrc || NEAR(CutPath(CpDest)) != CpDest)
        return;
    file = FnFile(md.WCBcrn, CpSrc);
    if (file >= wcb->EndBuf)
        return;
    entry = (u8 *)wcb + file;
    for (i = 0; i < 12; i++)
        entry[i] = CpDest[i];
    entry[12] = 0;
}

/* Rename a file. Returns DONE, NOTHING or STOP, or COPY for a file that
   is to be copied and deleted after all: DOS would not rename it, or the
   user wants it appended to the one that is there. `fits` is not 0 if
   the name of the target is short enough, and gets what C20_00 is to
   take for that then.

   A file that has the name already is asked about, and deleted if it is
   to be overwritten; if it is read-only, that is asked about too.

   Ported as it is: what C20_00 gets for `fits` is what the assembly has
   in CX there, which it does not set again. That is what comparing the
   names left, 0 if the source's path only goes on where the target's
   ends; or, for a file to append to, the attributes of that file, 0 if
   it has none. C20_00 then says that it cannot open the file. */
static u16 C10_00(u16 *fits)
{
    struct Box box;
    struct FileAttr attr;
    struct Answer asked;
    u16 result, answer, error, report;

    CopyMov = 3;
    OncZoom(0x528, OFFSET(RenFi_P), 0, &box);
    error = 0;
    if (*fits == 0)
        goto failed;
    result = DONE;
    if (C40_00(fits))
        goto close;
    if (ChgDisk(CpDest))
        goto stop;
    asked = FileRename(CpSrc, CpDest);
    if (!asked.failed)
        goto renamed;
    if (asked.value == ERR_NOT_READY)
        goto stop;
    if (asked.value != ERR_NO_ACCESS) {
        CopyMov = 2;
        goto copy;
    }
    attr = FileAttributes(CpDest);
    if (attr.failed) {
        error = attr.value;
        goto failed;
    }
    if (attr.attributes & ATTR_DIRECTORY)
        goto failed;
    if (CpAll == 0) {
        answer = (u8)Ask(OFFSET(OvrCo_B), OFFSET(Ren0_S));
        if (answer == 2) {
            *fits = attr.attributes;
            goto copy;
        }
        if (answer > 2)
            goto nothing;
        CpAll = (u8)answer;
    }
    asked = FileDelete(CpDest);
    if (asked.failed) {
        error = asked.value;
        if (error != ERR_NO_ACCESS)
            goto undeleted;
        C50_00();
        if ((u8)Ask(OFFSET(Prot_B), F35_40()) != 0)
            goto nothing;
        asked = FileSetAttributes(CpDest, 0x20);
        if (asked.failed) {
            error = asked.value;
            goto undeleted;
        }
        FncChg = 1;
        asked = FileDelete(CpDest);
        if (asked.failed) {
            error = asked.value;
            goto undeleted;
        }
    }
    asked = FileRename(CpSrc, CpDest);
    if (!asked.failed)
        goto renamed;
    error = asked.value;
failed:
    /* None of the lines of this box has ^I: no include line. */
    report = OFFSET(RenEr_B);
    goto say;
undeleted:
    report = OFFSET(DelEr_B);
say:
    if (error == ERR_NOT_READY)
        goto stop;
    if ((u8)Ask(report, OFFSET(Ren0_S)) != 0)
        goto stop;
nothing:
    result = NOTHING;
    goto close;
stop:
    result = STOP;
    goto close;
copy:
    result = COPY;
    goto close;
renamed:
    LetLin(CpSrc);
    LetLin(CpDest);
    C10_13();
    result = DONE;
close:
    ResWin(box.begin, box.size, box.saved);
    Window(box.begin, box.size);
    return result;
}

/* Copy a file, and delete the source if it is to be moved. Returns DONE,
   NOTHING or STOP. `fits` is not 0 if the name of the target is short
   enough.

   The source is opened and read, as much of it as the memory holds, in
   pieces of 32K; then the target is opened and that much written, and so
   on until the source has ended. A device may be the source, but for the
   console, and is read until it gives nothing. A target that is there is
   asked about, unless All has been answered before: it is deleted and
   made anew, or opened and written to from its end, over the end-of-file
   character if it has one; if it is read-only, that is asked about too.
   Esc, Ctrl+C or F10 between two pieces stops, and what was written is
   deleted. The target gets the date, the time and the attributes of the
   source, or keeps its own if it was appended to.

   When the disk is full, what still fits of the piece is written and the
   user is asked whether to go on with another disk: the target is then
   made there, with its directories, and gets the rest.

   Ported as they are: a source that turns out to be the console, or
   whose attributes DOS will not tell, stays open; so does a target that
   is the console; a target that was appended to is deleted like any
   other when the copying stops; and after a piece that took more than
   one call of DOS to read, which only a device gives, the room left in
   the memory is counted down by the last of them only. */
static u16 C20_00(u16 fits)
{
    struct Box box;
    u32 left, free;
    struct Answer opened, moved, asked;
    struct Device device;
    struct FileAttr attr;
    struct FileStamp stamp;
    struct Space space;
    u16 result, error, report, answer, info, attributes;
    u16 segment, offset, count, got, i;
    u16 *screen;
    u8 *part, *end;
    u8 c;

    C50_00();
    report = OFFSET(CopFi_P);
    if (CopyMov != 0)
        report = OFFSET(MovFi_P);
    /* None of the lines of these boxes has ^I: no include line. */
    OncZoom(0x428, report, 0, &box);
    CpCntBg = box.begin + 0x605;
    CpCntSz = (u8)(box.size - 12);
    screen = BasAdr(CpCntBg);
    count = (u8)Color(Box_C) << 8 | 0xB0;
    for (i = CpCntSz; i; i--)
        *screen++ = count;
    Window(CpCntBg, 0x100 | CpCntSz);
    report = OFFSET(OpnEr_B);
    if (fits == 0)
        goto ask;
    if (C40_00(&count)) {
        report = OFFSET(Itslf_B);
        goto ask;
    }
    if (ChgDisk(CpSrc))
        goto stop;
    opened = OpenFil(CpSrc, 0x40);
    error = opened.value;
    if (opened.failed)
        goto unopened;
    CpHndl1 = opened.value;
    device = FileDevice(CpHndl1);
    info = device.kind;
    /* The assembly goes on with DX as it is, which DOS has not set if it
       would not say what the handle is: the address of the name, here
       and for the target. */
    if (device.failed)
        info = MD_PLACE(CpSrc);
    if ((info & 0x81) == 0x81)
        goto no_source;
    CpDev1 = (u8)(info & 0x80);
    CpSize = 0xFFFFFFFF;
    CpAttr = 0x20;
    if (CpDev1 == 0) {
        attr = FileAttributes(CpSrc);
        if (attr.failed) {
            error = attr.value;
            goto unopened;
        }
        CpAttr = attr.attributes & 0x27 | 0x20;
        CpSize = FileSeek(CpHndl1, 2, 0).place;
        FileSeek(CpHndl1, 0, 0);
    }
    stamp = FileTime(CpHndl1);
    if (stamp.failed && CpDev1 != 0) {
        /* The assembly goes on with CX and DX as they are, which DOS has
           not set then. For a file both are 0, of the place just gone
           to, as FileTime has them; for a device they are what comparing
           the two names left and the address of the name. */
        stamp.time = count;
        stamp.date = MD_PLACE(CpSrc);
    }
    CpDate = stamp.date;
    CpTime = stamp.time;
    CpRead = 0;
    CpWrite = 0;
    CpHndl2 = 0;
    CpCont = 0;
    SplitFl = 0;

read:
    left = (u32)((MAIN_SEG + md.FreMem2 - md.ViewSeg) & 0xFFE0) << 4;
    segment = md.ViewSeg;
    if (ChgDisk(CpSrc))
        goto terminate;
    do {
        if (TstBrk())
            goto terminate;
        offset = 0;
        count = 0x8000;
        if (HIGH(left) == 0 && count > LOW(left))
            count = LOW(left);
        do {
            got = 0;
            if (count != 0) {
                moved = FileRead(CpHndl1, AT(segment, offset), count);
                if (moved.failed)
                    goto terminate;
                got = moved.value;
            }
            CpRead += got;
            PerCent();
            if (got == 0)
                goto ended;
            offset += got;
            count -= got;
        } while (count != 0);
        segment += 0x800;
        left -= got;
    } while (left != 0);
    goto target;
ended:
    FileClose(CpHndl1);
    CpHndl1 = 0;
    if (CpDev1 != 0) {
        CpSize = CpRead;
        if (CpRead == 0)
            goto terminate;
        PerCent();
    }

target:
    if (ChgDisk(CpDest))
        goto terminate;
    if (CpHndl2 != 0)
        goto write;
open:
    C50_00();
    if (CpDev2 != 0) {
        opened = OpenFil(CpDest, 0x11);
        error = opened.value;
        if (opened.failed)
            goto no_target;
        goto opened;
    }
    attr = FileAttributes(CpDest);
    if (attr.failed) {
        error = attr.value;
        if (error > ERR_NO_PATH)
            goto no_target;
        goto create;
    }
    if (CpAll == 0) {
        if (CopyMov == 3)
            goto append;
        answer = (u8)Ask(OFFSET(OvrCo_B), F35_40());
        if (answer > 1) {
            if (answer != 2)
                goto skip;
            goto append;
        }
        CpAll |= (u8)answer;
    }
    if (attr.attributes & 1) {
        if ((u8)Ask(OFFSET(Prot_B), F35_40()) != 0)
            goto skip;
        asked = FileSetAttributes(CpDest, 0x20);
        if (asked.failed) {
            error = asked.value;
            goto undeleted;
        }
        FncChg = 1;
    }
    asked = FileDelete(CpDest);
    if (asked.failed) {
        error = asked.value;
        goto undeleted;
    }
    FncChg = 1;
create:
    opened = FileCreate(CpDest, 0x20);
    error = opened.value;
    if (opened.failed)
        goto no_target;
opened:
    CpHndl2 = opened.value;
    FncChg = 1;
    device = FileDevice(CpHndl2);
    info = device.kind;
    if (device.failed)
        info = MD_PLACE(CpDest);
    if ((info & 0x82) != 0x82)
        goto write;
    error = 0;
    goto no_target;
undeleted:
    report = OFFSET(DelEr_B);
    goto say;
no_target:
    report = OFFSET(OpnEr_B);
say:
    if (error == ERR_NOT_READY)
        goto terminate;
    if ((u8)Ask(report, F35_40()) != 0)
        goto terminate;
skip:
    result = NOTHING;
    goto close_source;

append:
    attr = FileAttributes(CpDest);
    if (attr.failed) {
        error = attr.value;
        goto no_target;
    }
    CpAttr = attr.attributes & 0x27 | 0x20;
    attributes = CpAttr;
    opened = OpenFil(CpDest, 0x12);
    if (opened.failed) {
        error = opened.value;
        if (error != ERR_NO_ACCESS)
            goto no_target;
        if ((u8)Ask(OFFSET(Prot_B), F35_40()) != 0)
            goto skip;
        attributes = 0x20;
        asked = FileSetAttributes(CpDest, attributes);
        if (asked.failed) {
            error = asked.value;
            goto no_target;
        }
        FncChg = 1;
        opened = OpenFil(CpDest, 0x12);
        error = opened.value;
        if (opened.failed)
            goto no_target;
    }
    FncChg = 1;
    CpHndl2 = opened.value;
    stamp = FileTime(CpHndl2);
    if (stamp.failed) {
        /* CX and DX as they are, again: the attributes that were last
           kept or set, and the address of the name. */
        stamp.time = attributes;
        stamp.date = MD_PLACE(CpDest);
    }
    CpDate = stamp.date;
    CpTime = stamp.time;
    left = FileSeek(CpHndl2, 2, 0).place;
    if (left != 0) {
        left--;
        FileSeek(CpHndl2, 0, left);
        if (FileRead(CpHndl2, md.TmpErr, 1).failed)
            goto terminate;
        if (md.TmpErr[0] == 0x1A)
            FileSeek(CpHndl2, 0, left);
    }

write:
    left = CpRead - CpWrite;
    segment = md.ViewSeg;
    if (CpCont != 0)
        segment = CpCont;
    CpCont = 0;
    do {
        if (TstBrk())
            goto terminate;
        count = 0x8000;
        if (HIGH(left) == 0 && count > LOW(left))
            count = LOW(left);
        got = 0;
        if (count != 0) {
            moved = FileWrite(CpHndl2, AT(segment, 0), count);
            if (moved.failed)
                goto terminate;
            got = moved.value;
            if (got == 0)
                goto full;
        }
        CpWrite += got;
        PerCent();
        left -= got;
        segment += got >> 4;
    } while (left != 0);
    if (CpHndl1 != 0)
        goto read;

    SetDate(CpHndl2);
    if (FileClose(CpHndl2).failed)
        goto stop;
    if (CpDev2 == 0) {
        asked = FileSetAttributes(CpDest, CpAttr);
        if (asked.failed && asked.value != ERR_NO_ACCESS)
            goto stop;
    }
    if (CopyMov == 0)
        goto done;
    if (ChgDisk(CpSrc))
        goto stop;
    asked = FileDelete(CpSrc);
    if (!asked.failed)
        goto done;
    error = asked.value;
    if (error == ERR_NO_ACCESS) {
        answer = (u8)Ask(OFFSET(RdOnl_B), F35_40());
        if (answer == 1)
            goto done;
        if (answer > 1)
            goto stop;
        asked = FileSetAttributes(CpSrc, 0x20);
        error = asked.value;
        if (!asked.failed) {
            asked = FileDelete(CpSrc);
            if (!asked.failed)
                goto done;
            error = asked.value;
        }
    }
    if (error == ERR_NOT_READY)
        goto stop;
    for (i = 0; i < LenName; i++)
        CpDest[i] = CpSrc[i];
    if ((u8)Ask(OFFSET(DelEr_B), F35_40()) != 0)
        goto stop;
done:
    result = DONE;
    goto close;

full:
    /* Not a byte of the piece was written. How much room is there? */
    error = *(const u16 *)CpDest;
    if (error >> 8 != ':')
        error = md.DosPth[0];
    /* DOS leaves the carry flag as it finds it here: the assembly has it
       from taking 'A'-1 off the letter. */
    space = DriveSpace((u8)(error - ('A' - 1)), (u8)error < 'A' - 1);
    if (space.failed) {
        /* The assembly goes on with BX and CX as they are, which DOS has
           not set then: the handle, and the size of the piece. */
        space.free = CpHndl2;
        space.sector = count;
    }
    got = 0;
    if (space.value != 0xFFFF) {
        /* The bytes of a cluster as 16 bits, times the clusters. */
        free = (u32)(u16)(space.value * space.sector) * space.free;
        if (HIGH(free) == 0 && LOW(free) < count) {
            moved = FileWrite(CpHndl2, AT(segment, 0), LOW(free));
            if (moved.failed)
                goto terminate;
            got = moved.value;
        }
    }
    CpWrite += got;
    PerCent();
    CpCont = (got >> 4) + segment;
    SetDate(CpHndl2);
    /* Is anything in the file? */
    info = FileSeek(CpHndl2, 2, 0).place != 0;
    if (FileClose(CpHndl2).failed)
        goto forget;
    if (info == 0) {
        if (FileDelete(CpDest).failed)
            goto terminate;
    } else {
        asked = FileSetAttributes(CpDest, CpAttr);
        if (asked.failed && asked.value != ERR_NO_ACCESS)
            goto forget;
    }
    if ((u8)Ask(OFFSET(Cont_B), F35_40()) != 1) {
        asked = FileDelete(CpDest);
        if (asked.failed && asked.value == ERR_NO_ACCESS
            && !FileSetAttributes(CpDest, 0x20).failed)
            FileDelete(CpDest);
        goto forget;
    }
    /* On another disk: the directories of the target are made there, one
       after the other, if the last of them is not there. */
    SplitFl = 2;
    if (CpWrite != 0)
        SplitFl = 1;
    for (i = 0; i < LenName; i++)
        md.TmpErr[i] = CpDest[i];
    part = NEAR(CutPath(md.TmpErr));
    if (part <= md.TmpErr)
        goto open;
    part--;
    if (*part != '\\' || part <= md.TmpErr || part[-1] == ':')
        goto open;
    *part = 0;
    attr = FileAttributes(md.TmpErr);
    if (!attr.failed)
        goto open;
    if (attr.value == ERR_NOT_READY)
        goto forget;
    part = md.TmpErr;
    if (part[1] == ':')
        part += 2;
    if (*part == '\\')
        part++;
    while ((c = *part++) != 0)
        if (c == '\\')
            part[-1] = 0;
    *part = 0;
    for (;;) {
        asked = DirMake(md.TmpErr);
        if (asked.failed && asked.value == ERR_NOT_READY)
            goto forget;
        count = LenName;
        end = (u8 *)C00_End(md.TmpErr, &count);
        if (*end == 0)
            goto open;
        end[-1] = '\\';
    }

forget:
    CpHndl2 = 0;
terminate:
    result = STOP;
    md.BlocEr = 1;
    if (CpHndl2 != 0 && !ChgDisk(CpDest)) {
        FileClose(CpHndl2);
        FileDelete(CpDest);
    }
close_source:
    if (CpHndl1 != 0)
        FileClose(CpHndl1);
    md.BlocEr = 0;
    goto close;

unopened:
    if (error == ERR_NOT_READY)
        goto stop;
no_source:
    LetLin(CpSrc);
    for (i = 0; i < LenName; i++)
        CpDest[i] = CpSrc[i];
    report = OFFSET(OpnEr_B);
ask:
    if ((u8)Ask(report, F35_40()) != 0)
        goto stop;
    result = NOTHING;
    goto close;
stop:
    result = STOP;
close:
    ResWin(box.begin, box.size, box.saved);
    Window(box.begin, box.size);
    return result;
}

/* Copy, rename or move a directory. Returns what C00_00 does. `across`
   is 0 for a directory that DOS may be able to rename, and `fits` is not
   0 if the name of the target is short enough.

   If DOS does not rename it, the target is made, with the attributes of
   the source, or is there already; a source that is to be moved and is
   empty is removed at once. Otherwise DirFunc has C00_00 called for all
   that is in the source, and a source that is to be moved is removed
   after that, if it is empty then. A device for a target gets the files
   and nothing else.

   The first time a directory is made or goes, the tree file of its drive
   is marked as out of date. */
static u16 C30_00(u16 across, u16 fits)
{
    struct Box box;
    struct FileAttr attr, there;
    struct Answer marked, asked;
    u16 result, error, i;
    u8 *end;

    if (CpDev2 != 0)
        goto walk;
    result = OFFSET(CopDi_P);
    if (CopyMov != 0)
        result = OFFSET(MovDi_P);
    OncZoom(0x428, result, F35_40(), &box);
    if (fits == 0)
        goto unmade;
    if (C40_00(&i))
        goto same;
    if (CopyMov == 0 || across != 0)
        goto make;
    if (ChgDisk(CpDest))
        goto stop;
    asked = FileRename(CpSrc, CpDest);
    if (asked.failed) {
        if (asked.value == ERR_NOT_READY)
            goto stop;
        goto make;
    }
    FncChg = 1;
    if (CpTree1 == 0) {
        marked = DelTree(CpSrc);
        if (marked.failed && marked.value == ERR_NOT_READY)
            goto stop;
        CpTree1 = 1;
        CpTree2 = 1;
    }
same:
    PathLin(CpSrc);
    PathLin(CpDest);
    C10_13();
    result = DONE;
    goto close;

make:
    if (ChgDisk(CpSrc))
        goto stop;
    attr = FileAttributes(CpSrc);
    error = attr.value;
    if (attr.failed)
        goto failed;
    if (ChgDisk(CpDest))
        goto stop;
    asked = DirMake(CpDest);
    if (asked.failed) {
        if (asked.value == ERR_NOT_READY)
            goto stop;
        there = FileAttributes(CpDest);
        error = there.value;
        if (there.failed)
            goto failed;
        if (!(there.attributes & ATTR_DIRECTORY))
            goto unmade;
    } else {
        FncChg = 1;
        asked = FileSetAttributes(CpDest, attr.attributes & 0x27);
        error = asked.value;
        if (asked.failed)
            goto failed;
        if (CpTree2 == 0) {
            CpTree2 = 1;
            marked = DelTree(CpDest);
            if (marked.failed && marked.value == ERR_NOT_READY)
                goto stop;
        }
    }
    if (CopyMov != 0) {
        if (ChgDisk(CpSrc))
            goto stop;
        asked = DirRemove(CpSrc);
        if (!asked.failed)
            goto removed;
        if (asked.value == ERR_NOT_READY)
            goto stop;
        if (asked.value != ERR_NO_ACCESS && asked.value != ERR_CURRENT)
            goto nothing;
    }
    ResWin(box.begin, box.size, box.saved);
    Window(box.begin, box.size);
walk:
    result = DirFunc();
    FncChg |= (u8)result;
    if (result >> 8 != 0 || CopyMov == 0)
        return result;

    md.SftErr = 0;
    OncZoom(0x528, OFFSET(DelDr_P), OFFSET(Move0_S),
            &box);
    if (ChgDisk(CpSrc))
        goto stop;
    asked = DirRemove(CpSrc);
    if (asked.failed) {
        if (asked.value == ERR_NOT_READY)
            goto stop;
        if (asked.value == ERR_NO_ACCESS) {
            /* Is anything left in it? Its own two entries are. */
            for (end = CpSrc; *end != 0; end++)
                ;
            for (i = 0; i < sizeof AllIn_S; i++)
                end[i] = AllIn_S[i];
            asked = FindFirst(CpSrc, 0x37);
            *end = 0;
            if (!asked.failed)
                goto nothing;
            if (asked.value == ERR_NOT_READY)
                goto stop;
        }
        Ask(OFFSET(CntDi_B), OFFSET(Move0_S));
        goto nothing;
    }
removed:
    FncChg = 1;
    if (CpTree1 == 0) {
        CpTree1 = 1;
        marked = DelTree(CpSrc);
        if (marked.failed && marked.value == ERR_NOT_READY)
            goto stop;
    }
    result = DONE;
    goto close;

failed:
    if (error == ERR_NOT_READY)
        goto stop;
unmade:
    Ask(OFFSET(MkErr_B), F35_40());
nothing:
    result = NOTHING;
    goto close;
stop:
    result = STOP;
close:
    ResWin(box.begin, box.size, box.saved);
    Window(box.begin, box.size);
    return result;
}

/* Copy, rename or move the file or the directory CpSrc to what ToFile
   says, as CopyMov has it: 0 to copy, 1 to rename or move, 3 to rename a
   directory of the tree. `attributes` are those of the source. Returns
   in the low byte 1 if it was done, and in the high byte 1 to stop.

   A file or a directory that is to be moved is renamed if the source and
   the target are on the same drive, and copied otherwise, or to a
   device.

   Ported as it is: a name of 79 characters and more counts as too long,
   and is reported as a file that cannot be opened or renamed, or a
   directory that cannot be made. */
u16 C00_00(u16 attributes)
{
    u16 result, fits, drive, rename;

    md.SftErr = 1;
    result = STOP;
    if (C00_01()) {
        fits = LenPath + 12;
        C00_End(CpDest, &fits);
        rename = 0;
        if (CpDev2 == 0 && CopyMov != 2) {
            if (CopyMov > 2)
                rename = 1;
            else if (CopyMov != 0) {
                drive = C00_30(CpSrc);
                if (drive != 0 && C00_30(CpDest) == drive)
                    rename = 1;
            }
        }
        if (attributes & ATTR_DIRECTORY)
            result = C30_00(!rename, fits);
        else {
            result = COPY;
            if (rename)
                result = C10_00(&fits);
            if (result == COPY)
                result = C20_00(fits);
        }
    }
    md.SftErr = 0;
    if (CopyMov != 0)
        CopyMov = 1;
    return result;
}

/*
 * The commands: Copy and Rename, F5 and F6, for what is selected or what
 * is under the cursor; with Ctrl for the file under the cursor alone; and
 * with Shift for files whose name is asked for too. They ask where to,
 * and say where the main loop is to go on.
 */

/* What a command returns, struct Copied of vc.h: the place where the
   program goes on, a GO_ there; or that it goes on with the files of a
   mask, GO_MASKED, with the title of the boxes. */
static struct Copied Go(u16 place)
{
    struct Copied copied;

    copied.place = place;
    copied.title = 0;
    return copied;
}

static struct Copied Masked(u16 title)
{
    struct Copied copied;

    copied.place = GO_MASKED;
    copied.title = title;
    return copied;
}

#define ASM(object, offset) AT(AsmSeg(), OFFSET(object) + (offset))

/* Init variables: nothing has been copied yet, and C00_00 is what is
   called for every file. */
static void F35_70(void)
{
    CpCase = 0;
    CpDev2 = 0;
    CpTree1 = 0;
    CpTree2 = 0;
    CpAll = 0;
    FncChg = 0;
    md.FncAddr = FNC_C00_00;
}

/* Set default target path, in CpInpt: the directory of the other panel,
   if that shows files or the tree, and else the last target; but for
   Rename, of one file or with nothing selected, the name of the file
   under the cursor. Also the title, in CpVar1, and in CpVar2 a line of
   spaces that begins "Copy " or "Rename or move ". Returns the address
   behind those words.

   `directories` and `files` are the numbers of what is selected, where
   the command has counted, and what BX and DX happened to hold where it
   has not: after Shift-F6, or F6 with the panel switched off. */
static u8 *F35_50(u16 panel, u16 directories, u16 files)
{
    const struct WCBdef *wcb;
    const u8 *from;
    u8 *to;
    u16 other, title, words, length, i;

    from = md.ToFile;                                   /* F35_55 */
    if (md.OneFile == 0 || CopyMov == 0) {
        other = InvWCB(panel);
        wcb = PANEL(other);
        if (wcb->Visible != 0 && wcb->WinTyp <= 1) {
            from = md.CrnPath;
            if (wcb->WinTyp == 0)
                from = wcb->WinPath;
            goto F35_57;
        }
        if (CopyMov == 0 || (directories | files) != 0)
            goto F35_57;
    }
    wcb = PANEL(panel);                                 /* F35_54 */
    if (wcb->CurAdr < wcb->EndBuf
        && ENTRY(wcb, wcb->CurAdr)->NameF[0] != '.')
        from = ENTRY(wcb, wcb->CurAdr)->NameF;
F35_57:
    for (i = 0; i < LenStr; i++)
        CpInpt[i] = from[i];
    CpInpt[LenStr] = 0;

    title = OFFSET(Copy0_S);
    words = OFFSET(Copy0_S) + 1;
    length = 5;
    if (CopyMov != 0) {
        title = OFFSET(Ren0_S);
        words = OFFSET(RnMv_S);
        length = 15;
    }
    /* The title and what follows it, 20 bytes: see copy_t.c. */
    for (i = 0; i < CpVar2 - CpVar1; i++)
        CpVar1[i] = *AT(AsmSeg(), title + i);
    for (i = 0; i < LenStr; i++)
        CpVar2[i] = ' ';
    CpVar2[LenStr] = 0;
    to = CpVar2;
    for (i = 0; i < length; i++)
        *to++ = *AT(AsmSeg(), words + i);
    return to;
}

/* Copy or rename, as the 1 in the low byte of `command` says, files that
   are asked for by name: Shift-F5 and Shift-F6, and F5 and F6 where the
   panel shows no files. Returns where to go on. */
struct Copied F45_01(u16 panel, u16 command, u16 directories, u16 files)
{
    u8 *to;
    u16 i;

    CopyMov = (u8)command;
    md.HlpPage = (u8)(20 + command);
    to = F35_50(panel, directories, files);
    for (i = 0; i < 4; i++)                             /* "file" */
        to[i] = *ASM(CopFi_S, 12 + i);
    if ((u8)Ask(OFFSET(Copy2_B), F35_40()) != 0)
        return Go(GO_PANELS);
    for (i = 0; i < LenStr + 1; i++)
        md.ToFile[i] = CpInpt[i];
    for (i = 0; i < LenStr + 1; i++)
        CpSrc[i] = md.ViewFil[i];
    if (md.ViewFil[0] == 0 || md.ToFile[0] == 0)
        return Go(GO_PANELS);
    CpBgNam = MD_PLACE(NEAR(CutPath(CpSrc)));
    F35_70();
    return Masked(F35_40());
}

/* Copy or rename the directory of the tree that the cursor is on, once
   its new place is in ToFile, and take the current directory of DOS
   along if it was that one or below it. The command that calls it goes
   on at GO_DONE.

   Ported as it is: the current directory counts as below the one that
   was renamed if its path begins with that one's, whether a name ends
   there or not. With DOCS renamed to X while DOCS2 is the current one,
   the program goes to X2, if there is such a directory. */
static void F36_04(void)
{
    const u8 *rest;
    u8 *to;
    u16 length, i;

    for (i = 0; i < LenPath; i++)
        CpSrc[i] = md.CrnPath[i];
    CpBgNam = MD_PLACE(NEAR(CutPath(CpSrc)));
    F35_70();
    md.ZoomOne = 0;
    if ((u8)C00_00(ATTR_DIRECTORY) == 0 || CopyMov == 0)
        return;
    length = 0;
    while (length < LenName - 1 && md.CrnPath[length] != 0)
        length++;
    for (i = 0; i < length; i++)
        if (md.CrnPath[i] != md.DosPth[i])
            goto F36_05;
    rest = md.DosPth + length;
    to = NEAR(MvzLine(CpInpt, md.ToFile, LenStr + 1));
    for (i = 0; i < LenPath; i++)
        to[i] = rest[i];
    DirChange(CpInpt + 2);
F36_05:
    for (i = 0; i < LenStr + 1; i++)
        md.CrnPath[i] = md.ToFile[i];
}

/* Copy or rename, as the 1 in the low byte of `command` says: F5 and F6,
   and with OneFile set Ctrl-F5 and Ctrl-F6. In a panel of files that is
   what is selected, or the file under the cursor if nothing is or if
   only that one is wanted; in a tree, the directory under the cursor,
   and to rename that only a name is asked for. `directories` and `files`
   are what BX and DX hold when the key arrives: see F35_50. Returns
   where to go on. */
struct Copied F35_01(u16 panel, u16 command, u16 directories, u16 files)
{
    const struct WCBdef *wcb = PANEL(panel);
    const u8 *from;
    const u8 *name, *typed;
    u8 *to, *end;
    struct Selected selected;
    u16 length, i;

    command &= 0xFF;
    CopyMov = (u8)command;
    md.HlpPage = (u8)(20 + command);
    if (wcb->Visible == 0 || wcb->WinTyp > 1)
        return F45_01(panel, command, directories, files);
    if (wcb->WinTyp == 1) {
        if (md.TreAdr <= TRELIST)                       /* the root */
            return Go(GO_KEYS);
        if (command != 0)
            goto F36_03;
    } else {
        directories = 0;
        files = 0;
        if (md.OneFile == 0) {
            selected = TotSel(panel);
            directories = selected.directories;
            files = selected.files;
        }
        if ((directories | files) == 0
            && (wcb->CurAdr >= wcb->EndBuf
                || ENTRY(wcb, wcb->CurAdr)->NameF[0] == '.'))
            return Go(GO_KEYS);
    }

    /* F35_04: what is to be copied, and the dialog. */
    to = F35_50(panel, directories, files);
    if (wcb->WinTyp == 1) {
        for (i = 0; i < LenName; i++)
            md.TmpErr[i] = md.CrnPath[i];
        ShrtLin(md.TmpErr, LenStr - 10);
        from = md.TmpErr;
        length = LenStr;
        goto F35_06;
    }
    if ((directories | files) != 0) {
        to = FilDir(to, directories, files) - 1;
    } else {
        from = ENTRY(wcb, wcb->CurAdr)->NameF;
        length = 13;
F35_06:
        *to++ = '"';
        to = NEAR(MvzLine(to, from, length));
        *to++ = '"';
    }
    for (i = 0; i < 3; i++)                             /* " to" */
        *to++ = *ASM(CopTo_S, 2 + i);
    end = CpVar2 + LenStr;
    while (to < end)
        *to++ = ' ';
    *to = 0;
    if ((u8)Ask(OFFSET(Copy1_B), F35_40()) != 0)
        return Go(GO_PANELS);
    for (i = 0; i < LenStr + 1; i++)
        md.ToFile[i] = CpInpt[i];
    if (md.ToFile[0] == 0)
        return Go(GO_PANELS);
    if (PANEL(md.WCBcrn)->WinTyp == 1) {
        F36_04();
        return Go(GO_DONE);
    }
    F35_70();
    CpBgNam = MD_PLACE(CpSrc);
    return Go(GO_SELECTED);

F36_03:
    /* A new name for the directory, which stays where it is. */
    for (i = 0; i < 13; i++)
        CpDest[i] = *MD_POINTER(md.TreAdr + i);
    if ((u8)Ask(OFFSET(Renam_B), 0) != 0)
        return Go(GO_KEYS);
    typed = CpDest;
    if (NEAR(CutPath(typed)) != typed)
        goto F36_01;
    for (i = 0; i < LenPath; i++)
        CpInpt[i] = md.CrnPath[i];
    name = NEAR(CutPath(CpInpt));
    for (i = 0; i < 13; i++)
        ((u8 *)name)[i] = typed[i];
    for (i = 0; CpInpt[i] != 0; i++)
        if (i == LenStr)
            goto F36_01;                /* too long */
    for (i = 0; i < LenStr + 1; i++)
        md.ToFile[i] = CpInpt[i];
    CopyMov = 3;
    F36_04();
    return Go(GO_DONE);

F36_01:
    Ask(OFFSET(MkErr_B), F35_40());
    return Go(GO_KEYS);
}

/* What the dialogs of the commands have Dialog call for an answer:
   `answer` is its number in the low byte, 1 for the button F10-Tree and
   for F10 and Alt-F10, and in the high byte not 0 for F10 itself.
   `place` is where the line is that the target is typed into. That
   answer has a directory chosen from the tree, of the drive that was
   typed if it was F10 and of the current one otherwise, and puts its
   path into the line. Returns 1 if the dialog is to go on, as it is
   after that answer, and 0 for any other. */
u16 F35_61(u16 answer, u16 place)
{
    u8 *line = md.DosBuf + 124 + 10 + 3;        /* DosBuf+LenCmd+10+3 */
    u8 *kept = md.MenuBuf + 350;
    u8 *end;
    u16 pair, left, i;

    if ((answer & 0xFF) != 1)
        return 0;
    for (i = 0; i < LenPath; i++)
        kept[i] = md.DosPth[i];
    left = 0;
    if (answer >> 8 != 0) {
        for (i = 0; i < LenStr + 1; i++)
            md.TmpBuf[i] = line[i];
        NormStr(md.TmpBuf);
        pair = *(u16 *)md.TmpBuf;
        if (pair >> 8 == ':') {
            DriveSelect((u8)(UpCase(pair) - 'A'));
            if (SetPth())
                left = 1;
        }
    }
    if (!left)
        left = ChTree(28);
    /* Back to the drive and the directory that were the current ones. */
    pair = kept[0];
    for (i = 0; i < LenPath; i++)
        md.DosPth[i] = kept[i];
    DriveSelect((u8)(pair - 'A'));
    if (left)
        return 1;
    end = NEAR(MvzLine(line, md.TmpBuf, LenName));
    if (end[-1] != '\\') {
        end[0] = '\\';
        end[1] = 0;
    }
    line[LenStr] = 0;
    for (i = 0; line[i] != 0; i++)
        ;
    *(u16 *)(line - 3) = i;             /* the cursor behind the path */
    line[-1] = 1;                       /* and what is typed added to it */
    InSt10(place, MD_PLACE(line), LenStr, BoxCu_C << 8);
    return 1;
}
