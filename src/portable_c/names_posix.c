/*
 * Names of DOS's form for the names of the host, and DOS's attributes
 * of a file of the host. See emulate_posix.h.
 */
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "emulate_posix.h"

/* What DOS does not let a name have beside the bytes 00h to 20h: the
   bytes of FreeDOS 1.4's table of what ends a name, "FCHAR" of its
   COUNTRY.SYS, for the code page 437, and the two that match. The dot is among them, and is a name's own where it parts the
   extension from the rest. */
static const char NotInName[] = ".\"/\\[]:|<>+=;,*?";

bool DosNameByte(u8 byte)
{
    return byte > 0x20 && strchr(NotInName, byte) == NULL;
}

/* Whether `count` bytes at `name` are a part of a name in capitals. */
static bool Capitals(const u8 *name, i32 count)
{
    i32 i;

    for (i = 0; i < count; i++)
        if (!DosNameByte(name[i])
            || CodePageUpper(name[i]) != name[i])
            return false;
    return true;
}

bool DosNameValid(const u8 *name)
{
    const u8 *dot = (const u8 *)strchr((const char *)name, '.');
    i32 length = (i32)strlen((const char *)name);
    i32 base;

    if (dot == NULL)
        return length >= 1 && length <= 8 && Capitals(name, length);
    base = (i32)(dot - name);
    return base >= 1 && base <= 8 && length - base - 1 >= 1
        && length - base - 1 <= 3 && Capitals(name, base)
        && Capitals(dot + 1, length - base - 1);
}

u8 DosAttributes(const struct stat *status, const char *name, uid_t user,
                 bool member)
{
    u8 attributes = 0;
    mode_t writable;

    if (S_ISDIR(status->st_mode))
        attributes |= DOS_DIRECTORY;
    else {
        attributes |= DOS_ARCHIVE;
        /* A directory is not made read-only: DOS lets a file be made in
           a directory that has the attribute, and a directory of the
           host that the program may not write would say otherwise. */
        if (user == 0)
            writable = S_IWUSR | S_IWGRP | S_IWOTH;
        else if (status->st_uid == user)
            writable = S_IWUSR;
        else if (member)
            writable = S_IWGRP;
        else
            writable = S_IWOTH;
        if ((status->st_mode & writable) == 0)
            attributes |= DOS_READ_ONLY;
    }
    if (name[0] == '.' && strcmp(name, ".") != 0 && strcmp(name, "..") != 0)
        attributes |= DOS_HIDDEN;
    return attributes;
}

/* The longest name of the host, in bytes, as Linux has it (NAME_MAX of
   <limits.h>); and so the most characters that a name can be. */
#define HOST_NAME_LONGEST 255

