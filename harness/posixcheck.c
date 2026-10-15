/*
 * The test portable_c-posix-unit: what the program for Linux emulates of
 * DOS without input or output of its own, the files src/portable_c/
 * *_posix.c of the piece L1 (emulate_posix.h), held to what DOS, DOSBox
 * and the documents that they follow do. Each expectation says where it
 * comes from; none was worked out from the code.
 *
 * Built by CMakeLists.txt with clang, its full list of warnings and the
 * sanitizers of addresses, of undefined behaviour, of integers and of
 * implicit conversions, all fatal, and read by clang-tidy in the test
 * portable_c-stub (harness/stubcheck.py). It prints each check that
 * fails and exits with 1 if any did.
 *
 * The sources named:
 *   CP437.TXT              Unicode's mapping file of the code page,
 *                          VENDORS/MICSFT/PC/
 *   COUNTRY.SYS            FreeDOS 1.4's, of the bench: its tables of
 *                          capitals for names, FUCASE, and of what ends a
 *                          name, FCHAR, for the country 1 with the code
 *                          page 437
 *   fatgen103              Microsoft's specification of FAT, 1.03: how a
 *                          short name is made of a long one
 *   DOS_MakeName           DOSBox 0.74, src/dos/dos_files.cpp: how DOSBox
 *                          makes a full path of a path of a DOS program
 *   DOSBox localDrive      DOSBox 0.74, src/dos/drive_local.cpp: a file of
 *                          a directory of the host has the attribute
 *                          archive, a directory that of a directory
 *   RBIL                   Ralf Brown's list of interrupts, INT 21h: the
 *                          errors of each call, the attributes, the
 *                          packed date and time
 *   XDG                    the XDG Base Directory Specification, 0.8:
 *                          $XDG_CONFIG_HOME and its default
 *   RFC 3629               UTF-8
 *   fs/fat/misc.c          Linux, fat_time_unix2fat: a time of the host on
 *                          a disk of DOS's
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "emulate_posix.h"

static int failures;

static void Check(bool good, const char *what, int line)
{
    if (!good) {
        printf("posixcheck.c:%d: %s\n", line, what);
        failures++;
    }
}

#define CHECK(condition) Check((condition), #condition, __LINE__)
#define SAME(one, other) \
    Check(strcmp((const char *)(one), (const char *)(other)) == 0, \
          #one " is " #other, __LINE__)

/*
 * The code page.
 */

static void CodePages(void)
{
    i32 byte;

    /* CP437.TXT */
    CHECK(CodePageToUnicode(0x80) == 0x00C7);
    CHECK(CodePageToUnicode(0x82) == 0x00E9);
    CHECK(CodePageToUnicode(0xB3) == 0x2502);
    CHECK(CodePageToUnicode(0xE1) == 0x00DF);
    CHECK(CodePageToUnicode(0xFF) == 0x00A0);
    CHECK(CodePageToUnicode('A') == 'A');
    CHECK(CodePageFromUnicode(0x0451) == -1);
    CHECK(CodePageFromUnicode(0x00E9) == 0x82);
    CHECK(CodePageFromUnicode(0x20AC) == -1);
    /* The file maps 256 bytes to 256 characters: every byte comes
       back. */
    for (byte = 0; byte < 256; byte++)
        CHECK(CodePageFromUnicode(CodePageToUnicode((u8)byte)) == byte);

    /* COUNTRY.SYS, FUCASE of 437 */
    CHECK(CodePageUpper('a') == 'A');
    CHECK(CodePageUpper('Z') == 'Z');
    CHECK(CodePageUpper('~') == '~');
    CHECK(CodePageUpper(0x81) == 0x9A);
    CHECK(CodePageUpper(0x82) == 'E');
    CHECK(CodePageUpper(0x83) == 'A');
    CHECK(CodePageUpper(0x84) == 0x8E);
    CHECK(CodePageUpper(0x98) == 'Y');
    CHECK(CodePageUpper(0xA4) == 0xA5);
    CHECK(CodePageUpper(0xE1) == 0xE1);
}

