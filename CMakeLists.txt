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

# --- Open Watcom --------------------------------------------------------------
# The C compiler and the linker that build the C trees: an open-source
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
# The test of a C tree's data goes by them (harness/datacheck.py).
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
# program, which the linker writes: add_portable_c, further down.

# mkexe turns the linker's output into the program that is run.
add_executable(mkexe tools/mkexe.c)
set_target_properties(mkexe PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools)

# c_compile(<tree> <model> <source> <object> <segment> <class>
# [<option>...]) compiles a file of src/<tree>/ into
# build/<tree>/<object>.obj, with a listing next to it.
function(c_compile tree model source object text class)
    set(source_dir ${CMAKE_SOURCE_DIR}/src/${tree})
    set(c_dir ${CMAKE_BINARY_DIR}/${tree})
    # -0     code for the 8086, like the original
    # -ms    the model that the tree is compiled in, which its caller
    # -mm    names: near data in both, and a function near with -ms
    #        unless it says otherwise, far with -mm unless it is static
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
    # vc.h and tables.h, which both trees have, and sys.h, which the
    # portable C has and vc.h includes there.
    set(headers "")
    if(EXISTS ${source_dir}/sys.h)
        set(headers ${source_dir}/sys.h)
    endif()
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
                ${headers}
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
    # linker says that it cannot reach them. With the model m it calls
    # them far, from either segment. A segment holds F000h bytes, not 64K:
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

# add_portable_c(<file of the second segment>...) builds src/portable_c/
# into build/portable_c/VC.EXE, with the linker's map VC.MAP and the
# object files next to it, as the target portable_c.
#
# The portable C is one program, and the linker writes its file: DOS
# loads it, sets the words of it that hold a segment and starts it at
# Alloc0 of resident.c, on the stack that the file's header names. It is
# compiled in the medium model: a function is far by the compiler's own
# rule, and needs no word of this compiler to say so.
function(add_portable_c)
    set(tree portable_c)
    set(source_dir ${CMAKE_SOURCE_DIR}/src/${tree})
    set(c_dir ${CMAKE_BINARY_DIR}/${tree})
    # Every C file of the tree is part of the program: a new one needs no
    # mention here. The build looks again each time it runs.
    file(GLOB c_files CONFIGURE_DEPENDS RELATIVE ${source_dir}
         ${source_dir}/*.c)
    # The compiled code is more than one segment holds, so it has two:
    # CTEXT, and CTEXT2 for the files that the caller names. Functions of
    # different files call each other far, so a file may be in either. A
    # segment holds F000h bytes, not 64K, for the machine of the tests:
    # the test portable_c-scenarios says why, and fails when one is
    # fuller (check_layout of harness/ported.py). Name another file here
    # then.
    set(c_second ${ARGN})

    # The data of the program is three segments, because it is more than
    # one holds. The main data and the variables of the C files are the
    # compiler's own group, DGROUP, which DS is on. The tables, the files
    # whose names end in _t, and the stack, stack.c, do not fit there
    # beside the main data, and are each a group of its own, by an
    # option of Open Watcom that gives the data of a file one: -nd=T
    # makes T_GROUP of the tables and -nd=S makes S_GROUP of the stack
    # (see the README, "What rests on Open Watcom").
    file(MAKE_DIRECTORY ${c_dir})
    set(c_compiled "")
    foreach(source ${c_files})
        string(REGEX REPLACE "\\.c$" "" object ${source})
        if(source MATCHES "_t\\.c$")
            c_compile(${tree} m ${source} ${object} CTEXT CCODE -nd=T)
        elseif(source STREQUAL "stack.c")
            c_compile(${tree} m ${source} ${object} CTEXT CCODE -nd=S)
        elseif(source IN_LIST c_second)
            c_compile(${tree} m ${source} ${object} CTEXT2 CCODE)
        else()
            c_compile(${tree} m ${source} ${object} CTEXT CCODE)
        endif()
        list(APPEND c_compiled ${c_dir}/${object}.obj)
    endforeach()

    # The program is linked of the compiled code alone, with a link map
    # next to it. The order of the segments is said here, by their
    # classes: the main data, which main.c has in a segment and a class
    # of its own, MAIN, so that it is the first thing of DGROUP and of
    # the program, at offset 0 of the segment that DS is on, as the code
    # takes it to be; the rest of DGROUP; the code; the tables; and last
    # the stack, where the program ends, which is how it knows its
    # length (Alloc0 of resident.c). The tables stand between the code
    # and the stack: the machine of the tests is many times slower at a
    # write to a page of the memory that has code which it has run, and
    # a push is a write. The directive is Open Watcom's, and its help
    # does not list it for DOS.
    list(JOIN c_compiled "," compiled)
    add_custom_command(
        OUTPUT ${c_dir}/VC.EXE ${c_dir}/VC.MAP
        COMMAND ${watcom} ${watcom_bin}/wlink option quiet format dos
                name ${c_dir}/VC.EXE
                order clname MAIN clname DATA clname BSS
                      clname CCODE segment CTEXT segment CTEXT2
                      clname FAR_DATA clname STACK
                file ${compiled}
                option map=${c_dir}/VC.MAP,start=Alloc0_,static
        DEPENDS ${c_compiled}
        COMMENT "Linking the portable C"
        VERBATIM)
    # The program as it is shipped.
    add_custom_target(${tree} ALL DEPENDS ${c_dir}/VC.EXE)
endfunction()

# The trees, each with the files of its second code segment.
#
# The C translation is compiled in the small model: its functions are far
# where a #pragma aux of theirs says so, which every function has that
# another file calls, with the registers that the assembly called it in.
set(c_trees c portable_c)
add_c_tree(c "the C translation" s
    blank.c box.c cmdline.c commands.c dirfunc.c dos.c enter.c keys.c
    main.c memory.c menu.c misc.c mouse.c setup.c start.c system.c view.c
    window.c)
add_portable_c(
    blank.c box.c cmdline.c commands.c dirfunc.c dos.c enter.c keys.c
    main.c menu.c misc.c mouse.c setup.c start.c system.c view.c window.c)

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
# Each C tree has the same three tests, named by the tree they judge.
foreach(tree ${c_trees})
    set(c_dir ${CMAKE_BINARY_DIR}/${tree})
    # The texts and tables of the tree against the original's, by their
    # names: see harness/datacheck.py.
    add_test(NAME ${tree}-data
        COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/datacheck.py
                --quiet --source ${CMAKE_SOURCE_DIR}/src/${tree}
                ${layout_dir} ${c_dir})
    # The two routines that the compiler calls for arithmetic on 32 bits,
    # against Python's: see harness/arithcheck.py. No scenario divides by
    # more than 16 bits.
    add_test(NAME ${tree}-arith
        COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/arithcheck.py
                ${c_dir})
    add_test(NAME ${tree}-scenarios
        COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/ported.py
                ${c_dir}/VC.EXE ${c_dir}/VC.MAP ${layout_dir}/VC.LST)
    # That test runs every scenario, and on a machine that other tests
    # share it takes many times as long as by itself: it gets an hour
    # before it counts as stuck, where CTest would give it 25 minutes.
    set_tests_properties(${tree}-scenarios PROPERTIES TIMEOUT 3600)
endforeach()
# The source of the portable C against a compiler of this computer, gcc,
# for a machine of 64 bits: what it refuses, counted by kind and held to
# what was counted last. See harness/hostcheck.py. It builds nothing and
# runs no machine, and is skipped where there is no gcc.
add_test(NAME portable_c-host
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/hostcheck.py)
set_tests_properties(portable_c-host PROPERTIES SKIP_RETURN_CODE 77)
