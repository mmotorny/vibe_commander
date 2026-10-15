/*
 * DOS's services for files, directories and drives on the host's files,
 * the piece L2 of step 6. See emulate_posix.h, which says what they are
 * and where what each answers comes from.
 */
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include "emulate_posix.h"

/* DOS's errors that these answer with. */
#define INVALID_FUNCTION 0x01
#define FILE_NOT_FOUND 0x02
#define PATH_NOT_FOUND 0x03
#define TOO_MANY_OPEN_FILES 0x04
#define ACCESS_DENIED 0x05
#define INVALID_HANDLE 0x06
#define NO_MEMORY 0x08
#define INVALID_ACCESS 0x0C
#define INVALID_DRIVE 0x0F
#define REMOVE_CURRENT 0x10
#define NOT_SAME_DEVICE 0x11
#define NO_MORE_FILES 0x12

/* A path of the host. */
#define HOST_ROOM HOST_PATH_ROOM

/* The most bytes that a file of DOS has, and the furthest place in one:
   its size is 32 bits. */
#define LARGEST 0xFFFFFFFFU

/* What DOS answers in AX where a call did not fail and has nothing of
   its own to say there: what the bench's DOS, FreeDOS 1.4, leaves, which
   is what it was handed, the number of the call (harness/trial32/
   asked.py). */
#define AX_CLOSE 0x3E00
#define AX_DELETE 0x4100
#define AX_RENAME 0x5600
#define AX_ATTRIBUTES 0x4300
#define AX_TIME 0x5700
#define AX_SET_TIME 0x5701
#define AX_DIR_MAKE 0x3900
#define AX_DIR_REMOVE 0x3A00
#define AX_DIR_CHANGE 0x3B00
#define AX_DIR_CURRENT 0x0100
#define AX_PARSE 0x2900
#define AX_FCB_DELETE 0x1300
#define AX_LETTER 0x4400
#define AX_REMOTE 0x0300

/* What FreeDOS answers of a drive of its disk to INT 21h 4409h, which is
   no drive of a network (harness/trial32/asked.py). */
#define LOCAL_DRIVE 0x08C2

/* What DOS answers of a file to INT 21h 4400h (RBIL): its drive in the
   low bits, 40h while it has not been written since it was opened, and
   4000h where its date and time have been set and are kept when it is
   closed. */
#define NOT_WRITTEN 0x0040
#define STAMP_KEPT 0x4000

static struct Answer Failed(struct Dos *dos, u16 error)
{
    struct Answer answer;

    dos->error = error;
    answer.value = error;
    answer.failed = true;
    return answer;
}

static struct Answer Done(u16 value)
{
    struct Answer answer;

    answer.value = value;
    answer.failed = false;
    return answer;
}

/* The last name of a path of the host. */
static const char *Last(const char *path)
{
    const char *slash = strrchr(path, '/');

    return slash == NULL ? path : slash + 1;
}

/* `name` behind the directory `directory` of the host, at `to`, which has
   HOST_ROOM bytes: false where that is not enough. */
static bool Join(char *to, const char *directory, const char *name)
{
    size_t length = strlen(directory);
    size_t more = strlen(name);
    bool slash = length == 0 || directory[length - 1] != '/';

    if (length + (slash ? 1 : 0) + more >= HOST_ROOM)
        return false;
    memcpy(to, directory, length + 1);
    if (slash)
        to[length++] = '/';
    memcpy(to + length, name, more + 1);
    return true;
}

/* Whether the program runs as one of the host's group `group`. */
static bool Member(gid_t group)
{
    gid_t *groups;
    int count;
    int i;
    bool member = false;

    if (group == getegid())
        return true;
    count = getgroups(0, NULL);
    if (count <= 0)
        return false;
    groups = malloc((size_t)count * sizeof *groups);
    if (groups == NULL)
        return false;
    count = getgroups(count, groups);
    for (i = 0; i < count; i++)
        if (groups[i] == group)
            member = true;
    free(groups);
    return member;
}

/* DOS's attributes of a file of the host at `path`, of its status. */
static u8 Attributes(const struct stat *status, const char *path)
{
    return DosAttributes(status, Last(path), geteuid(),
                         Member(status->st_gid));
}

/*
 * Reading a directory of the host, for the table of its names.
 */

static bool ReadHost(void *context, const char *path,
                     struct DirectoryNames *table)
{
    DIR *directory;
    struct dirent *entry;
    char **names = NULL;
    char **grown;
    i32 count = 0;
    i32 room = 0;
    int error = 0;
    bool done;
    i32 i;

    (void)context;
    do
        directory = opendir(path);
    while (directory == NULL && errno == EINTR);
    if (directory == NULL)
        return false;
    for (;;) {
        errno = 0;
        entry = readdir(directory);
        if (entry == NULL) {
            error = errno;
            break;
        }
        if (count == room) {
            room = room == 0 ? 64 : 2 * room;
            grown = (char **)realloc((void *)names,
                                     (size_t)room * sizeof *names);
            if (grown == NULL) {
                error = ENOMEM;
                break;
            }
            names = grown;
        }
        names[count] = strdup(entry->d_name);
        if (names[count] == NULL) {
            error = ENOMEM;
            break;
        }
        count++;
    }
    closedir(directory);
    done = error == 0
        && DirectoryNamesUpdate(table, (const char *const *)names, count);
    if (error == 0 && !done)
        error = ENOMEM;
    for (i = 0; i < count; i++)
        free(names[i]);
    free((void *)names);
    errno = error;
    return done;
}

/*
 * The state.
 */

/* The home directory that $HOME names, at `to`, which has HOST_ROOM
   bytes, with its links followed: false where $HOME is not set, is not a
   path from the root, names nothing or no directory, or names the root,
   which is R:. */
static bool Home(const char *home, char *to)
{
    char *real;
    struct stat status;
    size_t length;
    bool taken;

    if (home == NULL || home[0] != '/')
        return false;
    real = realpath(home, NULL);
    if (real == NULL)
        return false;
    length = strlen(real);
    taken = length < HOST_ROOM && strcmp(real, "/") != 0
        && stat(real, &status) == 0 && S_ISDIR(status.st_mode);
    if (taken)
        memcpy(to, real, length + 1);
    free(real);
    return taken;
}

