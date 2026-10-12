cmake_minimum_required(VERSION 3.24)
project(vibe_commander LANGUAGES C)

# --- JWasm ------------------------------------------------------------------
# The assembler that builds src/asm/, the original program as its author
# published it, and the driver of the tests: a free, MASM-compatible
# assembler that runs natively and writes .COM files directly. It is
# fetched at a pinned commit and built from source. Nothing of the C
# translation is assembled: the tests hold the translation against the
# original, and hold the original, built here, against the binary that was
# shipped.

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

# --- Open Watcom --------------------------------------------------------------
# The C compiler and the linker that build the translation: an open-source
# compiler for 16-bit DOS, among other targets. It takes hours to build from
# source, so the project's own snapshot of 1 October 2026 is fetched, pinned
# by its SHA-256, and the tools that run on this computer are used from it.

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

# And VC once more with that layout, as an object file: its relocations say
# which words of the original hold the address of something, and of what.
# The test of the translation's data goes by them (harness/datacheck.py).
list(TRANSFORM VC_includes PREPEND ${CMAKE_SOURCE_DIR}/src/asm/
     OUTPUT_VARIABLE layout_includes)
add_custom_command(
    OUTPUT ${layout_dir}/VC.OBJ
    COMMAND ${CMAKE_COMMAND} -E make_directory ${layout_dir}
    COMMAND ${CMAKE_COMMAND} -E env --unset=JWASM
            $<TARGET_FILE:jwasm> -nologo -q -omf -DSHAREWARE_LAYOUT
            -I${CMAKE_SOURCE_DIR}/src/asm -Fo ${layout_dir}/VC.OBJ
            ${CMAKE_SOURCE_DIR}/src/asm/VC.ASM
    DEPENDS jwasm ${CMAKE_SOURCE_DIR}/src/asm/VC.ASM ${layout_includes}
    COMMENT "Assembling VC.OBJ (shareware-layout)"
    VERBATIM)
add_custom_target(shareware-relocations ALL DEPENDS ${layout_dir}/VC.OBJ)

# --- The C translation --------------------------------------------------------
# Volkov Commander in C: src/c/*.c, compiled and linked into one program,
# VC.EXE. Nothing of it is assembled: the assembler above builds the
# original, for the tests to hold the translation against, and the driver
# of the tests. See src/c/vc.h and tools/mkexe.c for how the program is
# laid out and what that asks of the C.

