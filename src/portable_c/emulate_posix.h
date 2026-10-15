/*
 * DOS as the program for Linux emulates it: the part that has no input
 * or output of its own, the piece L1 of step 6 of the portable C's road
 * (CLAUDE.md, "The portable C", the road, steps 3 and 6; TODO.md, "L1,
 * drives, names, code page, dates, errors"). The functions of sys.h for
 * Linux, sys_posix.c when it comes, ask these what a drive letter is on
 * the host, which name of the host a name of DOS's form leads to and the
 * other way round, how a name is written in the code page, what DOS's
 * attributes, date and time of a file are, and which of DOS's errors a
 * failure of the host's is. Each answers in DOS's terms, as sys.h has
 * them.
 *
 * Nothing of that reads or writes a file, the terminal or the
 * environment: what the host has, a directory's names, the mode of a
 * file, a time, the variables of the environment, is handed in. So all
 * of it is tried by itself on the host, by harness/posixcheck.c, the
 * test portable_c-posix-unit.
 *
 * The last part, of the piece L2 (TODO.md, "L2, files, directories,
 * drives"), is DOS's services for files, directories and drives
 * themselves, which do ask the host: struct Dos and the functions of
 * files_posix.c. The same test tries them on a tree of its own that it
 * makes in a temporary directory.
 *
 * On Linux there is one drive, C:, the root of the host's file system,
 * and one code page, 437, always, and nothing of either is a setting, on
 * the user's word of 14 October 2026 (CLAUDE.md, step 6).
 *
 * Host names and paths are `char`, as the host's functions take them,
 * bytes in UTF-8 by custom and not by rule. DOS's names and paths are
 * `u8`, bytes of the code page, as the rest of the program has them.
 */
#ifndef EMULATE_POSIX_H
#define EMULATE_POSIX_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "vc.h"

/*
 * The code page.
 *
 * The program writes names and texts in the code page of DOS's of the
 * United States, 437. Every byte of it is a character of Unicode, by
 * Unicode's mapping file of the code page (VENDORS/MICSFT/PC/CP437.TXT);
 * below 80h a byte is ASCII.
 */
#define CODE_PAGE 437

/* The character of Unicode that a byte of the code page is. */
u32 CodePageToUnicode(u8 byte);

/* The byte of the code page that a character of Unicode is, or -1 where
   the code page has no such character. */
i32 CodePageFromUnicode(u32 character);

/* The capital of a byte as DOS has it for a name of a file: DOS's table
   of capitals for names of the country 1 and the code page 437, as the
   bench's DOS, FreeDOS 1.4, has it in COUNTRY.SYS. Below 80h it is
   ASCII's. Most accented small letters become a plain capital, é E and
   â A, and only ä, å, æ, ç, ñ, ö and ü their own: that is DOS's, and a
   name that DOS is handed is made so. */
u8 CodePageUpper(u8 byte);

/* The character that the UTF-8 at `text` begins with, of the `length`
   bytes there, at `character`. Answers how many bytes it is, 1 to 4; or
   0 where the bytes there are no character of UTF-8, then `character`
   is not set. A sequence longer than it needs to be, a half of a pair
   of UTF-16 and anything above 10FFFFh are no characters. */
i32 Utf8Decode(const char *text, i32 length, u32 *character);

/* The bytes of UTF-8 of a character, at `to`, which has room for 4.
   Answers how many. */
i32 Utf8Encode(u32 character, char *to);

/* The small letter of a character of Unicode, by Unicode's simple
   mappings to lower case, UnicodeData.txt, for the capitals of Latin-1
   and of Greek, which are those that 437 has. Any other character is
   itself. */
u32 UnicodeSmall(u32 character);

/* A text of the code page, `length` bytes at `from`, in UTF-8 at `to`,
   which has room for `room` bytes, with a zero byte behind it, and with
   every letter small where `small` says so. Answers how many bytes it
   wrote, the zero not counted, or -1 where the room is not enough.
   Every byte of the code page has a character. */
i32 CodePageToUtf8(const u8 *from, i32 length, bool small, char *to,
                   i32 room);

/*
 * Names.
 *
 * A name of DOS's form, 8.3, is a name of 1 to 8 characters, and a dot
 * and 1 to 3 more where it has an extension, of the characters that DOS
 * lets a name have, in capitals as DOS's table has them. What DOS takes
 * is that of the bench's DOS, FreeDOS 1.4: none of 00h to 20h and none of
 * . " / \ [ ] : | < > + = ; , (its table of what ends a name, in
 * COUNTRY.SYS), nor * and ? (which match), as the Microsoft's
 * specification of FAT, fatgen103, has it too.
 */