void DosBegin(struct Dos *dos, const char *home)
{
    char directory[HOST_ROOM];
    /* What DOS answers of its own five handles to INT 21h 4400h: the
       console, as the bench's FreeDOS answers of 0 and 1, the serial port,
       and the printer, as it answers of 4 (harness/trial32/asked.py). */
    static const u16 Kinds[DOS_STANDARD] = {
        0x80D3, 0x80D3, 0x80D3, 0x80C0, 0xA0C0
    };
    i32 i;

    memset(dos, 0, sizeof *dos);
    DirectoriesBegin(&dos->directories, ReadHost, NULL);
    DrivesBegin(&dos->drives, Home(home, directory) ? directory : NULL);
    for (i = 0; i < DOS_HANDLES; i++) {
        dos->handles[i].fd = -1;
        dos->handles[i].open = i < DOS_STANDARD;
        dos->handles[i].kind = i < DOS_STANDARD ? Kinds[i] : 0;
        dos->handles[i].access = 2;
    }
    dos->record = &dos->own;
    dos->clock = NULL;
}

void DosEnd(struct Dos *dos)
{
    i32 i;

    for (i = 0; i < DOS_HANDLES; i++)
        if (dos->handles[i].open && dos->handles[i].fd >= 0)
            close(dos->handles[i].fd);
    DirectoriesEnd(&dos->directories);
}

/*
 * Names of the program's.
 */

/* The table of names of the directory that a path of the host is in
   read again: the emulation has changed what it has, or another program
   has. */
static void Reread(struct Dos *dos, const char *host)
{
    char directory[HOST_ROOM];
    const char *last = Last(host);
    size_t length = (size_t)(last - host);

    if (length > 1)
        length--;                       /* the slash, but for the root's */
    memcpy(directory, host, length);
    directory[length] = '\0';
    DirectoryNumber(&dos->directories, directory, true);
}

/* A name of the program's made full, at `full`, which has DOS_PATH_ROOM
   bytes, and led to the host, at `host`, which has HOST_ROOM: `value` is
   1 where the last name was found and 0 where it was not. A name that
   the table has and the host no longer has, another program having
   deleted or renamed it, is looked for again in what the host has now.
   A drive that there is not is DOS's 3 here, as the bench's DOS answers
   it to a call that takes a name (RBIL has no 0Fh among the errors of
   these calls). */
static struct Answer Lead(struct Dos *dos, const u8 *name, u8 *full,
                          char *host)
{
    struct Answer led = DosPathFull(name, &dos->drives, full);
    struct stat status;

    if (!led.failed)
        led = HostPathOfDos(&dos->directories, &dos->drives, full, host,
                            HOST_ROOM);
    if (!led.failed && led.value == 1 && lstat(host, &status) != 0
        && errno == ENOENT) {
        Reread(dos, host);
        led = HostPathOfDos(&dos->directories, &dos->drives, full, host,
                            HOST_ROOM);
    }
    if (led.failed)
        return Failed(dos, led.value == INVALID_DRIVE ? PATH_NOT_FOUND
                                                       : led.value);
    return led;
}

/* Whether a full path of DOS's has * or ?, which name no file. */
static bool Wild(const u8 *full)
{
    return strchr((const char *)full, '*') != NULL
        || strchr((const char *)full, '?') != NULL;
}

/* The full path of DOS's of the current directory of a drive, by its
   index. */
static void Current(const struct Dos *dos, i32 drive, u8 *full)
{
    const struct DosDrive *of = &dos->drives.drive[drive];
    size_t length = strlen((const char *)of->current);

    full[0] = of->letter;
    full[1] = ':';
    full[2] = '\\';
    memcpy(full + 3, of->current, length + 1);
}

/* The index of the drive of a full path of DOS's, which HostPathOfDos
   has led to the host, so that there is such a drive. */
static i32 DriveOfPath(const struct Dos *dos, const u8 *full)
{
    i32 drive = DriveIndex(&dos->drives, full[0]);

    assert(drive >= 0);
    return drive;
}

/* The index of the drive that DOS's number of a drive names, the low
   byte of `drive`, counted from A: as 1, or 0 for the current one: -1
   where there is no such drive. */
static i32 DriveNumbered(const struct Dos *dos, u16 drive)
{
    u16 number = drive & 0xFF;

    if (number == 0)
        return dos->drives.current;
    if (number > 'Z' - 'A' + 1)
        return -1;
    return DriveIndex(&dos->drives, (u8)('A' - 1 + number));
}

/*
 * Handles.
 */

static struct DosHandle *Handle(struct Dos *dos, u16 handle)
{
    if (handle >= DOS_HANDLES || !dos->handles[handle].open)
        return NULL;
    return &dos->handles[handle];
}

/* The first handle that is free, or -1. */
static i32 Free(const struct Dos *dos)
{
    i32 i;

    for (i = 0; i < DOS_HANDLES; i++)
        if (!dos->handles[i].open)
            return i;
    return -1;
}

static int OpenHost(const char *path, int flags, mode_t mode)
{
    int fd;

    /* Not blocking, so that a FIFO that has no one at its other end
       answers at once; and the file is not handed on to a program that
       this one runs. */
    do
        fd = open(path, flags | O_NONBLOCK | O_CLOEXEC | O_NOCTTY, mode);
    while (fd < 0 && errno == EINTR);
    return fd;
}

/* DOS's error of an open that the host refused: a FIFO without a reader
   and a socket answer ENXIO, and are no file to DOS. */
static u16 OpenError(int error)
{
    return error == ENXIO ? ACCESS_DENIED : DosErrorOf(error, false);
}

/* What the host has opened, `fd`, of `host`, held to what DOS opens and
   made the handle `slot`: a file and no other thing, and to write only
   where DOS lets the program write it, if `held` says so.

   A file that is longer than DOS can say, 4 GB or more, is not opened to
   be read, DOS's 5, access denied: on the user's word it is never copied
   or moved cut short (CLAUDE.md, step 6; TODO.md, L2). Refused at the
   open, the program says so at once, as of any file that it cannot
   open, by the boxes that it has: Copy and Move "Can't open the file",
   nothing made at the target and the source left as it was; the viewer
   and the editor "Can't find the file", the editor not taking it for a
   new one; the file search for a text passes it by. Refused at the read
   that reaches 4 GB, a copy would have read and written that much first
   and then stopped with no box, and the viewer failed at the file's
   end. To write alone it is opened, as the program does to set its date
   and time, and a write goes no further than DOS's last place. `letter`
   is the drive's, which FileDevice answers of the handle. */
