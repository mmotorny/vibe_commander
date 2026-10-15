cmake_minimum_required(VERSION 3.24)
project(vibe_commander LANGUAGES C)

# --- JWasm ------------------------------------------------------------------
# The assembler that builds src/asm/, the original program as its author
# published it, and the driver of the tests: a free, MASM-compatible
# assembler that runs natively and writes .COM files directly. It is
# fetched at a pinned commit and built from source. Nothing of the C
# trees is assembled: the tests hold them against the original, and hold
# the original, built here, against the binary that was shipped.

include(FetchContent)
FetchContent_Declare(jwasm
    GIT_REPOSITORY https://github.com/Baron-von-Riedesel/JWasm.git
    GIT_TAG ac54827ff40b77ecd6e77ed0866a43fc60cd5fe1)   # v2.20
FetchContent_MakeAvailable(jwasm)

# JWasm ships makefiles rather than a CMake project. Its Unix makefile
# compiles every file in src/ except the memory tracer, with these settings.
file(GLOB jwasm_sources ${jwasm_SOURCE_DIR}/src/*.c)
list(FILTER jwasm_sources EXCLUDE REGEX "/trmem\\.c$")
add_executable(jwasm ${jwasm_sources})
target_include_directories(jwasm PRIVATE ${jwasm_SOURCE_DIR}/src/H)
target_compile_definitions(jwasm PRIVATE __UNIX__ NDEBUG)
target_compile_options(jwasm PRIVATE -w)        # not our code to tidy up
set_target_properties(jwasm PROPERTIES
    C_STANDARD 99
    C_EXTENSIONS ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools)

# --- What is built ------------------------------------------------------------
# Two options say which downloads the build takes, beside JWasm, which is
# small and always built: the driver of the tests, which it assembles, is
# what every test of a machine runs, the program for Linux's too, whose
# bench's setup file is made once on a machine of DOS.
#   VC_DOS   Open Watcom, which builds the C translation and the program
#            for DOS of the portable C. ON by default. Every test of DOS
#            and of the frozen trees needs it: off, those are not made,
#            and what is left is the original assembled, the program for
#            Linux and its tests, the tests that read the source, and the
#            web's where VC_WEB is on.
#   VC_WEB   Emscripten, which builds the portable C for the web page,
#            step 7 of the road. OFF by default; on, it is fetched and
#            the web's tests are made.
option(VC_DOS "Fetch Open Watcom; build and test the programs for DOS" ON)
option(VC_WEB "Fetch Emscripten; build and test the web page" OFF)

# --- Open Watcom --------------------------------------------------------------
# The C compiler and the linker that build the C trees: an open-source
# compiler for 16-bit DOS, among other targets. It takes hours to build from
# source, so the project's own snapshot of 1 October 2026 is fetched, pinned
# by its SHA-256, and the tools that run on this computer are used from it.

if(VC_DOS)
    FetchContent_Declare(open_watcom
        URL https://github.com/open-watcom/open-watcom-v2/releases/download/2026-10-01-Build/ow-snapshot.tar.xz
        URL_HASH SHA256=e6aa1b1e40ac8bbf97658d2c70fff8a4242d6ca4a1c60806f2baa5317083d4fe
        DOWNLOAD_NO_EXTRACT FALSE)
    FetchContent_MakeAvailable(open_watcom)

    set(watcom_hosts
        Linux-x86_64 binl64  Linux-aarch64 arml64
        Darwin-x86_64 bino64 Darwin-arm64 armo64
        Windows-AMD64 binnt64)
    set(host ${CMAKE_HOST_SYSTEM_NAME}-${CMAKE_HOST_SYSTEM_PROCESSOR})
    list(FIND watcom_hosts ${host} index)
    if(index EQUAL -1)
        message(FATAL_ERROR "Open Watcom has no tools for ${host}")
endif()
math(EXPR index "${index} + 1")
list(GET watcom_hosts ${index} watcom_bin)
set(watcom_bin ${open_watcom_SOURCE_DIR}/${watcom_bin})
set(watcom ${CMAKE_COMMAND} -E env WATCOM=${open_watcom_SOURCE_DIR})
endif()

# --- Volkov Commander ---------------------------------------------------------

# The files each program's main source includes.
set(VC_includes
    VCLAYOUT.INC
    VCFUNC.INC VCMENU.INC VCF1.INC VCCOPY.INC VCVIEW.INC VCEDIT.INC
    VCHELP.INC VCMEM.INC VCFIND.INC VCSUB1.INC VCSUB2.INC VCSUB3.INC)
set(VCSETUP_includes
    VCLAYOUT.INC
    VCSTHLP.INC VCSTSUB.INC)

# add_programs(<target> <directory> [<jwasm option>...]) assembles
# src/asm/VC.ASM and src/asm/VCSETUP.ASM into <directory>/VC.COM and
# <directory>/VCSETUP.COM, with listings next to them.
function(add_programs target directory)
    set(source_dir ${CMAKE_SOURCE_DIR}/src/asm)
    set(outputs "")
    foreach(name VC VCSETUP)
        set(output ${directory}/${name}.COM)
        list(TRANSFORM ${name}_includes PREPEND ${source_dir}/
             OUTPUT_VARIABLE includes)
        add_custom_command(
            OUTPUT ${output}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${directory}
            # JWasm takes default options from the JWASM environment variable.
            COMMAND ${CMAKE_COMMAND} -E env --unset=JWASM
                    $<TARGET_FILE:jwasm> -nologo -q -bin ${ARGN}
                    -I${source_dir} -Fo ${output} -Fl=${directory}/${name}.LST
                    ${source_dir}/${name}.ASM
            DEPENDS jwasm ${source_dir}/${name}.ASM ${includes}
            COMMENT "Assembling ${name}.COM (${target})"
            VERBATIM)
        list(APPEND outputs ${output})
    endforeach()
    add_custom_target(${target} ALL DEPENDS ${outputs})
endfunction()

# The programs themselves.
add_programs(programs ${CMAKE_BINARY_DIR})

# The same source laid out like the binaries shipped in 2000, which were
# shareware: room is reserved for the registration code and data that they
# have and this source does not. Only the tests use this build; they compare
# it with the originals byte for byte. See src/asm/VCLAYOUT.INC.
set(layout_dir ${CMAKE_BINARY_DIR}/shareware-layout)
add_programs(shareware-layout ${layout_dir} -DSHAREWARE_LAYOUT)

# And both once more with that layout, as object files: their relocations
# say which words of the original hold the address of something, and of
# what. The test of a C tree's data goes by them (harness/datacheck.py):
# by VC's for the main program, and by VCSETUP's for the setup program
# where a tree has that in itself.
set(layout_objects "")
foreach(name VC VCSETUP)
    list(TRANSFORM ${name}_includes PREPEND ${CMAKE_SOURCE_DIR}/src/asm/
         OUTPUT_VARIABLE layout_includes)
    add_custom_command(
        OUTPUT ${layout_dir}/${name}.OBJ
        COMMAND ${CMAKE_COMMAND} -E make_directory ${layout_dir}
        COMMAND ${CMAKE_COMMAND} -E env --unset=JWASM
                $<TARGET_FILE:jwasm> -nologo -q -omf -DSHAREWARE_LAYOUT
                -I${CMAKE_SOURCE_DIR}/src/asm -Fo ${layout_dir}/${name}.OBJ
                ${CMAKE_SOURCE_DIR}/src/asm/${name}.ASM
        DEPENDS jwasm ${CMAKE_SOURCE_DIR}/src/asm/${name}.ASM
                ${layout_includes}
        COMMENT "Assembling ${name}.OBJ (shareware-layout)"
        VERBATIM)
    list(APPEND layout_objects ${layout_dir}/${name}.OBJ)
endforeach()
add_custom_target(shareware-relocations ALL DEPENDS ${layout_objects})

# --- The C trees --------------------------------------------------------------
# Volkov Commander in C, twice: src/c/, the C translation, which is the
# original translated for DOS and frozen, and src/portable_c/, the portable
# C, which began as a copy of it and is where work goes on. Each is its
# *.c files, compiled and linked into one program, VC.EXE, in a directory
# of the build that has the tree's name. Nothing of either is assembled:
# the assembler above builds the original, for the tests to hold the trees
# against, and the driver of the tests.
#
# The C translation is built by add_c_tree: it is two parts, one that
# stays in the memory while a command runs and one that is given up and
# loaded again, as the original is, and tools/mkexe.c lays its file out
# for that. See vc.h of that tree and tools/mkexe.c. The portable C is one
# program, a flat one of 32 bits that the linker writes and an extender
# starts: add_portable_c, further down.

# mkexe turns the linker's output into the program that is run.
add_executable(mkexe tools/mkexe.c)
set_target_properties(mkexe PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools)

# stub switches off the banner of the extender in the portable C as the
# linker wrote it: see add_portable_c, further down.
add_executable(stub tools/stub.c)
set_target_properties(stub PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools)

# c_compile(<tree> <model> <source> <object> <segment> <class>
# [<option>...]) compiles a file of src/<tree>/ for 16 bits into
# build/<tree>/<object>.obj, with a listing next to it. It is how the C
# translation is compiled; the portable C was, while it was a program of
# 16 bits.
function(c_compile tree model source object text class)
    set(source_dir ${CMAKE_SOURCE_DIR}/src/${tree})
    set(c_dir ${CMAKE_BINARY_DIR}/${tree})
    # -0     code for the 8086, like the original
    # -ms    the model that the tree is compiled in, which its caller
    #        names: near data, and a function near unless it says
    #        otherwise
    # -os    favour small code
    # -s     no checks for stack overflow, which need the C library
    # -zu    the stack is not in the data segment
    # -zl    name no libraries to link: the program has none
    # -zls   no reference to the C library's start-up code either
    # -wx -we   all warnings, as errors
    # -nt, -nc  the name and class of the code segment: CTEXT, CTEXT2
    #           for a file of the second segment, RTEXT for the part that
    #           stays
    #
    # What the compiler made of it is written next to the object, as a
    # listing: that is where to read which registers a function saves and
    # how much stack it takes.
    # A file is compiled again when a header of its tree has changed:
    # vc.h and tables.h.
    add_custom_command(
        OUTPUT ${c_dir}/${object}.obj
        COMMAND ${watcom} ${watcom_bin}/wcc -q -0 -m${model} -os -s -zu -zl -zls
                -wx -we -nt=${text} -nc=${class} ${ARGN}
                -fo=${c_dir}/${object}.obj
                ${source_dir}/${source}
        COMMAND ${watcom} ${watcom_bin}/wdis -l=${c_dir}/${object}.lst
                ${c_dir}/${object}.obj
        BYPRODUCTS ${c_dir}/${object}.lst
        DEPENDS ${source_dir}/${source}
                ${source_dir}/vc.h
                ${source_dir}/tables.h
        COMMENT "Compiling ${tree}/${source} (${text})"
        VERBATIM)
endfunction()

# add_c_tree(<tree> <description> <model> [<file of the second
# segment>...]) builds src/<tree>/ into build/<tree>/VC.EXE, with the
# linker's map VC.MAP and the object files next to it, as the target
# <tree>.
function(add_c_tree tree description model)
    set(source_dir ${CMAKE_SOURCE_DIR}/src/${tree})
    set(c_dir ${CMAKE_BINARY_DIR}/${tree})
    # Every C file of the tree is part of the program, in the order of
    # their names: a new one needs no mention here. The build looks again
    # each time it runs. One whose name ends in _t holds constants and no
    # code: the tables of the file it is named after (see tables.h).
    file(GLOB c_files CONFIGURE_DEPENDS RELATIVE ${source_dir}
         ${source_dir}/*.c)
    # Two files are linked before the others, whatever their names. The
    # file is the program as it lies in the memory, and it begins with the
    # part that stays there while a command runs: resident.c, whose code
    # begins with the room of the PSP. A segment holds what the object
    # files have of it in their order. And the part that goes begins with
    # transit_t.c, the room of the header that DOS keeps in front of its
    # block of memory.
    list(REMOVE_ITEM c_files resident.c transit_t.c)
    list(PREPEND c_files resident.c transit_t.c)
    # The compiled code is more than one segment holds, so it has two:
    # CTEXT, and CTEXT2 for the files that the caller names. Functions of
    # different files call each other far, so a file may be in either. The
    # routines that the compiler calls for arithmetic on 32 bits, __U4M
    # and __U4D of arith.c, are in CTEXT. With the model s the compiler
    # calls them near: a file that needs them stays in CTEXT then, or the
    # linker says that it cannot reach them. A segment holds F000h bytes,
    # not 64K:
    # tools/mkexe.c says why, and stops the build when one is fuller. Name
    # another file to the function then.
    set(c_second ${ARGN})
    # The part of the program that stays in the memory while a command
    # runs has a code segment of its own, RTEXT, which is the first of the
    # file: the files named here. That part cannot call the rest of the
    # program, which is not in the memory then, so dos.c, the one file
    # that calls DOS and the BIOS, is compiled a second time for it, into
    # RTEXT as well: one source, two copies of its code. RESIDENT is
    # defined for both, and by it vc.h gives the functions of the second
    # copy names of their own.
    #
    # That part is one segment to the program, the PSP's, as the whole
    # program was when it was a .COM file: CS, DS and SS are on the PSP
    # there, and every offset counts from it. Two options of Open Watcom
    # make it so, and nothing of the C language does (see the README,
    # "What rests on Open Watcom"):
    #   -nd=R        the data of a file is in a group of its own, R_GROUP,
    #                which the code reads through DS, in four segments of
    #                the class FAR_DATA: RCONST, RCONST2, R_DATA, R_BSS
    #   -g=R_GROUP   the code segment is of a group of that name
    # With both the object file names two groups of one name, and the
    # linker makes one of them, which begins with the code and so with the
    # PSP. The second copy of dos.c has no data and gets the second option
    # alone.
    set(c_resident resident.c)

    file(MAKE_DIRECTORY ${c_dir})
    set(c_compiled "")
    foreach(source ${c_files})
        string(REGEX REPLACE "\\.c$" "" object ${source})
        if(source IN_LIST c_resident)
            c_compile(${tree} ${model} ${source} ${object} RTEXT RCODE -dRESIDENT
                      -nd=R -g=R_GROUP)
        elseif(source IN_LIST c_second)
            c_compile(${tree} ${model} ${source} ${object} CTEXT2 CCODE)
        else()
            c_compile(${tree} ${model} ${source} ${object} CTEXT CCODE)
        endif()
        list(APPEND c_compiled ${c_dir}/${object}.obj)
    endforeach()
    c_compile(${tree} ${model} dos.c dos_r RTEXT RCODE -dRESIDENT -g=R_GROUP)
    list(APPEND c_compiled ${c_dir}/dos_r.obj)

    # The program is linked of the compiled code alone, with a link map
    # next to it, and tools/mkexe.c makes the file that is run of what the
    # linker writes. It starts at Alloc0, a function of resident.c.
    #
    # The order of the segments is said here, by their classes and within
    # a class by their names: the linker would have them as it first meets
    # them in the object files, and every object file of C names its code
    # segment first. The code of the part that stays, with the PSP's room;
    # its data, which ends with the stack; the constants of the rest,
    # which begin with the room of a block's header; and the code of the
    # rest. No code of the rest may stand within a page of the stack:
    # tools/mkexe.c says why, and stops the build if it does. The
    # directive is Open Watcom's, and its help does not list it for DOS
    # (see the README, "What rests on Open Watcom").
    #
    # Warning 1014 says that there is no stack segment: the stack is of
    # the part that stays in the memory, and tools/mkexe.c names it in the
    # header of the file.
    list(JOIN c_compiled "," compiled)
    add_custom_command(
        OUTPUT ${c_dir}/VC.EXE ${c_dir}/VC.MAP
        COMMAND ${watcom} ${watcom_bin}/wlink option quiet format dos
                name ${c_dir}/linked.exe
                order clname RCODE
                      clname FAR_DATA segment RCONST segment RCONST2
                                      segment R_DATA segment R_BSS
                      clname DATA segment CONST segment CONST2 segment _DATA
                      clname CCODE segment CTEXT segment CTEXT2
                file ${compiled}
                option map=${c_dir}/VC.MAP,start=Alloc0,static
                disable 1014
        COMMAND mkexe ${c_dir}/linked.exe ${c_dir}/VC.MAP
                ${c_dir}/VC.EXE ${c_compiled}
        DEPENDS mkexe ${c_compiled}
        COMMENT "Linking ${description}"
        VERBATIM)
    # The program as it is shipped.
    add_custom_target(${tree} ALL DEPENDS ${c_dir}/VC.EXE)
endfunction()

# The files of src/portable_c/ that a build of each implementation of
# sys.h takes. The tree has the program's common code, which every
# implementation is built with, and the files of the implementations: a
# file whose name ends in _<implementation>.c, of one of those named in
# portable_c_implementations, is of that implementation, and every other
# file is common (a file of tables ends in _t.c, which is common code and
# no implementation). A build of an implementation takes the common code
# and the files of the implementations that portable_c_takes_<it> names,
# but those that portable_c_leaves_<it> names: the web page's (step 7,
# TODO.md, "Step 7, the web page") takes the emulation of DOS of the
# program for Linux, *_posix.c, but the two files that have a terminal
# and a shell, sys_posix.c and term_posix.c, and has its own, *_web.c. A
# file of a new implementation is named here when it is added, or it is
# taken for common code.
set(portable_c_implementations dos posix web)
set(portable_c_takes_dos dos)
set(portable_c_takes_posix posix)
set(portable_c_takes_web posix web)
set(portable_c_leaves_web sys_posix.c term_posix.c)

# portable_c_sources(<variable> <implementation>) sets <variable> to the
# files that a build of <implementation> takes, by the lists above, by
# their names relative to src/portable_c/ and in the order of the names.
function(portable_c_sources variable implementation)
    set(source_dir ${CMAKE_SOURCE_DIR}/src/portable_c)
    file(GLOB all CONFIGURE_DEPENDS RELATIVE ${source_dir}
         ${source_dir}/*.c)
    set(files "")
    foreach(file ${all})
        set(of "")
        foreach(other ${portable_c_implementations})
            if(file MATCHES "_${other}\\.c$")
                set(of ${other})
            endif()
        endforeach()
        if(of STREQUAL "")
            list(APPEND files ${file})
        elseif(of IN_LIST portable_c_takes_${implementation}
               AND NOT file IN_LIST portable_c_leaves_${implementation})
            list(APPEND files ${file})
        endif()
    endforeach()
    set(${variable} ${files} PARENT_SCOPE)
endfunction()

# add_portable_c(<target> <directory> <option>...) builds src/portable_c/
# as a flat program of 32 bits that the extender DOS/32A starts: step 1
# of the portable C's road (DIARY.md, "Write down the plan for the 32
# bits, with the scripts of its trials"). It is <directory>/VC.EXE, with
# the linker's map VC.MAP and the objects and their listings next to it,
# as the target <target>.
#
# The options are the caller's where the two programs of the plan differ,
# the one that is shipped and the one for finding a fault: each is a call
# of this function with a target and a directory of its own.
function(add_portable_c target directory)
    set(source_dir ${CMAKE_SOURCE_DIR}/src/portable_c)
    # The files of this build, the implementation for DOS, _dos.c:
    # sys_dos.c, the system's functions, and stack_dos.c, the stack in a
    # segment of its own, with the common code (portable_c_sources).
    portable_c_sources(c_files dos)
    # The files in the order of their names, and sys_dos.c, the file of
    # the system's functions, behind them all: the linker's command,
    # below, says why it was, and that nothing rests on it now.
    if(NOT sys_dos.c IN_LIST c_files)
        message(FATAL_ERROR
            "src/portable_c/sys_dos.c is named in CMakeLists.txt and is not "
            "there: the file that is to be linked last has another name")
    endif()
    list(REMOVE_ITEM c_files sys_dos.c)
    list(APPEND c_files sys_dos.c)
    # -3r    code for the 386, with arguments in registers
    # -mf    the flat model: one segment of code and one of data, and a
    #        pointer is an offset of 32 bits
    # -s -zl -zls -wx -we   as in c_compile above: no check of the stack,
    #        no library, and every warning an error
    # -i=    the compiler's own headers, for two of them: vc.h has
    #        offsetof of <stddef.h> and bool of <stdbool.h>, as any
    #        other compiler has them
    # -fr    no file of the errors beside what the compiler writes out:
    #        it would be build/<file>.err, which is where the compiler
    #        for 16 bits leaves its own of a file of the same name, of
    #        the C translation
    set(compile ${watcom} ${watcom_bin}/wcc386 -q -3r -mf -s -zl -zls
        -wx -we ${ARGN} -i=${open_watcom_SOURCE_DIR}/h -fr)
    file(MAKE_DIRECTORY ${directory})
    # A file is compiled again when a header of the tree has changed, and
    # when the command that compiles it has: the command is written into
    # a file of the directory, which is written anew only when it says
    # something else, and every object is made to follow that file. The
    # build would not see it otherwise, and an object that another
    # compiler left under the same name, as the one for 16 bits did in
    # build/portable_c/ while the portable C was a program of 16 bits,
    # would be linked as it is.
    file(GENERATE OUTPUT ${directory}/compiled.txt
         CONTENT "${compile}\n")
    set(c_compiled "")
    foreach(source ${c_files})
        string(REGEX REPLACE "\\.c$" "" object ${source})
        # The listing has the lines of the source between the
        # instructions where the object says which they are, which it
        # does when it is compiled for a debugger (-d1 or more).
        set(lines "")
        if("-d2" IN_LIST ARGN)
            set(lines -s=${source_dir}/${source})
        endif()
        add_custom_command(
            OUTPUT ${directory}/${object}.obj
            COMMAND ${compile} -fo=${directory}/${object}.obj
                    ${source_dir}/${source}
            COMMAND ${watcom} ${watcom_bin}/wdis ${lines}
                    -l=${directory}/${object}.lst
                    ${directory}/${object}.obj
            BYPRODUCTS ${directory}/${object}.lst
            DEPENDS ${source_dir}/${source} ${source_dir}/vc.h
                    ${source_dir}/tables.h ${source_dir}/sys.h
                    ${source_dir}/compiler.h
                    ${directory}/compiled.txt
            COMMENT "Compiling portable_c/${source} (${target})"
            VERBATIM)
        list(APPEND c_compiled ${directory}/${object}.obj)
    endforeach()

    # The program is linked of those objects alone, as a linear
    # executable, which is the kind of file that the extender loads
    # (format os2 le), with the extender in front of it as its stub:
    # binw/dos32a.exe of the Open Watcom fetched above, DOS/32A. The stub
    # is in no file of this repository; its licence is, LICENSE.DOS32A.
    # The linker's file of systems for Linux knows no `dos32a`, so the
    # format and the stub are said here. The extender enters the program
    # at Start of sys_dos.c, on the stack that the header of the file
    # names, which is the segment of the class STACK, of stack_dos.c.
    #
    # The order of the segments is said here, by their classes, and the
    # tests rest on it:
    #
    #   CODE    the code, an object of the file by itself
    #   STACK   the stack, the first thing of the data
    #   DATA    the constants of the code and the tables, CONST2, and
    #           behind them what has a value, _DATA, which is little
    #           more than the tables that the program writes to
    #   MAIN    the main data, md of main.c, in the file as zeros
    #   BSS     what has no value, which the file does not hold
    #
    # - The stack is in the file as zeros, by which the scenarios' test
    #   tells how much of it a scenario has left: so it stands in front
    #   of what has no value, which would otherwise be in the file as
    #   zeros in front of it. It is first, for the reason below.
    # - The program itself rests on no place of the linker's since the
    #   piece F10 (DIARY.md, "Take the megabyte down: its words out of
    #   vc.h, its rules out of the test of the layout"): until then a
    #   segment and an offset were numbers of 16 bits in it, counted from
    #   64K below the main data, and the tables had to lie within 32K of
    #   Title_S of main_t.c, IniSave of setup.c within 64K of md, and the
    #   program within the megabyte that a segment could name. sys_dos.c is
    #   linked last as it was for that, so that its variables are behind
    #   everything else that has none: nothing rests on it now, and it
    #   leaves the program as it was.
    # - The machine of the tests is many times slower at a write to a
    #   page of the memory that has code which it has run, and the
    #   extender packs the data right behind the code, in the same page
    #   of 4K. What shares that page is the low end of the stack, which
    #   a program writes to that has used three quarters of its stack,
    #   and the scenarios' test fails one that uses half. The constants
    #   stand between the stack and everything else that is written.
    #
    # The test <target>-layout holds the linked file to what the tests
    # rest on of these, by the linker's map: harness/layoutcheck.py.
    #
    # The extender writes two lines of its name when it starts, onto what
    # would be the program's first screen: tools/stub.c switches that off
    # in the file, by one bit of the stub's settings.
    list(JOIN c_compiled "," compiled)
    set(extender ${open_watcom_SOURCE_DIR}/binw/dos32a.exe)
    add_custom_command(
        OUTPUT ${directory}/VC.EXE ${directory}/VC.MAP
        COMMAND ${watcom} ${watcom_bin}/wlink option quiet format os2 le
                name ${directory}/VC.EXE
                order clname CODE clname STACK clname DATA clname MAIN
                      clname BSS
                file ${compiled}
                option map=${directory}/VC.MAP,start=Start_,static
                option stub=${extender}
        COMMAND stub ${directory}/VC.EXE
        DEPENDS stub ${c_compiled} ${extender}
        COMMENT "Linking the portable C (${target})"
        VERBATIM)
    add_custom_target(${target} ALL DEPENDS ${directory}/VC.EXE)
endfunction()

# --- clang --------------------------------------------------------------------
# The builds of src/portable_c/ for this computer, with clang: the program
# for Linux, step 6 of the portable C's road. clang is the one that the
# path has; without it they are not made, and their
# test is skipped. Every warning of the list below is an error, on the
# user's word of 14 October 2026: a list written out, close to curl's,
# and not -Weverything.
#   -std=c99 -Wpedantic   the C that the tree is written in, and no more
#   -Wall -Wextra         clang's usual sets
#   -Wconversion -Wsign-compare   a number put into fewer bits or met by
#                         one of another sign, which CLAUDE.md has the code
#                         say with a cast
#   -Wshadow -Wundef -Wcast-qual -Wpointer-arith -Wwrite-strings -Wvla
#   -Wstrict-prototypes -Wmissing-prototypes -Wmissing-variable-declarations
#   -Wformat=2 -Wconditional-uninitialized -Wcomma -Wassign-enum
#   -Wshift-sign-overflow -Wextra-semi-stmt -Wimplicit-fallthrough
#   -Wbad-function-cast -Wcast-function-type-strict -Wunreachable-code
#                         what curl and its like add; a fall-through of a
#                         switch is said by FALLTHROUGH of compiler.h
# Left out, each named in the survey the user decided from: -Wcast-align,
# since the common code reads words out of bytes as DOS does, which is
# the layer's (DIARY.md, "Write down the piece 5a, the split, and what 5b
# should know"); -Wpadded, -Wswitch-default and -Wswitch-enum,
# -Wdeclaration-after-statement, -Wdouble-promotion and
# -Wunsafe-buffer-usage, which say how C is to be written and not what is
# wrong in it.
set(clang_warnings -std=c99 -Werror -Wall -Wextra -Wpedantic -Wconversion
    -Wsign-compare -Wshadow -Wundef -Wcast-qual -Wpointer-arith
    -Wwrite-strings -Wvla -Wstrict-prototypes -Wmissing-prototypes
    -Wmissing-variable-declarations -Wformat=2 -Wconditional-uninitialized
    -Wcomma -Wassign-enum -Wshift-sign-overflow -Wextra-semi-stmt
    -Wimplicit-fallthrough -Wbad-function-cast -Wcast-function-type-strict
    -Wunreachable-code)
find_program(clang NAMES clang)
find_program(clang_tidy NAMES clang-tidy)
if(NOT clang)
    message(STATUS "No clang: the program for Linux is not built, and its "
                   "tests are skipped")
endif()

# add_portable_c_host(<target> <directory> <implementation> [BY_NAME]
#                     [COMPILE <option>...] [LINK <option>...])
# builds src/portable_c/ for this computer with clang, as
# <directory>/vc, of the common code and the files of <implementation>
# (posix), with the warnings above and the options given: those of
# COMPILE to the compiler and the linker alike, those of LINK to the
# linker alone. -fno-strict-aliasing is every such build's: the common
# code reads a word of 16 or 32 bits out of bytes through a cast pointer,
# as DOS does, which C promises nothing of without it (DIARY.md, "Write
# down the piece 5a, the split, and what 5b should know"). The command of
# the compiler is written into <directory>/compiled.txt, which every
# object follows, as add_portable_c has it, and which the test
# portable_c-tidy reads. With BY_NAME the target is not of the default
# one: it is built when it is named,
# `cmake --build build --target <target>`.
function(add_portable_c_host target directory implementation)
    cmake_parse_arguments(PARSE_ARGV 3 host "BY_NAME" "" "COMPILE;LINK")
    set(source_dir ${CMAKE_SOURCE_DIR}/src/portable_c)
    portable_c_sources(c_files ${implementation})
    set(compile ${clang} ${clang_warnings} -fno-strict-aliasing
        ${host_COMPILE} -I${source_dir})
    file(MAKE_DIRECTORY ${directory})
    file(GENERATE OUTPUT ${directory}/compiled.txt
         CONTENT "${compile}\n")
    set(objects "")
    foreach(source ${c_files})
        string(REGEX REPLACE "\\.c$" ".o" object ${source})
        add_custom_command(
            OUTPUT ${directory}/${object}
            COMMAND ${compile} -c -o ${directory}/${object}
                    ${source_dir}/${source}
            DEPENDS ${source_dir}/${source} ${source_dir}/vc.h
                    ${source_dir}/tables.h ${source_dir}/sys.h
                    ${source_dir}/compiler.h ${source_dir}/emulate_posix.h
                    ${directory}/compiled.txt
            COMMENT "Compiling portable_c/${source} (${target})"
            VERBATIM)
        list(APPEND objects ${directory}/${object})
    endforeach()
    add_custom_command(
        OUTPUT ${directory}/vc
        COMMAND ${compile} ${host_LINK} -o ${directory}/vc ${objects}
        DEPENDS ${objects}
        COMMENT "Linking the portable C (${target})"
        VERBATIM)
    if(host_BY_NAME)
        add_custom_target(${target} DEPENDS ${directory}/vc)
    else()
        add_custom_target(${target} ALL DEPENDS ${directory}/vc)
    endif()
endfunction()

# The trees.
#
# The C translation is compiled in the small model: its functions are far
# where a #pragma aux of theirs says so, which every function has that
# another file calls, with the registers that the assembly called it in.
# The files named are those of its second code segment.
#
# The portable C is compiled for 32 bits, in the flat model, with no
# file named but the one that is linked last: add_portable_c says which.
if(VC_DOS)
    add_c_tree(c "the C translation" s
        blank.c box.c cmdline.c commands.c dirfunc.c dos.c enter.c keys.c
        main.c memory.c menu.c misc.c mouse.c setup.c start.c system.c view.c
        window.c)
    # Of the portable C, two programs of every build:
    #   portable_c/VC.EXE         the program: -os, small code, and nothing
    #                             for a debugger. It is what is shipped and
    #                             what a test judges.
    #   portable_c/debug/VC.EXE   the same source for finding a fault, run by
    #                             hand: -od, as it is written, with nothing
    #                             moved or left out; -d2, so that its listings
    #                             have the lines of the source; and CHECKED
    #                             defined, for the words of the addresses to
    #                             look at what they are handed. Nothing reads
    #                             CHECKED yet: see DIARY.md, "Write down what
    #                             linking the program of 32 bits met, and
    #                             where it stops".
    add_portable_c(portable_c ${CMAKE_BINARY_DIR}/portable_c -os)
    add_portable_c(portable_c-debug ${CMAKE_BINARY_DIR}/portable_c/debug
        -od -d2 -dCHECKED)
endif()
# The program for Linux, step 6 of the road: src/portable_c/*_posix.c for
# sys.h, sys_posix.c with the emulation of DOS of the pieces L1 and L2
# behind it (DIARY.md, "Run the program on Linux, with a test channel and
# no terminal yet"). Three builds, each its own directory of
# build/portable_c/, all with -D_XOPEN_SOURCE=700, as the test of the
# emulation below has it:
#   posix/vc        the program, with the usual hardening of a program of
#                   Linux, the options that the distributions build with
#   posix-asan/vc   with the sanitizers of addresses, of undefined
#                   behaviour, of integers and of implicit conversions,
#                   each fatal, the alignment of what is read among them: a word in
#                   a buffer of bytes is made of its bytes, GET16 and its
#                   like of vc.h (DIARY.md, "Turn the sanitizer's check of
#                   alignment on in the build for Linux"); the one that the
#                   tests run
#   posix-msan/vc   with the sanitizer of memory, which goes with no other,
#                   for an audit by hand: not of the default target, built
#                   by its name, and run by no test (DIARY.md, "Take the
#                   program for Linux through the scenarios, with its
#                   clock, masks and not-run list", says how)
if(clang)
    add_portable_c_host(portable_c-posix ${CMAKE_BINARY_DIR}/portable_c/posix
        posix
        COMPILE -D_XOPEN_SOURCE=700 -O2 -D_FORTIFY_SOURCE=3
                -fstack-protector-strong -fstack-clash-protection
                -fcf-protection=full -fPIE
        LINK -pie -Wl,-z,relro,-z,now,-z,noexecstack)
    add_portable_c_host(portable_c-posix-asan
        ${CMAKE_BINARY_DIR}/portable_c/posix-asan posix
        COMPILE -D_XOPEN_SOURCE=700 -O1 -g -fno-omit-frame-pointer
                -fsanitize=address,undefined,integer,implicit-conversion,float-divide-by-zero
                -fno-sanitize-recover=all)
    add_portable_c_host(portable_c-posix-msan
        ${CMAKE_BINARY_DIR}/portable_c/posix-msan posix BY_NAME
        COMPILE -D_XOPEN_SOURCE=700 -O1 -g -fno-omit-frame-pointer
                -fsanitize=memory -fsanitize-memory-track-origins
                -fno-sanitize-recover=all)
endif()

# What the program for Linux emulates of DOS without input or output of
# its own, the files src/portable_c/*_posix.c of the piece L1 of step 6
# (DIARY.md, "Write down the piece L1 of step 6, and what is open of
# it"), and its test, harness/posixcheck.c, built together as
# build/portable_c/posix-unit/check: with the warnings above, the
# sanitizers of addresses, of undefined behaviour, of integers and of
# implicit conversions, each fatal, and -D_XOPEN_SOURCE=700, which gives
# the files of Linux the functions of POSIX and X/Open beside C's (a
# #define in a file is a name that C reserves, which clang-tidy names).
# Its compiled.txt is what the test portable_c-tidy reads these files
# with.
if(clang)
    set(unit_dir ${CMAKE_BINARY_DIR}/portable_c/posix-unit)
    set(unit_source ${CMAKE_SOURCE_DIR}/src/portable_c)
    file(GLOB unit_files CONFIGURE_DEPENDS ${unit_source}/*_posix.c)
    # Of the files of Linux, those of the emulation: sys_posix.c is the
    # program's, with its main, and machine_posix.c the part of it that
    # the web page shares, which reads the program's main data; the
    # program's two builds above have them.
    list(FILTER unit_files EXCLUDE REGEX "/(sys|machine)_posix\\.c$")
    list(APPEND unit_files ${CMAKE_SOURCE_DIR}/harness/posixcheck.c)
    set(unit_compile ${clang} ${clang_warnings} -fno-strict-aliasing
        -D_XOPEN_SOURCE=700 -O1 -g -fno-omit-frame-pointer
        -fsanitize=address,undefined,integer,implicit-conversion,float-divide-by-zero
        -fno-sanitize-recover=all -I${unit_source})
    file(MAKE_DIRECTORY ${unit_dir})
    file(GENERATE OUTPUT ${unit_dir}/compiled.txt
         CONTENT "${unit_compile}\n")
    set(unit_objects "")
    foreach(source ${unit_files})
        get_filename_component(name ${source} NAME_WE)
        add_custom_command(
            OUTPUT ${unit_dir}/${name}.o
            COMMAND ${unit_compile} -c -o ${unit_dir}/${name}.o ${source}
            DEPENDS ${source} ${unit_source}/emulate_posix.h
                    ${unit_source}/vc.h ${unit_source}/tables.h
                    ${unit_source}/sys.h ${unit_source}/compiler.h
                    ${unit_dir}/compiled.txt
            COMMENT "Compiling ${name}.c (portable_c-posix-unit)"
            VERBATIM)
        list(APPEND unit_objects ${unit_dir}/${name}.o)
    endforeach()
    add_custom_command(
        OUTPUT ${unit_dir}/check
        COMMAND ${unit_compile} -o ${unit_dir}/check ${unit_objects}
        DEPENDS ${unit_objects}
        COMMENT "Linking the test of the emulation of DOS for Linux"
        VERBATIM)
    add_custom_target(portable_c-posix-unit-check ALL
        DEPENDS ${unit_dir}/check)
endif()

# --- Emscripten ---------------------------------------------------------------
# The compiler of the web page, step 7 of the road, with VC_WEB alone: the
# portable C built to WebAssembly by Emscripten, which CLAUDE.md allows as
# that step's download. The release of 6.0.12, the newest on 15 October
# 2026, as emsdk fetches it, the archive of its LLVM, Binaryen and
# Emscripten for Linux on x86-64 at the commit that emsdk's
# emscripten-releases-tags.json names for that version, pinned by its
# SHA-256; none of emsdk's scripts is run. Its licences are MIT and the
# University of Illinois/NCSA (Emscripten), Apache-2.0 with the LLVM
# exception (LLVM), Apache-2.0 (Binaryen) and MIT (musl). The
# configuration that emcc reads, which emsdk would write, is written here,
# build/emscripten/config: its LLVM and Binaryen are the archive's, its
# Node the one that the path has, and its cache, where emcc keeps the
# libraries that it builds of its sources at first use, build/emscripten/
# cache. `emcc` below is the command that runs it so.
if(VC_WEB)
    if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux"
       OR NOT CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
        message(FATAL_ERROR "The build for the web fetches Emscripten "
                            "for Linux on x86-64 alone")
    endif()
    FetchContent_Declare(emscripten
        URL https://storage.googleapis.com/webassembly/emscripten-releases-builds/linux/6e9f6885cb17a3d6c8fd488a5cff730f50aba61d/wasm-binaries.tar.xz
        URL_HASH SHA256=7b073a102b07eca5a0fe344037681e53162639f3540432456874c0446a5e8fa7)
    FetchContent_MakeAvailable(emscripten)
    find_program(node NAMES node nodejs)
    if(NOT node)
        message(FATAL_ERROR "The build for the web needs Node.js, which "
                            "Emscripten runs")
    endif()
    set(emscripten_dir ${CMAKE_BINARY_DIR}/emscripten)
    file(MAKE_DIRECTORY ${emscripten_dir}/cache)
    file(WRITE ${emscripten_dir}/config
         "LLVM_ROOT = '${emscripten_SOURCE_DIR}/bin'\n"
         "BINARYEN_ROOT = '${emscripten_SOURCE_DIR}'\n"
         "NODE_JS = '${node}'\n"
         "CACHE = '${emscripten_dir}/cache'\n")
    set(emcc ${emscripten_SOURCE_DIR}/emscripten/emcc
        --em-config ${emscripten_dir}/config)
endif()

# The web probe, the piece W0 of step 7: what the build for the web is to
# compile, held to the warnings above with -Werror, by emcc, for wasm32,
# where an int, a long and a pointer have 32 bits each and clang is
# Emscripten's, of another version than the one of the path. It is the
# common code and the files of the emulation that the web has of Linux's,
# every *_posix.c but sys_posix.c and term_posix.c, which are the
# terminal's and the shell's, and the web's own, *_web.c (TODO.md, "Step
# 7, the web page"). Nothing is linked: the test portable_c-web-probe
# compiles them by the command in compiled.txt, harness/webprobe.py.
if(VC_WEB)
    set(web_probe_dir ${CMAKE_BINARY_DIR}/portable_c/web-probe)
    file(MAKE_DIRECTORY ${web_probe_dir})
    file(GENERATE OUTPUT ${web_probe_dir}/compiled.txt
         CONTENT "${emcc};${clang_warnings};-fno-strict-aliasing;-D_XOPEN_SOURCE=700;-O2;-I${CMAKE_SOURCE_DIR}/src/portable_c\n")
endif()

# --- Tests --------------------------------------------------------------------
# The tests need python3. Those that run Volkov Commander do so under FreeDOS
# in QEMU (see harness/) and also need qemu-system-i386 and mtools. The
# original program and FreeDOS are downloaded on first use.

enable_testing()
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# The driver of the tests: a resident program that takes the original and
# the C trees through the scenarios, in the machine. See
# harness/DRIVER.ASM.
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/DRIVER.COM
    COMMAND ${CMAKE_COMMAND} -E env --unset=JWASM
            $<TARGET_FILE:jwasm> -nologo -q -bin
            -Fo ${CMAKE_BINARY_DIR}/DRIVER.COM
            ${CMAKE_SOURCE_DIR}/harness/DRIVER.ASM
    DEPENDS jwasm ${CMAKE_SOURCE_DIR}/harness/DRIVER.ASM
    COMMENT "Assembling the driver of the tests"
    VERBATIM)
add_custom_target(driver ALL DEPENDS ${CMAKE_BINARY_DIR}/DRIVER.COM)

add_test(NAME asm-bytes
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/bytecheck.py
            ${layout_dir}/VC.COM ${layout_dir}/VCSETUP.COM)
# The C trees have the same tests, named by the tree they judge.
#
# The texts and tables of a tree against the original's, by their names:
# see harness/datacheck.py. Of the portable C, a flat program, field by
# field where an object has a word that names something: that is a
# pointer of four bytes there.
if(VC_DOS)
    foreach(tree c portable_c)
        add_test(NAME ${tree}-data
            COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/datacheck.py
                    --quiet --source ${CMAKE_SOURCE_DIR}/src/${tree}
                    ${layout_dir} ${CMAKE_BINARY_DIR}/${tree})
    endforeach()
    # The two routines that the compiler for 16 bits calls for arithmetic on
    # 32 bits, against Python's: see harness/arithcheck.py. No scenario
    # divides by more than 16 bits. The portable C has neither: its compiler
    # multiplies and divides such numbers by itself.
    add_test(NAME c-arith
        COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/arithcheck.py
                ${CMAKE_BINARY_DIR}/c)
    # A tree through every scenario, against the goldens of the original with
    # what is declared for the tree made to them, and what ran of it: see
    # harness/ported.py. The program for finding a fault, in debug, is run by
    # hand and is in no test of the machine.
    foreach(tree c portable_c)
        set(c_dir ${CMAKE_BINARY_DIR}/${tree})
        add_test(NAME ${tree}-scenarios
            COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/ported.py
                    ${c_dir}/VC.EXE ${c_dir}/VC.MAP ${layout_dir}/VC.LST)
        # That test runs every scenario, and on a machine that other tests
        # share it takes many times as long as by itself: it gets an hour
        # before it counts as stuck, where CTest would give it 25 minutes.
        set_tests_properties(${tree}-scenarios PROPERTIES TIMEOUT 3600)
    endforeach()
endif()
# The source of the portable C against a compiler of this computer, gcc,
# for a machine of 64 bits: what it refuses, counted by kind and held to
# what was counted last. See harness/hostcheck.py. It builds nothing and
# runs no machine, and is skipped where there is no gcc.
add_test(NAME portable_c-host
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/hostcheck.py)
set_tests_properties(portable_c-host PROPERTIES SKIP_RETURN_CODE 77)
# The arithmetic of the portable C that an int of 32 bits can tell from
# one of 16: every place where a number that the two builds may have
# otherwise is compared, divided, shifted, made wider or added to a
# pointer, held to a record that has the reason why each is the same in
# both. See harness/int16.py. It builds nothing and runs no machine, and
# is skipped where there is no clang.
add_test(NAME portable_c-int16
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/int16.py)
set_tests_properties(portable_c-int16 PROPERTIES SKIP_RETURN_CODE 77)
# The words by which the portable C says where a thing is in the memory,
# which stand for what a segment and an offset were on DOS, counted file
# by file and held to what was counted last: the measure of the work
# that takes them out. See harness/words.py. It reads the source and
# builds nothing. It also fails for a line that a merge's conflict has
# left in any file that git tracks, but for src/asm and the goldens:
# harness/conflicts.py.
add_test(NAME portable_c-words
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/words.py)
# The numbers of 8 and 16 bits of the portable C, which step 2 of the
# road widens, makes booleans or keeps with a reason: every one that a
# file declares, held to a record that has the reason of each in what
# has been sorted and the counts of the rest; and every file that is
# sorted held to clang's warnings of a conversion, so that a number
# which meets one of another sign or is put into fewer bits has a cast
# there. See harness/width.py. It asks clang, as the
# test of the int does, builds nothing and runs no machine, and is
# skipped where there is no clang.
add_test(NAME portable_c-width
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/width.py)
set_tests_properties(portable_c-width PROPERTIES SKIP_RETURN_CODE 77)
# A cut that Open Watcom's compiler loses, and those that it keeps. A byte
# with a sign made a number of 16 bits without one is extended with its
# sign to all 32 bits wherever it goes on into 32 bits, unless a short
# stands between: the test reads the tree through clang and fails for the
# form that is lost, anywhere, and it compiles harness/cutcheck.c with
# the compiler and the options that the build compiles the tree with and
# holds every function of it to the instructions written there, the
# fault's and those of every cast that narrows. See harness/cut.py. It
# runs no machine; without clang the tree is not read, and the test
# counts as skipped if the compiler's half has passed.
if(VC_DOS)
    add_test(NAME portable_c-cut
        COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/cut.py
                --compiled ${CMAKE_BINARY_DIR}/portable_c/compiled.txt
                --out ${CMAKE_BINARY_DIR}/cutcheck)
    set_tests_properties(portable_c-cut PROPERTIES SKIP_RETURN_CODE 77)
endif()
# The program for Linux held to clang's warnings, with the commands that
# the build compiles it with, and to clang-tidy, by .clang-tidy at the top
# of the checkout, with clang-tidy 23: every place that it names fails the
# test, but those that a piece of their own is to mend, which the test
# lists. The common code and sys_posix.c are read with the command of
# posix/vc, the files of the emulation and their test,
# harness/posixcheck.c, with that of the build of that test. It held the
# stub of step 5 so until the stub went (DIARY.md, "Take the stub out, and
# hold the program for Linux to clang-tidy in its place (L7)"). See
# harness/tidycheck.py. It runs no machine, and is skipped where there is
# no clang.
if(clang_tidy)
    set(host_tidy ${clang_tidy})
else()
    set(host_tidy clang-tidy)
endif()
add_test(NAME portable_c-tidy
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/tidycheck.py
            --compiled ${CMAKE_BINARY_DIR}/portable_c/posix/compiled.txt
            --posix-compiled
            ${CMAKE_BINARY_DIR}/portable_c/posix-unit/compiled.txt
            --clang-tidy ${host_tidy})
set_tests_properties(portable_c-tidy PROPERTIES SKIP_RETURN_CODE 77)
# What the program for Linux emulates of DOS without input or output, the
# piece L1 of step 6, held by harness/posixcheck.c, built above with the
# sanitizers: it prints every check that fails. It runs no machine, and is
# skipped where there is no clang.
if(clang)
    add_test(NAME portable_c-posix-unit
        COMMAND ${CMAKE_BINARY_DIR}/portable_c/posix-unit/check)
else()
    add_test(NAME portable_c-posix-unit
        COMMAND Python3::Interpreter -c
                "print('No clang: the test of the emulation of DOS for Linux is skipped.'); raise SystemExit(77)")
endif()
set_tests_properties(portable_c-posix-unit PROPERTIES SKIP_RETURN_CODE 77)
# The program for Linux in a terminal, the piece L4 of step 6: the
# sanitized build run in a pseudo-terminal with a few keys and the mouse,
# what it draws replayed through pyte, fetched pinned (harness/deps.py),
# and held cell by cell to the grid that the test channel writes for the
# same steps, in 24 bits and in 16 colours, with a terminal too small,
# SIGTSTP, SIGCONT and SIGTERM; and with the kitty protocol of the
# keyboard, and the keys that the program reads held to the BIOS's words
# (L5). See harness/terminal.py. It runs no machine, and is skipped
# where there is no clang.
add_test(NAME portable_c-terminal
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/terminal.py
            ${CMAKE_BINARY_DIR}/portable_c/posix-asan/vc)
set_tests_properties(portable_c-terminal PROPERTIES SKIP_RETURN_CODE 77)
# The modules of the web page, the piece W3a of step 7: render.js, which
# draws the program's grid as text, and keys.js and mouse.js, which make
# the browser's events the escape codes of a terminal with the kitty
# protocol, run in Node with the grids that the program for Linux writes
# through its test channel, the HTML read back and held to the cells, and
# their keys and reports typed into that program in a pseudo-terminal and
# held to what it reads. See harness/webrender.py. Nothing is downloaded
# for it, and it runs no machine; it is skipped where there is no clang or
# no Node.
add_test(NAME portable_c-web-render
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/webrender.py
            ${CMAKE_BINARY_DIR}/portable_c/posix-asan/vc)
set_tests_properties(portable_c-web-render PROPERTIES SKIP_RETURN_CODE 77)
# The program for Linux through the scenarios, the piece L3b of step 6:
# the sanitized build run on a bench of the host's laid out as the DOS
# bench's, its screens and its disks held to the goldens of the original
# with what is declared for the portable C and, on top of that, for Linux
# (ON_LINUX of harness/deviations.py: the scenarios not run, those taken
# as far as a step, the masks; its own patches in harness/goldens/posix/).
# See harness/posixrun.py. The bench's setup file is made on a machine
# of DOS the first time, and kept in build/posix-bench. Skipped where
# there is no clang.
if(clang)
    add_test(NAME portable_c-posix-scenarios
        COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/compare.py
                ${CMAKE_BINARY_DIR}/portable_c/posix-asan/vc)
else()
    add_test(NAME portable_c-posix-scenarios
        COMMAND Python3::Interpreter -c
                "print('No clang: the scenarios of the program for Linux are skipped.'); raise SystemExit(77)")
endif()
set_tests_properties(portable_c-posix-scenarios PROPERTIES
    SKIP_RETURN_CODE 77 LABELS "portable_c;machine")
# The harness's reader of a flat program of 32 bits, its file and the
# linker's map of it, against a toy that the test builds with the
# compiler, the linker and the stub of the Open Watcom fetched above: see
# harness/lecheck.py. It runs no machine.
if(VC_DOS)
    add_test(NAME harness-le
        COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/lecheck.py
                --watcom ${open_watcom_SOURCE_DIR} --tools ${watcom_bin}
                --out ${CMAKE_BINARY_DIR}/lecheck)
    # The two programs of the portable C, each against the layout that it
    # rests on: where the linker has put the tables, the main data, the stack
    # and what has no value, by the file and the linker's map of it, and that
    # its stub is DOS/32A's with the banner switched off. See
    # harness/layoutcheck.py. It runs no machine.
    foreach(tree portable_c portable_c-debug)
        # Each under the name of its target, and the second in the directory
        # debug of the first.
        string(REPLACE "-" "/" c_dir ${tree})
        set(c_dir ${CMAKE_BINARY_DIR}/${c_dir})
        add_test(NAME ${tree}-layout
            COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/layoutcheck.py
                    --stub ${open_watcom_SOURCE_DIR}/binw/dos32a.exe
                    ${c_dir}/VC.EXE ${c_dir}/VC.MAP)
    endforeach()
endif()
# The web probe of the piece W0 of step 7, with VC_WEB alone: every file
# that the build for the web is to compile, compiled by emcc with the
# warnings of the program for Linux and -Werror, each failure printed.
# See harness/webprobe.py. It runs no machine.
if(VC_WEB)
    add_test(NAME portable_c-web-probe
        COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/webprobe.py
                --compiled ${web_probe_dir}/compiled.txt
                --out ${web_probe_dir})
    set_tests_properties(portable_c-web-probe PROPERTIES LABELS portable_c)
endif()
# What each test judges, as a label, so that a run can pick by it: `ctest
# -L portable_c` for a change to the portable C alone, which cannot
# change what the frozen trees are judged by ("Work by several agents"
# in CLAUDE.md); `-LE machine` for the tests that start no machine.
set_tests_properties(asm-bytes PROPERTIES LABELS frozen)
set_tests_properties(portable_c-host portable_c-int16
    portable_c-words portable_c-width portable_c-tidy
    portable_c-posix-unit portable_c-terminal portable_c-web-render
    PROPERTIES LABELS portable_c)
if(VC_DOS)
    set_tests_properties(c-data c-arith PROPERTIES LABELS frozen)
    set_tests_properties(portable_c-data portable_c-cut
        portable_c-layout portable_c-debug-layout
        PROPERTIES LABELS portable_c)
    set_tests_properties(portable_c-scenarios PROPERTIES
        LABELS "portable_c;machine")
    set_tests_properties(c-scenarios PROPERTIES LABELS "frozen;machine")
    set_tests_properties(harness-le PROPERTIES LABELS harness)
endif()
