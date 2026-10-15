/*
 * Drives and paths of the program for Linux: the file of settings, a
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
 * The file of settings.
 */

bool SettingsFile(const char *named, const char *config_home,
                  const char *home, char *to, i32 room)
{
    const char *base;
    const char *behind;
    size_t length;
    size_t more;

    if (named != NULL && named[0] != '\0') {
        base = named;
        behind = "";
    } else if (config_home != NULL && config_home[0] == '/') {
        base = config_home;
        behind = "/vc/settings";
    } else if (home != NULL && home[0] != '\0') {
        base = home;
        behind = "/.config/vc/settings";
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

bool SettingsBegin(struct Settings *settings)
{
    i32 i;

    for (i = 0; i < DOS_DRIVES; i++)
        settings->drive[i] = NULL;
    settings->code_page = 0;
    settings->drive['C' - 'A'] = Copy("/", 1);
    return settings->drive['C' - 'A'] != NULL;
}

void SettingsEnd(struct Settings *settings)
{
    i32 i;

    for (i = 0; i < DOS_DRIVES; i++) {
        free(settings->drive[i]);
        settings->drive[i] = NULL;
    }
}

static bool Spacing(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

/* The word of `length` bytes at `text` is `word`. */
static bool Is(const char *text, size_t length, const char *word)
{
    return length == strlen(word) && memcmp(text, word, length) == 0;
}

/* One line of the file, of `length` bytes at `line`: 1 where it is
   understood, 0 where it is not, -1 where the memory ran out. */
static i32 Setting(struct Settings *settings, const char *line,
                   size_t length)
{
    const char *end = line + length;
    const char *word;
    size_t size;
    char letter;
    char *directory;

    while (line < end && Spacing(*line))
        line++;
    while (end > line && Spacing(end[-1]))
        end--;
    if (line == end || *line == '#')
        return 1;
    for (word = line; line < end && !Spacing(*line); line++)
        ;
    size = (size_t)(line - word);
    while (line < end && Spacing(*line))
        line++;
    if (Is(word, size, "codepage")) {
        if (Is(line, (size_t)(end - line), "437"))
            settings->code_page = CODE_PAGE_US;
        else if (Is(line, (size_t)(end - line), "866"))
            settings->code_page = CODE_PAGE_RUSSIA;
        else
            return 0;
        return 1;
    }
    if (!Is(word, size, "mount") || line == end)
        return 0;
    letter = *line++;
    if (letter >= 'a' && letter <= 'z')
        letter = (char)(letter - ('a' - 'A'));
    if (letter < 'A' || letter > 'Z')
        return 0;
    if (line < end && *line == ':')
        line++;
    if (line == end || !Spacing(*line))
        return 0;
    while (line < end && Spacing(*line))
        line++;
    if (line == end || *line != '/')
        return 0;
    while (end - line > 1 && end[-1] == '/')
        end--;
    directory = Copy(line, (size_t)(end - line));
    if (directory == NULL)
        return -1;
    free(settings->drive[letter - 'A']);
    settings->drive[letter - 'A'] = directory;
    return 1;
}

i32 SettingsRead(struct Settings *settings, const char *text)
{
    const char *end;
    i32 number;
    i32 understood;

    for (number = 1;; number++) {
        end = strchr(text, '\n');
        if (end == NULL)
            end = text + strlen(text);
        understood = Setting(settings, text, (size_t)(end - text));
        if (understood <= 0)
            return understood < 0 ? -1 : number;
        if (*end == '\0')
            return 0;
        text = end + 1;
    }
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
static i32 PathName(u16 code_page, const u8 *name, i32 length, u8 *to,
                    i32 at)
{
    u8 base[8];
    i32 based = 0;
    u8 extension[3];
    i32 extended = 0;
    bool dot = false;
    u8 byte;
    i32 i;

    for (i = 0; i < length; i++) {
        byte = CodePageUpper(code_page, name[i]);
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
static i32 PathNames(u16 code_page, const u8 *path, u8 *to, i32 at,
                     i32 root)
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
        at = PathName(code_page, name, length, to, at);
        if (at < 0)
            return -1;
    }
    return at;
}

struct Answer DosPathFull(u16 code_page, const u8 *path, u16 drive,
                          const u8 *const *current, u8 *to)
{
    u8 letter;
    i32 at;

    letter = (u8)('A' + (drive & 0xFF));
    if (path[0] != '\0' && path[1] == ':') {
        letter = CodePageUpper(code_page, path[0]);
        path += 2;
    }
    if (letter < 'A' || letter > 'Z')
        return Fails(INVALID_DRIVE);
    to[0] = letter;
    to[1] = ':';
    at = 2;
    if (!Separator(path[0]) && current != NULL
        && current[letter - 'A'] != NULL) {
        at = PathNames(code_page, current[letter - 'A'], to, at, 2);
        if (at < 0)
            return Fails(PATH_NOT_FOUND);
    }
    at = PathNames(code_page, path, to, at, 2);
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

void DirectoriesBegin(struct Directories *directories, u16 code_page,
                      DirectoryRead *read, void *context)
{
    directories->directories = NULL;
    directories->count = 0;
    directories->room = 0;
    directories->code_page = code_page;
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

/* The names of a directory of the host, read where it is new or where
   `again` says so: NULL where it cannot be read, with errno, or where
   the memory ran out, with errno ENOMEM. */
static struct DirectoryNames *Names(struct Directories *directories,
                                    const char *path, bool again)
{
    struct Directory *directory = NULL;
    struct Directory *grown;
    i32 room;
    i32 i;

    for (i = 0; i < directories->count; i++)
        if (strcmp(directories->directories[i].path, path) == 0) {
            directory = &directories->directories[i];
            break;
        }
    if (directory == NULL) {
        if (directories->count == directories->room) {
            room = directories->room == 0 ? 16 : 2 * directories->room;
            grown = realloc(directories->directories,
                            (size_t)room * sizeof *grown);
            if (grown == NULL) {
                errno = ENOMEM;
                return NULL;
            }
            directories->directories = grown;
            directories->room = room;
        }
        assert(directories->count >= 0
               && directories->count < directories->room);
        directory = &directories->directories[directories->count];
        directory->path = Copy(path, strlen(path));
        if (directory->path == NULL) {
            errno = ENOMEM;
            return NULL;
        }
        DirectoryNamesBegin(&directory->names);
        if (!directories->read(directories->context, path,
                               &directory->names, directories->code_page)) {
            DirectoryNamesEnd(&directory->names);
            free(directory->path);
            return NULL;
        }
        directories->count++;
        return &directory->names;
    }
    if (again && !directories->read(directories->context, path,
                                    &directory->names,
                                    directories->code_page))
        return NULL;
    return &directory->names;
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
                            const struct Settings *settings,
                            const u8 *path, char *to, i32 room)
{
    const char *drive;
    struct DirectoryNames *names;
    u8 name[DOS_NAME_ROOM];
    char written[4 * DOS_NAME_ROOM];
    const char *host;
    const u8 *end;
    i32 length;
    i32 at;
    bool found = true;

    if (path[0] < 'A' || path[0] > 'Z' || path[1] != ':' || path[2] != '\\')
        return Fails(INVALID_DRIVE);
    drive = settings->drive[path[0] - 'A'];
    if (drive == NULL)
        return Fails(INVALID_DRIVE);
    at = HostJoin(to, 0, room, drive, (i32)strlen(drive));
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
            if (CodePageToUtf8(directories->code_page, name, length, true,
                               written, (i32)sizeof written) < 0)
                return Fails(PATH_NOT_FOUND);
            host = written;
        }
        at = HostJoin(to, at, room, host, (i32)strlen(host));
        if (at < 0)
            return Fails(PATH_NOT_FOUND);
    }
    return Does(found ? 1 : 0);
}

/* Whether the directory of a drive holds a path of the host: answers
   how many bytes of the path it is, or -1 where it does not hold it. */
static i32 Holds(const char *drive, const char *path)
{
    size_t length = strlen(drive);

    if (strcmp(drive, "/") == 0)
        return path[0] == '/' ? 1 : -1;
    if (strncmp(drive, path, length) != 0
        || (path[length] != '\0' && path[length] != '/'))
        return -1;
    return (i32)length;
}

struct Answer DosPathOfHost(struct Directories *directories,
                            const struct Settings *settings,
                            const char *path, u8 *to)
{
    i32 best = -1;
    i32 letter = 0;
    i32 held;
    i32 i;
    char directory[4096];
    i32 at;
    i32 written;
    const char *name;
    const char *end;
    struct DirectoryNames *names;
    const u8 *dos;
    i32 length;
    char *part;

    for (i = 0; i < DOS_DRIVES; i++) {
        if (settings->drive[i] == NULL)
            continue;
        held = Holds(settings->drive[i], path);
        if (held > best) {
            best = held;
            letter = i;
        }
    }
    if (best < 0)
        return Fails(PATH_NOT_FOUND);
    at = HostJoin(directory, 0, (i32)sizeof directory,
                  settings->drive[letter],
                  (i32)strlen(settings->drive[letter]));
    if (at < 0)
        return Fails(PATH_NOT_FOUND);
    to[0] = (u8)('A' + letter);
    to[1] = ':';
    to[2] = '\\';
    written = 3;
    for (name = path + best; *name != '\0'; name = end) {
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