static struct Answer Opened(struct Dos *dos, i32 slot, int fd,
                            const char *host, u16 access, bool held,
                            u8 letter)
{
    struct DosHandle *handle = &dos->handles[slot];
    struct stat status;
    int flags;
    u16 error = 0;

    if (fstat(fd, &status) != 0)
        error = DosErrorOf(errno, false);
    else if (!S_ISREG(status.st_mode))
        error = ACCESS_DENIED;
    else if (held && access != 0
             && (Attributes(&status, host) & DOS_READ_ONLY) != 0)
        error = ACCESS_DENIED;
    else if (access != 1 && (uint64_t)status.st_size > LARGEST)
        error = ACCESS_DENIED;
    if (error != 0) {
        close(fd);
        return Failed(dos, error);
    }
    flags = fcntl(fd, F_GETFL);
    if (flags >= 0)
        fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    handle->fd = fd;
    handle->access = access;
    handle->kind = (u16)(letter - 'A');
    handle->open = true;
    handle->written = false;
    handle->stamped = false;
    return Done((u16)slot);
}

/*
 * Files.
 */

struct Answer DosFileCreate(struct Dos *dos, const u8 *name,
                            u16 attributes)
{
    u8 full[DOS_PATH_ROOM];
    char host[HOST_ROOM];
    struct Answer led;
    struct stat status;
    i32 slot = Free(dos);
    int fd;

    if (slot < 0)
        return Failed(dos, TOO_MANY_OPEN_FILES);
    if ((attributes & (DOS_LABEL | DOS_DIRECTORY)) != 0)
        return Failed(dos, ACCESS_DENIED);
    led = Lead(dos, name, full, host);
    if (led.failed)
        return led;
    if (Wild(full))
        return Failed(dos, PATH_NOT_FOUND);
    if (led.value == 1 && stat(host, &status) == 0
        && (!S_ISREG(status.st_mode)
            || (Attributes(&status, host) & DOS_READ_ONLY) != 0))
        return Failed(dos, ACCESS_DENIED);
    fd = OpenHost(host, O_RDWR | O_CREAT | O_TRUNC,
                  (attributes & DOS_READ_ONLY) != 0 ? 0444 : 0666);
    if (fd < 0)
        return Failed(dos, OpenError(errno));
    return Opened(dos, slot, fd, host, 2, false, full[0]);
}

struct Answer DosFileOpen(struct Dos *dos, const u8 *name, u16 mode)
{
    static const int Flags[3] = { O_RDONLY, O_WRONLY, O_RDWR };
    u8 full[DOS_PATH_ROOM];
    char host[HOST_ROOM];
    struct Answer led;
    u16 access = mode & 0x07;           /* what others may do is not
                                           kept: no other program of
                                           DOS's opens these files */
    i32 slot = Free(dos);
    int fd;

    if (access > 2)
        return Failed(dos, INVALID_ACCESS);
    if (slot < 0)
        return Failed(dos, TOO_MANY_OPEN_FILES);
    led = Lead(dos, name, full, host);
    if (led.failed)
        return led;
    if (led.value == 0 || Wild(full))
        return Failed(dos, FILE_NOT_FOUND);
    fd = OpenHost(host, Flags[access], 0);
    if (fd < 0)
        return Failed(dos, OpenError(errno));
    return Opened(dos, slot, fd, host, access, true, full[0]);
}

/* Set the time that a file was last written at, of DOS's stamp, leaving
   when it was last read. */
static bool Stamp(int fd, struct DosStamp stamp)
{
    struct tm local;
    struct timespec times[2];

    DosStampToLocal(stamp, &local);
    times[0].tv_sec = 0;
    times[0].tv_nsec = UTIME_OMIT;
    times[1].tv_sec = mktime(&local);
    times[1].tv_nsec = 0;
    return times[1].tv_sec != (time_t)-1 && futimens(fd, times) == 0;
}

struct Answer DosFileClose(struct Dos *dos, u16 handle)
{
    struct DosHandle *open = Handle(dos, handle);

    if (open == NULL)
        return Failed(dos, INVALID_HANDLE);
    if (open->fd >= 0) {
        /* DOS writes the time that was set when it closes the file, so
           that a write behind the setting does not change it (RBIL, INT
           21h 5701h). A close that the host interrupts has closed all
           the same, on Linux, and is not made again. */
        if (open->stamped)
            Stamp(open->fd, open->stamp);
        else if (open->written && dos->clock != NULL)
            Stamp(open->fd, dos->clock());
        close(open->fd);
    }
    open->fd = -1;
    open->open = false;
    return Done(AX_CLOSE);
}

/* An answer of a read or a write that failed. */
static struct Moved Refused(struct Dos *dos, u16 error)
{
    struct Moved moved;

    dos->error = error;
    moved.count = 0;
    moved.error = error;
    moved.failed = true;
    return moved;
}

/* Where an open file of the host is at. */
static u32 At(int fd)
{
    off_t at = lseek(fd, 0, SEEK_CUR);

    if (at < 0)
        return 0;
    return (uint64_t)at > LARGEST ? LARGEST : (u32)at;
}

/* Whether an open file of the host is longer than DOS can say. */
static bool Beyond(int fd)
{
    struct stat status;

    return fstat(fd, &status) == 0
        && (uint64_t)status.st_size > LARGEST;
}

struct Moved DosFileRead(struct Dos *dos, u16 handle, u8 *to, i32 count)
{
    struct DosHandle *open = Handle(dos, handle);
    struct Moved moved;
    u32 left;
    ssize_t piece;

    if (open == NULL || open->fd < 0)
        return Refused(dos, INVALID_HANDLE);
    if (open->access == 1)
        return Refused(dos, ACCESS_DENIED);
    moved.count = 0;
    moved.error = 0;
    moved.failed = false;
    /* No further than DOS's last place. A file that has grown beyond it
       since it was opened, by another program, is refused at a read that
       would go past it, 5, as it is at the open (Opened): what is read of
       it is never taken for the whole, and a copy that meets this stops
       and deletes what it has written. */
    left = LARGEST - At(open->fd);
    if (count > 0 && (u32)count > left) {
        if (Beyond(open->fd))
            return Refused(dos, ACCESS_DENIED);
        count = (i32)left;
    }
    while (moved.count < count) {
        piece = read(open->fd, to + moved.count,
                     (size_t)(count - moved.count));
        if (piece < 0 && errno == EINTR)
            continue;
        if (piece < 0) {
            /* What DOS would have read in its pieces before the one
               that failed, as sys.h has it. */
            moved.count = moved.count / 0xFFFF * 0xFFFF;
            moved.error = DosErrorOf(errno, false);
            moved.failed = true;
            dos->error = moved.error;
            break;
        }
        if (piece == 0)
            break;
        moved.count += (i32)piece;
    }
    return moved;
}