void DosNameMake(const char *host, DosNameTaken *taken, const void *context,
                 u8 *to)
{
    u8 characters[HOST_NAME_LONGEST + 1];
    i32 count = 0;                      /* of characters */
    i32 length = (i32)strlen(host);
    bool lossy = false;
    u8 base[8];
    i32 based = 0;
    u8 extension[3];
    i32 extended = 0;
    i32 first;                          /* the character behind the dots
                                           that the name begins with */
    i32 last = -1;                      /* the last dot */
    i32 at;
    i32 step;
    u32 character;
    i32 byte;
    i32 i;
    u32 tail;
    u8 number[12];
    i32 digits;
    i32 kept;

    /* In capitals and in the code page, with no spaces, and `_` for
       what it cannot have. */
    for (at = 0; at < length && count < HOST_NAME_LONGEST; at += step) {
        step = Utf8Decode(host + at, length - at, &character);
        if (step == 0) {
            step = 1;                   /* no UTF-8: one byte is `_` */
            byte = -1;
        } else
            byte = CodePageFromUnicode(character);
        if (byte < 0) {
            characters[count++] = '_';
            lossy = true;
            continue;
        }
        byte = CodePageUpper((u8)byte);
        if (byte == ' ') {
            lossy = true;
            continue;
        }
        if (byte != '.' && !DosNameByte((u8)byte)) {
            byte = '_';
            lossy = true;
        }
        characters[count++] = (u8)byte;
    }
    if (at < length)
        lossy = true;

    for (first = 0; first < count && characters[first] == '.'; first++)
        lossy = true;
    for (i = first; i < count; i++)
        if (characters[i] == '.')
            last = i;
    for (i = first; i < count && characters[i] != '.'; i++)
        if (based < 8)
            base[based++] = characters[i];
        else
            lossy = true;
    if (last >= 0) {
        if (i != last || last == count - 1)
            lossy = true;               /* dots within, or one at the end */
        for (i = last + 1; i < count; i++)
            if (extended < 3)
                extension[extended++] = characters[i];
            else
                lossy = true;
    }
    if (based == 0) {
        base[based++] = '_';
        lossy = true;
    }

    memcpy(to, base, (size_t)based);
    at = based;
    if (extended > 0) {
        to[at++] = '.';
        memcpy(to + at, extension, (size_t)extended);
        at += extended;
    }
    to[at] = '\0';
    if (!lossy && !taken(context, to))
        return;

    for (tail = 1;; tail++) {
        digits = 0;
        for (character = tail; character != 0; character /= 10)
            number[digits++] = (u8)('0' + character % 10);
        kept = based < 7 - digits ? based : 7 - digits;
        memcpy(to, base, (size_t)kept);
        at = kept;
        to[at++] = '~';
        while (digits > 0)
            to[at++] = number[--digits];
        if (extended > 0) {
            to[at++] = '.';
            memcpy(to + at, extension, (size_t)extended);
            at += extended;
        }
        to[at] = '\0';
        if (!taken(context, to))
            return;
    }
}

/*
 * The table of a directory. Its names are found by two indexes of
 * open addressing, by the name of the host and by the name of DOS's,
 * each with twice as many places as the table has room for names; a
 * place holds the number of a name and one more, or 0 for none.
 */

/* FNV-1a of a name, which is short: its product is cut to 32 bits, as
   FNV is meant, and is reckoned in 64 so that nothing wraps. */
static u32 Hash(const char *name)
{
    u32 hash = 2166136261U;

    for (; *name != '\0'; name++)
        hash = (u32)((uint64_t)(hash ^ (u8)*name) * 16777619U
                     & 0xFFFFFFFFU);
    return hash;
}

static i32 Mask(const struct DirectoryNames *table)
{
    return 2 * table->room - 1;
}

/* The place of a name in an index: where it is, or the empty place where
   it would go. */
static i32 PlaceOfHost(const struct DirectoryNames *table, const char *host)
{
    i32 place = (i32)(Hash(host) & (u32)Mask(table));

    while (table->by_host[place] != 0
           && strcmp(table->names[table->by_host[place] - 1].host, host)
                  != 0)
        place = (place + 1) & Mask(table);
    return place;
}

static i32 PlaceOfDos(const struct DirectoryNames *table, const u8 *dos)
{
    i32 place = (i32)(Hash((const char *)dos) & (u32)Mask(table));

    while (table->by_dos[place] != 0
           && strcmp((const char *)table->names[table->by_dos[place] - 1].dos,
                     (const char *)dos) != 0)
        place = (place + 1) & Mask(table);
    return place;
}

void DirectoryNamesBegin(struct DirectoryNames *table)
{
    table->names = NULL;
    table->by_host = NULL;
    table->by_dos = NULL;
    table->count = 0;
    table->room = 0;
}

void DirectoryNamesEnd(struct DirectoryNames *table)
{
    i32 i;

    for (i = 0; i < table->count; i++)
        free(table->names[i].host);
    free(table->names);
    free(table->by_host);
    free(table->by_dos);
    DirectoryNamesBegin(table);
}