static void Utf8(void)
{
    u32 character = 0;
    char bytes[4];
    char text[32];
    static const u8 Cafe437[] = { 'c', 'a', 'f', 0x82 };

    /* RFC 3629, its examples */
    CHECK(Utf8Decode("\xE6\x97\xA5", 3, &character) == 3
          && character == 0x65E5);
    CHECK(Utf8Decode("\xF0\xA3\x8E\xB4", 4, &character) == 4
          && character == 0x233B4);
    CHECK(Utf8Decode("A", 1, &character) == 1 && character == 'A');
    CHECK(Utf8Decode("\xD0\x9F", 2, &character) == 2
          && character == 0x041F);
    /* RFC 3629: a sequence longer than it needs, a surrogate, past
       10FFFFh, a lone byte that continues, one cut short */
    CHECK(Utf8Decode("\xC0\xAF", 2, &character) == 0);
    CHECK(Utf8Decode("\xE0\x80\xAF", 3, &character) == 0);
    CHECK(Utf8Decode("\xED\xA0\x80", 3, &character) == 0);
    CHECK(Utf8Decode("\xF4\x90\x80\x80", 4, &character) == 0);
    CHECK(Utf8Decode("\x80", 1, &character) == 0);
    CHECK(Utf8Decode("\xD0", 1, &character) == 0);
    CHECK(Utf8Decode("\xE9.txt", 5, &character) == 0);
    CHECK(Utf8Encode(0x65E5, bytes) == 3 && memcmp(bytes, "\xE6\x97\xA5", 3)
          == 0);
    CHECK(Utf8Encode(0x233B4, bytes) == 4
          && memcmp(bytes, "\xF0\xA3\x8E\xB4", 4) == 0);

    /* CP437.TXT: café */
    CHECK(CodePageToUtf8(Cafe437, 4, false, text, (i32)sizeof text) == 5);
    SAME(text, "caf\xC3\xA9");
    CHECK(CodePageToUtf8(Cafe437, 4, false, text, 5) == -1);
    CHECK(CodePageToUtf8(Cafe437, 4, false, text, 6) == 5);

    /* UnicodeData.txt, the simple mappings to lower case, as Python's
       str.lower has them for every byte of the code page; CLAUDE.md,
       step 6: what the program makes on the host is in small letters. */
    CHECK(UnicodeSmall('N') == 'n');
    CHECK(UnicodeSmall('n') == 'n');
    CHECK(UnicodeSmall('~') == '~');
    CHECK(UnicodeSmall(0x00DC) == 0x00FC);
    CHECK(UnicodeSmall(0x00C7) == 0x00E7);
    CHECK(UnicodeSmall(0x00D7) == 0x00D7);
    CHECK(UnicodeSmall(0x00DF) == 0x00DF);
    CHECK(UnicodeSmall(0x0393) == 0x03B3);
    CHECK(UnicodeSmall(0x03A3) == 0x03C3);
    CHECK(UnicodeSmall(0x03A9) == 0x03C9);
    CHECK(UnicodeSmall(0x03C6) == 0x03C6);
    CHECK(UnicodeSmall(0x00B5) == 0x00B5);
    CHECK(UnicodeSmall(0x2502) == 0x2502);
    CHECK(CodePageToUtf8((const u8 *)"\x9A\x80.TXT", 6, true, text,
                         (i32)sizeof text) == 8);
    SAME(text, "\xC3\xBC\xC3\xA7.txt");
}

/*
 * Names.
 */

static const u8 *DosOf(const struct DirectoryNames *table, const char *host)
{
    const u8 *dos = DirectoryNamesDos(table, host);

    return dos != NULL ? dos : (const u8 *)"(none)";
}

static const char *HostOf(const struct DirectoryNames *table, const char *dos)
{
    const char *host = DirectoryNamesHost(table, (const u8 *)dos);

    return host != NULL ? host : "(none)";
}

/* Документы, "documents", in UTF-8. */
#define DOCUMENTS \
    ("\xD0\x94\xD0\xBE\xD0\xBA\xD1\x83\xD0\xBC\xD0\xB5\xD0\xBD\xD1\x82" \
     "\xD1\x8B")

