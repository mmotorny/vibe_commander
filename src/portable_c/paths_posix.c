/*
 * Drives and paths of the program for Linux: where the setup file is, a
 * path of DOS's made a full one, and a full path of DOS's to a path of
 * the host and back. See emulate_posix.h.
 */
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "emulate_posix.h"

/* DOS's errors that these answer with. */
#define PATH_NOT_FOUND 0x03
#define NO_MEMORY 0x08
#define INVALID_DRIVE 0x0F

static struct Answer Fails(u16 error)
{
    struct Answer answer;

    answer.value = error;
    answer.failed = true;
    return answer;
}

static struct Answer Does(u16 value)
{
    struct Answer answer;

    answer.value = value;
    answer.failed = false;
    return answer;
}

/* A copy of `length` bytes at `text`, with a zero behind it. */
static char *Copy(const char *text, size_t length)
{
    char *copy = malloc(length + 1);

    if (copy != NULL) {
        memcpy(copy, text, length);
        copy[length] = '\0';
    }
    return copy;
}

/*
 * The setup file.
 */

bool SetupFile(const char *config_home, const char *home, char *to,
               i32 room)
{
    const char *base;
    const char *behind;
    size_t length;
    size_t more;

    if (config_home != NULL && config_home[0] == '/') {
        base = config_home;
        behind = "/vc/vc.ini";
    } else if (home != NULL && home[0] != '\0') {
        base = home;
        behind = "/.config/vc/vc.ini";
    } else
        return false;
    length = strlen(base);
    more = strlen(behind);
    if (room <= 0 || length + more >= (size_t)room)
        return false;
    memcpy(to, base, length);
    memcpy(to + length, behind, more + 1);
    return true;
}

/*
 * The drives.
 */

/* Where each is in the table, by its letter: R: is always there. */
#define HOME 0
#define ROOT 1

/* One drive, its current directory the root. */
static void DriveMake(struct DosDrive *drive, u8 letter, const char *label,
                      const char *root)
{
    size_t length = strlen(root);

    assert(length < sizeof drive->root);
    drive->letter = letter;
    drive->label = label;
    memcpy(drive->root, root, length + 1);
    drive->current[0] = '\0';
}

void DrivesBegin(struct Drives *drives, const char *home)
{
    memset(drives, 0, sizeof *drives);
    if (home != NULL && strlen(home) < sizeof drives->drive[0].root)
        DriveMake(&drives->drive[HOME], DOS_HOME_DRIVE, "HOME", home);
    DriveMake(&drives->drive[ROOT], DOS_ROOT_DRIVE, "ROOT", "/");
    drives->current = ROOT;
}

void DrivesBeginGiven(struct Drives *drives, const struct DriveGiven *given,
                      i32 count)
{
    i32 i;

    memset(drives, 0, sizeof *drives);
    for (i = 0; i < count && i < DOS_DRIVES; i++)
        DriveMake(&drives->drive[i], given[i].letter, given[i].label,
                  given[i].root);
    drives->current = 0;
}

i32 DriveIndex(const struct Drives *drives, u8 letter)
{
    i32 i;

    for (i = 0; i < DOS_DRIVES; i++)
        if (drives->drive[i].letter != 0 && drives->drive[i].letter == letter)
            return i;
    return -1;
}

/*
 * A path of DOS's made a full one.
 */

static bool Separator(u8 byte)
{
    return byte == '\\' || byte == '/';
}

/* One name of a path, `length` bytes at `name`, in capitals and cut to
   8.3, behind what `to` has, `at` bytes, with a backslash in front:
   answers how many bytes `to` has then, or -1 for a name that DOS does
   not take. * and ? are taken, since a path may be one to look for. */
static i32 PathName(const u8 *name, i32 length, u8 *to, i32 at)
{
    u8 base[8];
    i32 based = 0;
    u8 extension[3];
    i32 extended = 0;
    bool dot = false;
    u8 byte;
    i32 i;

    for (i = 0; i < length; i++) {
        byte = CodePageUpper(name[i]);
        if (byte == '.') {
            if (dot)
                return -1;              /* DOSBox: a second dot */
            dot = true;
        } else if (!DosNameByte(byte) && byte != '*' && byte != '?')
            return -1;
        else if (!dot) {
            if (based < 8)
                base[based++] = byte;
        } else if (extended < 3)
            extension[extended++] = byte;
    }
    if (based == 0)
        return -1;
    if (at + 1 + based + 1 + extended >= DOS_PATH_ROOM)
        return -1;
    to[at++] = '\\';
    memcpy(to + at, base, (size_t)based);
    at += based;
    if (extended > 0) {
        to[at++] = '.';
        memcpy(to + at, extension, (size_t)extended);
        at += extended;
    }
    return at;
}

/* The names of a path, `path` up to its zero, behind what `to` has,
   with . and .. worked out: answers how many bytes `to` has then, or -1
   for a name that DOS does not take or a path that is too long. `root`
   is how many bytes of `to` are the drive and are never gone back over. */
