/*
 * The list of files of a panel: which of them are shown, in what order,
 * and where one of them is.
 *
 * Translated from VCSUB2.INC and VCSUB3.INC.
 *
 * A panel has a block to itself, and its list is the file entries from
 * DskBuf on. Those up to EndBuf are shown; behind them, up to EndFul, are
 * the files of the directory that the panel's filter leaves out. A place
 * in the list is an offset in the block, the address that the assembly
 * has in the panel's segment.
 *
 * Nothing here but SetMask touches the main data: FnFile and NearFl
 * take what they read as pointers. NearFl names the room there, Room,
 * for where the old list is, since a pointer does not say which segment
 * it is in.
 */
#include "vc.h"

#define SIZE_FILENT sizeof(struct FILENT)

u16 TstExe(const u8 *name);
/* SetMask, Sort, FnFile and NearFl are in vc.h. */

/* The extensions of programs. Names in a list are in small letters. */
static const u8 Ext_S[9] = {
    'c', 'o', 'm', 'e', 'x', 'e', 'b', 'a', 't'
};

/* Whether a file is a program, by the three characters behind the first
   full stop of its name: 1 if it is, 0 if not. */
u16 TstExe(const u8 *name)
{
    u16 i;
    u8 c;

    do {
        c = *name++;
        if (c == 0)
            return 0;
    } while (c != '.');
    for (i = 0; i < sizeof Ext_S; i += 3)
        if (name[0] == Ext_S[i] && name[1] == Ext_S[i + 1]
            && name[2] == Ext_S[i + 2])
            return 1;
    return 0;
}

/* Choose the files of a panel that it shows, by its filter, and put the
   others behind them: hidden and system files unless it shows those, and
   of the rest, but for directories, what is no program where it shows
   only programs, and what does not fit its mask. A file that is left out
   is no longer selected. The mask is put into its fixed form at TmpBuf. */
void SetMask(struct WCBdef *panel)
{
    u8 *mask = md.TmpBuf;
    u8 *one;
    u8 *other;
    u16 file, last, i;
    u8 attributes, c;

    for (i = 0; i < 11; i++)
        mask[i] = '?';
    if (panel->DskMas[0] != 0)
        Convert(panel->DskMas, mask);
    file = DSKBUF;
    last = panel->EndFul;
    if (file < last) {
        last -= SIZE_FILENT;
        for (;;) {
            attributes = ENTRY(panel, file)->AttrF;
            if ((panel->Hidden != 0
                 || !(attributes & (ATTR_HIDDEN | ATTR_SYSTEM)))
                && ((attributes & ATTR_DIRECTORY)
                    || ((panel->MasTyp == 0
                         || TstExe(ENTRY(panel, file)->NameF))
                        && TstMas(ENTRY(panel, file)->NameF, mask)))) {
                file += SIZE_FILENT;
                if (file > last)
                    break;
            } else {
                if (file == last)
                    break;
                one = (u8 *)panel + file;
                other = (u8 *)panel + last;
                for (i = 0; i < SIZE_FILENT; i++) {
                    c = one[i];
                    one[i] = other[i];
                    other[i] = c;
                }
                ENTRY(panel, last)->AttrF &= 0x3F;
                last -= SIZE_FILENT;
            }
        }
    }
    panel->EndBuf = file;
}

/* Whether two files of a list are in the wrong order, `first` being the
   one that comes first now: 1 if they are, 0 if not. Their names are in
   the form that Sort gives them, the extension at 9.

   Unsorted is the order in which they were read. Otherwise directories
   come before files, with . and .. before the others, and system files
   before other files; then it goes by the extension, by the time, the
   latest first, or by the size, the largest first; and with those the
   same, or sorting by name, by the name and then the extension. */
static u16 CmpPar(const struct FILENT *first,
                  const struct FILENT *second, u16 type)
{
    u16 i;

    if (type >= 4)
        return second->NumbF < first->NumbF;
    if (first->AttrF & ATTR_DIRECTORY) {
        if (!(second->AttrF & ATTR_DIRECTORY))
            return 0;
        if (first->NameF[0] == '.')
            return 0;
        if (second->NameF[0] == '.')
            return 1;
    } else {
        if (second->AttrF & ATTR_DIRECTORY)
            return 1;
        if (first->AttrF & ATTR_SYSTEM) {
            if (!(second->AttrF & ATTR_SYSTEM))
                return 0;
        } else if (second->AttrF & ATTR_SYSTEM)
            return 1;
    }
    if (type == 1) {
        for (i = 9; i < 12; i++)
            if (second->NameF[i] != first->NameF[i])
                return second->NameF[i] < first->NameF[i];
    } else if (type == 2) {
        if (first->DateF != second->DateF)
            return first->DateF < second->DateF;
        if (first->TimeF != second->TimeF)
            return first->TimeF < second->TimeF;
    } else if (type == 3) {
        if (first->SizeF != second->SizeF)
            return first->SizeF < second->SizeF;
    }
    for (i = 0; i < 12; i++)
        if (i != 8 && second->NameF[i] != first->NameF[i])
            return second->NameF[i] < first->NameF[i];
    return 0;
}

/* Put the files that a panel shows into the order it shows them in.

   For that every name is first laid out in columns, the name filled up
   with spaces to eight characters and the extension at 9, and afterwards
   made a name again. A name of twelve characters is laid out like that
   already and is left alone, as are . and .. and a name without a full
   stop.

   The list is taken to have a file in it: with none, the entry at its
   start is laid out and made a name of all the same. And two entries
   change places by way of the room behind the last file of the
   directory, at EndFul. */