#define DOS_NAME_ROOM 13                /* 8 + dot + 3 + zero */

/* Whether a byte may be in a name of DOS's form, the dot apart. */
bool DosNameByte(u8 byte);

/* Whether `name` is a name of DOS's form, in capitals: what a name of a
   host is shown as where it has its own, and what an alias is. */
bool DosNameValid(const u8 *name);

/* The attributes of DOS. */
#define DOS_READ_ONLY 0x01
#define DOS_HIDDEN 0x02
#define DOS_SYSTEM 0x04
#define DOS_LABEL 0x08
#define DOS_DIRECTORY 0x10
#define DOS_ARCHIVE 0x20

/* The attributes of DOS of a file of the host whose name is `name` and
   whose status is `status`: a directory or not; read-only where the
   mode's bits say that the program may not write it, by the bits of the
   owner where the program is its owner (`user`), by those of the group
   where it is of the file's group (`member`), and by the others'
   otherwise, with any bit of the three for the superuser; hidden where
   the name begins with a dot, but . and ..; archive for anything that is
   not a directory, as DOSBox has every file of a directory of the host;
   never system, never a label. */
u8 DosAttributes(const struct stat *status, const char *name, uid_t user,
                 bool member);

/* The names of one directory of the host, and the name of DOS's form
   that each is shown under: for the run, so that a name keeps the name
   that it was first given and an alias is never given to another. */
struct DirectoryName {
    char *host;                         /* the name of the host, its bytes */
    u8 dos[DOS_NAME_ROOM];              /* the name of DOS's form */
    bool present;                       /* if the host had it when it was
                                           last asked */
};
struct DirectoryNames {
    struct DirectoryName *names;
    i32 *by_host;                       /* two indexes of the names, */
    i32 *by_dos;                        /* names_posix.c's */
    i32 count;
    i32 room;
};

/* An empty table, and the table given back. */
void DirectoryNamesBegin(struct DirectoryNames *table);
void DirectoryNamesEnd(struct DirectoryNames *table);

/* What the host has in the directory now: `count` names at `names`, in
   any order, . and .. among them or not. A name that the table has keeps
   the name of DOS's that it has; one that it has not gets one, in the
   order of their bytes, by the rule of DosNameMake; one that the host
   no longer has is kept and is not found, and keeps its name of DOS's
   from every other name, for the run. Answers false where the memory
   ran out: the names that it had not come to then are not there. */
bool DirectoryNamesUpdate(struct DirectoryNames *table,
                          const char *const *names, i32 count);

/* The name of DOS's form of a name of the host that the directory has
   now, or NULL; and the name of the host that a name of DOS's form, in
   capitals, leads to, or NULL. . and .. are themselves. */
const u8 *DirectoryNamesDos(const struct DirectoryNames *table,
                            const char *host);
const char *DirectoryNamesHost(const struct DirectoryNames *table,
                               const u8 *dos);

/* The name of DOS's form that a name of the host gets, at `to`, where
   `taken` says which names the directory has given already: by the rule
   of fatgen103 for a short name. The name in capitals, in the code page,
   each character that the code page has not or that DOS does not let a
   name have made `_`; spaces and the dots that it begins with left out;
   up to 8 characters in front of the first dot that is left and up to 3
   of what follows the last. That is the name, where nothing was made
   `_`, left out or cut off and no name has it; otherwise the first of
   ~1, ~2 and on that no name has is put behind as much of the first part
   as leaves 8 characters at the most, LONGNA~1. A first part that
   nothing is left of is `_`. */
typedef bool DosNameTaken(const void *context, const u8 *name);
void DosNameMake(const char *host, DosNameTaken *taken, const void *context,
                 u8 *to);

/*
 * Drives and paths.
 *
 * A drive of DOS is a directory of the host, as DOSBox's `mount` has it,
 * and there is one: C:, the root of the host's file system. A path that
 * DOS's 67 characters do not hold is reached through the aliases.
 */
#define DOS_DRIVE 'C'                   /* the one drive there is */
#define DOS_PATH_ROOM 128               /* a full path, with its zero, as
                                           sys.h's FileTrueName has it */
#define DOS_PATH_LONGEST 67             /* what the program holds, its
                                           characters, the zero not
                                           counted */