static i32 PathNames(const u8 *path, u8 *to, i32 at, i32 root)
{
    const u8 *name;
    i32 length;

    while (*path != '\0') {
        while (Separator(*path))
            path++;
        for (name = path; *path != '\0' && !Separator(*path); path++)
            ;
        length = (i32)(path - name);
        if (length == 0 || (length == 1 && name[0] == '.'))
            continue;
        if (length == 2 && name[0] == '.' && name[1] == '.') {
            /* DOSBox: one name back, and the root stays the root. */
            while (at > root && to[at - 1] != '\\')
                at--;
            if (at > root)
                at--;
            continue;
        }
        at = PathName(name, length, to, at);
        if (at < 0)
            return -1;
    }
    return at;
}

struct Answer DosPathFull(const u8 *path, const struct Drives *drives,
                          u8 *to)
{
    u8 letter = drives->drive[drives->current].letter;
    i32 drive;
    i32 at;

    if (path[0] != '\0' && path[1] == ':') {
        letter = CodePageUpper(path[0]);
        path += 2;
    }
    if (letter < 'A' || letter > 'Z')
        return Fails(INVALID_DRIVE);
    to[0] = letter;
    to[1] = ':';
    at = 2;
    drive = DriveIndex(drives, letter);
    if (!Separator(path[0]) && drive >= 0) {
        at = PathNames(drives->drive[drive].current, to, at, 2);
        if (at < 0)
            return Fails(PATH_NOT_FOUND);
    }
    at = PathNames(path, to, at, 2);
    if (at < 0)
        return Fails(PATH_NOT_FOUND);
    if (at == 2)
        to[at++] = '\\';
    to[at] = '\0';
    return Does(0);
}

/*
 * The directories of the host and their names.
 */

void DirectoriesBegin(struct Directories *directories, DirectoryRead *read,
                      void *context)
{
    directories->directories = NULL;
    directories->count = 0;
    directories->room = 0;
    directories->read = read;
    directories->context = context;
}

void DirectoriesEnd(struct Directories *directories)
{
    i32 i;

    for (i = 0; i < directories->count; i++) {
        free(directories->directories[i].path);
        DirectoryNamesEnd(&directories->directories[i].names);
    }
    free(directories->directories);
    directories->directories = NULL;
    directories->count = 0;
    directories->room = 0;
}

i32 DirectoryNumber(struct Directories *directories, const char *path,
                    bool again)
{
    struct Directory *directory;
    struct Directory *grown;
    i32 room;
    i32 i;

    for (i = 0; i < directories->count; i++)
        if (strcmp(directories->directories[i].path, path) == 0) {
            if (again
                && !directories->read(directories->context, path,
                                      &directories->directories[i].names))
                return -1;
            return i;
        }
    if (directories->count == directories->room) {
        room = directories->room == 0 ? 16 : 2 * directories->room;
        grown = realloc(directories->directories,
                        (size_t)room * sizeof *grown);
        if (grown == NULL) {
            errno = ENOMEM;
            return -1;
        }
        directories->directories = grown;
        directories->room = room;
    }
    assert(directories->count >= 0 && directories->count < directories->room);
    directory = &directories->directories[directories->count];
    directory->path = Copy(path, strlen(path));
    if (directory->path == NULL) {
        errno = ENOMEM;
        return -1;
    }
    DirectoryNamesBegin(&directory->names);
    if (!directories->read(directories->context, path, &directory->names)) {
        DirectoryNamesEnd(&directory->names);
        free(directory->path);
        return -1;
    }
    return directories->count++;
}

/* The names of a directory of the host, read where it is new or where
   `again` says so: NULL where it cannot be read, with errno, or where
   the memory ran out, with errno ENOMEM. */
static struct DirectoryNames *Names(struct Directories *directories,
                                    const char *path, bool again)
{
    i32 number = DirectoryNumber(directories, path, again);

    return number < 0 ? NULL : &directories->directories[number].names;
}

/* `length` bytes at `name` behind the path of the host at `to`, which
   has `at` bytes and room for `room`, with a slash between: how many
   bytes `to` has then, or -1 where the room is not enough. */
static i32 HostJoin(char *to, i32 at, i32 room, const char *name,
                    i32 length)
{
    if (at > 0 && to[at - 1] != '/') {
        if (at + 1 >= room)
            return -1;
        to[at++] = '/';
    }
    if (at + length >= room)
        return -1;
    memcpy(to + at, name, (size_t)length);
    at += length;
    to[at] = '\0';
    return at;
}