/* Whether the host's error is that of a disk that is full, where DOS
   writes fewer bytes and does not fail (RBIL, INT 21h 40h). */
static bool Full(int error)
{
    return error == ENOSPC || error == EDQUOT || error == EFBIG;
}

struct Moved DosFileWrite(struct Dos *dos, u16 handle, const u8 *from,
                          i32 count)
{
    struct DosHandle *open = Handle(dos, handle);
    struct Moved moved;
    u32 at;
    ssize_t piece;
    int cut;

    if (open == NULL || open->fd < 0)
        return Refused(dos, INVALID_HANDLE);
    if (open->access == 0)
        return Refused(dos, ACCESS_DENIED);
    open->written = true;
    moved.count = 0;
    moved.error = 0;
    moved.failed = false;
    at = At(open->fd);
    if (count <= 0) {
        /* None cuts the file off where it is at, or makes it as long. */
        do
            cut = ftruncate(open->fd, (off_t)at);
        while (cut != 0 && errno == EINTR);
        if (cut != 0)
            return Refused(dos, DosErrorOf(errno, false));
        return moved;
    }
    if ((u32)count > LARGEST - at)
        count = (i32)(LARGEST - at);
    while (moved.count < count) {
        piece = write(open->fd, from + moved.count,
                      (size_t)(count - moved.count));
        if (piece < 0 && errno == EINTR)
            continue;
        if (piece < 0 && Full(errno))
            break;
        if (piece < 0) {
            moved.count = moved.count / 0xFFFF * 0xFFFF;
            moved.error = DosErrorOf(errno, false);
            moved.failed = true;
            dos->error = moved.error;
            break;
        }
        if (piece == 0)
            break;
        moved.count += (i32)piece;
    }
    return moved;
}

/* Go `offset` bytes from where `from` says: DOS reckons the place in 32
   bits, so that a step back from the start is a place far on (RBIL, INT
   21h 42h). If it fails, the place has the low word of `offset` in its
   high word and the error in its low one, as DOS leaves DX and AX. */
static struct FilePlace Sought(struct Dos *dos, u16 handle, u16 from,
                               u32 offset)
{
    struct DosHandle *open = Handle(dos, handle);
    struct FilePlace sought;
    struct stat status;
    u32 base = 0;
    u16 error = 0;

    sought.place = 0;
    if (open == NULL || open->fd < 0)
        error = INVALID_HANDLE;
    else if ((from & 0xFF) > 2)
        error = INVALID_FUNCTION;
    else if ((from & 0xFF) == 1)
        base = At(open->fd);
    else if ((from & 0xFF) == 2) {
        if (fstat(open->fd, &status) != 0)
            error = DosErrorOf(errno, false);
        else
            base = (uint64_t)status.st_size > LARGEST
                ? LARGEST : (u32)status.st_size;
    }
    if (error == 0) {
        sought.place = (u32)(((uint64_t)base + offset) & LARGEST);
        if (lseek(open->fd, (off_t)sought.place, SEEK_SET) < 0)
            error = DosErrorOf(errno, false);
    }
    sought.failed = error != 0;
    if (sought.failed) {
        dos->error = error;
        sought.place = (offset & 0xFFFF) << 16 | error;
    }
    return sought;
}

struct FilePlace DosFileSeek(struct Dos *dos, u16 handle, u32 place)
{
    return Sought(dos, handle, 0, place);
}

struct FilePlace DosFileStep(struct Dos *dos, u16 handle, u16 from,
                             i32 step)
{
    return Sought(dos, handle, from, (u32)step);
}

struct Answer DosFileDelete(struct Dos *dos, const u8 *name)
{
    u8 full[DOS_PATH_ROOM];
    char host[HOST_ROOM];
    struct Answer led = Lead(dos, name, full, host);
    struct stat status;

    if (led.failed)
        return led;
    if (led.value == 0 || Wild(full))
        return Failed(dos, FILE_NOT_FOUND);
    /* No directory, and no file that is read-only to DOS. A link that
       leads nowhere has no status of what it leads to, and goes. */
    if (stat(host, &status) == 0
        && (S_ISDIR(status.st_mode)
            || (Attributes(&status, host) & DOS_READ_ONLY) != 0))
        return Failed(dos, ACCESS_DENIED);
    if (unlink(host) != 0)
        return Failed(dos, DosErrorOf(errno, false));
    Reread(dos, host);
    return Done(AX_DELETE);
}

struct Answer DosFileRename(struct Dos *dos, const u8 *from, const u8 *to)
{
    u8 full[DOS_PATH_ROOM];
    char host[HOST_ROOM];
    u8 target[DOS_PATH_ROOM];
    char goes[HOST_ROOM];
    struct Answer led = Lead(dos, from, full, host);
    struct stat status;

    if (led.failed)
        return led;
    if (led.value == 0 || Wild(full))
        return Failed(dos, FILE_NOT_FOUND);
    led = DosPathFull(to, &dos->drives, target);
    if (led.failed)
        return Failed(dos, led.value == INVALID_DRIVE ? PATH_NOT_FOUND
                                                       : led.value);
    if (target[0] != full[0])
        return Failed(dos, NOT_SAME_DEVICE);
    led = Lead(dos, to, target, goes);
    if (led.failed)
        return led;
    if (Wild(target))
        return Failed(dos, PATH_NOT_FOUND);
    /* A name that is there, by DOS's name or by the host's, is not
       replaced: DOS refuses, where the host would replace it. */
    if (led.value == 1 || lstat(goes, &status) == 0)
        return Failed(dos, ACCESS_DENIED);
    if (rename(host, goes) != 0)
        /* A directory into itself is the host's EINVAL. */
        return Failed(dos, errno == EINVAL ? ACCESS_DENIED
                                           : DosErrorOf(errno, false));
    Reread(dos, host);
    Reread(dos, goes);
    return Done(AX_RENAME);
}

/* The status of a file of the host, or of a link that leads nowhere. */
static bool Status(const char *host, struct stat *status, bool *dangling)
{
    *dangling = false;
    if (stat(host, status) == 0)
        return true;
    if (errno != ENOENT || lstat(host, status) != 0)
        return false;
    *dangling = true;
    return true;
}

struct FileAttr DosFileAttributes(struct Dos *dos, const u8 *name)
{
    u8 full[DOS_PATH_ROOM];
    char host[HOST_ROOM];
    struct Answer led = Lead(dos, name, full, host);
    struct FileAttr attr;
    struct stat status;
    bool dangling;