/* Where the program's setup file is, the original's VC.INI: vc/vc.ini in
   $XDG_CONFIG_HOME, where that is set and is a path from the root, and
   in $HOME/.config otherwise, as the XDG Base Directory Specification
   has it, on the user's word of 14 October 2026 (CLAUDE.md, step 6).
   The two are handed in as the environment has them, or NULL. At `to`,
   which has room for `room` bytes: answers false where there is no $HOME
   to go by or the room is not enough. Reading and writing the file is
   not done here. */
bool SetupFile(const char *config_home, const char *home, char *to,
               i32 room);

/* A path of DOS's that the program hands over made a full one, as DOS
   does with it, to `to`, which has room for DOS_PATH_ROOM bytes: with its
   drive and from the root of the drive, / taken for \, . and .. worked
   out, and each name in capitals and cut to 8.3, as DOSBox's DOS_MakeName
   does. The current drive is C:, the one that there is, and `current` is
   its current directory, as DirCurrent of sys.h has it, without the drive
   and the first backslash, or NULL for the root; a path that names
   another drive goes from that drive's root, and HostPathOfDos says that
   there is no such drive. Answers whether it failed, with DOS's error 3
   where a name has a character that a name may not have or a second dot,
   0Fh for a drive that is no letter, and 3 where the full path is too
   long. */
struct Answer DosPathFull(const u8 *path, const u8 *current, u8 *to);

/* Every directory of the host that the program has named, with its
   names; and how a directory of the host is read, which is handed in:
   it calls DirectoryNamesUpdate for the table with what the host has
   there, and answers what DirectoryNamesUpdate answers, or false with
   errno where the directory cannot be read. */
struct Directory {
    char *path;                         /* of the host, from its root */
    struct DirectoryNames names;
};
typedef bool DirectoryRead(void *context, const char *path,
                           struct DirectoryNames *table);
struct Directories {
    struct Directory *directories;
    i32 count;
    i32 room;
    DirectoryRead *read;
    void *context;
};

void DirectoriesBegin(struct Directories *directories, DirectoryRead *read,
                      void *context);
void DirectoriesEnd(struct Directories *directories);

/* The number of a directory of the host, by its path from the root, in
   the table: it is read where it is new, and read again where `again`
   says so. The number stays the directory's for the run, whatever is
   added to the table. Answers -1 with errno where the directory cannot
   be read, and with ENOMEM where the memory ran out. */
i32 DirectoryNumber(struct Directories *directories, const char *path,
                    bool again);

/* The path of the host that a full path of DOS's leads to, at `to`,
   which has room for `room` bytes: the root, which is C:, and for each
   name the name of the host that it leads to in the directory before it;
   a name that the host has not, the last only, is the name that the
   program makes there, in UTF-8 and in small letters, as the user
   decided on 14 October 2026 (CLAUDE.md, step 6). Answers whether it
   failed, with DOS's error 0Fh for a drive other than C:, 3 for a
   directory on the way that the host has not or cannot be read and
   where the room is not enough, and 8 where the memory ran out; else
   `value` is 1 where the last name was found and 0 where it was not. */
struct Answer HostPathOfDos(struct Directories *directories,
                            const u8 *path, char *to, i32 room);

/* The full path of DOS's that a path of the host, from its root, is
   shown as, at `to`, which has room for DOS_PATH_ROOM bytes: on C:, and
   each name the name of DOS's form that it has in the directory before
   it. Answers whether it failed: DOS's error 3 where the path is not from
   the root or has . or .. in it, a directory on the way cannot be read, a
   name is not there, or the path is longer than DOS_PATH_LONGEST
   characters; 8 where the memory ran out. */
struct Answer DosPathOfHost(struct Directories *directories,
                            const char *path, u8 *to);

/*
 * Dates and times.
 *
 * DOS has a date and a time of the day packed in a word each, in the
 * local time: the year from 1980 in the bits from 9 up, the month in 5
 * to 8 and the day in 0 to 4; the hour in the bits from 11 up, the minute
 * in 5 to 10 and half the second in 0 to 4. That holds 1980 to 2107.
 */
struct DosStamp {
    u16 time;
    u16 date;
};

/* The stamp of DOS of a time of the host, in the local time. A time
   before 1980 is the first that DOS has, 1 January 1980 at midnight, and
   one after 2107 the last, 31 December 2107 at 23:59:58, as Linux writes
   them on a disk of DOS's (fs/fat/misc.c, fat_time_unix2fat); an odd
   second is the even one before it. */
