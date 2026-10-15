/*
 * A command of DOS's made one for the host's shell, the piece L6 of step
 * 6 (TODO.md, "L6, commands"). The program hands the shell what it hands
 * COMMAND.COM behind /C: a line of the code page, with paths of DOS's in
 * it. The shell is handed the same line in UTF-8, each path of DOS's that
 * begins with a drive that there is, R: or H:, made the path of the host
 * that it leads to (HostPathOfDos), quoted for the shell. See
 * emulate_posix.h for what each answers.
 */
#include <string.h>

#include "emulate_posix.h"

/* Where a path of DOS's in a command ends: a blank, a quote, or what the
   shell takes for the end of a word. */
static bool Ends(u8 byte)
{
    return byte == 0 || byte == ' ' || byte == '\t' || byte == '"'
        || byte == '\'' || strchr("<>|&;()", byte) != 0;
}

/* Where one may begin: at the start of the command, or behind what the
   shell takes for the end of a word, or behind = (VAR=H:\X). */
static bool Begins(const u8 *command, i32 at)
{
    return at == 0 || Ends(command[at - 1]) || command[at - 1] == '=';
}

/* A text of the host added at `*made`, as far as `room` holds it. */
static bool Add(char *to, i32 room, i32 *made, const char *text,
                i32 length)
{
    if (length >= room - *made)
        return false;
    memcpy(to + *made, text, (size_t)length);
    *made += length;
    to[*made] = 0;
    return true;
}

/* A path of the host added, quoted as the shell has it where it stands:
   in single quotes outside any, a ' in it as '\''; inside double quotes
   with \ in front of what is special there. */
static bool Quoted(char *to, i32 room, i32 *made, const char *path,
                   bool double_quoted)
{
    const char *at;

    if (!double_quoted && !Add(to, room, made, "'", 1))
        return false;
    for (at = path; *at != 0; at++) {
        if (!double_quoted && *at == '\'') {
            if (!Add(to, room, made, "'\\''", 4))
                return false;
            continue;
        }
        if (double_quoted && strchr("\\\"$`", *at) != 0
            && !Add(to, room, made, "\\", 1))
            return false;
        if (!Add(to, room, made, at, 1))
            return false;
    }
    return double_quoted || Add(to, room, made, "'", 1);
}

/* The path of DOS's of `length` bytes at `from` led to the host and added
   quoted, or false where it leads nowhere: then it is added as text. A
   name with * or ? in it, the last, is added behind its directory as it
   is, in small letters and not quoted, so that the shell takes it for a
   pattern. */
static bool Path(struct Dos *dos, const u8 *from, i32 length, char *to,
                 i32 room, i32 *made, bool double_quoted)
{
    u8 path[DOS_PATH_ROOM], full[DOS_PATH_ROOM];
    char host[HOST_PATH_ROOM], pattern[DOS_PATH_ROOM * 3];
    i32 last = length, i, written, hosted;
    struct Answer led;

    if (length >= DOS_PATH_ROOM)
        return false;
    for (i = 0; i < length; i++)
        if (from[i] == '*' || from[i] == '?')
            break;
    if (i < length) {
        while (last > 0 && from[last - 1] != '\\' && from[last - 1] != '/'
               && from[last - 1] != ':')
            last--;
        for (i = last; i < length; i++)
            if (from[i] == '\\' || from[i] == '/')
                return false;
    }
    memcpy(path, from, (size_t)last);
    path[last] = 0;
    led = DosPathFull(path, &dos->drives, full);
    if (led.failed)
        return false;
    led = HostPathOfDos(&dos->directories, &dos->drives, full, host,
                        (i32)sizeof host);
    if (led.failed)
        return false;
    hosted = (i32)strlen(host);
    if (last < length && strcmp(host, "/") != 0
        && !Add(host, (i32)sizeof host, &hosted, "/", 1))
        return false;
    if (!Quoted(to, room, made, host, double_quoted))
        return false;
    if (last == length)
        return true;
    written = CodePageToUtf8(from + last, length - last, true, pattern,
                             (i32)sizeof pattern);
    return written >= 0 && Add(to, room, made, pattern, written);
}

i32 DosCommandToHost(struct Dos *dos, const u8 *command, i32 length,
                     char *to, i32 room)
{
    char character[8];
    bool single = false, double_quoted = false;
    i32 made = 0, at = 0, end, written;
    i32 before;

    if (room < 1)
        return -1;
    to[0] = 0;
    while (at < length) {
        u8 byte = command[at];

        if (!single && Begins(command, at) && at + 1 < length
            && command[at + 1] == ':'
            && ((byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z'))
            && DriveIndex(&dos->drives, CodePageUpper(byte)) >= 0) {
            for (end = at; end < length && !Ends(command[end]); end++)
                ;
            before = made;
            if (Path(dos, command + at, end - at, to, room, &made,
                     double_quoted)) {
                at = end;
                continue;
            }
            made = before;
            to[made] = 0;
        }
        if (byte == '\'' && !double_quoted)
            single = !single;
        else if (byte == '"' && !single)
            double_quoted = !double_quoted;
        written = CodePageToUtf8(&byte, 1, false, character,
                                 (i32)sizeof character);
        if (written < 0 || !Add(to, room, &made, character, written))
            return -1;
        at++;
    }
    return made;
}

struct Answer DosCurrentHost(struct Dos *dos, char *to, i32 room)
{
    const struct DosDrive *drive = &dos->drives.drive[dos->drives.current];
    u8 full[DOS_PATH_ROOM];
    size_t length = strlen((const char *)drive->current);

    full[0] = drive->letter;
    full[1] = ':';
    full[2] = '\\';
    memcpy(full + 3, drive->current, length + 1);
    return HostPathOfDos(&dos->directories, &dos->drives, full, to, room);
}