    attr.attributes = 0;
    if (!led.failed && (led.value == 0 || Wild(full)))
        led = Failed(dos, FILE_NOT_FOUND);
    if (!led.failed && !Status(host, &status, &dangling))
        led = Failed(dos, DosErrorOf(errno, false));
    attr.value = led.failed ? led.value : AX_ATTRIBUTES;
    attr.failed = led.failed;
    if (!led.failed)
        attr.attributes = Attributes(&status, host);
    return attr;
}

/* Read-only is the one attribute that the host keeps: set, it takes the
   write bits of the mode away, and cleared, it gives the owner the write
   bit, the one who may change the mode. The others are taken and not
   kept, as DOSBox takes them of a file of the host; directory and label
   are refused, as DOS refuses them (RBIL, INT 21h 4301h). */
struct Answer DosFileSetAttributes(struct Dos *dos, const u8 *name,
                                   u16 attributes)
{
    u8 full[DOS_PATH_ROOM];
    char host[HOST_ROOM];
    struct Answer led = Lead(dos, name, full, host);
    struct stat status;
    bool now, wanted;
    mode_t mode;

    if (led.failed)
        return led;
    if (led.value == 0 || Wild(full))
        return Failed(dos, FILE_NOT_FOUND);
    if ((attributes & (DOS_LABEL | DOS_DIRECTORY)) != 0)
        return Failed(dos, ACCESS_DENIED);
    if (stat(host, &status) != 0)
        return Failed(dos, DosErrorOf(errno, false));
    if (S_ISREG(status.st_mode)) {
        now = (Attributes(&status, host) & DOS_READ_ONLY) != 0;
        wanted = (attributes & DOS_READ_ONLY) != 0;
        mode = status.st_mode & 07777;
        if (now != wanted
            && chmod(host, wanted ? mode & ~(mode_t)0222
                                  : mode | S_IWUSR) != 0)
            return Failed(dos, DosErrorOf(errno, false));
    }
    return Done(attributes);
}

struct FileStamp DosFileTime(struct Dos *dos, u16 handle)
{
    struct DosHandle *open = Handle(dos, handle);
    struct FileStamp stamp;
    struct DosStamp dos_stamp = { 0, 0 };
    struct stat status;
    u16 error = 0;

    stamp.time = 0;
    stamp.date = 0;
    if (open == NULL || open->fd < 0)
        error = INVALID_HANDLE;
    else if (open->stamped)
        dos_stamp = open->stamp;
    else if (fstat(open->fd, &status) != 0)
        error = DosErrorOf(errno, false);
    else
        dos_stamp = DosStampOfTime(status.st_mtime);
    stamp.failed = error != 0;
    if (stamp.failed) {
        dos->error = error;
        stamp.value = error;
    } else {
        stamp.value = AX_TIME;
        stamp.time = dos_stamp.time;
        stamp.date = dos_stamp.date;
    }
    return stamp;
}

struct Answer DosFileSetTime(struct Dos *dos, u16 handle, u16 time,
                             u16 date)
{
    struct DosHandle *open = Handle(dos, handle);

    if (open == NULL || open->fd < 0)
        return Failed(dos, INVALID_HANDLE);
    open->stamp.time = time;
    open->stamp.date = date;
    open->stamped = true;
    Stamp(open->fd, open->stamp);
    return Done(AX_SET_TIME);
}

struct Device DosFileDevice(struct Dos *dos, u16 handle)
{
    struct DosHandle *open = Handle(dos, handle);
    struct Device device;

    device.kind = 0;
    device.failed = open == NULL;
    if (open == NULL) {
        dos->error = INVALID_HANDLE;
        device.value = INVALID_HANDLE;
        return device;
    }
    if (open->fd < 0)
        device.kind = open->kind;
    else
        device.kind = (u16)(open->kind | (open->written ? 0 : NOT_WRITTEN)
                            | (open->stamped ? STAMP_KEPT : 0));
    device.value = device.kind;         /* as FreeDOS leaves AX */
    return device;
}

/* The full path, whether there is such a file or not: of a drive that
   there is not, DOS's 3, and `to` is left as it is (FreeDOS 1.4,
   harness/trial32/asked.py). */
struct Answer DosFileTrueName(struct Dos *dos, const u8 *name, u8 *to)
{
    u8 full[DOS_PATH_ROOM];
    struct Answer made = DosPathFull(name, &dos->drives, full);

    if (made.failed || DriveIndex(&dos->drives, full[0]) < 0)
        return Failed(dos, PATH_NOT_FOUND);
    memcpy(to, full, strlen((const char *)full) + 1);
    return Done(0);
}

/*
 * Files by a file control block.
 */

/* Of a name of DOS's form, its eleven characters as a file control block
   and a directory have them: the name to 8 and the extension to 3, each
   filled with blanks; . and .. are themselves. */
static void Eleven(const u8 *name, u8 *to)
{
    i32 i;
    const u8 *dot;

    memset(to, ' ', 11);
    if (strcmp((const char *)name, ".") == 0
        || strcmp((const char *)name, "..") == 0) {
        memcpy(to, name, strlen((const char *)name));
        return;
    }
    dot = (const u8 *)strchr((const char *)name, '.');
    for (i = 0; i < 8 && name + i != dot && name[i] != '\0'; i++)
        to[i] = name[i];
    if (dot != NULL)
        for (i = 0; i < 3 && dot[1 + i] != '\0'; i++)
            to[8 + i] = dot[1 + i];
}

/* Whether a name of DOS's form is one that a template of eleven
   characters names: a ? there takes any character, a blank too. */
static bool Matches(const u8 *pattern, const u8 *name)
{
    u8 eleven[11];
    i32 i;

    Eleven(name, eleven);
    for (i = 0; i < 11; i++)
        if (pattern[i] != '?' && pattern[i] != eleven[i])
            return false;
    return true;
}

/* A part of a name, up to `length` characters, at `to`, from `name`, as
   DOS parses one into a file control block: in capitals, a * filling
   what is left with ?, as far as a character that ends a name. Answers
   where the name goes on, and notes whether it had * or ?. */
static const u8 *Part(const u8 *name, u8 *to, i32 length, bool *wild)
{
    i32 i = 0;

    memset(to, ' ', (size_t)length);
    for (;; name++) {
        if (*name == '*') {
            *wild = true;
            for (; i < length; i++)
                to[i] = '?';
        } else if (*name == '?') {
            *wild = true;
            if (i < length)
                to[i++] = '?';
        } else if (*name != '.' && DosNameByte(*name)) {
            if (i < length)
                to[i++] = CodePageUpper(*name);
        } else
            return name;
    }
}