#define COUNT(names) ((i32)(sizeof (names) / sizeof *(names)))

static void Names(void)
{
    struct DirectoryNames table;
    static const char *const Directory[] = {
        "README.TXT", "readme.txt", "Makefile", "The quick brown.fox",
        "The quick brown.foxes", ".bashrc", "archive.tar.gz", "a+b.c",
        "LongFileName", "caf\xC3\xA9.txt", "caf\xE9.txt", DOCUMENTS, ".",
        ".."
    };
    static const char *const Tens[] = {
        "LongFileName0", "LongFileName1", "LongFileName2", "LongFileName3",
        "LongFileName4", "LongFileName5", "LongFileName6", "LongFileName7",
        "LongFileName8", "LongFileName9", "LongFileName10"
    };
    static const char *const First[] = { "longfilename1", "longfilename2" };
    static const char *const Second[] = { "longfilename2", "longfilename3" };
    static const char *const Third[] = {
        "longfilename3", "longfilename1", "longfilename2"
    };
    static const char *const Lower[] = { "readme.txt" };
    static const char *const Both[] = { "readme.txt", "README.TXT" };

    /* COUNTRY.SYS, FCHAR, and fatgen103: what a name of DOS's form is */
    CHECK(DosNameValid((const u8 *)"README.TXT"));
    CHECK(DosNameValid((const u8 *)"A"));
    CHECK(DosNameValid((const u8 *)"LONGFI~1.TXT"));
    CHECK(DosNameValid((const u8 *)"VIBE-C~1"));
    CHECK(!DosNameValid((const u8 *)"readme.txt"));
    CHECK(!DosNameValid((const u8 *)"TOOLONGNAME"));
    CHECK(!DosNameValid((const u8 *)"A.B.C"));
    CHECK(!DosNameValid((const u8 *)".TXT"));
    CHECK(!DosNameValid((const u8 *)"NAME."));
    CHECK(!DosNameValid((const u8 *)"A.TEXT"));
    CHECK(!DosNameValid((const u8 *)"A+B"));
    CHECK(!DosNameValid((const u8 *)"A B"));
    CHECK(!DosNameValid((const u8 *)"*.*"));
    CHECK(!DosNameValid((const u8 *)""));
    CHECK(!DosNameValid((const u8 *)"\x82"));
    CHECK(DosNameValid((const u8 *)"\x90"));

    /* fatgen103, the basis name and its numeric tail, in the order of the
       names' bytes; FCHAR for `+`; DOS's capitals of 437 for é. */
    DirectoryNamesBegin(&table);
    CHECK(DirectoryNamesUpdate(&table, Directory, COUNT(Directory)));
    SAME(DosOf(&table, "README.TXT"), "README.TXT");
    SAME(DosOf(&table, "readme.txt"), "README~1.TXT");
    SAME(DosOf(&table, "Makefile"), "MAKEFILE");
    SAME(DosOf(&table, "The quick brown.fox"), "THEQUI~1.FOX");
    SAME(DosOf(&table, "The quick brown.foxes"), "THEQUI~2.FOX");
    SAME(DosOf(&table, ".bashrc"), "BASHRC~1");
    SAME(DosOf(&table, "archive.tar.gz"), "ARCHIV~1.GZ");
    SAME(DosOf(&table, "a+b.c"), "A_B~1.C");
    SAME(DosOf(&table, "LongFileName"), "LONGFI~1");
    SAME(DosOf(&table, "caf\xC3\xA9.txt"), "CAFE.TXT");
    SAME(DosOf(&table, "caf\xE9.txt"), "CAF_~1.TXT");
    SAME(DosOf(&table, DOCUMENTS), "______~1");
    SAME(DosOf(&table, "."), ".");
    SAME(DosOf(&table, ".."), "..");
    SAME(DosOf(&table, "elsewhere"), "(none)");
    /* Each leads back to its bytes, the one that is no UTF-8 too. */
    SAME(HostOf(&table, "README.TXT"), "README.TXT");
    SAME(HostOf(&table, "README~1.TXT"), "readme.txt");
    SAME(HostOf(&table, "CAF_~1.TXT"), "caf\xE9.txt");
    SAME(HostOf(&table, "CAFE.TXT"), "caf\xC3\xA9.txt");
    SAME(HostOf(&table, "THEQUI~2.FOX"), "The quick brown.foxes");
    SAME(HostOf(&table, ".."), "..");
    SAME(HostOf(&table, "NOWHERE"), "(none)");
    DirectoryNamesEnd(&table);

    /* fatgen103: ~10 leaves 5 characters of the first part. */
    DirectoryNamesBegin(&table);
    CHECK(DirectoryNamesUpdate(&table, Tens, COUNT(Tens)));
    SAME(DosOf(&table, "LongFileName0"), "LONGFI~1");
    SAME(DosOf(&table, "LongFileName10"), "LONGFI~3");
    SAME(DosOf(&table, "LongFileName7"), "LONGFI~9");
    SAME(DosOf(&table, "LongFileName8"), "LONGF~10");
    SAME(DosOf(&table, "LongFileName9"), "LONGF~11");
    DirectoryNamesEnd(&table);

    /* CLAUDE.md, step 3 of the road: an alias stays the same for the run
       and leads back to its name; one that is gone is given to no other. */
    DirectoryNamesBegin(&table);
    CHECK(DirectoryNamesUpdate(&table, First, COUNT(First)));
    SAME(DosOf(&table, "longfilename1"), "LONGFI~1");
    SAME(DosOf(&table, "longfilename2"), "LONGFI~2");
    CHECK(DirectoryNamesUpdate(&table, Second, COUNT(Second)));
    SAME(DosOf(&table, "longfilename2"), "LONGFI~2");
    SAME(DosOf(&table, "longfilename3"), "LONGFI~3");
    SAME(DosOf(&table, "longfilename1"), "(none)");
    SAME(HostOf(&table, "LONGFI~1"), "(none)");
    CHECK(DirectoryNamesUpdate(&table, Third, COUNT(Third)));
    SAME(DosOf(&table, "longfilename1"), "LONGFI~1");
    SAME(HostOf(&table, "LONGFI~1"), "longfilename1");
    CHECK(DirectoryNamesUpdate(&table, NULL, 0));
    SAME(HostOf(&table, "LONGFI~2"), "(none)");
    DirectoryNamesEnd(&table);

    /* A name of the host that comes beside one that differs in case only
       gets the alias: the first keeps its name. */
    DirectoryNamesBegin(&table);
    CHECK(DirectoryNamesUpdate(&table, Lower, COUNT(Lower)));
    SAME(DosOf(&table, "readme.txt"), "README.TXT");
    CHECK(DirectoryNamesUpdate(&table, Both, COUNT(Both)));
    SAME(DosOf(&table, "readme.txt"), "README.TXT");
    SAME(DosOf(&table, "README.TXT"), "README~1.TXT");
    DirectoryNamesEnd(&table);
}

