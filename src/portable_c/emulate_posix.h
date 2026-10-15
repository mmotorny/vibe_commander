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
 * Nothing here reads or writes a file, the terminal or the environment:
 * what the host has, a directory's names, the mode of a file, a time, the
 * text of the file of settings, the locale's variables, is handed in. So
 * all of it is tried by itself on the host, by harness/posixcheck.c, the
 * test portable_c-posix-unit.
 *
 * Host names and paths are `char`, as the host's functions take them,
 * bytes in UTF-8 by custom and not by rule. DOS's names and paths are
 * `u8`, bytes of the code page, as the rest of the program has them.
 */
#ifndef EMULATE_POSIX_H
#define EMULATE_POSIX_H

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "vc.h"

/*
 * The code page.
 *
 * The program writes names and texts in one code page of DOS's: 437, of
 * the United States, or 866, of Russia, the two that the scenarios test.
 * Every byte of either is a character of Unicode, by Unicode's mapping
 * files of the code pages (VENDORS/MICSFT/PC/CP437.TXT and CP866.TXT);
 * below 80h a byte is ASCII.
 */
#define CODE_PAGE_US 437
#define CODE_PAGE_RUSSIA 866

/* Which code page the program uses: `setting` where the file of
   settings names one, which is 0 where it names none; else 866 where
   the locale is Russian and 437 otherwise. The locale is the first of
   LC_ALL, LC_CTYPE and LANG that is set and not empty, as POSIX has it
   for the type of characters; it is Russian where its language is "ru".
   Each of the three is handed in as the environment has it, or NULL. */
u16 CodePageChosen(u16 setting, const char *lc_all, const char *lc_ctype,
                   const char *lang);

/* The character of Unicode that a byte of the code page is. */
u32 CodePageToUnicode(u16 code_page, u8 byte);

/* The byte of the code page that a character of Unicode is, or -1 where
   the code page has no such character. */
i32 CodePageFromUnicode(u16 code_page, u32 character);

/* The capital of a byte as DOS has it for a name of a file: DOS's table
   of capitals for names of the country and code page, as the bench's
   DOS, FreeDOS 1.4, has it in COUNTRY.SYS. Below 80h it is ASCII's.
   Of 437 most accented small letters become a plain capital, é E and
   â A, and only ä, å, æ, ç, ñ, ö and ü their own: that is DOS's, and a
   name that DOS is handed is made so. */
u8 CodePageUpper(u16 code_page, u8 byte);

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
   mappings to lower case, UnicodeData.txt, for the capitals that 437
   and 866 have: Latin, Greek and Cyrillic. Any other character is
   itself. */
u32 UnicodeSmall(u32 character);

/* A text of the code page, `length` bytes at `from`, in UTF-8 at `to`,
   which has room for `room` bytes, with a zero byte behind it, and with
   every letter small where `small` says so. Answers how many bytes it
   wrote, the zero not counted, or -1 where the room is not enough.
   Every byte of the code page has a character. */
