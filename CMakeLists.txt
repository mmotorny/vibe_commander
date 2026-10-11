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

# --- Volkov Commander ---------------------------------------------------------

# add_dos_program(<name> <include>...) assembles src/<name>.ASM, which
# includes the listed files, into <build>/<name>.COM.
function(add_dos_program name)
    set(source_dir ${CMAKE_SOURCE_DIR}/src)
    set(output ${CMAKE_BINARY_DIR}/${name}.COM)
    list(TRANSFORM ARGN PREPEND ${source_dir}/ OUTPUT_VARIABLE includes)
    add_custom_command(
        OUTPUT ${output}
        # JWasm takes default options from the JWASM environment variable.
        COMMAND ${CMAKE_COMMAND} -E env --unset=JWASM
                $<TARGET_FILE:jwasm> -nologo -q -bin -I${source_dir}
                -Fo ${output} ${source_dir}/${name}.ASM
        DEPENDS jwasm ${source_dir}/${name}.ASM ${includes}
        COMMENT "Assembling ${name}.COM"
        VERBATIM)
    add_custom_target(${name} ALL DEPENDS ${output})
endfunction()

add_dos_program(VC
    VCFUNC.INC VCMENU.INC VCF1.INC VCCOPY.INC VCVIEW.INC VCEDIT.INC
    VCHELP.INC VCMEM.INC VCFIND.INC VCSUB1.INC VCSUB2.INC VCSUB3.INC)
add_dos_program(VCSETUP
    VCSTHLP.INC VCSTSUB.INC)

# --- Tests --------------------------------------------------------------------
# The tests run Volkov Commander under FreeDOS in QEMU; see harness/. They
# need qemu-system-i386 and mtools, and download the original program and
# FreeDOS on first use.

enable_testing()
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_test(NAME harness-self-test
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/selftest.py)
add_test(NAME start-screen
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/harness/compare.py
            ${CMAKE_BINARY_DIR}/VC.COM)