static void Attributes(void)
{
    struct stat status;

    /* RBIL, INT 21h 4300h: the bits; DOSBox localDrive: archive for a
       file, directory for a directory; CLAUDE.md and TODO.md, the
       surveys: read-only of the mode, hidden of the dot. */
    memset(&status, 0, sizeof status);
    status.st_uid = 1000;
    status.st_mode = S_IFREG | 0644;
    CHECK(DosAttributes(&status, "file", 1000, true) == DOS_ARCHIVE);
    status.st_mode = S_IFREG | 0444;
    CHECK(DosAttributes(&status, "file", 1000, true)
          == (DOS_ARCHIVE | DOS_READ_ONLY));
    status.st_mode = S_IFDIR | 0755;
    CHECK(DosAttributes(&status, "directory", 1000, true) == DOS_DIRECTORY);
    status.st_mode = S_IFDIR | 0555;
    CHECK(DosAttributes(&status, "directory", 1000, true) == DOS_DIRECTORY);
    CHECK(DosAttributes(&status, ".", 1000, true) == DOS_DIRECTORY);
    CHECK(DosAttributes(&status, "..", 1000, true) == DOS_DIRECTORY);
    CHECK(DosAttributes(&status, ".config", 1000, true)
          == (DOS_DIRECTORY | DOS_HIDDEN));
    status.st_mode = S_IFREG | 0644;
    CHECK(DosAttributes(&status, ".bashrc", 1000, true)
          == (DOS_ARCHIVE | DOS_HIDDEN));
    /* The bits of the group, then the others', as POSIX's access does. */
    status.st_uid = 0;
    status.st_mode = S_IFREG | 0660;
    CHECK(DosAttributes(&status, "file", 1000, true) == DOS_ARCHIVE);
    CHECK(DosAttributes(&status, "file", 1000, false)
          == (DOS_ARCHIVE | DOS_READ_ONLY));
    status.st_mode = S_IFREG | 0646;
    CHECK(DosAttributes(&status, "file", 1000, false) == DOS_ARCHIVE);
    status.st_mode = S_IFREG | 0444;
    CHECK(DosAttributes(&status, "file", 0, true)
          == (DOS_ARCHIVE | DOS_READ_ONLY));
    status.st_mode = S_IFREG | 0604;
    CHECK(DosAttributes(&status, "file", 0, false) == DOS_ARCHIVE);
}