/* INT 21h 29h, as RBIL has it and as the bench's DOS answers it
   (harness/trial32/asked.py): blanks and, by bit 0 of `how`, the
   characters that part names skipped; a drive that there is not ends
   the parsing with FFh and leaves the block as it is; the drive is 0
   where none is named but by bit 1, the name and the extension blanks
   but by bits 2 and 3; the four bytes behind the name are 0. Answers AX
   as FreeDOS leaves it: 29h, and 1 where the name has * or ?. */
u16 DosFcbParse(struct Dos *dos, const u8 *name, u8 *fcb, u16 how)
{
    static const u8 Separators[] = ":.;,=+";
    u8 part[8];
    bool wild = false;
    u8 letter;

    while (*name == ' ' || *name == '\t'
           || ((how & 1) != 0 && *name != '\0'
               && strchr((const char *)Separators, *name) != NULL))
        name++;
    if (name[0] != '\0' && name[1] == ':') {
        letter = CodePageUpper(name[0]);
        if (DriveIndex(&dos->drives, letter) < 0)
            return AX_PARSE | 0xFF;
        fcb[0] = (u8)(letter - 'A' + 1);
        name += 2;
    } else if ((how & 2) == 0)
        fcb[0] = 0;
    name = Part(name, part, 8, &wild);
    if (part[0] != ' ' || (how & 4) == 0)
        memcpy(fcb + 1, part, 8);
    if (*name == '.')
        Part(name + 1, part, 3, &wild);
    else
        memset(part, ' ', 3);
    if (part[0] != ' ' || (how & 8) == 0)
        memcpy(fcb + 9, part, 3);
    memset(fcb + 12, 0, 4);
    return AX_PARSE | (wild ? 1 : 0);
}

/* INT 21h 13h: every file of the current directory of the block's drive
   that the block names, which is not read-only (RBIL) and has no
   attribute but those that an extended block allows, hidden and system.
   A label there is none of.
   The low byte of `value` is 0 where a file was deleted, FFh where none
   was. DOS's search for them ends with its error 12h, which a program
   that asks for the last error is told (FreeDOS 1.4, harness/trial32/
   asked.py): `flagged` as sys.h has it. */
struct Answer DosFcbDelete(struct Dos *dos, const u8 *fcb, bool flagged)
{
    u8 full[DOS_PATH_ROOM];
    char host[HOST_ROOM];
    char path[HOST_ROOM];
    const struct DirectoryNames *names;
    struct Answer led;
    struct stat status;
    u16 allowed = 0;
    bool deleted = false;
    i32 drive;
    i32 number;
    i32 i;

    if (fcb[0] == 0xFF) {
        allowed = fcb[6];
        fcb += 7;
    }
    drive = DriveNumbered(dos, fcb[0]);
    if (drive >= 0 && (allowed & DOS_LABEL) == 0) {
        Current(dos, drive, full);
        led = HostPathOfDos(&dos->directories, &dos->drives, full, host,
                            HOST_ROOM);
        number = led.failed ? -1
            : DirectoryNumber(&dos->directories, host, true);
        names = number < 0 ? NULL
            : &dos->directories.directories[number].names;
        for (i = 0; names != NULL && i < names->count; i++) {
            if (!names->names[i].present
                || !Matches(fcb + 1, names->names[i].dos)
                || !Join(path, host, names->names[i].host)
                || stat(path, &status) != 0
                || S_ISDIR(status.st_mode)
                || (Attributes(&status, path)
                    & (DOS_READ_ONLY | (~allowed & (DOS_HIDDEN | DOS_SYSTEM))))
                       != 0)
                continue;
            if (unlink(path) == 0)
                deleted = true;
        }
        if (deleted)
            DirectoryNumber(&dos->directories, host, true);
    }
    dos->error = NO_MORE_FILES;
    if (flagged)
        return Failed(dos, dos->error);
    return Done(AX_FCB_DELETE | (deleted ? 0 : 0xFF));
}

/*
 * Directories.
 */

void DosFindRecord(struct Dos *dos, struct DTAs *record)
{
    dos->record = record;
}

/* What DOS keeps of a search in the record, in its first 21 bytes, laid
   out as FreeDOS lays them out (its struct dmatch): the drive, the
   template of eleven characters, the attributes searched for, and where
   the search is at, the entry of the directory and the directory, here
   its number in the table of the emulation, with the high word of the
   entry in the last two bytes. */
#define AT_DRIVE 0
#define AT_TEMPLATE 1
#define AT_ATTRIBUTES 12
#define AT_ENTRY 13
#define AT_DIRECTORY 15
#define AT_ENTRY_HIGH 19

static void PutWord(u8 *at, u32 value)
{
    at[0] = (u8)(value & 0xFF);
    at[1] = (u8)(value >> 8 & 0xFF);
}

static u32 Word(const u8 *at)
{
    return (u32)at[0] | (u32)at[1] << 8;
}

static void PutEntry(u8 *state, i32 entry)
{
    PutWord(state + AT_ENTRY, (u32)entry & 0xFFFF);
    PutWord(state + AT_ENTRY_HIGH, (u32)entry >> 16);
}

/* What a search finds, in its record: DOS's attributes, the time of
   the host, the size and the name. */
static void Found(struct DTAs *record, u8 attributes, time_t time, u32 size,
                  const u8 *name)
{
    struct DosStamp stamp = DosStampOfTime(time);

    record->FilAttr = attributes;
    record->FilTime = stamp.time;
    record->FilDate = stamp.date;
    record->FilSize = size;
    memset(record->FilName, 0, sizeof record->FilName);
    memcpy(record->FilName, name, strlen((const char *)name));
}

/* The label of a drive, which a search for a label alone finds in the
   drive's root and nowhere else, if the template names it, once:
   emulate_posix.h says where that comes from. Its time is that of the
   directory of the host that the drive is, as a label's entry has the
   time that it was written. */
static struct Answer Label(struct Dos *dos, const struct DosDrive *drive,
                           const struct Directory *directory, i32 entry)
{
    struct DTAs *record = dos->record;
    u8 *state = record->Reserved;
    struct stat status;

    if (entry != 0 || strcmp(directory->path, drive->root) != 0
        || !Matches(state + AT_TEMPLATE, (const u8 *)drive->label)
        || stat(drive->root, &status) != 0)
        return Failed(dos, NO_MORE_FILES);
    Found(record, DOS_LABEL, status.st_mtime, 0, (const u8 *)drive->label);
    PutEntry(state, 1);
    return Done(0);
}