struct DosStamp DosStampOfLocal(const struct tm *local);
struct DosStamp DosStampOfTime(time_t time);

/* The local time that a stamp of DOS is, for mktime: the fields as they
   are packed, which mktime makes a time of also where the stamp names no
   day that there is. */
void DosStampToLocal(struct DosStamp stamp, struct tm *local);

/*
 * Errors: DOS's number for a failure of the host, by its errno. `path`
 * says that the name that was not found is a directory on the way, or
 * one that the call takes for a directory: DOS answers 3 for that and 2
 * for a file. A number that has no better one is DOS's 1Fh, a general
 * failure.
 */
u16 DosErrorOf(int error, bool path);

/*
 * DOS's services for files, directories and drives, on the host's files:
 * the piece L2. Each is the function of sys.h of the same name behind
 * Dos, with the emulation's state in front, and answers as sys.h says
 * and as the bench's DOS, FreeDOS 1.4, answers there, AX as it leaves it
 * too where nothing failed (harness/trial32/asked.py shows that of every
 * function; the test names it at each check). The binding of sys.h for
 * Linux has one struct Dos and calls each in a line:
 *
 *     struct Answer FileOpen(const u8 *name, u16 mode)
 *     {
 *         return DosFileOpen(&Emulated, name, mode);
 *     }
 *
 * and the Bare functions of sys.h call the same function as the others:
 * the host has no device that asks the program what to do, so a call
 * that the program would have asked about fails as it is, with its
 * error. What the host answers that DOS has no number for is DOS's 1Fh,
 * a general failure (DosErrorOf). A call that the host interrupts, EINTR,
 * is made again.
 *
 * What a name of the program leads to is HostPathOfDos's: a name of
 * DOS's form, an alias, or for a file or a directory that is made, the
 * name in small letters. DOS's attributes are DosAttributes': a file of
 * the host is read-only where the mode's bits say that the program may
 * not write it, and DOS refuses then to open it to write, to make it
 * anew and to delete it, whoever the program runs as. A directory entry
 * that is no file and no directory, a FIFO, a socket or a device, is
 * listed as a file with what its status says, and is not opened: DOS's
 * 5, as for a directory. A link that leads nowhere is listed by its own
 * status and is not found when it is opened.
 *
 * A file of DOS is shorter than 4 GB: its size is 32 bits, on the user's
 * word that it stays so (CLAUDE.md, step 6). A file of the host that is
 * not is listed as FFFFFFFFh bytes long, the most that DOS has, and is
 * never copied or moved cut short, on the user's word too: it is not
 * opened to be read, DOS's 5, access denied, and a read that would go
 * past DOS's last place in a file that has grown beyond it since it was
 * opened is refused the same way (Opened and DosFileRead of
 * files_posix.c say why there). Opened to write alone, as the program
 * does to set its date and time, it is written no further than that
 * place, a write that would go beyond writing what fits, as on a disk
 * that is full.
 */

/* The handles of a program, as DOS's table of them in a program's
   prefix has room for by default; the first five are DOS's own, for the
   console, the serial port and the printer, which a program has when it
   starts. */
#define DOS_HANDLES 20
#define DOS_STANDARD 5

/* What one handle is. */
struct DosHandle {
    int fd;                             /* the host's, or -1 for one of
                                           DOS's own */
    u16 access;                         /* 0 to read, 1 to write, 2 both */
    u16 kind;                           /* of a device, what FileDevice
                                           answers */
    bool open;
    bool written;                       /* since it was opened */
    bool stamped;                       /* a time was set for it: it is
                                           set again when it is closed, as
                                           DOS has it then */
    struct DosStamp stamp;
};

/* The current directory of C: as DirCurrent answers it, 63 characters at
   the most and the zero: a path that DOS's 64 bytes do not hold is not
   made the current one. */
#define DOS_CURRENT_ROOM 64

struct Dos {
    struct Directories directories;
    u8 current[DOS_CURRENT_ROOM];       /* without the drive and the first
                                           backslash */
    struct DosHandle handles[DOS_HANDLES];
    struct DTAs *record;                /* of FindRecord, or `own` */
    struct DTAs own;                    /* the record before FindRecord
                                           has named one, as DOS has it in
                                           the program's prefix */
    u16 error;                          /* the error of the last call that
                                           failed, which DOS answers to a
                                           program that asks for it */
};