/*
 * Paths.
 */

static void Full(void)
{
    u8 to[DOS_PATH_ROOM];
    struct Answer answer;
    const u8 *current = (const u8 *)"A\\B";
    u8 deep[200];
    i32 i;

    /* DOS_MakeName of DOSBox */
    answer = DosPathFull((const u8 *)"c:/foo/bar", current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\FOO\\BAR");
    answer = DosPathFull((const u8 *)"..\\x", current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\A\\X");
    answer = DosPathFull((const u8 *)"LONGFILENAME.TEXT", current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\A\\B\\LONGFILE.TEX");
    answer = DosPathFull((const u8 *)".\\file", current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\A\\B\\FILE");
    answer = DosPathFull((const u8 *)"C:\\..", current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\");
    answer = DosPathFull((const u8 *)"\\..\\..\\x", current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\X");
    answer = DosPathFull((const u8 *)"\\", current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\");
    answer = DosPathFull((const u8 *)"C:", current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\A\\B");
    /* CLAUDE.md, step 6 of the road: C: is the one drive, and the
       current one; another goes from its root, and is found to be none
       when it is used. */
    answer = DosPathFull((const u8 *)"d:", current, to);
    CHECK(!answer.failed);
    SAME(to, "D:\\");
    answer = DosPathFull((const u8 *)"d:name", current, to);
    CHECK(!answer.failed);
    SAME(to, "D:\\NAME");
    answer = DosPathFull((const u8 *)"name", current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\A\\B\\NAME");
    answer = DosPathFull((const u8 *)"*.*", current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\A\\B\\*.*");
    answer = DosPathFull((const u8 *)"c:\\caf\x82.txt", current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\CAFE.TXT");
    answer = DosPathFull((const u8 *)"a.b.c", current, to);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathFull((const u8 *)"a+b", current, to);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathFull((const u8 *)"1:\\x", current, to);
    CHECK(answer.failed && answer.value == 0x0F);
    /* A path longer than DOS's 128 bytes */
    for (i = 0; i < 20; i++)
        memcpy(deep + 9 * i, "\\ABCDEFGH", 9);
    deep[180] = '\0';
    answer = DosPathFull(deep, current, to);
    CHECK(answer.failed && answer.value == 3);
}

static void SetupFileChecked(void)
{
    char file[64];

    /* CLAUDE.md, step 6 of the road: vc/vc.ini where XDG says. XDG:
       $HOME/.config where $XDG_CONFIG_HOME is not set or is empty, and a
       path in it that is not absolute is ignored. */
    CHECK(SetupFile("/home/user/.cfg", "/home/user", file, (i32)sizeof file));
    SAME(file, "/home/user/.cfg/vc/vc.ini");
    CHECK(SetupFile(NULL, "/home/user", file, (i32)sizeof file));
    SAME(file, "/home/user/.config/vc/vc.ini");
    CHECK(SetupFile("", "/home/user", file, (i32)sizeof file));
    SAME(file, "/home/user/.config/vc/vc.ini");
    CHECK(SetupFile("relative", "/home/user", file, (i32)sizeof file));
    SAME(file, "/home/user/.config/vc/vc.ini");
    CHECK(!SetupFile(NULL, NULL, file, (i32)sizeof file));
    CHECK(!SetupFile(NULL, "", file, (i32)sizeof file));
    CHECK(!SetupFile(NULL, "/home/user", file, 28));
    CHECK(SetupFile(NULL, "/home/user", file, 29));
}

/* A file system of the host for the test: each directory and its
   names, read as the program would read one, and a directory that
   cannot be read. */
struct FakeDirectory {
    const char *path;
    const char *names[12];
};

static struct FakeDirectory Fake[] = {
    { "/", { "home", "etc", "root", "\xD0\x94\xD0\xBE\xD0\xBA", NULL } },
    { "/home", { "user", NULL } },
    { "/home/user", { "Projects", "notes.txt", "LongFileName.txt", ".",
                      "..", NULL } },
    { "/home/user/Projects", { "vibe-commander", NULL } },
    { "/home/user/Projects/vibe-commander", { NULL } },
    { "/etc", { NULL } },
    { "/deep", { "DIRNAME2", NULL } },
};

static bool FakeRead(void *context, const char *path,
                     struct DirectoryNames *table)
{
    i32 *reads = context;
    i32 i;
    i32 count;

    (*reads)++;
    for (i = 0; i < COUNT(Fake); i++) {
        if (strcmp(Fake[i].path, path) != 0)
            continue;
        for (count = 0; Fake[i].names[count] != NULL; count++)
            ;
        return DirectoryNamesUpdate(table, Fake[i].names, count);
    }
    errno = strcmp(path, "/root") == 0 ? EACCES : ENOENT;
    return false;
}

/* A directory of the deep chain: /DIRNAME1/DIRNAME2/... each with the
   next in it, to DIRNAME9. */
static bool DeepRead(void *context, const char *path,
                     struct DirectoryNames *table)
{
    char name[9];
    const char *names[1];
    i32 depth = 0;
    const char *at;

    (void)context;
    for (at = path; *at != '\0'; at++)
        if (*at == '/')
            depth++;
    if (strcmp(path, "/") == 0)
        depth = 0;
    if (depth >= 9)
        return DirectoryNamesUpdate(table, NULL, 0);
    memcpy(name, "DIRNAME", 7);
    name[7] = (char)('1' + depth);
    name[8] = '\0';
    names[0] = name;
    return DirectoryNamesUpdate(table, names, 1);
}

static void Paths(void)
{
    struct Directories directories;
    struct Answer answer;
    char host[256];
    u8 dos[DOS_PATH_ROOM];
    i32 reads = 0;

    DirectoriesBegin(&directories, FakeRead, &reads);

    /* CLAUDE.md, step 3 of the road: a drive letter is a directory of
       the host, as DOSBox's mount; step 6: C: is the root of the file
       system, and there is no other; an alias leads back. */
    answer = HostPathOfDos(&directories,
                           (const u8 *)"C:\\HOME\\USER\\NOTES.TXT", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/home/user/notes.txt");
    answer = HostPathOfDos(&directories,
                           (const u8 *)"C:\\HOME\\USER\\LONGFI~1.TXT", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/home/user/LongFileName.txt");
    answer = HostPathOfDos(&directories, (const u8 *)"C:\\", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/");
    answer = HostPathOfDos(&directories,
                           (const u8 *)"C:\\HOME\\USER\\NEW.TXT", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 0);
    SAME(host, "/home/user/new.txt");
    /* RBIL, INT 21h 3Dh: 3 where a directory on the way is not there,
       0Fh for a drive that there is not (INT 21h 0Eh and 47h) */
    answer = HostPathOfDos(&directories,
                           (const u8 *)"C:\\HOME\\USER\\NOPE\\X.TXT", host,
                           (i32)sizeof host);
    CHECK(answer.failed && answer.value == 3);
    answer = HostPathOfDos(&directories, (const u8 *)"D:\\X", host,
                           (i32)sizeof host);
    CHECK(answer.failed && answer.value == 0x0F);
    answer = HostPathOfDos(&directories, (const u8 *)"A:\\", host,
                           (i32)sizeof host);
    CHECK(answer.failed && answer.value == 0x0F);
    answer = HostPathOfDos(&directories, (const u8 *)"C:\\ROOT\\X", host,
                           (i32)sizeof host);
    CHECK(answer.failed && answer.value == 3);
    answer = HostPathOfDos(&directories, (const u8 *)"C:\\HOME\\USER", host,
                           10);
    CHECK(answer.failed && answer.value == 3);

    /* A name that the host has since it was read is found: the directory
       is read again for a name that it has not. */
    Fake[2].names[5] = "added.txt";
    answer = HostPathOfDos(&directories,
                           (const u8 *)"C:\\HOME\\USER\\ADDED.TXT", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/home/user/added.txt");
    Fake[2].names[5] = NULL;

    /* CLAUDE.md, step 6 of the road: every path of the host is on C:. */
    answer = DosPathOfHost(&directories,
                           "/home/user/Projects/vibe-commander", dos);
    CHECK(!answer.failed);
    SAME(dos, "C:\\HOME\\USER\\PROJECTS\\VIBE-C~1");
    answer = DosPathOfHost(&directories, "/home/user", dos);
    CHECK(!answer.failed);
    SAME(dos, "C:\\HOME\\USER");
    answer = DosPathOfHost(&directories, "/etc", dos);
    CHECK(!answer.failed);
    SAME(dos, "C:\\ETC");
    answer = DosPathOfHost(&directories, "/", dos);
    CHECK(!answer.failed);
    SAME(dos, "C:\\");
    answer = DosPathOfHost(&directories, "/home//user/", dos);
    CHECK(!answer.failed);
    SAME(dos, "C:\\HOME\\USER");
    answer = DosPathOfHost(&directories, "/home/users", dos);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathOfHost(&directories, "/home/../etc", dos);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathOfHost(&directories, "relative", dos);
    CHECK(answer.failed && answer.value == 3);
    DirectoriesEnd(&directories);

    /* sys.h: a path holds 67 characters. C:\ and seven names of eight
       with the backslashes between are 65, and an eighth makes 74. */
    DirectoriesBegin(&directories, DeepRead, NULL);
    answer = DosPathOfHost(&directories,
                           "/DIRNAME1/DIRNAME2/DIRNAME3/DIRNAME4/DIRNAME5/"
                           "DIRNAME6/DIRNAME7", dos);
    CHECK(!answer.failed && strlen((const char *)dos) == 65);
    answer = DosPathOfHost(&directories,
                           "/DIRNAME1/DIRNAME2/DIRNAME3/DIRNAME4/DIRNAME5/"
                           "DIRNAME6/DIRNAME7/DIRNAME8", dos);
    CHECK(answer.failed && answer.value == 3);
    DirectoriesEnd(&directories);
}

/*
 * Dates, times and errors.
 */

static struct tm Local(int year, int month, int day, int hour, int minute,
                       int second)
{
    struct tm local;

    memset(&local, 0, sizeof local);
    local.tm_year = year - 1900;
    local.tm_mon = month - 1;
    local.tm_mday = day;
    local.tm_hour = hour;
    local.tm_min = minute;
    local.tm_sec = second;
    return local;
}

static void Stamps(void)
{
    struct tm local;
    struct DosStamp stamp;

    /* RBIL, INT 21h 5700h, the packing; fs/fat/misc.c, the first and the
       last, and the second halved */
    local = Local(1980, 1, 1, 0, 0, 0);
    stamp = DosStampOfLocal(&local);
    CHECK(stamp.date == 0x0021 && stamp.time == 0);
    local = Local(2107, 12, 31, 23, 59, 59);
    stamp = DosStampOfLocal(&local);
    CHECK(stamp.date == 0xFF9F && stamp.time == 0xBF7D);
    local = Local(1979, 12, 31, 23, 59, 59);
    stamp = DosStampOfLocal(&local);
    CHECK(stamp.date == 0x0021 && stamp.time == 0);
    local = Local(2108, 1, 1, 0, 0, 0);
    stamp = DosStampOfLocal(&local);
    CHECK(stamp.date == 0xFF9F && stamp.time == 0xBF7D);
    local = Local(2026, 10, 14, 12, 34, 57);
    stamp = DosStampOfLocal(&local);
    CHECK(stamp.date == 0x5D4E && stamp.time == 0x645C);

    /* In UTC, the seconds since 1970 of each (Python's calendar.timegm) */
    CHECK(setenv("TZ", "UTC0", 1) == 0);
    tzset();
    stamp = DosStampOfTime(1791981297);
    CHECK(stamp.date == 0x5D4E && stamp.time == 0x645C);
    stamp = DosStampOfTime(315532800);
    CHECK(stamp.date == 0x0021 && stamp.time == 0);
    stamp = DosStampOfTime(0);
    CHECK(stamp.date == 0x0021 && stamp.time == 0);
    stamp = DosStampOfTime((time_t)4354819200LL);
    CHECK(stamp.date == 0xFF9F && stamp.time == 0xBF7D);

    stamp.date = 0x5D4E;
    stamp.time = 0x645C;
    DosStampToLocal(stamp, &local);
    CHECK(local.tm_year == 126 && local.tm_mon == 9 && local.tm_mday == 14
          && local.tm_hour == 12 && local.tm_min == 34 && local.tm_sec == 56);
    CHECK(mktime(&local) == 1791981296);
}

static void Errors(void)
{
    /* RBIL, INT 21h: 3Dh answers 2 for a file and 3 for a path, 39h and
       3Ah 5 for a directory that is there or is not empty, 56h 11h
       across drives; 59h, the numbers of the extended errors. */
    CHECK(DosErrorOf(ENOENT, false) == 0x02);
    CHECK(DosErrorOf(ENOENT, true) == 0x03);
    CHECK(DosErrorOf(ENOTDIR, false) == 0x03);
    CHECK(DosErrorOf(ENAMETOOLONG, false) == 0x03);
    CHECK(DosErrorOf(EMFILE, false) == 0x04);
    CHECK(DosErrorOf(EACCES, false) == 0x05);
    CHECK(DosErrorOf(EPERM, false) == 0x05);
    CHECK(DosErrorOf(EROFS, false) == 0x05);
    CHECK(DosErrorOf(EEXIST, false) == 0x05);
    CHECK(DosErrorOf(ENOTEMPTY, true) == 0x05);
    CHECK(DosErrorOf(EBADF, false) == 0x06);
    CHECK(DosErrorOf(ENOMEM, false) == 0x08);
    CHECK(DosErrorOf(EXDEV, false) == 0x11);
    CHECK(DosErrorOf(ENOMEDIUM, false) == 0x15);
    CHECK(DosErrorOf(EIO, false) == 0x1F);
    CHECK(DosErrorOf(EINTR, false) == 0x1F);
    CHECK(DosErrorOf(EBUSY, false) == 0x20);
    CHECK(DosErrorOf(ETXTBSY, false) == 0x20);
    CHECK(DosErrorOf(ENOSPC, false) == 0x27);
    CHECK(DosErrorOf(EDQUOT, false) == 0x27);
}

int main(void)
{
    CodePages();
    Utf8();
    Names();
    Attributes();
    Full();
    SetupFileChecked();
    Paths();
    Stamps();
    Errors();
    if (failures != 0) {
        printf("%d checks failed.\n", failures);
        return 1;
    }
    printf("Every check passed.\n");
    return 0;
}
