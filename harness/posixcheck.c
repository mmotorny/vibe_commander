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
 * portable_c-tidy (harness/tidycheck.py). It prints each check that
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
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

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
       file, directory for a directory; DIARY.md, "Take out the
       documents of the layer that was set aside, and what rested on
       them", the surveys: read-only of the mode, hidden of the dot. */
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

/* The drives of the checks of paths: H: at /home/user, R: the current
   one, with R:\A\B its current directory and H:\DOCS that of H:. */
static struct Drives Checked;

static void DrivesChecked(const char *home)
{
    DrivesBegin(&Checked, home);
    memcpy(Checked.drive[DriveIndex(&Checked, 'R')].current, "A\\B", 4);
    if (home != NULL)
        memcpy(Checked.drive[DriveIndex(&Checked, 'H')].current, "DOCS", 5);
}

static void Full(void)
{
    u8 to[DOS_PATH_ROOM];
    struct Answer answer;
    u8 deep[200];
    i32 i;

    /* CLAUDE.md, step 6 of the road: R: and H:, and R: alone where there
       is no home directory to be H:. */
    DrivesChecked("/home/user");
    CHECK(DriveIndex(&Checked, 'R') >= 0 && DriveIndex(&Checked, 'H') >= 0);
    CHECK(DriveIndex(&Checked, 'C') < 0 && DriveIndex(&Checked, 'r') < 0);
    CHECK(Checked.drive[Checked.current].letter == 'R');
    SAME(Checked.drive[DriveIndex(&Checked, 'R')].root, "/");
    SAME(Checked.drive[DriveIndex(&Checked, 'R')].label, "ROOT");
    SAME(Checked.drive[DriveIndex(&Checked, 'H')].root, "/home/user");
    SAME(Checked.drive[DriveIndex(&Checked, 'H')].label, "HOME");

    /* DOS_MakeName of DOSBox */
    answer = DosPathFull((const u8 *)"r:/foo/bar", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "R:\\FOO\\BAR");
    answer = DosPathFull((const u8 *)"..\\x", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "R:\\A\\X");
    answer = DosPathFull((const u8 *)"LONGFILENAME.TEXT", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "R:\\A\\B\\LONGFILE.TEX");
    answer = DosPathFull((const u8 *)".\\file", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "R:\\A\\B\\FILE");
    answer = DosPathFull((const u8 *)"R:\\..", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "R:\\");
    answer = DosPathFull((const u8 *)"\\..\\..\\x", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "R:\\X");
    answer = DosPathFull((const u8 *)"\\", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "R:\\");
    answer = DosPathFull((const u8 *)"R:", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "R:\\A\\B");
    /* DOS_MakeName of DOSBox: a drive that is named goes from its own
       current directory, and .. at its root stays there; one that there
       is not goes from its root, and is found to be none when it is
       used. */
    answer = DosPathFull((const u8 *)"h:", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "H:\\DOCS");
    answer = DosPathFull((const u8 *)"h:name", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "H:\\DOCS\\NAME");
    answer = DosPathFull((const u8 *)"H:\\..\\..\\x", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "H:\\X");
    answer = DosPathFull((const u8 *)"d:", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "D:\\");
    answer = DosPathFull((const u8 *)"d:name", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "D:\\NAME");
    answer = DosPathFull((const u8 *)"name", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "R:\\A\\B\\NAME");
    answer = DosPathFull((const u8 *)"*.*", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "R:\\A\\B\\*.*");
    answer = DosPathFull((const u8 *)"r:\\caf\x82.txt", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "R:\\CAFE.TXT");
    answer = DosPathFull((const u8 *)"a.b.c", &Checked, to);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathFull((const u8 *)"a+b", &Checked, to);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathFull((const u8 *)"1:\\x", &Checked, to);
    CHECK(answer.failed && answer.value == 0x0F);
    /* A path longer than DOS's 128 bytes */
    for (i = 0; i < 20; i++)
        memcpy(deep + 9 * i, "\\ABCDEFGH", 9);
    deep[180] = '\0';
    answer = DosPathFull(deep, &Checked, to);
    CHECK(answer.failed && answer.value == 3);
    /* A path without a drive is on the current drive. */
    Checked.current = DriveIndex(&Checked, 'H');
    answer = DosPathFull((const u8 *)"name", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "H:\\DOCS\\NAME");
    answer = DosPathFull((const u8 *)"\\", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "H:\\");

    DrivesChecked(NULL);
    CHECK(DriveIndex(&Checked, 'H') < 0 && DriveIndex(&Checked, 'R') >= 0);
    answer = DosPathFull((const u8 *)"h:name", &Checked, to);
    CHECK(!answer.failed);
    SAME(to, "H:\\NAME");
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
    DrivesChecked("/home/user");

    /* CLAUDE.md, step 3 of the road: a drive letter is a directory of
       the host, as DOSBox's mount; step 6: R: is the root of the file
       system and H: the home directory, and there is no other; an alias
       leads back. */
    answer = HostPathOfDos(&directories, &Checked,
                           (const u8 *)"R:\\HOME\\USER\\NOTES.TXT", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/home/user/notes.txt");
    answer = HostPathOfDos(&directories, &Checked,
                           (const u8 *)"H:\\NOTES.TXT", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/home/user/notes.txt");
    answer = HostPathOfDos(&directories, &Checked,
                           (const u8 *)"H:\\LONGFI~1.TXT", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/home/user/LongFileName.txt");
    answer = HostPathOfDos(&directories, &Checked, (const u8 *)"R:\\", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/");
    answer = HostPathOfDos(&directories, &Checked, (const u8 *)"H:\\", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/home/user");
    answer = HostPathOfDos(&directories, &Checked,
                           (const u8 *)"H:\\NEW.TXT", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 0);
    SAME(host, "/home/user/new.txt");
    /* RBIL, INT 21h 3Dh: 3 where a directory on the way is not there,
       0Fh for a drive that there is not (INT 21h 0Eh and 47h) */
    answer = HostPathOfDos(&directories, &Checked,
                           (const u8 *)"R:\\HOME\\USER\\NOPE\\X.TXT", host,
                           (i32)sizeof host);
    CHECK(answer.failed && answer.value == 3);
    answer = HostPathOfDos(&directories, &Checked, (const u8 *)"C:\\X", host,
                           (i32)sizeof host);
    CHECK(answer.failed && answer.value == 0x0F);
    answer = HostPathOfDos(&directories, &Checked, (const u8 *)"A:\\", host,
                           (i32)sizeof host);
    CHECK(answer.failed && answer.value == 0x0F);
    answer = HostPathOfDos(&directories, &Checked, (const u8 *)"R:\\ROOT\\X",
                           host, (i32)sizeof host);
    CHECK(answer.failed && answer.value == 3);
    answer = HostPathOfDos(&directories, &Checked,
                           (const u8 *)"R:\\HOME\\USER", host, 10);
    CHECK(answer.failed && answer.value == 3);
    answer = HostPathOfDos(&directories, &Checked, (const u8 *)"H:\\", host,
                           10);
    CHECK(answer.failed && answer.value == 3);

    /* A name that the host has since it was read is found: the directory
       is read again for a name that it has not. */
    Fake[2].names[5] = "added.txt";
    answer = HostPathOfDos(&directories, &Checked,
                           (const u8 *)"H:\\ADDED.TXT", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/home/user/added.txt");
    Fake[2].names[5] = NULL;

    /* CLAUDE.md, step 6 of the road: a path of the host under the home
       directory is on H:, from the home directory, and any other on R:,
       name by name, so that /home/users is not under /home/user. */
    answer = DosPathOfHost(&directories, &Checked,
                           "/home/user/Projects/vibe-commander", dos);
    CHECK(!answer.failed);
    SAME(dos, "H:\\PROJECTS\\VIBE-C~1");
    answer = DosPathOfHost(&directories, &Checked, "/home/user", dos);
    CHECK(!answer.failed);
    SAME(dos, "H:\\");
    answer = DosPathOfHost(&directories, &Checked, "/home//user/", dos);
    CHECK(!answer.failed);
    SAME(dos, "H:\\");
    answer = DosPathOfHost(&directories, &Checked, "/home", dos);
    CHECK(!answer.failed);
    SAME(dos, "R:\\HOME");
    answer = DosPathOfHost(&directories, &Checked, "/etc", dos);
    CHECK(!answer.failed);
    SAME(dos, "R:\\ETC");
    answer = DosPathOfHost(&directories, &Checked, "/", dos);
    CHECK(!answer.failed);
    SAME(dos, "R:\\");
    answer = DosPathOfHost(&directories, &Checked, "/home/users", dos);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathOfHost(&directories, &Checked, "/home/../etc", dos);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathOfHost(&directories, &Checked, "/home/user/../etc", dos);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathOfHost(&directories, &Checked, "relative", dos);
    CHECK(answer.failed && answer.value == 3);

    /* With no H:, everything is on R:, and H: is a drive that there is
       not. */
    DrivesChecked(NULL);
    answer = DosPathOfHost(&directories, &Checked, "/home/user", dos);
    CHECK(!answer.failed);
    SAME(dos, "R:\\HOME\\USER");
    answer = HostPathOfDos(&directories, &Checked, (const u8 *)"H:\\", host,
                           (i32)sizeof host);
    CHECK(answer.failed && answer.value == 0x0F);
    DirectoriesEnd(&directories);

    /* sys.h: a path holds 67 characters. R:\ and seven names of eight
       with the backslashes between are 65, and an eighth makes 74; on H:
       a path goes from the home directory, and holds more of the host's
       (CLAUDE.md, step 6 of the road: H: as a way round DOS's 67
       characters). */
    DirectoriesBegin(&directories, DeepRead, NULL);
    answer = DosPathOfHost(&directories, &Checked,
                           "/DIRNAME1/DIRNAME2/DIRNAME3/DIRNAME4/DIRNAME5/"
                           "DIRNAME6/DIRNAME7", dos);
    CHECK(!answer.failed && strlen((const char *)dos) == 65);
    answer = DosPathOfHost(&directories, &Checked,
                           "/DIRNAME1/DIRNAME2/DIRNAME3/DIRNAME4/DIRNAME5/"
                           "DIRNAME6/DIRNAME7/DIRNAME8", dos);
    CHECK(answer.failed && answer.value == 3);
    DrivesChecked("/DIRNAME1/DIRNAME2/DIRNAME3/DIRNAME4/DIRNAME5/DIRNAME6");
    answer = DosPathOfHost(&directories, &Checked,
                           "/DIRNAME1/DIRNAME2/DIRNAME3/DIRNAME4/DIRNAME5/"
                           "DIRNAME6/DIRNAME7/DIRNAME8", dos);
    CHECK(!answer.failed);
    SAME(dos, "H:\\DIRNAME7\\DIRNAME8");
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

/*
 * DOS's services on the host's files, the piece L2: a tree that the test
 * makes in a temporary directory, which the emulation is started in, as
 * the program is started in the directory that it is run from. What each
 * call answers is held to what the bench's DOS answers, FreeDOS 1.4, as
 * harness/trial32/asked.py has it ask every function of sys.h there
 * ("the bench" below: the toy harness/trial32/sys.c, whose steps these
 * follow, with its names), or to RBIL where the bench was not asked.
 */

/* The temporary directory of the host, and a struct Dos on it. */
static char Tree[512];
static struct Dos Emulated;

/* A name of the tree, at `to`, which has room for 600 bytes. */
static void InTree(const char *name, char *to)
{
    size_t length = strlen(Tree);
    size_t more = strlen(name);

    memcpy(to, Tree, length + 1);
    to[length] = '/';
    memcpy(to + length + 1, name, more + 1);
}

/* A file of the tree made by the host with `text` in it. */
static void HostFile(const char *name, const char *text)
{
    char path[600];
    FILE *file;

    InTree(name, path);
    file = fopen(path, "w");
    CHECK(file != NULL);
    if (file == NULL)
        return;
    fputs(text, file);
    fclose(file);
}

/* Whether the tree has a name, as the host has it. */
static bool HostHas(const char *name)
{
    char path[600];
    struct stat status;

    InTree(name, path);
    return lstat(path, &status) == 0;
}

static mode_t HostMode(const char *name)
{
    char path[600];
    struct stat status;

    InTree(name, path);
    if (stat(path, &status) != 0)
        return 0;
    return status.st_mode & 07777;
}

/* Make a file of the tree `size` bytes long, or as long, by the host,
   which holds it without its bytes: a sparse file costs no disk. Whether
   the host could. */
static bool HostSized(const char *name, off_t size)
{
    char path[600];
    int fd;
    bool made;

    InTree(name, path);
    fd = open(path, O_RDWR | O_CREAT, 0644);
    if (fd < 0)
        return false;
    made = ftruncate(fd, size) == 0;
    close(fd);
    return made;
}

static off_t HostSize(const char *name)
{
    char path[600];
    struct stat status;

    InTree(name, path);
    if (stat(path, &status) != 0)
        return -1;
    return status.st_size;
}

static int Removed(const char *path, const struct stat *status, int kind,
                   struct FTW *walk)
{
    (void)status;
    (void)kind;
    (void)walk;
    remove(path);
    return 0;
}

#define NAME(text) ((const u8 *)(text))
#define BIG_SIZE 30000

static u8 Pattern(i32 at)
{
    return (u8)((at * 7 + at / 251) & 0xFF);
}

static u8 Big[BIG_SIZE];

/* What the bench's toy does with files, Files and Names of sys.c. */
static void ServiceFiles(void)
{
    struct Dos *dos = &Emulated;
    struct Answer asked;
    struct Moved moved;
    struct FilePlace place;
    struct FileStamp stamp;
    struct FileAttr attr;
    struct Device device;
    u16 handle;
    i32 at;
    i32 wrong;

    for (at = 0; at < BIG_SIZE; at++)
        Big[at] = Pattern(at);
    /* The bench: the first handle is 5; CLAUDE.md, step 6: what the
       program makes is named in small letters. */
    asked = DosFileCreate(dos, NAME("TOY.DAT"), 0);
    CHECK(!asked.failed && asked.value == 5);
    CHECK(HostHas("toy.dat") && !HostHas("TOY.DAT"));
    handle = asked.value;
    moved = DosFileWrite(dos, handle, Big, BIG_SIZE);
    CHECK(!moved.failed && moved.count == BIG_SIZE && moved.error == 0);
    place = DosFileSeek(dos, handle, 100);
    CHECK(!place.failed && place.place == 100);
    place = DosFileStep(dos, handle, 2, 0);
    CHECK(!place.failed && place.place == 30000);
    place = DosFileStep(dos, handle, 2, -10);
    CHECK(!place.failed && place.place == 29990);
    place = DosFileStep(dos, handle, 1, 5);
    CHECK(!place.failed && place.place == 29995);
    place = DosFileSeek(dos, handle, 0x12345678);
    CHECK(!place.failed && place.place == 0x12345678);
    /* The bench: a place to count from that there is not is DOS's 1, and
       one of no handle 6, with the low word of what was asked above. */
    place = DosFileStep(dos, handle, 3, 0x89AB);
    CHECK(place.failed && place.place == 0x89AB0001);
    place = DosFileSeek(dos, 99, 0x12345);
    CHECK(place.failed && place.place == 0x23450006);
    /* RBIL, INT 21h 42h: a step back from the start is a place far on. */
    place = DosFileStep(dos, handle, 0, -1);
    CHECK(!place.failed && place.place == 0xFFFFFFFF);
    place = DosFileSeek(dos, handle, 20000);
    CHECK(!place.failed && place.place == 20000);
    /* The bench: none written cuts the file off. */
    moved = DosFileWrite(dos, handle, Big, 0);
    CHECK(!moved.failed && moved.count == 0);
    place = DosFileStep(dos, handle, 2, 0);
    CHECK(!place.failed && place.place == 20000);
    asked = DosFileSetTime(dos, handle, 0x6000, 0x28D0);
    CHECK(!asked.failed && asked.value == 0x5701);
    stamp = DosFileTime(dos, handle);
    CHECK(!stamp.failed && stamp.value == 0x5700 && stamp.time == 0x6000
          && stamp.date == 0x28D0);
    stamp = DosFileTime(dos, 99);
    CHECK(stamp.failed && stamp.value == 6 && stamp.time == 0
          && stamp.date == 0);
    asked = DosFileSetTime(dos, 99, 0x6000, 0x28D0);
    CHECK(asked.failed && asked.value == 6);
    /* The bench: written, its time set, on C:, 2 in the low bits (RBIL,
       INT 21h 4400h: the drive); here on R:, 11h. */
    device = DosFileDevice(dos, handle);
    CHECK(!device.failed && device.kind == 0x4011 && device.value == 0x4011);
    asked = DosFileClose(dos, handle);
    CHECK(!asked.failed && asked.value == 0x3E00);
    asked = DosFileClose(dos, handle);
    CHECK(asked.failed && asked.value == 6);

    asked = DosFileOpen(dos, NAME("toy.dat"), 0);
    CHECK(!asked.failed && asked.value == 5);
    handle = asked.value;
    device = DosFileDevice(dos, handle);
    CHECK(!device.failed && device.kind == 0x0051);
    memset(Big, 0, sizeof Big);
    moved = DosFileRead(dos, handle, Big, BIG_SIZE);
    CHECK(!moved.failed && moved.count == 20000);
    for (at = wrong = 0; at < 20000; at++)
        wrong += Big[at] != Pattern(at);
    for (; at < BIG_SIZE; at++)
        wrong += Big[at] != 0;
    CHECK(wrong == 0);
    moved = DosFileRead(dos, handle, Big, 100);
    CHECK(!moved.failed && moved.count == 0);
    /* RBIL, INT 21h 5701h: the time that was set is the file's once it
       is closed, with what was written before it. */
    stamp = DosFileTime(dos, handle);
    CHECK(!stamp.failed && stamp.time == 0x6000 && stamp.date == 0x28D0);
    moved = DosFileRead(dos, 99, Big, 5);
    CHECK(moved.failed && moved.error == 6 && moved.count == 0);
    moved = DosFileWrite(dos, handle, Big, 5);
    CHECK(moved.failed && moved.error == 5 && moved.count == 0);
    moved = DosFileWrite(dos, 99, Big, 5);
    CHECK(moved.failed && moved.error == 6);
    asked = DosFileClose(dos, handle);
    CHECK(!asked.failed);
    asked = DosFileOpen(dos, NAME("NONE.DAT"), 0);
    CHECK(asked.failed && asked.value == 2);
    asked = DosFileOpen(dos, NAME("NODIR\\X.DAT"), 0);
    CHECK(asked.failed && asked.value == 3);
    /* The bench: only the low byte of the mode counts, and what others
       may do is not in the way. RBIL, INT 21h 3Dh: 0Ch for a mode that
       there is not. */
    asked = DosFileOpen(dos, NAME("TOY.DAT"), 0x0142);
    CHECK(!asked.failed && asked.value == 5);
    DosFileClose(dos, 5);
    asked = DosFileOpen(dos, NAME("TOY.DAT"), 3);
    CHECK(asked.failed && asked.value == 0x0C);

    /* The bench, Names. */
    attr = DosFileAttributes(dos, NAME("TOY.DAT"));
    CHECK(!attr.failed && attr.value == 0x4300 && attr.attributes == 0x20);
    attr = DosFileAttributes(dos, NAME("DOCS"));
    CHECK(!attr.failed && attr.attributes == 0x10);
    attr = DosFileAttributes(dos, NAME("NONE.DAT"));
    CHECK(attr.failed && attr.value == 2 && attr.attributes == 0);
    attr = DosFileAttributes(dos, NAME("NODIR\\X.DAT"));
    CHECK(attr.failed && attr.value == 3);
    asked = DosFileSetAttributes(dos, NAME("TOY.DAT"), 1);
    CHECK(!asked.failed && asked.value == 1);
    CHECK((HostMode("toy.dat") & 0222) == 0);
    attr = DosFileAttributes(dos, NAME("TOY.DAT"));
    CHECK(!attr.failed && attr.attributes == 0x21);
    asked = DosFileDelete(dos, NAME("TOY.DAT"));
    CHECK(asked.failed && asked.value == 5);
    asked = DosFileOpen(dos, NAME("TOY.DAT"), 1);
    CHECK(asked.failed && asked.value == 5);
    /* RBIL, INT 21h 3Ch: 5 for a file that is read-only. */
    asked = DosFileCreate(dos, NAME("TOY.DAT"), 0);
    CHECK(asked.failed && asked.value == 5);
    asked = DosFileSetAttributes(dos, NAME("TOY.DAT"), 0);
    CHECK(!asked.failed && asked.value == 0);
    CHECK((HostMode("toy.dat") & 0200) != 0);
    asked = DosFileSetAttributes(dos, NAME("NONE.DAT"), 0);
    CHECK(asked.failed && asked.value == 2);
    /* RBIL, INT 21h 4301h: no directory, no label. */
    asked = DosFileSetAttributes(dos, NAME("TOY.DAT"), 0x10);
    CHECK(asked.failed && asked.value == 5);
    asked = DosFileRename(dos, NAME("TOY.DAT"), NAME("TOY2.DAT"));
    CHECK(!asked.failed && asked.value == 0x5600);
    CHECK(HostHas("toy2.dat") && !HostHas("toy.dat"));
    asked = DosFileRename(dos, NAME("TOY.DAT"), NAME("TOY2.DAT"));
    CHECK(asked.failed && asked.value == 2);
    asked = DosFileRename(dos, NAME("TOY2.DAT"), NAME("HELLO.TXT"));
    CHECK(asked.failed && asked.value == 5);
    asked = DosFileRename(dos, NAME("TOY2.DAT"), NAME("NODIR\\X.DAT"));
    CHECK(asked.failed && asked.value == 3);
    /* RBIL, INT 21h 56h: 11h across drives. */
    asked = DosFileRename(dos, NAME("TOY2.DAT"), NAME("D:\\X.DAT"));
    CHECK(asked.failed && asked.value == 0x11);
    asked = DosFileDelete(dos, NAME("TOY2.DAT"));
    CHECK(!asked.failed && asked.value == 0x4100);
    asked = DosFileDelete(dos, NAME("TOY2.DAT"));
    CHECK(asked.failed && asked.value == 2);
    asked = DosFileDelete(dos, NAME("NODIR\\X.DAT"));
    CHECK(asked.failed && asked.value == 3);
    asked = DosFileCreate(dos, NAME("NODIR\\X.DAT"), 0);
    CHECK(asked.failed && asked.value == 3);
    asked = DosFileCreate(dos, NAME("DOCS"), 0);
    CHECK(asked.failed && asked.value == 5);
    /* A directory is not opened and not deleted as a file: DOS's 5
       (RBIL, INT 21h 3Dh and 41h). */
    asked = DosFileOpen(dos, NAME("DOCS"), 0);
    CHECK(asked.failed && asked.value == 5);
    asked = DosFileDelete(dos, NAME("DOCS"));
    CHECK(asked.failed && asked.value == 5);
    /* A name with * or ? names no file to make (RBIL, INT 21h 3Ch: 3). */
    asked = DosFileCreate(dos, NAME("TOY?.DAT"), 0);
    CHECK(asked.failed && asked.value == 3);
    CHECK(!HostHas("toy?.dat"));
    /* A name of the host that is no name of DOS's form is reached by its
       alias (L1, fatgen103), and one in capitals by its own. */
    asked = DosFileOpen(dos, NAME("LONGFI~1.TXT"), 0);
    CHECK(!asked.failed);
    moved = DosFileRead(dos, asked.value, Big, 100);
    CHECK(!moved.failed && moved.count == 4 && memcmp(Big, "long", 4) == 0);
    DosFileClose(dos, asked.value);
    asked = DosFileOpen(dos, NAME("readme.txt"), 0);
    CHECK(!asked.failed);
    DosFileClose(dos, asked.value);
}

/* The current directory of a drive of the emulation. */
static const u8 *Current(u8 letter)
{
    return Emulated.drives.drive[DriveIndex(&Emulated.drives, letter)].current;
}

/* The bench, the end of Files: DOS's own five handles. RBIL, INT 21h
   3Dh: 4 where the program's table of handles is full, 20 with DOS's. */
static void ServiceHandles(void)
{
    struct Dos *dos = &Emulated;
    struct Answer asked;
    struct Device device;
    u16 handle;

    device = DosFileDevice(dos, 0);
    CHECK(!device.failed && device.kind == 0x80D3 && device.value == 0x80D3);
    device = DosFileDevice(dos, 1);
    CHECK(!device.failed && device.kind == 0x80D3);
    device = DosFileDevice(dos, 4);
    CHECK(!device.failed && device.kind == 0xA0C0);
    device = DosFileDevice(dos, 99);
    CHECK(device.failed && device.value == 6 && device.kind == 0);
    for (handle = 5; handle < DOS_HANDLES; handle++) {
        asked = DosFileOpen(dos, NAME("HELLO.TXT"), 0);
        CHECK(!asked.failed && asked.value == handle);
    }
    asked = DosFileOpen(dos, NAME("HELLO.TXT"), 0);
    CHECK(asked.failed && asked.value == 4);
    asked = DosFileCreate(dos, NAME("MORE.TXT"), 0);
    CHECK(asked.failed && asked.value == 4);
    for (handle = 5; handle < DOS_HANDLES; handle++)
        CHECK(!DosFileClose(dos, handle).failed);
}

/* What is no file and no directory, and a file of 4 GB or more:
   emulate_posix.h says what is done with them, as the user decided
   (DIARY.md, "Emulate DOS's files, directories and drives on the host's
   files for Linux"). */
static void ServiceOthers(void)
{
    struct Dos *dos = &Emulated;
    struct Answer asked;
    struct Moved moved;
    struct FilePlace place;
    char path[600];
    char target[600];

    InTree("pipe", path);
    CHECK(mkfifo(path, 0644) == 0);
    asked = DosFileOpen(dos, NAME("PIPE"), 0);
    CHECK(asked.failed && asked.value == 5);
    asked = DosFileOpen(dos, NAME("PIPE"), 1);
    CHECK(asked.failed && asked.value == 5);
    asked = DosFileCreate(dos, NAME("PIPE"), 0);
    CHECK(asked.failed && asked.value == 5);
    asked = DosFileOpen(dos, NAME("R:\\DEV\\NULL"), 0);
    CHECK(asked.failed && asked.value == 5);
    InTree("dangling", path);
    InTree("nowhere", target);
    CHECK(symlink(target, path) == 0);
    asked = DosFileOpen(dos, NAME("DANGLING"), 0);
    CHECK(asked.failed && asked.value == 2);

    /* A file of 5 GB, which the host holds without its bytes. */
    if (!HostSized("large.bin", (off_t)5 << 30)) {
        printf("posixcheck.c: no file of 5 GB in %s: its checks are left "
               "out\n", Tree);
        return;
    }
    /* Listed as the most that DOS can say; never opened to be read, so
       that no copy of it is cut short: 5, as the program shows of a file
       that it cannot open. */
    asked = DosFindFirst(dos, NAME("LARGE.BIN"), 0);
    CHECK(!asked.failed && Emulated.record->FilSize == 0xFFFFFFFF);
    asked = DosFileOpen(dos, NAME("LARGE.BIN"), 0);
    CHECK(asked.failed && asked.value == 5);
    asked = DosFileOpen(dos, NAME("LARGE.BIN"), 2);
    CHECK(asked.failed && asked.value == 5);
    asked = DosFileOpen(dos, NAME("LARGE.BIN"), 0x40);
    CHECK(asked.failed && asked.value == 5);
    /* To write alone it is opened, as the program does to set the date
       and the time, and written no further than DOS's last place. */
    asked = DosFileOpen(dos, NAME("LARGE.BIN"), 1);
    CHECK(!asked.failed);
    place = DosFileStep(dos, asked.value, 2, 0);
    CHECK(!place.failed && place.place == 0xFFFFFFFF);
    place = DosFileSeek(dos, asked.value, 0xFFFFFFF0);
    CHECK(!place.failed);
    moved = DosFileWrite(dos, asked.value, Big, 100);
    CHECK(!moved.failed && moved.count == 15);
    DosFileClose(dos, asked.value);
    CHECK(HostSize("large.bin") == (off_t)5 << 30);

    /* A file of FFFFFFFFh bytes is DOS's and is read to its end; grown
       beyond it on the host while it is open, a read that would go past
       DOS's last place is refused, 5, and one short of it is not. */
    HostSized("edge.bin", (off_t)0xFFFFFFFF);
    asked = DosFileOpen(dos, NAME("EDGE.BIN"), 0);
    CHECK(!asked.failed);
    DosFileSeek(dos, asked.value, 0xFFFFFFF0);
    moved = DosFileRead(dos, asked.value, Big, 100);
    CHECK(!moved.failed && moved.count == 15);
    moved = DosFileRead(dos, asked.value, Big, 100);
    CHECK(!moved.failed && moved.count == 0);
    HostSized("edge.bin", (off_t)5 << 30);
    DosFileSeek(dos, asked.value, 0xFFFFFFF0);
    moved = DosFileRead(dos, asked.value, Big, 10);
    CHECK(!moved.failed && moved.count == 10);
    moved = DosFileRead(dos, asked.value, Big, 100);
    CHECK(moved.failed && moved.error == 5 && moved.count == 0);
    DosFileClose(dos, asked.value);
}

/* The bench, Older. */
static void ServiceOlder(void)
{
    struct Dos *dos = &Emulated;
    struct Answer asked;
    u8 to[DOS_PATH_ROOM];
    u8 fcb[48];
    u8 start[DOS_PATH_ROOM];
    char expected[DOS_PATH_ROOM + 16];
    size_t length;

    memcpy(start, "R:\\", 4);
    length = strlen((const char *)Current('R'));
    memcpy(start + 3, Current('R'), length + 1);
    memset(to, '#', sizeof to);
    asked = DosFileTrueName(dos, NAME("HELLO.TXT"), to);
    CHECK(!asked.failed && asked.value == 0);
    snprintf(expected, sizeof expected, "%s\\HELLO.TXT", (const char *)start);
    SAME(to, expected);
    asked = DosFileTrueName(dos, NAME("hello.txt"), to);
    CHECK(!asked.failed);
    SAME(to, expected);
    asked = DosFileTrueName(dos, NAME("DOCS\\..\\.\\hello.txt"), to);
    CHECK(!asked.failed);
    SAME(to, expected);
    asked = DosFileTrueName(dos, NAME("R:\\NODIR\\X.DAT"), to);
    CHECK(!asked.failed);
    SAME(to, "R:\\NODIR\\X.DAT");
    memset(to, '#', sizeof to);
    asked = DosFileTrueName(dos, NAME("Q:X.DAT"), to);
    CHECK(asked.failed && asked.value == 3 && to[0] == '#' && to[127] == '#');

    memset(fcb, '#', sizeof fcb);
    CHECK(DosFcbParse(dos, NAME("TOY?.DAT"), fcb, 1) == 0x2901);
    CHECK(fcb[0] == 0 && memcmp(fcb + 1, "TOY?    DAT", 11) == 0);
    CHECK(memcmp(fcb + 12, "\0\0\0\0####", 8) == 0);
    CHECK(DosFcbParse(dos, NAME("HELLO.TXT"), fcb, 1) == 0x2900);
    CHECK(fcb[0] == 0 && memcmp(fcb + 1, "HELLO   TXT", 11) == 0);
    CHECK(DosFcbParse(dos, NAME("R:TOY?.DAT"), fcb, 1) == 0x2901);
    CHECK(fcb[0] == 18 && memcmp(fcb + 1, "TOY?    DAT", 11) == 0);
    CHECK(DosFcbParse(dos, NAME("Q:X.DAT"), fcb, 1) == 0x29FF);
    CHECK(fcb[0] == 18 && memcmp(fcb + 1, "TOY?    DAT", 11) == 0);
    CHECK(DosFcbParse(dos, NAME("h:X.DAT"), fcb, 1) == 0x2900);
    CHECK(fcb[0] == 8 && memcmp(fcb + 1, "X       DAT", 11) == 0);
    fcb[0] = 2;
    CHECK(DosFcbParse(dos, NAME("*.*"), fcb, 2) == 0x2901);
    CHECK(fcb[0] == 2 && memcmp(fcb + 1, "???????????", 11) == 0);
    /* RBIL, INT 21h 29h: separators skipped by bit 0; a name ends at a
       character that a name may not have (COUNTRY.SYS, FCHAR). */
    CHECK(DosFcbParse(dos, NAME(" ;a.b+c"), fcb, 1) == 0x2900);
    CHECK(fcb[0] == 0 && memcmp(fcb + 1, "A       B  ", 11) == 0);

    DosFileClose(dos, DosFileCreate(dos, NAME("TOY1.DAT"), 0).value);
    DosFileClose(dos, DosFileCreate(dos, NAME("TOY2.DAT"), 0).value);
    memset(fcb, 0, sizeof fcb);
    DosFcbParse(dos, NAME("TOY?.DAT"), fcb, 1);
    asked = DosFcbDelete(dos, fcb, false);
    CHECK(!asked.failed && asked.value == 0x1300);
    CHECK(!HostHas("toy1.dat") && !HostHas("toy2.dat"));
    asked = DosFcbDelete(dos, fcb, false);
    CHECK(!asked.failed && asked.value == 0x13FF);
    asked = DosFcbDelete(dos, fcb, true);
    CHECK(asked.failed && asked.value == 0x12);
    /* An extended one, as the program deletes: hidden and system too
       with 26h. RBIL, INT 21h 13h: a file that is read-only stays. */
    HostFile(".toy3.dat", "hidden");
    HostFile("toy4.dat", "plain");
    DosFileClose(dos, DosFileCreate(dos, NAME("TOY5.DAT"), 1).value);
    DosFcbParse(dos, NAME("TOY*.DAT"), fcb, 1);
    asked = DosFcbDelete(dos, fcb, false);
    CHECK(!asked.failed && asked.value == 0x1300);
    CHECK(HostHas(".toy3.dat") && !HostHas("toy4.dat")
          && HostHas("toy5.dat"));
    fcb[0] = 0xFF;
    fcb[6] = 0x26;
    fcb[7] = 0;
    DosFcbParse(dos, NAME("TOY*.DAT"), fcb + 7, 2);
    asked = DosFcbDelete(dos, fcb, false);
    CHECK(!asked.failed && asked.value == 0x1300);
    CHECK(!HostHas(".toy3.dat") && HostHas("toy5.dat"));
    DosFileSetAttributes(dos, NAME("TOY5.DAT"), 0);
    DosFileDelete(dos, NAME("TOY5.DAT"));
}

/* The bench, Directories. */
static void ServiceDirectories(void)
{
    struct Dos *dos = &Emulated;
    struct Answer asked;
    u8 to[80];
    u8 here[80];
    char expected[160];

    memset(to, 0, sizeof to);
    asked = DosDirCurrent(dos, 0, here);
    CHECK(!asked.failed && asked.value == 0x0100);
    SAME(here, Current('R'));
    asked = DosDirCurrent(dos, 4, to);
    CHECK(asked.failed && asked.value == 0x0F);
    asked = DosDirCurrent(dos, 0x0312, to);
    CHECK(!asked.failed);
    SAME(to, here);
    asked = DosDirCurrent(dos, 26, to);
    CHECK(asked.failed && asked.value == 0x0F);
    asked = DosDirMake(dos, NAME("NEWDIR"));
    CHECK(!asked.failed && asked.value == 0x3900);
    CHECK(HostHas("newdir"));
    asked = DosDirMake(dos, NAME("NEWDIR"));
    CHECK(asked.failed && asked.value == 5);
    asked = DosDirMake(dos, NAME("NODIR\\X"));
    CHECK(asked.failed && asked.value == 3);
    asked = DosDirChange(dos, NAME("NEWDIR"));
    CHECK(!asked.failed && asked.value == 0x3B00);
    asked = DosDirCurrent(dos, 0, to);
    CHECK(!asked.failed);
    snprintf(expected, sizeof expected, "%s\\NEWDIR", (const char *)here);
    SAME(to, expected);
    snprintf(expected, sizeof expected, "R:\\%s\\NEWDIR",
             (const char *)here);
    asked = DosDirRemove(dos, NAME(expected));
    CHECK(asked.failed && asked.value == 0x10);
    asked = DosDirChange(dos, NAME(".."));
    CHECK(!asked.failed && asked.value == 0x3B00);
    asked = DosDirRemove(dos, NAME("NEWDIR"));
    CHECK(!asked.failed && asked.value == 0x3A00);
    CHECK(!HostHas("newdir"));
    asked = DosDirRemove(dos, NAME("NEWDIR"));
    CHECK(asked.failed && asked.value == 3);
    asked = DosDirChange(dos, NAME("NEWDIR"));
    CHECK(asked.failed && asked.value == 3);
    asked = DosDirRemove(dos, NAME("DOCS"));
    CHECK(asked.failed && asked.value == 5);
    /* RBIL, INT 21h 3Bh: a file is no directory to go to. */
    asked = DosDirChange(dos, NAME("HELLO.TXT"));
    CHECK(asked.failed && asked.value == 3);
}

/* A search to its end, each name found behind the last, with its
   attributes in two digits of hex behind a slash, at `to`: and the
   error that ended it. */
static u16 Listed(const char *pattern, u16 attributes, char *to,
                  size_t room)
{
    struct Answer found;
    size_t at = 0;
    int wrote;

    to[0] = '\0';
    found = DosFindFirst(&Emulated, NAME(pattern), attributes);
    while (!found.failed) {
        CHECK(found.value == 0);
        wrote = snprintf(to + at, room - at, "%s/%02X ",
                         (const char *)Emulated.record->FilName,
                         (unsigned)Emulated.record->FilAttr);
        if (wrote < 0 || (size_t)wrote >= room - at)
            break;
        at += (size_t)wrote;
        found = DosFindNext(&Emulated);
    }
    return found.value;
}

/* The bench, Finds, in a directory of the tree with the files that the
   bench's has, and the order in which their names are given. */
static void ServiceFinds(void)
{
    struct Dos *dos = &Emulated;
    struct Answer asked;
    struct DTAs kept;
    struct DTAs elsewhere;
    char list[1024];
    char path[600];

    CHECK(!DosDirMake(dos, NAME("FINDS")).failed);
    CHECK(!DosDirChange(dos, NAME("FINDS")).failed);
    CHECK(!DosDirMake(dos, NAME("DOCS")).failed);
    HostFile("finds/hello.txt", "Hello, world!\r\n");
    HostFile("finds/readme.txt", "read me");
    HostFile("finds/data", "no extension");
    HostFile("finds/.hidden.txt", "hidden");
    InTree("finds/dangling", path);
    CHECK(symlink("nowhere", path) == 0);
    /* . and .. first, as on a disk of DOS's (the bench); the names then
       in the order of their bytes, and a name that begins with a dot
       hidden and with a tail, the dot being lost (L1, fatgen103); 12h at
       the end (the bench). */
    CHECK(Listed("*.*", 0x37, list, sizeof list) == 0x12);
    SAME(list, "./10 ../10 HIDDEN~1.TXT/22 DANGLING/20 DATA/20 DOCS/10 "
               "HELLO.TXT/20 README.TXT/20 ");
    /* RBIL, INT 21h 4Eh: no hidden file and no directory unless they are
       searched for. */
    CHECK(Listed("*.*", 0, list, sizeof list) == 0x12);
    SAME(list, "DANGLING/20 DATA/20 HELLO.TXT/20 README.TXT/20 ");
    CHECK(Listed("*.TXT", 0, list, sizeof list) == 0x12);
    SAME(list, "HELLO.TXT/20 README.TXT/20 ");
    /* L1: * without a dot names only names without an extension, as the
       eleven characters of a file control block have it, . and .. among
       them. */
    CHECK(Listed("*", 0x37, list, sizeof list) == 0x12);
    SAME(list, "./10 ../10 DANGLING/20 DATA/20 DOCS/10 ");
    /* The bench: nothing found, a directory that is not there, and no
       label but in the root (fatgen103), which a search for anything else
       does not find (RBIL, INT 21h 4Eh). */
    CHECK(Listed("NONE.*", 0x37, list, sizeof list) == 0x12);
    SAME(list, "");
    CHECK(Listed("R:\\NODIR\\*.*", 0x37, list, sizeof list) == 3);
    CHECK(Listed("*.*", 8, list, sizeof list) == 0x12);
    SAME(list, "");
    /* The root has no . and no .. (RBIL, INT 21h 4Eh). */
    asked = DosFindFirst(dos, NAME("R:\\*.*"), 0x37);
    CHECK(!asked.failed && Emulated.record->FilName[0] != '.');
    /* What a search found, of the record: the size, and a name that is
       ended by zeros. */
    asked = DosFindFirst(dos, NAME("HELLO.TXT"), 0);
    CHECK(!asked.failed && Emulated.record->FilSize == 15
          && Emulated.record->FilAttr == 0x20);
    CHECK(memcmp(Emulated.record->FilName, "HELLO.TXT\0\0\0\0", 13) == 0);
    asked = DosFindFirst(dos, NAME("DANGLING"), 0);
    CHECK(!asked.failed && Emulated.record->FilSize == 0);

    /* The bench: other things asked between two finds of one search. */
    asked = DosFindFirst(dos, NAME("*.TXT"), 0);
    CHECK(!asked.failed);
    SAME(Emulated.record->FilName, "HELLO.TXT");
    DosFileClose(dos, DosFileOpen(dos, NAME("README.TXT"), 0).value);
    DosFileAttributes(dos, NAME("DOCS"));
    asked = DosFindNext(dos);
    CHECK(!asked.failed);
    SAME(Emulated.record->FilName, "README.TXT");
    /* The record of one search kept while another is made, and put
       back. */
    DosFindFirst(dos, NAME("*.TXT"), 0);
    kept = *Emulated.record;
    asked = DosFindFirst(dos, NAME("R:\\*.*"), 0x37);
    CHECK(!asked.failed);
    *Emulated.record = kept;
    asked = DosFindNext(dos);
    CHECK(!asked.failed);
    SAME(Emulated.record->FilName, "README.TXT");
    /* The record elsewhere, with what it held. */
    DosFindFirst(dos, NAME("*.TXT"), 0);
    elsewhere = *Emulated.record;
    DosFindRecord(dos, &elsewhere);
    asked = DosFindNext(dos);
    CHECK(!asked.failed);
    SAME(elsewhere.FilName, "README.TXT");
    SAME(Emulated.own.FilName, "HELLO.TXT");
    asked = DosFindNext(dos);
    CHECK(asked.failed && asked.value == 0x12);
    DosFindRecord(dos, &Emulated.own);
    CHECK(!DosDirChange(dos, NAME("..")).failed);
}

/* The bench, the drives; DosSpaceOf against what emulate_posix.h says
   of the shape, worked out by hand. */
static void ServiceDrives(void)
{
    struct Dos *dos = &Emulated;
    struct Space space;
    struct Answer asked;
    struct Device device;

    /* CLAUDE.md, step 6 of the road: R:, 17, the current drive where
       the program is not started under the home directory, and H:, 7;
       RBIL, INT 21h 0Eh: a letter that is no drive leaves the current
       one as it was. */
    CHECK(DosDriveCurrent(dos) == 17);
    DosDriveSelect(dos, 2);
    CHECK(DosDriveCurrent(dos) == 17);
    DosDriveSelect(dos, 7);
    CHECK(DosDriveCurrent(dos) == 7);
    DosDriveSelect(dos, 0x1A);
    CHECK(DosDriveCurrent(dos) == 7);
    DosDriveSelect(dos, 0x0311);
    CHECK(DosDriveCurrent(dos) == 17);
    space = DosSpaceOf(4096, 10000, 5000);
    CHECK(space.value == 8 && space.sector == 512 && space.total == 10000
          && space.free == 5000 && !space.failed);
    space = DosSpaceOf(512, 1000, 10);
    CHECK(space.value == 1 && space.total == 1000 && space.free == 10);
    /* 4,096,000,000 bytes: 62,500 clusters of 64 KB. */
    space = DosSpaceOf(4096, 1000000, 500000);
    CHECK(space.value == 128 && space.total == 62500 && space.free == 31250);
    /* 400 GB: the counts as far as 16 bits go. */
    space = DosSpaceOf(4096, 100000000, 100000000);
    CHECK(space.value == 128 && space.total == 0xFFFF
          && space.free == 0xFFFF);
    space = DosDriveSpace(dos, 0, false);
    CHECK(!space.failed && space.value >= 1 && space.value <= 128
          && (space.value & (space.value - 1)) == 0 && space.sector == 512
          && space.total >= 1 && space.total <= 0xFFFF
          && space.free <= space.total);
    space = DosDriveSpace(dos, 8, false);
    CHECK(!space.failed && space.sector == 512 && space.total >= 1);
    space = DosDriveSpace(dos, 3, false);
    CHECK(!space.failed && space.value == 0xFFFF && space.total == 3);
    space = DosDriveSpace(dos, 18, true);
    CHECK(space.failed && space.value == Emulated.error
          && space.sector == 512);
    space = DosDriveSpace(dos, 26, false);
    CHECK(!space.failed && space.value == 0xFFFF && space.free == 0
          && space.sector == 0 && space.total == 0x1A);
    space = DosDriveSpace(dos, 26, true);
    CHECK(space.failed && space.total == 0x1A);
    asked = DosDriveLetter(dos, 18);
    CHECK(!asked.failed && asked.value == 0x4400);
    asked = DosDriveLetter(dos, 8);
    CHECK(!asked.failed && asked.value == 0x4400);
    asked = DosDriveLetter(dos, 3);
    CHECK(asked.failed && asked.value == 0x0F);
    asked = DosDriveLetter(dos, 26);
    CHECK(asked.failed && asked.value == 0x0F);
    asked = DosDriveSetLetter(dos, 18);
    CHECK(!asked.failed && asked.value == 0x4400);
    asked = DosDriveSetLetter(dos, 1);
    CHECK(asked.failed && asked.value == 0x0F);
    device = DosDriveRemote(dos, 8);
    CHECK(!device.failed && device.value == 0x0300 && device.kind == 0x08C2);
    device = DosDriveRemote(dos, 26);
    CHECK(device.failed && device.value == 0x0F && device.kind == 0);
    asked = DosDriveFixed(dos, 18);
    CHECK(!asked.failed && asked.value == 1);
    asked = DosDriveFixed(dos, 26);
    CHECK(asked.failed && asked.value == 0x0F);
    CHECK(!DosDriveCd(dos, 17) && !DosDriveCd(dos, 7));
}

/* What a DosBegin makes of $HOME: H: at `root`, or no H: for NULL. */
static void HomeChecked(const char *home, const char *root)
{
    static struct Dos other;
    i32 drive;

    DosBegin(&other, home);
    drive = DriveIndex(&other.drives, 'H');
    CHECK(DriveIndex(&other.drives, 'R') >= 0);
    CHECK(other.drives.drive[other.drives.current].letter == 'R');
    if (root == NULL)
        CHECK(drive < 0);
    else {
        CHECK(drive >= 0);
        if (drive >= 0)
            SAME(other.drives.drive[drive].root, root);
    }
    DosEnd(&other);
}

/* A file of the host's in the home directory of the tree. */
static void InHome(const char *name, char *to)
{
    char path[600];

    snprintf(path, sizeof path, "home/%s", name);
    InTree(path, to);
}

/* H:, the home directory of the tree, beside R:, the root of the host,
   by what the user decided on 15 October 2026 (CLAUDE.md, step 6 of the
   road), and by RBIL and fatgen103 for what DOS does with two drives. */
static void ServiceHome(void)
{
    struct Dos *dos = &Emulated;
    struct Answer asked;
    struct Device device;
    char list[1024];
    char path[600];
    char real[PATH_MAX];
    u8 to[DOS_PATH_ROOM];
    u8 fcb[48];
    u8 here[DOS_CURRENT_ROOM];

    /* The labels, each found by a search for a label alone in the root
       of its drive, and nowhere else (fatgen103: the label's entry is in
       the root; RBIL, INT 21h 4Eh: with 08h only the label). */
    CHECK(Listed("H:\\*.*", 8, list, sizeof list) == 0x12);
    SAME(list, "HOME/08 ");
    CHECK(Emulated.record->FilSize == 0);
    CHECK(Listed("R:\\*.*", 8, list, sizeof list) == 0x12);
    SAME(list, "ROOT/08 ");
    CHECK(Listed("\\ROOT", 8, list, sizeof list) == 0x12);
    SAME(list, "ROOT/08 ");
    CHECK(Listed("\\HOME", 8, list, sizeof list) == 0x12);
    SAME(list, "");
    CHECK(Listed("H:\\SUB\\*.*", 8, list, sizeof list) == 3);
    /* The root of H: has no . and no .., and no label but for 08h. */
    HostFile("home/inhome.txt", "in the home directory");
    CHECK(Listed("H:\\*.*", 0x3F, list, sizeof list) == 0x12);
    SAME(list, "INHOME.TXT/20 ");
    /* RBIL, INT 21h 3Ch: no label is made (DIARY.md, "Emulate DOS's
       files, directories and drives on the host's files for Linux", 6). */
    asked = DosFileCreate(dos, NAME("H:\\LABEL"), DOS_LABEL);
    CHECK(asked.failed && asked.value == 5);

    /* RBIL, INT 21h 3Bh and 47h: each drive has its current directory,
       and a change of directory on another drive does not make that the
       current drive; 3Ah: the current directory of a drive is not
       removed, 10h. */
    memcpy(here, Current('R'), strlen((const char *)Current('R')) + 1);
    CHECK(!DosDirMake(dos, NAME("H:\\SUB")).failed);
    CHECK(Listed("H:\\SUB\\*.*", 8, list, sizeof list) == 0x12);
    SAME(list, "");
    CHECK(!DosDirChange(dos, NAME("H:SUB")).failed);
    CHECK(DosDriveCurrent(dos) == 17);
    asked = DosDirCurrent(dos, 8, to);
    CHECK(!asked.failed && asked.value == 0x0100);
    SAME(to, "SUB");
    asked = DosDirCurrent(dos, 0, to);
    CHECK(!asked.failed);
    SAME(to, here);
    asked = DosFileTrueName(dos, NAME("h:x.dat"), to);
    CHECK(!asked.failed);
    SAME(to, "H:\\SUB\\X.DAT");
    asked = DosDirRemove(dos, NAME("H:\\SUB"));
    CHECK(asked.failed && asked.value == 0x10);

    /* A file made on H: is the host's in the home directory; its drive
       is H:'s, 7, in what FileDevice answers (RBIL, INT 21h 4400h). */
    asked = DosFileCreate(dos, NAME("H:MADE.TXT"), 0);
    CHECK(!asked.failed);
    device = DosFileDevice(dos, asked.value);
    CHECK(!device.failed && device.kind == 0x0047);
    DosFileClose(dos, asked.value);
    InHome("sub/made.txt", path);
    CHECK(access(path, F_OK) == 0);
    /* RBIL, INT 21h 56h: not from one drive to another, 11h, on which
       the program copies and deletes, though both are of one file system
       here; within H: a rename. */
    asked = DosFileRename(dos, NAME("H:MADE.TXT"), NAME("R:MADE.TXT"));
    CHECK(asked.failed && asked.value == 0x11);
    asked = DosFileRename(dos, NAME("H:MADE.TXT"), NAME("H:\\MOVED.TXT"));
    CHECK(!asked.failed);
    CHECK(HostHas("home/moved.txt"));
    /* RBIL, INT 21h 13h: the block's drive, its current directory. */
    HostFile("home/sub/gone.txt", "gone");
    memset(fcb, 0, sizeof fcb);
    DosFcbParse(dos, NAME("H:GONE.TXT"), fcb, 1);
    asked = DosFcbDelete(dos, fcb, false);
    CHECK(!asked.failed && asked.value == 0x1300);
    CHECK(!HostHas("home/sub/gone.txt"));
    CHECK(!DosDirChange(dos, NAME("H:\\")).failed);
    CHECK(!DosDirRemove(dos, NAME("H:\\SUB")).failed);

    /* Started under the home directory, on H:; elsewhere on R:. */
    CHECK(!DosDirMake(dos, NAME("H:\\SUB")).failed);
    InHome("sub", path);
    CHECK(!DosStartIn(dos, path).failed);
    CHECK(DosDriveCurrent(dos) == 7);
    SAME(Current('H'), "SUB");
    CHECK(!DosStartIn(dos, Tree).failed);
    CHECK(DosDriveCurrent(dos) == 17);
    SAME(Current('R'), here);
    CHECK(!DosDirChange(dos, NAME("H:\\")).failed);
    CHECK(!DosDirRemove(dos, NAME("H:\\SUB")).failed);

    /* No H: where $HOME is not set, is no path from the root, names no
       directory or the root itself; one that is a link is the directory
       that it leads to. */
    InTree("home", path);
    CHECK(realpath(path, real) != NULL);
    SAME(Emulated.drives.drive[DriveIndex(&Emulated.drives, 'H')].root,
         real);
    HomeChecked(NULL, NULL);
    HomeChecked("", NULL);
    HomeChecked("/", NULL);
    HomeChecked("home", NULL);
    InTree("hello.txt", path);
    HomeChecked(path, NULL);
    InTree("nowhere", path);
    HomeChecked(path, NULL);
    InTree("homelink", path);
    CHECK(symlink(real, path) == 0);
    HomeChecked(path, real);
    InTree("home/", path);
    HomeChecked(path, real);
}

/* A command of DOS's made one for the host's shell (L6): the text in
   UTF-8, each path of DOS's at the start of a word on a drive that there
   is made the host's path, and each name of the current directory the
   host's name (L6b), quoted for the shell where it stands, and nothing
   else changed. */
static void Command(const char *dos, const char *want)
{
    char to[2048];
    i32 made = DosCommandToHost(&Emulated, (const u8 *)dos,
                                (i32)strlen(dos), to, (i32)sizeof to);

    CHECK(made == (i32)strlen(want));
    SAME(to, want);
    if (strcmp(to, want) != 0)
        fprintf(stderr, "  of %s: %s, where %s was wanted\n", dos, to, want);
}

static void ServiceCommands(void)
{
    char real[PATH_MAX], want[2048], dos[256], to[64];
    u8 tree[DOS_PATH_ROOM];

    CHECK(realpath(Tree, real) != NULL);
    CHECK(!DosPathOfHost(&Emulated.directories, &Emulated.drives, real,
                         tree).failed);
    Command("echo hi", "echo hi");
    Command("echo \x80\xE0", "echo \xC3\x87\xCE\xB1");   /* Ç, α */
    /* A name of the current directory by DOS's name, where the host's
       is another, alone or behind .\ (L6b). */
    Command("type HELLO.TXT", "type 'hello.txt'");
    Command("type hello.txt", "type hello.txt");
    Command("type README.TXT", "type README.TXT");
    Command("type LONGFI~1.TXT|wc", "type 'Long File Name.txt'|wc");
    Command(".\\HELLO.TXT", "'./hello.txt'");
    Command(".\\README.TXT x", "'./README.TXT' x");
    Command("echo \"HELLO.TXT\" 'HELLO.TXT'",
            "echo \"hello.txt\" 'HELLO.TXT'");
    Command("cd DOCS;cd ..;cd .", "cd 'docs';cd ..;cd .");
    Command("echo NOWHERE HELLO.TXTX DOCS\\INSIDE.TXT",
            "echo NOWHERE HELLO.TXTX DOCS\\INSIDE.TXT");
    snprintf(dos, sizeof dos, "cat %s\\HELLO.TXT>x", tree);
    snprintf(want, sizeof want, "cat '%s/hello.txt'>x", real);
    Command(dos, want);
    snprintf(dos, sizeof dos, "cat %s\\LONGFI~1.TXT", tree);
    snprintf(want, sizeof want, "cat '%s/Long File Name.txt'", real);
    Command(dos, want);
    snprintf(dos, sizeof dos, "ls %s\\DOCS\\*.TXT", tree);
    snprintf(want, sizeof want, "ls '%s/docs/'*.txt", real);
    Command(dos, want);
    snprintf(dos, sizeof dos, "echo \"%s\\HELLO.TXT\" a=%s", tree, tree);
    snprintf(want, sizeof want, "echo \"%s/hello.txt\" a='%s'", real, real);
    Command(dos, want);
    snprintf(dos, sizeof dos, "echo '%s' x%s", tree, tree);
    Command(dos, dos);
    Command("echo C:\\X", "echo C:\\X");
    snprintf(want, sizeof want, "cd '%s'", Emulated.drives.drive[
                 DriveIndex(&Emulated.drives, 'H')].root);
    Command("cd h:\\", want);
    snprintf(dos, sizeof dos, "cat %s\\NOWHERE\\X", tree);
    Command(dos, dos);
    CHECK(DosCommandToHost(&Emulated, (const u8 *)"echo hi", 7, to, 7)
          == -1);
    CHECK(!DosCurrentHost(&Emulated, want, (i32)sizeof want).failed);
    SAME(want, real);
}

static void Services(void)
{
    const char *base = getenv("TMPDIR");
    struct Answer started = { 3, true };
    char home[600];
    i32 tries;

    /* The tree where the environment keeps temporary files, or in /tmp
       where the program's path of that is longer than DOS holds. */
    for (tries = 0; tries < 2 && started.failed; tries++) {
        if (tries == 1 || base == NULL || base[0] != '/')
            base = "/tmp";
        snprintf(Tree, sizeof Tree, "%s/vcXXXXXX", base);
        if (mkdtemp(Tree) == NULL)
            continue;
        snprintf(home, sizeof home, "%s/home", Tree);
        if (mkdir(home, 0700) != 0) {
            rmdir(Tree);
            continue;
        }
        DosBegin(&Emulated, home);
        started = DosStartIn(&Emulated, Tree);
        if (started.failed) {
            DosEnd(&Emulated);
            rmdir(home);
            rmdir(Tree);
        }
    }
    CHECK(!started.failed);
    if (started.failed)
        return;
    HostFile("hello.txt", "Hello, world!\r\n");
    HostFile("README.TXT", "read me");
    HostFile("Long File Name.txt", "long");
    CHECK(!DosDirMake(&Emulated, NAME("DOCS")).failed);
    HostFile("docs/inside.txt", "inside");
    ServiceFiles();
    ServiceHandles();
    ServiceOthers();
    ServiceOlder();
    ServiceDirectories();
    ServiceFinds();
    ServiceDrives();
    ServiceHome();
    ServiceCommands();
    DosEnd(&Emulated);
    nftw(Tree, Removed, 16, FTW_DEPTH | FTW_PHYS);
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
    Services();
    if (failures != 0) {
        printf("%d checks failed.\n", failures);
        return 1;
    }
    printf("Every check passed.\n");
    return 0;
}