struct Answer HostPathOfDos(struct Directories *directories,
                            const struct Drives *drives, const u8 *path,
                            char *to, i32 room)
{
    struct DirectoryNames *names;
    u8 name[DOS_NAME_ROOM];
    char written[4 * DOS_NAME_ROOM];
    const char *host;
    const u8 *end;
    i32 length;
    i32 at;
    bool found = true;
    i32 drive = DriveIndex(drives, path[0]);
    const char *root;

    if (drive < 0 || path[1] != ':' || path[2] != '\\')
        return Fails(INVALID_DRIVE);
    root = drives->drive[drive].root;
    at = HostJoin(to, 0, room, root, (i32)strlen(root));
    if (at < 0)
        return Fails(PATH_NOT_FOUND);
    for (path += 3; *path != '\0'; path = *end == '\0' ? end : end + 1) {
        if (!found)
            return Fails(PATH_NOT_FOUND);
        end = (const u8 *)strchr((const char *)path, '\\');
        if (end == NULL)
            end = path + strlen((const char *)path);
        length = (i32)(end - path);
        if (length >= DOS_NAME_ROOM)
            return Fails(PATH_NOT_FOUND);
        memcpy(name, path, (size_t)length);
        name[length] = '\0';
        names = Names(directories, to, false);
        host = names == NULL ? NULL : DirectoryNamesHost(names, name);
        if (names != NULL && host == NULL) {
            names = Names(directories, to, true);
            host = names == NULL ? NULL : DirectoryNamesHost(names, name);
        }
        if (names == NULL)
            return Fails(errno == ENOMEM ? NO_MEMORY : PATH_NOT_FOUND);
        if (host == NULL) {
            /* Not there: the name that the program makes, small. */
            found = false;
            if (CodePageToUtf8(name, length, true, written,
                               (i32)sizeof written) < 0)
                return Fails(PATH_NOT_FOUND);
            host = written;
        }
        at = HostJoin(to, at, room, host, (i32)strlen(host));
        if (at < 0)
            return Fails(PATH_NOT_FOUND);
    }
    return Does(found ? 1 : 0);
}

/* Where a path of the host goes on behind `root`, both from the host's
   root, name by name, so that a slash more or less makes no difference:
   the rest of the path, or NULL where it is neither `root` nor a path
   under it. */
static const char *Behind(const char *root, const char *path)
{
    size_t length;

    for (;;) {
        while (*root == '/')
            root++;
        while (*path == '/')
            path++;
        if (*root == '\0')
            return path;
        length = strcspn(root, "/");
        if (strncmp(root, path, length) != 0
            || (path[length] != '\0' && path[length] != '/'))
            return NULL;
        root += length;
        path += length;
    }
}

struct Answer DosPathOfHost(struct Directories *directories,
                            const struct Drives *drives, const char *path,
                            u8 *to)
{
    char directory[HOST_PATH_ROOM];
    i32 at;
    i32 written;
    const char *name;
    const char *end;
    struct DirectoryNames *names;
    const u8 *dos;
    i32 length;
    char *part;
    const struct DosDrive *drive = NULL;
    const char *rest = NULL;
    i32 i;

    if (path[0] != '/')
        return Fails(PATH_NOT_FOUND);
    /* The first drive in the table that the path is under: on Linux H:
       before R:, the root, which every path is under. */
    for (i = 0; i < DOS_DRIVES && rest == NULL; i++)
        if (drives->drive[i].letter != 0) {
            drive = &drives->drive[i];
            rest = Behind(drive->root, path);
        }
    if (rest == NULL)
        return Fails(PATH_NOT_FOUND);
    at = HostJoin(directory, 0, (i32)sizeof directory, drive->root,
                  (i32)strlen(drive->root));
    if (at < 0)
        return Fails(PATH_NOT_FOUND);
    to[0] = drive->letter;
    to[1] = ':';
    to[2] = '\\';
    written = 3;
    for (name = rest; *name != '\0'; name = end) {
        while (*name == '/')
            name++;
        for (end = name; *end != '\0' && *end != '/'; end++)
            ;
        length = (i32)(end - name);
        if (length == 0)
            continue;
        part = Copy(name, (size_t)length);
        if (part == NULL)
            return Fails(NO_MEMORY);
        if (strcmp(part, ".") == 0 || strcmp(part, "..") == 0) {
            free(part);
            return Fails(PATH_NOT_FOUND);
        }
        names = Names(directories, directory, false);
        dos = names == NULL ? NULL : DirectoryNamesDos(names, part);
        if (names != NULL && dos == NULL) {
            names = Names(directories, directory, true);
            dos = names == NULL ? NULL : DirectoryNamesDos(names, part);
        }
        if (names == NULL || dos == NULL) {
            free(part);
            return Fails(names == NULL && errno == ENOMEM ? NO_MEMORY
                                                          : PATH_NOT_FOUND);
        }
        at = HostJoin(directory, at, (i32)sizeof directory, part, length);
        free(part);
        if (at < 0)
            return Fails(PATH_NOT_FOUND);
        length = (i32)strlen((const char *)dos);
        if (written > 3)
            to[written++] = '\\';
        if (written + length > DOS_PATH_LONGEST)
            return Fails(PATH_NOT_FOUND);
        memcpy(to + written, dos, (size_t)length);
        written += length;
    }
    to[written] = '\0';
    return Does(0);
}