/* The next of a search, from where its record says. */
static struct Answer Next(struct Dos *dos)
{
    struct DTAs *record = dos->record;
    u8 *state = record->Reserved;
    const struct Directory *directory;
    const struct DosDrive *drive;
    const u8 *name;
    const char *host;
    char path[HOST_ROOM];
    struct stat status;
    bool dangling;
    bool root;
    i32 at;
    i32 number;
    i32 entry;
    i32 dots;
    u8 attributes;
    u8 searched = state[AT_ATTRIBUTES];

    number = (i32)(Word(state + AT_DIRECTORY)
                   | Word(state + AT_DIRECTORY + 2) << 16);
    entry = (i32)(Word(state + AT_ENTRY) | Word(state + AT_ENTRY_HIGH) << 16);
    at = DriveIndex(&dos->drives, (u8)('A' + state[AT_DRIVE]));
    if (number < 0 || number >= dos->directories.count || entry < 0
        || at < 0)
        return Failed(dos, NO_MORE_FILES);
    directory = &dos->directories.directories[number];
    drive = &dos->drives.drive[at];
    if (searched == DOS_LABEL)
        return Label(dos, drive, directory, entry);
    root = strcmp(directory->path, drive->root) == 0;
    dots = root ? 0 : 2;
    for (; entry < dots + directory->names.count; entry++) {
        if (entry < dots) {
            name = (const u8 *)(entry == 0 ? "." : "..");
            host = entry == 0 ? "." : "..";
        } else {
            if (!directory->names.names[entry - dots].present)
                continue;
            name = directory->names.names[entry - dots].dos;
            host = directory->names.names[entry - dots].host;
        }
        if (!Matches(state + AT_TEMPLATE, name)
            || !Join(path, directory->path, host)
            || !Status(path, &status, &dangling))
            continue;
        attributes = Attributes(&status, path);
        if ((attributes & ~searched
             & (DOS_HIDDEN | DOS_SYSTEM | DOS_DIRECTORY)) != 0)
            continue;
        Found(record, attributes, status.st_mtime,
              dangling || S_ISDIR(status.st_mode) ? 0
              : (uint64_t)status.st_size > LARGEST ? LARGEST
              : (u32)status.st_size,
              name);
        PutEntry(state, entry + 1);
        return Done(0);
    }
    PutEntry(state, entry);
    return Failed(dos, NO_MORE_FILES);
}

/* What DOS finds is by the record of the search alone, so that a record
   that is put back goes on where it was (RBIL, INT 21h 4Eh and 4Fh). The
   entries of a directory are in the order of its table of names, with .
   and .. in front but in the root, as on a disk of DOS's; what is found
   has the attributes that are searched for, or read-only or archive, or
   none (RBIL). Nothing found is DOS's 12h, a directory that is not there
   3. */
struct Answer DosFindFirst(struct Dos *dos, const u8 *pattern,
                           u16 attributes)
{
    u8 full[DOS_PATH_ROOM];
    char host[HOST_ROOM];
    u8 *state = dos->record->Reserved;
    u8 *last;
    const u8 *after;
    struct Answer led = DosPathFull(pattern, &dos->drives, full);
    struct stat status;
    bool wild = false;
    i32 number;

    if (led.failed)
        return Failed(dos, led.value == INVALID_DRIVE ? PATH_NOT_FOUND
                                                       : led.value);
    last = (u8 *)strrchr((const char *)full, '\\');
    memset(state, 0, sizeof dos->record->Reserved);
    state[AT_DRIVE] = (u8)(full[0] - 'A');
    after = Part(last + 1, state + AT_TEMPLATE, 8, &wild);
    if (*after == '.')
        Part(after + 1, state + AT_TEMPLATE + 8, 3, &wild);
    else
        memset(state + AT_TEMPLATE + 8, ' ', 3);
    state[AT_ATTRIBUTES] = (u8)(attributes & 0xFF);
    last[last == full + 2 ? 1 : 0] = '\0';
    led = HostPathOfDos(&dos->directories, &dos->drives, full, host,
                        HOST_ROOM);
    if (led.failed)
        return Failed(dos, led.value == INVALID_DRIVE ? PATH_NOT_FOUND
                                                       : led.value);
    if (led.value == 0 || stat(host, &status) != 0
        || !S_ISDIR(status.st_mode))
        return Failed(dos, PATH_NOT_FOUND);
    number = DirectoryNumber(&dos->directories, host, true);
    if (number < 0)
        return Failed(dos, errno == ENOMEM ? NO_MEMORY : PATH_NOT_FOUND);
    PutWord(state + AT_DIRECTORY, (u32)number & 0xFFFF);
    PutWord(state + AT_DIRECTORY + 2, (u32)number >> 16);
    return Next(dos);
}

struct Answer DosFindNext(struct Dos *dos)
{
    return Next(dos);
}

struct Answer DosDirMake(struct Dos *dos, const u8 *name)
{
    u8 full[DOS_PATH_ROOM];
    char host[HOST_ROOM];
    struct Answer led = Lead(dos, name, full, host);

    if (led.failed)
        return led;
    if (Wild(full))
        return Failed(dos, PATH_NOT_FOUND);
    if (led.value == 1)
        return Failed(dos, ACCESS_DENIED);
    if (mkdir(host, 0777) != 0)
        return Failed(dos, DosErrorOf(errno, true));
    /* DOS gives a new directory the time of day, which a test has of its
       own clock, as it stamps a file that is written. */
    if (dos->clock != NULL) {
        int fd = open(host, O_RDONLY | O_DIRECTORY);

        if (fd >= 0) {
            Stamp(fd, dos->clock());
            close(fd);
        }
    }
    return Done(AX_DIR_MAKE);
}

struct Answer DosDirRemove(struct Dos *dos, const u8 *name)
{
    u8 full[DOS_PATH_ROOM];
    char host[HOST_ROOM];
    struct Answer led = Lead(dos, name, full, host);

    if (led.failed)
        return led;
    if (led.value == 0 || Wild(full))
        return Failed(dos, PATH_NOT_FOUND);
    if (strcmp((const char *)full + 3,
               (const char *)dos->drives.drive[DriveOfPath(dos, full)]
                   .current)
        == 0)
        return Failed(dos, full[3] == '\0' ? ACCESS_DENIED : REMOVE_CURRENT);
    if (rmdir(host) != 0)
        return Failed(dos, DosErrorOf(errno, true));
    Reread(dos, host);
    return Done(AX_DIR_REMOVE);
}