i32 CodePageToUtf8(u16 code_page, const u8 *from, i32 length, bool small,
                   char *to, i32 room);

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
bool DosNameValid(u16 code_page, const u8 *name);

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
bool DirectoryNamesUpdate(struct DirectoryNames *table, u16 code_page,
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
void DosNameMake(u16 code_page, const char *host, DosNameTaken *taken,
                 const void *context, u8 *to);

/*
 * Drives and paths.
 *
 * A drive of DOS is a directory of the host, as DOSBox's `mount` has it:
 * C: is the root of the host's file system, unless the file of settings
 * says otherwise, and the file of settings may name other letters.
 */
#define DOS_DRIVES 26
#define DOS_PATH_ROOM 128               /* a full path, with its zero, as
                                           sys.h's FileTrueName has it */
#define DOS_PATH_LONGEST 67             /* what the program holds, its
                                           characters, the zero not
                                           counted */

/* Where the file of settings is: the file that the variable of the
   environment VC_SETTINGS names, where it is set and not empty, for the
   tests; else vc/settings in $XDG_CONFIG_HOME, where that is set and is
   a path from the root, and in $HOME/.config otherwise, as the XDG Base
   Directory Specification has it, on the user's word of 14 October 2026
   (CLAUDE.md, step 6). The three are handed in as the environment has
   them, or NULL. At `to`, which has room for `room` bytes: answers false
   where there is no $HOME to go by or the room is not enough. */
#define SETTINGS_VARIABLE "VC_SETTINGS"
bool SettingsFile(const char *named, const char *config_home,
                  const char *home, char *to, i32 room);

/* What the file of settings says. */
struct Settings {
    char *drive[DOS_DRIVES];            /* the directory of each letter,
                                           A: first, or NULL for none */
    u16 code_page;                      /* 437 or 866, or 0 for none */
};

/* The settings that there are where the file says nothing: C: the root. */
bool SettingsBegin(struct Settings *settings);
void SettingsEnd(struct Settings *settings);

/* Read the text of the file of settings into `settings`, which
   SettingsBegin has made. A line is empty, or a comment behind #, or
   one of
       mount <letter> <directory>     the drive is that directory, a path
                                      that begins with /
       codepage <437 or 866>
   with the words apart by blanks; a directory is the rest of its line,
   its blanks at the ends left out. Answers 0, or the number of the first
   line that is none of these, counted from 1, with the settings as the
   lines before it made them; or -1 where the memory ran out. */
i32 SettingsRead(struct Settings *settings, const char *text);

/* A path of DOS's that the program hands over made a full one, as DOS
   does with it, to `to`, which has room for DOS_PATH_ROOM bytes: with its
   drive and from the root of the drive, / taken for \, . and .. worked
   out, and each name in capitals and cut to 8.3, as DOSBox's DOS_MakeName
   does. `drive` is the current drive, 0 for A:, and `current` the
   current directory of each, as DirCurrent of sys.h has it, without the
   drive and the first backslash, or NULL for the root. Answers whether
   it failed, with DOS's error 3 where a name has a character that a name
   may not have or a second dot, 0Fh for a drive that is no letter, and
   3 where the full path is too long. */
struct Answer DosPathFull(u16 code_page, const u8 *path, u16 drive,
                          const u8 *const *current, u8 *to);

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
                           struct DirectoryNames *table, u16 code_page);
struct Directories {
    struct Directory *directories;
    i32 count;
    i32 room;
    u16 code_page;
    DirectoryRead *read;
    void *context;
};

void DirectoriesBegin(struct Directories *directories, u16 code_page,
                      DirectoryRead *read, void *context);
void DirectoriesEnd(struct Directories *directories);

/* The path of the host that a full path of DOS's leads to, at `to`,
   which has room for `room` bytes: the directory of its drive, and for
   each name the name of the host that it leads to in the directory
   before it; a name that the host has not, the last only, is the name
   that the program makes there, in UTF-8 and in small letters, as the
   user decided on 14 October 2026 (CLAUDE.md, step 6). Answers whether it failed, with DOS's
   error 0Fh for a drive that is none, 3 for a directory on the way that
   the host has not or cannot be read and where the room is not enough,
   and 8 where the memory ran out; else `value` is 1 where the last name
   was found and 0 where it was not. */
struct Answer HostPathOfDos(struct Directories *directories,
                            const struct Settings *settings,
                            const u8 *path, char *to, i32 room);

/* The full path of DOS's that a path of the host, from its root, is
   shown as, at `to`, which has room for DOS_PATH_ROOM bytes: on the drive
   whose directory holds it with the most of its path, and each name the
   name of DOS's form that it has in the directory before it. Answers
   whether it failed: DOS's error 3 where no drive holds it, a directory
   on the way cannot be read, a name is not there, or the path is longer
   than DOS_PATH_LONGEST characters; 8 where the memory ran out. */
struct Answer DosPathOfHost(struct Directories *directories,
                            const struct Settings *settings,
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

#endif