/* Room for one name more, the indexes made anew where the room grows. */
static bool Grow(struct DirectoryNames *table)
{
    struct DirectoryName *names;
    i32 *by_host;
    i32 *by_dos;
    i32 room;
    i32 i;

    if (table->count < table->room)
        return true;
    room = table->room == 0 ? 16 : 2 * table->room;
    names = realloc(table->names, (size_t)room * sizeof *names);
    if (names == NULL)
        return false;
    table->names = names;
    by_host = calloc((size_t)room * 2, sizeof *by_host);
    by_dos = calloc((size_t)room * 2, sizeof *by_dos);
    if (by_host == NULL || by_dos == NULL) {
        free(by_host);
        free(by_dos);
        return false;
    }
    free(table->by_host);
    free(table->by_dos);
    table->by_host = by_host;
    table->by_dos = by_dos;
    table->room = room;
    for (i = 0; i < table->count; i++) {
        table->by_host[PlaceOfHost(table, table->names[i].host)] = i + 1;
        table->by_dos[PlaceOfDos(table, table->names[i].dos)] = i + 1;
    }
    return true;
}

static bool Taken(const void *context, const u8 *name)
{
    const struct DirectoryNames *table = context;

    return table->by_dos[PlaceOfDos(table, name)] != 0;
}

/* A name of the host, for sorting them. */
struct HostName {
    const char *name;
};

static int Order(const void *one, const void *other)
{
    const struct HostName *first = one;
    const struct HostName *second = other;

    return strcmp(first->name, second->name);
}

static bool Dots(const char *name)
{
    return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

/* A name of the host that the table has not, added with its name of
   DOS's form. */
static bool Add(struct DirectoryNames *table, const char *host)
{
    struct DirectoryName *name;
    size_t length = strlen(host) + 1;

    if (!Grow(table))
        return false;
    assert(table->count >= 0 && table->count < table->room);
    name = &table->names[table->count];
    name->host = malloc(length);
    if (name->host == NULL)
        return false;
    memcpy(name->host, host, length);
    DosNameMake(name->host, Taken, table, name->dos);
    name->present = true;
    table->count++;
    table->by_host[PlaceOfHost(table, name->host)] = table->count;
    table->by_dos[PlaceOfDos(table, name->dos)] = table->count;
    return true;
}

bool DirectoryNamesUpdate(struct DirectoryNames *table,
                          const char *const *names, i32 count)
{
    struct HostName *sorted;
    i32 found;
    bool added = true;
    i32 i;

    for (i = 0; i < table->count; i++)
        table->names[i].present = false;
    if (count <= 0)
        return true;
    sorted = malloc((size_t)count * sizeof *sorted);
    if (sorted == NULL)
        return false;
    for (i = 0; i < count; i++)
        sorted[i].name = names[i];
    qsort(sorted, (size_t)count, sizeof *sorted, Order);
    for (i = 0; i < count && added; i++) {
        if (Dots(sorted[i].name))
            continue;
        found = table->room == 0
            ? 0 : table->by_host[PlaceOfHost(table, sorted[i].name)];
        if (found != 0)
            table->names[found - 1].present = true;
        else
            added = Add(table, sorted[i].name);
    }
    free(sorted);
    return added;
}

static const u8 Dot[] = ".";
static const u8 DotDot[] = "..";

const u8 *DirectoryNamesDos(const struct DirectoryNames *table,
                            const char *host)
{
    i32 found;

    if (strcmp(host, ".") == 0)
        return Dot;
    if (strcmp(host, "..") == 0)
        return DotDot;
    if (table->room == 0)
        return NULL;
    found = table->by_host[PlaceOfHost(table, host)];
    if (found == 0 || !table->names[found - 1].present)
        return NULL;
    return table->names[found - 1].dos;
}

const char *DirectoryNamesHost(const struct DirectoryNames *table,
                               const u8 *dos)
{
    i32 found;

    if (strcmp((const char *)dos, ".") == 0)
        return ".";
    if (strcmp((const char *)dos, "..") == 0)
        return "..";
    if (table->room == 0)
        return NULL;
    found = table->by_dos[PlaceOfDos(table, dos)];
    if (found == 0 || !table->names[found - 1].present)
        return NULL;
    return table->names[found - 1].host;
}