struct Answer DosDirChange(struct Dos *dos, const u8 *name)
{
    u8 full[DOS_PATH_ROOM];
    char host[HOST_ROOM];
    struct Answer led = Lead(dos, name, full, host);
    struct stat status;
    size_t length;

    if (led.failed)
        return led;
    if (led.value == 0 || stat(host, &status) != 0
        || !S_ISDIR(status.st_mode))
        return Failed(dos, PATH_NOT_FOUND);
    length = strlen((const char *)full + 3);
    if (length >= DOS_CURRENT_ROOM)
        return Failed(dos, PATH_NOT_FOUND);
    /* The current directory of the drive that the path names, which is
       not made the current drive (RBIL, INT 21h 3Bh). */
    memcpy(dos->drives.drive[DriveOfPath(dos, full)].current, full + 3,
           length + 1);
    return Done(AX_DIR_CHANGE);
}

struct Answer DosDirCurrent(struct Dos *dos, u16 drive, u8 *to)
{
    i32 at = DriveNumbered(dos, drive);
    const u8 *current;

    if (at < 0)
        return Failed(dos, INVALID_DRIVE);
    current = dos->drives.drive[at].current;
    memcpy(to, current, strlen((const char *)current) + 1);
    return Done(AX_DIR_CURRENT);
}

struct Answer DosStartIn(struct Dos *dos, const char *path)
{
    u8 full[DOS_PATH_ROOM];
    struct Answer made = DosPathOfHost(&dos->directories, &dos->drives,
                                       path, full);
    size_t length;
    i32 drive;

    if (made.failed)
        return Failed(dos, made.value);
    length = strlen((const char *)full + 3);
    if (length >= DOS_CURRENT_ROOM)
        return Failed(dos, PATH_NOT_FOUND);
    drive = DriveOfPath(dos, full);
    memcpy(dos->drives.drive[drive].current, full + 3, length + 1);
    dos->drives.current = drive;
    return Done(0);
}

/*
 * Drives.
 */

/* Whether DOS's number of a drive, counted from A: as 1, or 0 for the
   current one, names a drive that there is. */
static bool IsDrive(const struct Dos *dos, u16 drive)
{
    return DriveNumbered(dos, drive) >= 0;
}

u16 DosDriveCurrent(struct Dos *dos)
{
    return (u16)(dos->drives.drive[dos->drives.current].letter - 'A');
}

/* INT 21h 0Eh, with the drive counted from A: as 0: a drive that there
   is becomes the current one, and a letter that is no drive leaves the
   current one as it was (RBIL). */
void DosDriveSelect(struct Dos *dos, u16 drive)
{
    i32 at = (drive & 0xFF) < 'Z' - 'A' + 1
        ? DriveIndex(&dos->drives, (u8)('A' + (drive & 0xFF))) : -1;

    if (at >= 0)
        dos->drives.current = at;
}

struct Space DosSpaceOf(u32 block, uint64_t blocks, uint64_t free)
{
    struct Space space;
    uint64_t total;
    uint64_t left;
    uint64_t cluster;
    u32 sectors = 1;

    if (block == 0)
        block = 512;
    total = blocks > UINT64_MAX / block ? UINT64_MAX : blocks * block;
    left = free > UINT64_MAX / block ? UINT64_MAX : free * block;
    while (sectors < 128 && 512U * sectors < block)
        sectors *= 2;
    while (sectors < 128 && total / (512U * sectors) > 0xFFFF)
        sectors *= 2;
    cluster = 512U * sectors;
    space.value = (u16)sectors;
    space.failed = false;
    space.sector = 512;
    space.total = total / cluster > 0xFFFF ? 0xFFFF : (u32)(total / cluster);
    space.free = left / cluster > 0xFFFF ? 0xFFFF : (u32)(left / cluster);
    return space;
}

/* Of a drive that there is not, DOS answers FFFFh and leaves the other
   registers as they were, DX the drive that it was asked of (FreeDOS 1.4,
   harness/trial32/asked.py); `flagged` as sys.h has it. A file system
   that the host cannot say the room of is a drive that there is not. */
struct Space DosDriveSpace(struct Dos *dos, u16 drive, bool flagged)
{
    u8 full[DOS_PATH_ROOM];
    char host[HOST_ROOM];
    struct Answer led;
    struct statvfs system;
    struct Space space;

    space.value = 0xFFFF;
    space.failed = false;
    space.free = 0;
    space.sector = 0;
    space.total = drive & 0xFF;
    if (IsDrive(dos, drive)) {
        Current(dos, DriveNumbered(dos, drive), full);
        led = HostPathOfDos(&dos->directories, &dos->drives, full, host,
                            HOST_ROOM);
        if (!led.failed && statvfs(host, &system) == 0)
            space = DosSpaceOf((u32)system.f_frsize, system.f_blocks,
                               system.f_bavail);
    }
    if (flagged) {
        space.value = dos->error;
        space.failed = true;
    }
    return space;
}

/* No drive shares its unit with another: each answers as FreeDOS answers
   of a drive that shares none, AL 0 with AH as it was handed (harness/
   trial32/asked.py), and a drive that there is not DOS's 0Fh. */
struct Answer DosDriveLetter(struct Dos *dos, u16 drive)
{
    return IsDrive(dos, drive) ? Done(AX_LETTER) : Failed(dos, INVALID_DRIVE);
}

struct Answer DosDriveSetLetter(struct Dos *dos, u16 drive)
{
    return IsDrive(dos, drive) ? Done(AX_LETTER) : Failed(dos, INVALID_DRIVE);
}

struct Device DosDriveRemote(struct Dos *dos, u16 drive)
{
    struct Device device;

    device.failed = !IsDrive(dos, drive);
    device.value = device.failed ? INVALID_DRIVE : AX_REMOTE;
    device.kind = device.failed ? 0 : LOCAL_DRIVE;
    if (device.failed)
        dos->error = INVALID_DRIVE;
    return device;
}

/* 1 for a disk that cannot be taken out (RBIL, INT 21h 4408h). */
struct Answer DosDriveFixed(struct Dos *dos, u16 drive)
{
    return IsDrive(dos, drive) ? Done(1) : Failed(dos, INVALID_DRIVE);
}

bool DosDriveCd(struct Dos *dos, u16 drive)
{
    (void)dos;
    (void)drive;
    return false;
}
