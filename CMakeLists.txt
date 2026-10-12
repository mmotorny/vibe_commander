cmake_minimum_required(VERSION 3.24)
project(vibe_commander LANGUAGES C)

# --- JWasm ------------------------------------------------------------------
# The assembler that builds src/: a free, MASM-compatible assembler that runs
# natively and writes .COM files directly. It is fetched at a pinned commit
# and built from source.

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
# The C compiler and the linker that build the port: an open-source compiler
# for 16-bit DOS, among other targets. It takes hours to build from source,
# so the project's own snapshot of 1 October 2026 is fetched, pinned by its
# SHA-256, and the tools that run on this computer are used from it.

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
# src/VC.ASM and src/VCSETUP.ASM into <directory>/VC.COM and
# <directory>/VCSETUP.COM, with listings next to them.
function(add_programs target directory)
    set(source_dir ${CMAKE_SOURCE_DIR}/src)
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
# it with the originals byte for byte. See src/VCLAYOUT.INC.
set(layout_dir ${CMAKE_BINARY_DIR}/shareware-layout)
add_programs(shareware-layout ${layout_dir} -DSHAREWARE_LAYOUT)

# --- The port -----------------------------------------------------------------
# Volkov Commander on its way from assembly to C: port/asm holds what is
# still assembly, port/*.c what has been translated, and both are linked
# into one program, VC.EXE. See port/vc.h and tools/mkexe.c for how the two
# halves live together.

set(port_dir ${CMAKE_BINARY_DIR}/port)
set(port_asm VC.ASM
    VCFUNC.INC VCMENU.INC VCF1.INC VCCOPY.INC VCVIEW.INC VCEDIT.INC
    VCHELP.INC VCMEM.INC VCFIND.INC VCSUB1.INC VCSUB2.INC VCSUB3.INC)
# Every C file in port/ is part of the program, in the order of their
# names: a new one needs no mention here. The build looks again each time
# it runs.
file(GLOB port_c CONFIGURE_DEPENDS RELATIVE ${CMAKE_SOURCE_DIR}/port
     ${CMAKE_SOURCE_DIR}/port/*.c)
# The compiled code is more than one segment holds, 64K, so it has two:
# CTEXT, and CTEXT2 for the files named here. Functions of different files
# call each other far, so a file may be in either. The routines that the
# compiler calls near for arithmetic on 32 bits, __U4M and __U4D at the end
# of port/asm/VC.ASM, are in CTEXT only: a file that needs them stays
# there, or the linker says that it cannot reach them. When the linker
# says that CTEXT exceeds 64K, name another file here.
set(port_c_second keys.c)
list(TRANSFORM port_asm PREPEND ${CMAKE_SOURCE_DIR}/port/asm/)

# mkexe turns the linker's output into the program that is run.
add_executable(mkexe tools/mkexe.c)
set_target_properties(mkexe PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools)

# The compiled code, which every build of the port shares.
file(MAKE_DIRECTORY ${port_dir})
set(port_compiled "")
foreach(source ${port_c})
    string(REGEX REPLACE "\\.c$" ".obj" object ${port_dir}/${source})
    string(REGEX REPLACE "\\.c$" ".lst" listing ${port_dir}/${source})
    # -0     code for the 8086, like the original
    # -ms    near code and data; the compiled code shares one segment
    # -os    favour small code
    # -s     no checks for stack overflow, which need the C library
    # -zu    the stack is not in the data segment
    # -zl    name no libraries to link: the program has none
    # -zls   no reference to the C library's start-up code either
    # -wx -we   all warnings, as errors
    # -nt, -nc  the name and class of the code segment: CTEXT, or CTEXT2
    #           for a file of port_c_second
    #
    # What the compiler made of it is written next to the object, as a
    # listing: that is where to read which registers a function saves and
    # how much stack it takes.
    set(text CTEXT)
    if(source IN_LIST port_c_second)
        set(text CTEXT2)
    endif()
    add_custom_command(
        OUTPUT ${object}
        COMMAND ${watcom} ${watcom_bin}/wcc -q -0 -ms -os -s -zu -zl -zls
                -wx -we -nt=${text} -nc=CCODE -fo=${object}
                ${CMAKE_SOURCE_DIR}/port/${source}
        COMMAND ${watcom} ${watcom_bin}/wdis -l=${listing} ${object}
        BYPRODUCTS ${listing}
        DEPENDS ${CMAKE_SOURCE_DIR}/port/${source}
                ${CMAKE_SOURCE_DIR}/port/vc.h
        COMMENT "Compiling ${source}"
        VERBATIM)
    list(APPEND port_compiled ${object})
endforeach()

# add_port(<target> <directory> [<jwasm option>...]) assembles port/asm and
# links it with the compiled code into <directory>/VC.EXE, with a link map
# next to it.
function(add_port target directory)
    file(MAKE_DIRECTORY ${directory})
    add_custom_command(
        OUTPUT ${directory}/VC.OBJ
        COMMAND ${CMAKE_COMMAND} -E env --unset=JWASM
                $<TARGET_FILE:jwasm> -nologo -q -omf ${ARGN}
                -I${CMAKE_SOURCE_DIR}/port/asm -Fo ${directory}/VC.OBJ
                ${CMAKE_SOURCE_DIR}/port/asm/VC.ASM
        DEPENDS jwasm ${port_asm}
        COMMENT "Assembling the port (${target})"
        VERBATIM)
    # The assembly comes first: the program starts in it and expects the
    # compiled code behind it. Warning 1014 says that there is no stack
    # segment: the program sets up its stack itself.
    list(JOIN port_compiled "," compiled)
    add_custom_command(
        OUTPUT ${directory}/VC.EXE ${directory}/VC.MAP
        COMMAND ${watcom} ${watcom_bin}/wlink option quiet format dos
                name ${directory}/linked.exe
                file ${directory}/VC.OBJ,${compiled}
                option map=${directory}/VC.MAP disable 1014
        COMMAND mkexe ${directory}/linked.exe ${directory}/VC.EXE
                ${port_compiled}
        DEPENDS mkexe ${directory}/VC.OBJ ${port_compiled}
        COMMENT "Linking the port (${target})"
        VERBATIM)
    add_custom_target(${target} ALL DEPENDS ${directory}/VC.EXE)
endfunction()

# The program as it is shipped.
add_port(port ${port_dir})

# The program as the tests run it: it signals where it is, so that they
# need not guess. See "Signals to the tests" in port/asm/VC.ASM.
set(signalling_dir ${port_dir}/signalling)
add_port(port-signalling ${signalling_dir} -DHARNESS)

# --- Tests --------------------------------------------------------------------
# The tests need python3. Those that run Volkov Commander do so under FreeDOS
# in QEMU (see harness/) and also need qemu-system-i386 and mtools. The
# original program and FreeDOS are downloaded on first use.

enable_testing()
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_test(NAME bytes-match-original
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/bytecheck.py
            ${layout_dir}/VC.COM ${layout_dir}/VCSETUP.COM)
add_test(NAME port
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/ported.py
            ${signalling_dir}/VC.EXE ${signalling_dir}/VC.MAP
            ${layout_dir}/VC.LST ${port_dir}/VC.EXE)
# The port's test runs every scenario, and on a machine that other tests
# share it takes many times as long as by itself: it gets an hour before
# it counts as stuck, where CTest would give it 25 minutes.
set_tests_properties(port PROPERTIES TIMEOUT 3600)
