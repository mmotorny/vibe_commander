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
 *   CP437.TXT, CP866.TXT   Unicode's mapping files of the code pages,
 *                          VENDORS/MICSFT/PC/
 *   COUNTRY.SYS            FreeDOS 1.4's, of the bench: its tables of
 *                          capitals for names, FUCASE, and of what ends a
 *                          name, FCHAR, for the country 1 with the code
 *                          page 437 and 7 with 866
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
 *   POSIX 8.2              XBD, Internationalization Variables: which of
 *                          LC_ALL, LC_CTYPE and LANG counts
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
    CHECK(CodePageToUnicode(437, 0x80) == 0x00C7);
    CHECK(CodePageToUnicode(437, 0x82) == 0x00E9);
    CHECK(CodePageToUnicode(437, 0xB3) == 0x2502);
    CHECK(CodePageToUnicode(437, 0xE1) == 0x00DF);
    CHECK(CodePageToUnicode(437, 0xFF) == 0x00A0);
    CHECK(CodePageToUnicode(437, 'A') == 'A');
    /* CP866.TXT */
    CHECK(CodePageToUnicode(866, 0x80) == 0x0410);
    CHECK(CodePageToUnicode(866, 0xA0) == 0x0430);
    CHECK(CodePageToUnicode(866, 0xE0) == 0x0440);
    CHECK(CodePageToUnicode(866, 0xF0) == 0x0401);
    CHECK(CodePageToUnicode(866, 0xF1) == 0x0451);
    CHECK(CodePageToUnicode(866, 0xFC) == 0x2116);
    CHECK(CodePageToUnicode(866, 0xB3) == 0x2502);
    CHECK(CodePageFromUnicode(866, 0x0451) == 0xF1);
    CHECK(CodePageFromUnicode(437, 0x0451) == -1);
    CHECK(CodePageFromUnicode(437, 0x00E9) == 0x82);
    CHECK(CodePageFromUnicode(437, 0x20AC) == -1);
    /* Each of the two files maps 256 bytes to 256 characters: every
       byte comes back. */
    for (byte = 0; byte < 256; byte++) {
        CHECK(CodePageFromUnicode(437, CodePageToUnicode(437, (u8)byte))
              == byte);
        CHECK(CodePageFromUnicode(866, CodePageToUnicode(866, (u8)byte))
              == byte);
    }

    /* COUNTRY.SYS, FUCASE of 437 */
    CHECK(CodePageUpper(437, 'a') == 'A');
    CHECK(CodePageUpper(437, 'Z') == 'Z');
    CHECK(CodePageUpper(437, '~') == '~');
    CHECK(CodePageUpper(437, 0x81) == 0x9A);
    CHECK(CodePageUpper(437, 0x82) == 'E');
    CHECK(CodePageUpper(437, 0x83) == 'A');
    CHECK(CodePageUpper(437, 0x84) == 0x8E);
    CHECK(CodePageUpper(437, 0x98) == 'Y');
    CHECK(CodePageUpper(437, 0xA4) == 0xA5);
    CHECK(CodePageUpper(437, 0xE1) == 0xE1);
    /* COUNTRY.SYS, FUCASE of 866 */
    CHECK(CodePageUpper(866, 0xA0) == 0x80);
    CHECK(CodePageUpper(866, 0xAF) == 0x8F);
    CHECK(CodePageUpper(866, 0xE0) == 0x90);
    CHECK(CodePageUpper(866, 0xEF) == 0x9F);
    CHECK(CodePageUpper(866, 0xF1) == 0xF0);
    CHECK(CodePageUpper(866, 0xF7) == 0xF6);
    CHECK(CodePageUpper(866, 0xF8) == 0xF8);
    CHECK(CodePageUpper(866, 0x80) == 0x80);

    /* CLAUDE.md, step 6 of the road: 866 where the locale is Russian,
       437 otherwise, a setting first; POSIX 8.2: LC_ALL, then LC_CTYPE,
       then LANG, each where it is set and not empty. */
    CHECK(CodePageChosen(0, NULL, NULL, "ru_RU.UTF-8") == 866);
    CHECK(CodePageChosen(0, NULL, NULL, "ru") == 866);
    CHECK(CodePageChosen(0, NULL, NULL, "en_US.UTF-8") == 437);
    CHECK(CodePageChosen(0, NULL, NULL, NULL) == 437);
    CHECK(CodePageChosen(0, NULL, NULL, "rue_UA") == 437);
    CHECK(CodePageChosen(0, "C", NULL, "ru_RU.UTF-8") == 437);
    CHECK(CodePageChosen(0, "", NULL, "ru_RU.UTF-8") == 866);
    CHECK(CodePageChosen(0, NULL, "ru_UA.KOI8-U", "en_US.UTF-8") == 866);
    CHECK(CodePageChosen(437, NULL, NULL, "ru_RU.UTF-8") == 437);
    CHECK(CodePageChosen(866, NULL, NULL, "en_US.UTF-8") == 866);
}

