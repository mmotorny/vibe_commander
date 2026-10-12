/*
 * The list of files of a panel: which of them are shown, in what order,
 * and where one of them is.
 *
 * Translated from VCSUB2.INC and VCSUB3.INC.
 *
 * A panel has a segment to itself, and its list is the file entries from
 * DskBuf on. Those up to EndBuf are shown; behind them, up to EndFul, are
 * the files of the directory that the panel's filter leaves out. A place
 * in the list is an address in the segment, as the assembly has it.
 *
 * Nothing here but SetMask touches the main data: FnFile and NearFl
 * take what they read as far pointers.
 */
#include "vc.h"

#define SIZE_FILENT sizeof(struct FILENT)

u16 TstExe(const u8 __far *name);
u32 TotSel(u16 panel);

/* How the assembly passes arguments to these, and what it expects back;
   SetMask, Sort, FnFile and NearFl are in vc.h. */
#pragma aux TstExe  "*_c" __far parm [dx si] value [ax] modify exact [ax]
#pragma aux TotSel  "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]

/* The extensions of programs. Names in a list are in small letters. */
static const u8 CODE Ext_S[9] = {
    'c', 'o', 'm', 'e', 'x', 'e', 'b', 'a', 't'
};

/* Whether a file is a program, by the three characters behind the first
   full stop of its name: 1 if it is, 0 if not. */
u16 TstExe(const u8 __far *name)
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
void SetMask(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u8 *mask = md.TmpBuf;
    u8 __far *one;
    u8 __far *other;
    u16 file, last, i;
    u8 attributes, c;

    for (i = 0; i < 11; i++)
        mask[i] = '?';
    if (wcb->DskMas[0] != 0)
        Convert(wcb->DskMas, mask);
    file = DSKBUF;
    last = wcb->EndFul;
    if (file < last) {
        last -= SIZE_FILENT;
        for (;;) {
            attributes = ENTRY(wcb, file)->AttrF;
            if ((wcb->Hidden != 0
                 || !(attributes & (ATTR_HIDDEN | ATTR_SYSTEM)))
                && ((attributes & ATTR_DIRECTORY)
                    || ((wcb->MasTyp == 0
                         || TstExe(ENTRY(wcb, file)->NameF))
                        && TstMas(ENTRY(wcb, file)->NameF, mask)))) {
                file += SIZE_FILENT;
                if (file > last)
                    break;
            } else {
                if (file == last)
                    break;
                one = (u8 __far *)wcb + file;
                other = (u8 __far *)wcb + last;
                for (i = 0; i < SIZE_FILENT; i++) {
                    c = one[i];
                    one[i] = other[i];
                    other[i] = c;
                }
                ENTRY(wcb, last)->AttrF &= 0x3F;
                last -= SIZE_FILENT;
            }
        }
    }
    wcb->EndBuf = file;
}

/* Whether two files of a list are in the wrong order, `first` being the
   one that comes first now: 1 if they are, 0 if not. Their names are in
   the form that Sort gives them, the extension at 9.

   Unsorted is the order in which they were read. Otherwise directories
   come before files, with . and .. before the others, and system files
   before other files; then it goes by the extension, by the time, the
   latest first, or by the size, the largest first; and with those the
   same, or sorting by name, by the name and then the extension. */
static u16 CmpPar(const struct FILENT __far *first,
                  const struct FILENT __far *second, u16 type)
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
void Sort(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u8 __far *name;
    u8 __far *other;
    u8 __far *spare;
    u16 file, gap, end, swapped, length, dot, count, i;
    u8 c, c0, c1, c2;

    file = DSKBUF;
    do {
        name = (u8 __far *)wcb + file;
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
    } while (file < wcb->EndBuf);

    /* A sort by exchanging entries a gap apart, the gap halved each time
       round. Behind the last exchange of a pass the list is in order, so
       the next pass ends there. */
    gap = wcb->EndBuf - DSKBUF;
    for (;;) {
        gap = gap / SIZE_FILENT >> 1;
        gap = (gap << 4) + (gap << 3);  /* times the size of an entry */
        if (gap == 0)
            break;
        end = wcb->EndBuf;
        do {
            swapped = 0;
            for (file = DSKBUF; (u16)(file + gap) < end;
                 file += SIZE_FILENT) {
                if (!CmpPar(ENTRY(wcb, file), ENTRY(wcb, file + gap),
                            wcb->SortTyp))
                    continue;
                swapped = file + gap;
                name = (u8 __far *)wcb + file;
                other = (u8 __far *)wcb + swapped;
                spare = (u8 __far *)wcb + wcb->EndFul;
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
        name = (u8 __far *)wcb + file;
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
    } while (file < wcb->EndBuf);
}

/* Find a file in the list that a panel shows, by its name. Returns the
   address of its entry, or one that is not below EndBuf if it is not
   there. */
u16 FnFile(u16 panel, const u8 __far *name)
{
    struct WCBdef __far *wcb = PANEL(panel);
    const u8 __far *entry;
    u16 file, i;

    for (file = DSKBUF; file < wcb->EndBuf; file += SIZE_FILENT) {
        entry = (u8 __far *)wcb + file;
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
   or 0 if there is none or `name` is not in the old list. */
u16 NearFl(u16 panel, const u8 __far *name, u16 end)
{
    u16 start = (u16)(u32)name;
    const u8 __far *list = name - start;
    u16 at = start;
    u16 forward = 1;
    u16 found;

    if (at >= end)
        return 0;
    for (;;) {
        found = FnFile(panel, list + at);
        if (found < PANEL(panel)->EndBuf)
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

/* Count what is selected of the files that a panel shows: the files in
   the high word, the directories in the low one. */
u32 TotSel(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u16 files = 0;
    u16 directories = 0;
    u16 file;
    u8 attributes;

    for (file = DSKBUF; file < wcb->EndBuf; file += SIZE_FILENT) {
        attributes = ENTRY(wcb, file)->AttrF;
        if (attributes & ATTR_SELECTED) {
            if (attributes & ATTR_DIRECTORY)
                directories++;
            else
                files++;
        }
    }
    return (u32)files << 16 | directories;
}