set(c_dir ${CMAKE_BINARY_DIR}/c)
# Every C file in src/c/ is part of the program, in the order of their
# names: a new one needs no mention here. The build looks again each time
# it runs. One whose name ends in _t holds constants and no code: the
# tables of the file it is named after (see src/c/tables.h).
file(GLOB c_files CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR}/src/c
     ${CMAKE_SOURCE_DIR}/src/c/*.c)
# Two files are linked before the others, whatever their names. The file
# is the program as it lies in the memory, and it begins with the part
# that stays there while a command runs: resident.c, whose code begins
# with the room of the PSP. A segment holds what the object files have of
# it in their order. And the part that goes begins with transit_t.c, the
# room of the header that DOS keeps in front of its block of memory.
list(REMOVE_ITEM c_files resident.c transit_t.c)
list(PREPEND c_files resident.c transit_t.c)
# The compiled code is more than one segment holds, so it has two: CTEXT,
# and CTEXT2 for the files named here. Functions of different files call
# each other far, so a file may be in either. The routines that the
# compiler calls near for arithmetic on 32 bits, __U4M and __U4D of
# src/c/arith.c, are in CTEXT only: a file that needs them stays
# there, or the linker says that it cannot reach them. A segment holds
# F000h bytes, not 64K: tools/mkexe.c says why, and stops the build when
# one is fuller. Name another file here then.
set(c_second blank.c box.c cmdline.c commands.c dirfunc.c dos.c enter.c
    keys.c main.c memory.c menu.c misc.c mouse.c setup.c start.c system.c
    view.c window.c)
# The part of the program that stays in the memory while a command runs
# has a code segment of its own, RTEXT, which is the first of the file:
# the files named here. That part cannot call the rest of the program,
# which is not in the memory then, so dos.c, the one file that calls DOS
# and the BIOS, is compiled a second time for it, into RTEXT as well: one
# source, two copies of its code. RESIDENT is defined for both, and by it
# src/c/vc.h gives the functions of the second copy names of their own.
#
# That part is one segment to the program, the PSP's, as the whole
# program was when it was a .COM file: CS, DS and SS are on the PSP there,
# and every offset counts from it. Two options of Open Watcom make it so,
# and nothing of the C language does (see the README, "What rests on Open
# Watcom"):
#   -nd=R        the data of a file is in a group of its own, R_GROUP,
#                which the code reads through DS, in four segments of
#                the class FAR_DATA: RCONST, RCONST2, R_DATA, R_BSS
#   -g=R_GROUP   the code segment is of a group of that name
# With both the object file names two groups of one name, and the linker
# makes one of them, which begins with the code and so with the PSP. The
# second copy of dos.c has no data and gets the second option alone.
set(c_resident resident.c)

# mkexe turns the linker's output into the program that is run.
add_executable(mkexe tools/mkexe.c)
set_target_properties(mkexe PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools)

# The compiled code, which every build of the translation shares.
file(MAKE_DIRECTORY ${c_dir})
set(c_compiled "")
# c_compile(<source> <object> <segment> <class> [<option>...]) compiles
# a file of src/c/ into <object>.obj, with a listing next to it.
function(c_compile source object text class)
    # -0     code for the 8086, like the original
    # -ms    near code and data; the compiled code shares one segment
    # -os    favour small code
    # -s     no checks for stack overflow, which need the C library
    # -zu    the stack is not in the data segment
    # -zl    name no libraries to link: the program has none
    # -zls   no reference to the C library's start-up code either
    # -wx -we   all warnings, as errors
    # -nt, -nc  the name and class of the code segment: CTEXT, CTEXT2
    #           for a file of c_second, RTEXT for the part that
    #           stays
    #
    # What the compiler made of it is written next to the object, as a
    # listing: that is where to read which registers a function saves and
    # how much stack it takes.
    add_custom_command(
        OUTPUT ${c_dir}/${object}.obj
        COMMAND ${watcom} ${watcom_bin}/wcc -q -0 -ms -os -s -zu -zl -zls
                -wx -we -nt=${text} -nc=${class} ${ARGN}
                -fo=${c_dir}/${object}.obj
                ${CMAKE_SOURCE_DIR}/src/c/${source}
        COMMAND ${watcom} ${watcom_bin}/wdis -l=${c_dir}/${object}.lst
                ${c_dir}/${object}.obj
        BYPRODUCTS ${c_dir}/${object}.lst
        DEPENDS ${CMAKE_SOURCE_DIR}/src/c/${source}
                ${CMAKE_SOURCE_DIR}/src/c/vc.h
                ${CMAKE_SOURCE_DIR}/src/c/tables.h
        COMMENT "Compiling ${source} (${text})"
        VERBATIM)
endfunction()

foreach(source ${c_files})
    string(REGEX REPLACE "\\.c$" "" object ${source})
    if(source IN_LIST c_resident)
        c_compile(${source} ${object} RTEXT RCODE -dRESIDENT -nd=R
                     -g=R_GROUP)
    elseif(source IN_LIST c_second)
        c_compile(${source} ${object} CTEXT2 CCODE)
    else()
        c_compile(${source} ${object} CTEXT CCODE)
    endif()
    list(APPEND c_compiled ${c_dir}/${object}.obj)
endforeach()
c_compile(dos.c dos_r RTEXT RCODE -dRESIDENT -g=R_GROUP)
list(APPEND c_compiled ${c_dir}/dos_r.obj)

# The program is linked of the compiled code alone, with a link map next
# to it, and tools/mkexe.c makes the file that is run of what the linker
# writes. It starts at Alloc0, a function of src/c/resident.c.
#
# The order of the segments is said here, by their classes and within a
# class by their names: the linker would have them as it first meets
# them in the object files, and every object file of C names its code
# segment first. The code of the part that stays, with the PSP's room;
# its data, which ends with the stack; the constants of the rest, which
# begin with the room of a block's header; and the code of the rest. No
# code of the rest may stand within a page of the stack: tools/mkexe.c
# says why, and stops the build if it does. The directive is Open
# Watcom's, and its help does not list it for DOS (see the README, "What
# rests on Open Watcom").
#
# Warning 1014 says that there is no stack segment: the stack is of the
# part that stays in the memory, and tools/mkexe.c names it in the header
# of the file.
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
    COMMENT "Linking the C translation"
    VERBATIM)
# The program as it is shipped.
add_custom_target(c ALL DEPENDS ${c_dir}/VC.EXE)

# --- Tests --------------------------------------------------------------------
# The tests need python3. Those that run Volkov Commander do so under FreeDOS
# in QEMU (see harness/) and also need qemu-system-i386 and mtools. The
# original program and FreeDOS are downloaded on first use.

enable_testing()
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# The driver of the tests: a resident program that takes the original and
# the translation through the scenarios, in the machine. See
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
# The texts and tables of the translation against the original's, by their
# names: see harness/datacheck.py.
add_test(NAME c-data
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/datacheck.py
            --quiet ${layout_dir} ${c_dir})
# The two routines that the compiler calls for arithmetic on 32 bits,
# against Python's: see harness/arithcheck.py. No scenario divides by
# more than 16 bits.
add_test(NAME c-arith
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/arithcheck.py
            ${c_dir})
add_test(NAME c-scenarios
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/ported.py
            ${c_dir}/VC.EXE ${c_dir}/VC.MAP ${layout_dir}/VC.LST)
# That test runs every scenario, and on a machine that other tests
# share it takes many times as long as by itself: it gets an hour before
# it counts as stuck, where CTest would give it 25 minutes.
set_tests_properties(c-scenarios PROPERTIES TIMEOUT 3600)