/* The state that the emulation begins with: C: the current drive, its
   root the current directory, DOS's five handles open, no record. And
   what it holds given back, every handle that is still open closed. */
void DosBegin(struct Dos *dos);
void DosEnd(struct Dos *dos);

/* The directory of the host that the program is started in, from its
   root, made the current directory of C:, as the program's path of it
   is (DosPathOfHost). Answers DOS's error 3 where that path is longer
   than DOS holds, and the current directory is not changed then. */
struct Answer DosStartIn(struct Dos *dos, const char *path);

/* Files. */
struct Answer DosFileCreate(struct Dos *dos, const u8 *name,
                            u16 attributes);
struct Answer DosFileOpen(struct Dos *dos, const u8 *name, u16 mode);
struct Answer DosFileClose(struct Dos *dos, u16 handle);
struct Moved DosFileRead(struct Dos *dos, u16 handle, u8 *to, i32 count);
struct Moved DosFileWrite(struct Dos *dos, u16 handle, const u8 *from,
                          i32 count);
struct FilePlace DosFileSeek(struct Dos *dos, u16 handle, u32 place);
struct FilePlace DosFileStep(struct Dos *dos, u16 handle, u16 from,
                             i32 step);
struct Answer DosFileDelete(struct Dos *dos, const u8 *name);
struct Answer DosFileRename(struct Dos *dos, const u8 *from, const u8 *to);
struct FileAttr DosFileAttributes(struct Dos *dos, const u8 *name);
struct Answer DosFileSetAttributes(struct Dos *dos, const u8 *name,
                                   u16 attributes);
struct FileStamp DosFileTime(struct Dos *dos, u16 handle);
struct Answer DosFileSetTime(struct Dos *dos, u16 handle, u16 time,
                             u16 date);
struct Device DosFileDevice(struct Dos *dos, u16 handle);
struct Answer DosFileTrueName(struct Dos *dos, const u8 *name, u8 *to);

/* Files by a file control block. */
u16 DosFcbParse(struct Dos *dos, const u8 *name, u8 *fcb, u16 how);
struct Answer DosFcbDelete(struct Dos *dos, const u8 *fcb, bool flagged);

/* Directories. */
void DosFindRecord(struct Dos *dos, struct DTAs *record);
struct Answer DosFindFirst(struct Dos *dos, const u8 *pattern,
                           u16 attributes);
struct Answer DosFindNext(struct Dos *dos);
struct Answer DosDirMake(struct Dos *dos, const u8 *name);
struct Answer DosDirRemove(struct Dos *dos, const u8 *name);
struct Answer DosDirChange(struct Dos *dos, const u8 *name);
struct Answer DosDirCurrent(struct Dos *dos, u16 drive, u8 *to);

/* Drives. C: is the one there is; it is fixed, of no network and no
   CD-ROM, and shares its unit with no other letter.

   How much room it has is that of the host's file system of its current
   directory, by statvfs: what is free to the program, in DOS's shape,
   which has no more than 16 bits for a count, as DOS answers INT 21h 36h.
   A sector is 512 bytes; a cluster is as many sectors as the host's
   block has, a power of two, and twice as many for as long as the
   clusters of the drive do not fit 16 bits, up to 128 sectors, 64 KB, as
   the program shows (src/portable_c/DEVIATIONS.md, 26); a count that does
   not fit even then is FFFFh. So a drive shows 4 GB less 64 KB at the
   most, of room and of room that is free. */
u16 DosDriveCurrent(struct Dos *dos);
void DosDriveSelect(struct Dos *dos, u16 drive);
struct Space DosDriveSpace(struct Dos *dos, u16 drive, bool flagged);
struct Answer DosDriveLetter(struct Dos *dos, u16 drive);
struct Answer DosDriveSetLetter(struct Dos *dos, u16 drive);
struct Device DosDriveRemote(struct Dos *dos, u16 drive);
struct Answer DosDriveFixed(struct Dos *dos, u16 drive);
bool DosDriveCd(struct Dos *dos, u16 drive);

/* The room of a drive in DOS's shape, of a file system of `block` bytes
   to a block with `blocks` blocks of which `free` are free to the
   program, as DosDriveSpace answers it: by itself, so that it is tried
   with file systems that the host where it is tried has not. */
struct Space DosSpaceOf(u32 block, uint64_t blocks, uint64_t free);

#endif
