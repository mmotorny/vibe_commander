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
# The linker that builds the port, and the C compiler that will: an
# open-source compiler for 16-bit DOS, among other targets. It takes hours to
# build from source, so the project's own snapshot of 1 October 2026 is
# fetched, pinned by its SHA-256, and the tools that run on this computer are
# used from it.

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
# <directory>/VCSETUP.COM.
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
                    -I${source_dir} -Fo ${output} ${source_dir}/${name}.ASM
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
# Volkov Commander on its way from assembly to C. So far it is all assembly,
# in port/asm, but it is linked the way it will be with compiled code next
# to it, into VC.EXE. See tools/mkexe.c for how.

set(port_dir ${CMAKE_BINARY_DIR}/port)
set(port_asm VC.ASM
    VCFUNC.INC VCMENU.INC VCF1.INC VCCOPY.INC VCVIEW.INC VCEDIT.INC
    VCHELP.INC VCMEM.INC VCFIND.INC VCSUB1.INC VCSUB2.INC VCSUB3.INC)
list(TRANSFORM port_asm PREPEND ${CMAKE_SOURCE_DIR}/port/asm/)

# mkexe turns the linker's output into the program that is run.
add_executable(mkexe tools/mkexe.c)
set_target_properties(mkexe PROPERTIES
    C_STANDARD 99
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools)

file(MAKE_DIRECTORY ${port_dir})
add_custom_command(
    OUTPUT ${port_dir}/VC.OBJ
    COMMAND ${CMAKE_COMMAND} -E env --unset=JWASM
            $<TARGET_FILE:jwasm> -nologo -q -omf
            -I${CMAKE_SOURCE_DIR}/port/asm -Fo ${port_dir}/VC.OBJ
            ${CMAKE_SOURCE_DIR}/port/asm/VC.ASM
    DEPENDS jwasm ${port_asm}
    COMMENT "Assembling the port"
    VERBATIM)

# Warning 1014 says that there is no stack segment: the program sets up its
# stack itself.
add_custom_command(
    OUTPUT ${port_dir}/VC.EXE
    COMMAND ${watcom} ${watcom_bin}/wlink option quiet format dos
            name ${port_dir}/linked.exe file ${port_dir}/VC.OBJ
            disable 1014
    COMMAND mkexe ${port_dir}/linked.exe ${port_dir}/VC.EXE
    DEPENDS mkexe ${port_dir}/VC.OBJ
    COMMENT "Linking the port"
    VERBATIM)
add_custom_target(port ALL DEPENDS ${port_dir}/VC.EXE)

# --- Tests --------------------------------------------------------------------
# The tests need python3. Those that run Volkov Commander do so under FreeDOS
# in QEMU (see harness/) and also need qemu-system-i386 and mtools. The
# original program and FreeDOS are downloaded on first use.

enable_testing()
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_test(NAME bytes-match-original
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/bytecheck.py
            ${layout_dir}/VC.COM ${layout_dir}/VCSETUP.COM)
add_test(NAME harness-self-test
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/selftest.py)
add_test(NAME start-screen
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/compare.py
            ${CMAKE_BINARY_DIR}/VC.COM)
add_test(NAME start-screen-shareware-layout
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/compare.py
            ${layout_dir}/VC.COM)
add_test(NAME port
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/compare.py
            ${port_dir}/VC.EXE all)
