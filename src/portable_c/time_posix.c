/*
 * DOS's date and time of a time of the host, and DOS's error of a
 * failure of the host's. See emulate_posix.h.
 */
#include <errno.h>

#include "emulate_posix.h"

/* The first and the last time that DOS's stamp holds, as Linux has them
   for a disk of DOS's, fs/fat/misc.c: 1 January 1980 at midnight, and
   31 December 2107 at 23:59:58. */
#define FIRST_DATE (0 << 9 | 1 << 5 | 1)
#define FIRST_TIME 0
#define LAST_DATE (127 << 9 | 12 << 5 | 31)
#define LAST_TIME (23 << 11 | 59 << 5 | 29)

struct DosStamp DosStampOfLocal(const struct tm *local)
{
    struct DosStamp stamp;
    i32 year = local->tm_year + 1900;

    if (year < 1980) {
        stamp.date = FIRST_DATE;
        stamp.time = FIRST_TIME;
    } else if (year > 2107) {
        stamp.date = LAST_DATE;
        stamp.time = LAST_TIME;
    } else {
        /* Each field within its bits: a second of 60, a leap second, is
           59 as Linux has it (fat_time_unix2fat counts seconds of the
           day, and has no leap second). */
        stamp.date = (u16)((year - 1980) << 9 | (local->tm_mon + 1) << 5
                           | local->tm_mday);
        stamp.time = (u16)(local->tm_hour << 11 | local->tm_min << 5
                           | (local->tm_sec > 59 ? 59 : local->tm_sec) / 2);
    }
    return stamp;
}

struct DosStamp DosStampOfTime(time_t time)
{
    struct tm local;
    struct DosStamp stamp;

    if (localtime_r(&time, &local) == NULL) {
        /* A time that the host cannot make a date of is past its years. */
        stamp.date = time < 0 ? FIRST_DATE : LAST_DATE;
        stamp.time = time < 0 ? FIRST_TIME : LAST_TIME;
        return stamp;
    }
    return DosStampOfLocal(&local);
}

void DosStampToLocal(struct DosStamp stamp, struct tm *local)
{
    local->tm_year = (stamp.date >> 9) + 80;
    local->tm_mon = (stamp.date >> 5 & 0x0F) - 1;
    local->tm_mday = stamp.date & 0x1F;
    local->tm_hour = stamp.time >> 11;
    local->tm_min = stamp.time >> 5 & 0x3F;
    local->tm_sec = (stamp.time & 0x1F) * 2;
    local->tm_wday = 0;
    local->tm_yday = 0;
    local->tm_isdst = -1;               /* mktime is to say */
}

/* DOS's errors, by their numbers as DOS answers INT 21h 59h with them. */
#define FILE_NOT_FOUND 0x02
#define PATH_NOT_FOUND 0x03
#define TOO_MANY_OPEN_FILES 0x04
#define ACCESS_DENIED 0x05
#define INVALID_HANDLE 0x06
#define NO_MEMORY 0x08
#define BAD_FORMAT 0x0B
#define NOT_SAME_DEVICE 0x11
#define NOT_READY 0x15
#define GENERAL_FAILURE 0x1F
#define SHARING_VIOLATION 0x20
#define DISK_FULL 0x27

u16 DosErrorOf(int error, bool path)
{
    switch (error) {
    case ENOENT:
        return path ? PATH_NOT_FOUND : FILE_NOT_FOUND;
    case ENOTDIR:
    case ENAMETOOLONG:
    case ELOOP:
        return PATH_NOT_FOUND;
    case EMFILE:
    case ENFILE:
        return TOO_MANY_OPEN_FILES;
    case EACCES:
    case EPERM:
    case EROFS:
    case EEXIST:
    case ENOTEMPTY:
    case EISDIR:
        return ACCESS_DENIED;
    case EBADF:
        return INVALID_HANDLE;
    case ENOMEM:
        return NO_MEMORY;
    case ENOEXEC:
        return BAD_FORMAT;
    case EXDEV:
        return NOT_SAME_DEVICE;
    case ENOMEDIUM:
    case ENXIO:
    case ENODEV:
        return NOT_READY;
    case EBUSY:
    case ETXTBSY:
        return SHARING_VIOLATION;
    case ENOSPC:
    case EDQUOT:
        return DISK_FULL;
    default:
        return GENERAL_FAILURE;
    }
}