void Sort(struct WCBdef *panel)
{
    u8 *name;
    u8 *other;
    u8 *spare;
    u16 file, gap, end, swapped, length, dot, count, i;
    u8 c, c0, c1, c2;

    file = DSKBUF;
    do {
        name = (u8 *)panel + file;
        for (length = 0; length < 13 && name[length] != 0; length++)
            ;
        if (length < 12) {
            for (i = length; i < 12; i++)
                name[i] = ' ';
            if (name[0] != '.') {
                for (dot = 0; dot < length && name[dot] != '.'; dot++)
                    ;
                if (dot < length) {
                    c0 = name[dot + 1];
                    c1 = name[dot + 2];
                    c2 = name[dot + 3];
                    name[9] = c0;
                    name[10] = c1;
                    name[11] = c2;
                    /* Spaces from the full stop to the eighth character.
                       A full stop behind the ninth, which DOS has in no
                       name, would make that most of the segment. */
                    count = 8 - dot;
                    if (count != 0) {
                        other = name + dot;
                        do
                            *other++ = ' ';
                        while (--count);
                    }
                }
            }
        }
        file += SIZE_FILENT;
    } while (file < panel->EndBuf);

    /* A sort by exchanging entries a gap apart, the gap halved each time
       round. Behind the last exchange of a pass the list is in order, so
       the next pass ends there. */
    gap = panel->EndBuf - DSKBUF;
    for (;;) {
        gap = gap / SIZE_FILENT >> 1;
        gap = (gap << 4) + (gap << 3);  /* times the size of an entry */
        if (gap == 0)
            break;
        end = panel->EndBuf;
        do {
            swapped = 0;
            for (file = DSKBUF; (u16)(file + gap) < end;
                 file += SIZE_FILENT) {
                if (!CmpPar(ENTRY(panel, file), ENTRY(panel, file + gap),
                            panel->SortTyp))
                    continue;
                swapped = file + gap;
                name = (u8 *)panel + file;
                other = (u8 *)panel + swapped;
                spare = (u8 *)panel + panel->EndFul;
                for (i = 0; i < SIZE_FILENT; i++) {
                    c = name[i];
                    spare[i] = c;
                    name[i] = other[i];
                    other[i] = c;
                }
            }
            end = swapped;
        } while (swapped != 0);
    }

    file = DSKBUF;
    do {
        name = (u8 *)panel + file;
        length = 0;
        for (i = 0; i < 8; i++)
            if (name[i] != ' ')
                length = i + 1;
        c0 = name[9];
        c1 = name[10];
        c2 = name[11];
        name += length;
        name[0] = '.';
        name[1] = c0;
        name[2] = c1;
        name[3] = c2;
        length = 0;
        for (i = 1; i < 4; i++)
            if (name[i] != ' ')
                length = i + 1;
        name[length] = 0;
        file += SIZE_FILENT;
    } while (file < panel->EndBuf);
}

/* Find a file in the list that a panel shows, by its name. Returns the
   address of its entry, or one that is not below EndBuf if it is not
   there. */
u16 FnFile(struct WCBdef *panel, const u8 *name)
{
    const u8 *entry;
    u16 file, i;

    for (file = DSKBUF; file < panel->EndBuf; file += SIZE_FILENT) {
        entry = (u8 *)panel + file;
        for (i = 0; entry[i] == name[i]; i++)
            if (entry[i] == 0)
                return file;
    }
    return file;
}

/* Find the file of a panel's list that is nearest to one of the list it
   had before, which may be gone. `name` is that file's entry in the old
   list, which is at the start of its segment and ends at `end`. Looked
   for are that file and those behind it, in their order, and then those
   before it, the nearest first. Returns the address of the entry found,
   or 0 if there is none or `name` is not in the old list.

   The old list is where ReDr50 of read.c has it, the one routine that
   calls this: where the room begins, Room of vc.h. On DOS the
   pointer says so itself, and its offset is where the entry is in the
   list. */
u16 NearFl(struct WCBdef *panel, const u8 *name, u16 end)
{
    u16 start = OFFSET_IN_SEG(SEG_OF(Room), name);
    const u8 *list = name - start;
    u16 at = start;
    u16 forward = 1;
    u16 found;

    if (at >= end)
        return 0;
    for (;;) {
        found = FnFile(panel, list + at);
        if (found < panel->EndBuf)
            return found;
        if (forward) {
            at += SIZE_FILENT;
            if (at >= end) {
                at = start;
                forward = 0;
            }
        } else {
            if (at < SIZE_FILENT)
                return 0;
            at -= SIZE_FILENT;
        }
    }
}

/* Count what is selected of the files that a panel shows: the files, and
   the directories. */
struct Selected TotSel(struct WCBdef *panel)
{
    struct Selected selected;
    u16 files = 0;
    u16 directories = 0;
    u16 file;
    u8 attributes;

    for (file = DSKBUF; file < panel->EndBuf; file += SIZE_FILENT) {
        attributes = ENTRY(panel, file)->AttrF;
        if (attributes & ATTR_SELECTED) {
            if (attributes & ATTR_DIRECTORY)
                directories++;
            else
                files++;
        }
    }
    selected.directories = directories;
    selected.files = files;
    return selected;
}