static void Utf8(void)
{
    u32 character = 0;
    char bytes[4];
    char text[32];
    static const u8 Hello866[] = { 0xAF, 0xE0, 0xA8, 0xA2, 0xA5, 0xE2 };
    static const u8 Capitals866[] = {
        0x8F, 0x90, 0x88, 0x82, 0x85, 0x92, '.', 'T', 'X', 'T'
    };

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

    /* CP866.TXT: привет */
    CHECK(CodePageToUtf8(866, Hello866, 6, false, text, (i32)sizeof text) == 12);
    SAME(text, "\xD0\xBF\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82");
    CHECK(CodePageToUtf8(866, Hello866, 6, false, text, 12) == -1);
    CHECK(CodePageToUtf8(866, Hello866, 6, false, text, 13) == 12);

    /* UnicodeData.txt, the simple mappings to lower case, as Python's
       str.lower has them for every byte of the two code pages; CLAUDE.md,
       step 6: what the program makes on the host is in small letters. */
    CHECK(UnicodeSmall('N') == 'n');
    CHECK(UnicodeSmall('n') == 'n');
    CHECK(UnicodeSmall('~') == '~');
    CHECK(UnicodeSmall(0x00DC) == 0x00FC);
    CHECK(UnicodeSmall(0x00C7) == 0x00E7);
    CHECK(UnicodeSmall(0x00D7) == 0x00D7);
    CHECK(UnicodeSmall(0x00DF) == 0x00DF);
    CHECK(UnicodeSmall(0x03A3) == 0x03C3);
    CHECK(UnicodeSmall(0x0410) == 0x0430);
    CHECK(UnicodeSmall(0x042F) == 0x044F);
    CHECK(UnicodeSmall(0x0401) == 0x0451);
    CHECK(UnicodeSmall(0x040E) == 0x045E);
    CHECK(UnicodeSmall(0x2502) == 0x2502);
    CHECK(CodePageToUtf8(866, Capitals866, 10, true, text, (i32)sizeof text)
          == 16);
    SAME(text, "\xD0\xBF\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82.txt");
    CHECK(CodePageToUtf8(437, (const u8 *)"\x9A\x80.TXT", 6, true, text,
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
    static const char *const Russian[] = {
        DOCUMENTS, "\xD0\xBF\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82.txt"
    };

    /* COUNTRY.SYS, FCHAR, and fatgen103: what a name of DOS's form is */
    CHECK(DosNameValid(437, (const u8 *)"README.TXT"));
    CHECK(DosNameValid(437, (const u8 *)"A"));
    CHECK(DosNameValid(437, (const u8 *)"LONGFI~1.TXT"));
    CHECK(DosNameValid(437, (const u8 *)"VIBE-C~1"));
    CHECK(!DosNameValid(437, (const u8 *)"readme.txt"));
    CHECK(!DosNameValid(437, (const u8 *)"TOOLONGNAME"));
    CHECK(!DosNameValid(437, (const u8 *)"A.B.C"));
    CHECK(!DosNameValid(437, (const u8 *)".TXT"));
    CHECK(!DosNameValid(437, (const u8 *)"NAME."));
    CHECK(!DosNameValid(437, (const u8 *)"A.TEXT"));
    CHECK(!DosNameValid(437, (const u8 *)"A+B"));
    CHECK(!DosNameValid(437, (const u8 *)"A B"));
    CHECK(!DosNameValid(437, (const u8 *)"*.*"));
    CHECK(!DosNameValid(437, (const u8 *)""));
    CHECK(!DosNameValid(437, (const u8 *)"\x82"));
    CHECK(DosNameValid(437, (const u8 *)"\x90"));
    CHECK(DosNameValid(866, (const u8 *)"\x8F\x90\x88\x82\x85\x92.TXT"));
    CHECK(!DosNameValid(866, (const u8 *)"\xAF\xE0\xA8\xA2\xA5\xE2"));

    /* fatgen103, the basis name and its numeric tail, in the order of the
       names' bytes; FCHAR for `+`; DOS's capitals of 437 for é. */
    DirectoryNamesBegin(&table);
    CHECK(DirectoryNamesUpdate(&table, 437, Directory, COUNT(Directory)));
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
    CHECK(DirectoryNamesUpdate(&table, 437, Tens, COUNT(Tens)));
    SAME(DosOf(&table, "LongFileName0"), "LONGFI~1");
    SAME(DosOf(&table, "LongFileName10"), "LONGFI~3");
    SAME(DosOf(&table, "LongFileName7"), "LONGFI~9");
    SAME(DosOf(&table, "LongFileName8"), "LONGF~10");
    SAME(DosOf(&table, "LongFileName9"), "LONGF~11");
    DirectoryNamesEnd(&table);

    /* CLAUDE.md, step 3 of the road: an alias stays the same for the run
       and leads back to its name; one that is gone is given to no other. */
    DirectoryNamesBegin(&table);
    CHECK(DirectoryNamesUpdate(&table, 437, First, COUNT(First)));
    SAME(DosOf(&table, "longfilename1"), "LONGFI~1");
    SAME(DosOf(&table, "longfilename2"), "LONGFI~2");
    CHECK(DirectoryNamesUpdate(&table, 437, Second, COUNT(Second)));
    SAME(DosOf(&table, "longfilename2"), "LONGFI~2");
    SAME(DosOf(&table, "longfilename3"), "LONGFI~3");
    SAME(DosOf(&table, "longfilename1"), "(none)");
    SAME(HostOf(&table, "LONGFI~1"), "(none)");
    CHECK(DirectoryNamesUpdate(&table, 437, Third, COUNT(Third)));
    SAME(DosOf(&table, "longfilename1"), "LONGFI~1");
    SAME(HostOf(&table, "LONGFI~1"), "longfilename1");
    CHECK(DirectoryNamesUpdate(&table, 437, NULL, 0));
    SAME(HostOf(&table, "LONGFI~2"), "(none)");
    DirectoryNamesEnd(&table);

    /* A name of the host that comes beside one that differs in case only
       gets the alias: the first keeps its name. */
    DirectoryNamesBegin(&table);
    CHECK(DirectoryNamesUpdate(&table, 437, Lower, COUNT(Lower)));
    SAME(DosOf(&table, "readme.txt"), "README.TXT");
    CHECK(DirectoryNamesUpdate(&table, 437, Both, COUNT(Both)));
    SAME(DosOf(&table, "readme.txt"), "README.TXT");
    SAME(DosOf(&table, "README.TXT"), "README~1.TXT");
    DirectoryNamesEnd(&table);

    /* CP866.TXT and COUNTRY.SYS's capitals of 866: Документы is nine
       letters, привет.txt six and an extension. */
    DirectoryNamesBegin(&table);
    CHECK(DirectoryNamesUpdate(&table, 866, Russian, COUNT(Russian)));
    SAME(DosOf(&table, Russian[0]), "\x84\x8E\x8A\x93\x8C\x85~1");
    SAME(DosOf(&table, Russian[1]), "\x8F\x90\x88\x82\x85\x92.TXT");
    SAME(HostOf(&table, "\x84\x8E\x8A\x93\x8C\x85~1"), Russian[0]);
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
    const u8 *current[DOS_DRIVES];
    u8 deep[200];
    i32 i;

    for (i = 0; i < DOS_DRIVES; i++)
        current[i] = NULL;
    current['C' - 'A'] = (const u8 *)"A\\B";

    /* DOS_MakeName of DOSBox */
    answer = DosPathFull(437, (const u8 *)"c:/foo/bar", 2, current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\FOO\\BAR");
    answer = DosPathFull(437, (const u8 *)"..\\x", 2, current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\A\\X");
    answer = DosPathFull(437, (const u8 *)"LONGFILENAME.TEXT", 2, current,
                         to);
    CHECK(!answer.failed);
    SAME(to, "C:\\A\\B\\LONGFILE.TEX");
    answer = DosPathFull(437, (const u8 *)".\\file", 2, current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\A\\B\\FILE");
    answer = DosPathFull(437, (const u8 *)"C:\\..", 2, current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\");
    answer = DosPathFull(437, (const u8 *)"\\..\\..\\x", 2, current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\X");
    answer = DosPathFull(437, (const u8 *)"\\", 2, current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\");
    answer = DosPathFull(437, (const u8 *)"C:", 2, current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\A\\B");
    answer = DosPathFull(437, (const u8 *)"d:", 2, current, to);
    CHECK(!answer.failed);
    SAME(to, "D:\\");
    answer = DosPathFull(437, (const u8 *)"name", 3, current, to);
    CHECK(!answer.failed);
    SAME(to, "D:\\NAME");
    answer = DosPathFull(437, (const u8 *)"*.*", 2, current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\A\\B\\*.*");
    answer = DosPathFull(866, (const u8 *)"c:\\\xAF\xE0\xA8\xA2\xA5\xE2.txt",
                         2, current, to);
    CHECK(!answer.failed);
    SAME(to, "C:\\\x8F\x90\x88\x82\x85\x92.TXT");
    answer = DosPathFull(437, (const u8 *)"a.b.c", 2, current, to);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathFull(437, (const u8 *)"a+b", 2, current, to);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathFull(437, (const u8 *)"1:\\x", 2, current, to);
    CHECK(answer.failed && answer.value == 0x0F);
    /* A path longer than DOS's 128 bytes */
    for (i = 0; i < 20; i++)
        memcpy(deep + 9 * i, "\\ABCDEFGH", 9);
    deep[180] = '\0';
    answer = DosPathFull(437, deep, 2, current, to);
    CHECK(answer.failed && answer.value == 3);
}

static void SettingsChecked(void)
{
    struct Settings settings;
    char file[64];

    /* CLAUDE.md, step 6 of the road: vc/ where XDG says, a variable
       naming another file for the tests. The XDG Base Directory
       Specification: $HOME/.config where $XDG_CONFIG_HOME is not set or
       is empty, and a path in it that is not absolute is ignored. */
    CHECK(SettingsFile(NULL, "/home/user/.cfg", "/home/user", file,
                       (i32)sizeof file));
    SAME(file, "/home/user/.cfg/vc/settings");
    CHECK(SettingsFile(NULL, NULL, "/home/user", file, (i32)sizeof file));
    SAME(file, "/home/user/.config/vc/settings");
    CHECK(SettingsFile(NULL, "", "/home/user", file, (i32)sizeof file));
    SAME(file, "/home/user/.config/vc/settings");
    CHECK(SettingsFile(NULL, "relative", "/home/user", file,
                       (i32)sizeof file));
    SAME(file, "/home/user/.config/vc/settings");
    CHECK(SettingsFile("/tmp/test/settings", "/home/user/.cfg",
                       "/home/user", file, (i32)sizeof file));
    SAME(file, "/tmp/test/settings");
    CHECK(SettingsFile("", NULL, "/home/user", file, (i32)sizeof file));
    SAME(file, "/home/user/.config/vc/settings");
    CHECK(!SettingsFile(NULL, NULL, NULL, file, (i32)sizeof file));
    CHECK(!SettingsFile(NULL, NULL, "/home/user", file, 30));
    CHECK(SettingsFile(NULL, NULL, "/home/user", file, 31));

    /* CLAUDE.md, step 6 of the road: C: is the root, and nothing else. */
    CHECK(SettingsBegin(&settings));
    SAME(settings.drive['C' - 'A'], "/");
    CHECK(settings.drive['D' - 'A'] == NULL);
    CHECK(settings.code_page == 0);
    CHECK(SettingsRead(&settings,
                       "# drives\n\nmount D /home/user/\n"
                       "  mount e: /media/usb stick  \r\n"
                       "codepage 866\nmount C /srv") == 0);
    SAME(settings.drive['D' - 'A'], "/home/user");
    SAME(settings.drive['E' - 'A'], "/media/usb stick");
    SAME(settings.drive['C' - 'A'], "/srv");
    CHECK(settings.code_page == 866);
    CHECK(SettingsRead(&settings, "mount F /a\nbogus\nmount G /b") == 2);
    SAME(settings.drive['F' - 'A'], "/a");
    CHECK(settings.drive['G' - 'A'] == NULL);
    CHECK(SettingsRead(&settings, "mount D relative") == 1);
    CHECK(SettingsRead(&settings, "codepage 850") == 1);
    CHECK(SettingsRead(&settings, "mount 1 /x") == 1);
    CHECK(SettingsRead(&settings, "mount DD /x") == 1);
    CHECK(SettingsRead(&settings, "mount D") == 1);
    CHECK(SettingsRead(&settings, "") == 0);
    SettingsEnd(&settings);
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
                     struct DirectoryNames *table, u16 code_page)
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
        return DirectoryNamesUpdate(table, code_page, Fake[i].names, count);
    }
    errno = strcmp(path, "/root") == 0 ? EACCES : ENOENT;
    return false;
}

/* A directory of the deep chain: /DIRNAME1/DIRNAME2/... each with the
   next in it, to DIRNAME9. */
static bool DeepRead(void *context, const char *path,
                     struct DirectoryNames *table, u16 code_page)
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
        return DirectoryNamesUpdate(table, code_page, NULL, 0);
    memcpy(name, "DIRNAME", 7);
    name[7] = (char)('1' + depth);
    name[8] = '\0';
    names[0] = name;
    return DirectoryNamesUpdate(table, code_page, names, 1);
}

static void Paths(void)
{
    struct Settings settings;
    struct Directories directories;
    struct Answer answer;
    char host[256];
    u8 dos[DOS_PATH_ROOM];
    i32 reads = 0;

    CHECK(SettingsBegin(&settings));
    CHECK(SettingsRead(&settings, "mount D /home/user") == 0);
    DirectoriesBegin(&directories, 437, FakeRead, &reads);

    /* CLAUDE.md, step 3 of the road: a drive letter is a directory of
       the host, by a setting, as DOSBox's mount; an alias leads back. */
    answer = HostPathOfDos(&directories, &settings,
                           (const u8 *)"C:\\HOME\\USER\\NOTES.TXT", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/home/user/notes.txt");
    answer = HostPathOfDos(&directories, &settings,
                           (const u8 *)"D:\\LONGFI~1.TXT", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/home/user/LongFileName.txt");
    answer = HostPathOfDos(&directories, &settings, (const u8 *)"D:\\", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/home/user");
    answer = HostPathOfDos(&directories, &settings, (const u8 *)"C:\\", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/");
    answer = HostPathOfDos(&directories, &settings,
                           (const u8 *)"D:\\NEW.TXT", host, (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 0);
    SAME(host, "/home/user/new.txt");
    /* RBIL, INT 21h 3Dh: 3 where a directory on the way is not there,
       0Fh for a drive that there is not (INT 21h 0Eh and 47h) */
    answer = HostPathOfDos(&directories, &settings,
                           (const u8 *)"D:\\NOPE\\X.TXT", host,
                           (i32)sizeof host);
    CHECK(answer.failed && answer.value == 3);
    answer = HostPathOfDos(&directories, &settings, (const u8 *)"E:\\X",
                           host, (i32)sizeof host);
    CHECK(answer.failed && answer.value == 0x0F);
    answer = HostPathOfDos(&directories, &settings,
                           (const u8 *)"C:\\ROOT\\X", host, (i32)sizeof host);
    CHECK(answer.failed && answer.value == 3);
    answer = HostPathOfDos(&directories, &settings,
                           (const u8 *)"C:\\HOME\\USER", host, 10);
    CHECK(answer.failed && answer.value == 3);

    /* A name that the host has since it was read is found: the directory
       is read again for a name that it has not. */
    Fake[2].names[5] = "added.txt";
    answer = HostPathOfDos(&directories, &settings,
                           (const u8 *)"D:\\ADDED.TXT", host,
                           (i32)sizeof host);
    CHECK(!answer.failed && answer.value == 1);
    SAME(host, "/home/user/added.txt");
    Fake[2].names[5] = NULL;

    /* The drive that holds the most of the path */
    answer = DosPathOfHost(&directories, &settings,
                           "/home/user/Projects/vibe-commander", dos);
    CHECK(!answer.failed);
    SAME(dos, "D:\\PROJECTS\\VIBE-C~1");
    answer = DosPathOfHost(&directories, &settings, "/home/user", dos);
    CHECK(!answer.failed);
    SAME(dos, "D:\\");
    answer = DosPathOfHost(&directories, &settings, "/etc", dos);
    CHECK(!answer.failed);
    SAME(dos, "C:\\ETC");
    answer = DosPathOfHost(&directories, &settings, "/", dos);
    CHECK(!answer.failed);
    SAME(dos, "C:\\");
    answer = DosPathOfHost(&directories, &settings, "/home//user/", dos);
    CHECK(!answer.failed);
    SAME(dos, "C:\\HOME\\USER");
    answer = DosPathOfHost(&directories, &settings, "/home/users", dos);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathOfHost(&directories, &settings, "/home/../etc", dos);
    CHECK(answer.failed && answer.value == 3);
    answer = DosPathOfHost(&directories, &settings, "relative", dos);
    CHECK(answer.failed && answer.value == 3);
    DirectoriesEnd(&directories);

    /* sys.h: a path holds 67 characters. C:\ and seven names of eight
       with the backslashes between are 65, and an eighth makes 74. */
    DirectoriesBegin(&directories, 437, DeepRead, NULL);
    answer = DosPathOfHost(&directories, &settings,
                           "/DIRNAME1/DIRNAME2/DIRNAME3/DIRNAME4/DIRNAME5/"
                           "DIRNAME6/DIRNAME7", dos);
    CHECK(!answer.failed && strlen((const char *)dos) == 65);
    answer = DosPathOfHost(&directories, &settings,
                           "/DIRNAME1/DIRNAME2/DIRNAME3/DIRNAME4/DIRNAME5/"
                           "DIRNAME6/DIRNAME7/DIRNAME8", dos);
    CHECK(answer.failed && answer.value == 3);
    DirectoriesEnd(&directories);
    SettingsEnd(&settings);
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
    SettingsChecked();
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
