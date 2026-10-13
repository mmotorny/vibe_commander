# Compile a file that is listed as not ready for it, and fail if the
# compiler takes the file: its name is then to come off the list. The
# build runs this for each file that add_portable_c32 of CMakeLists.txt
# has in its list of what does not compile for 32 bits yet.
#
#   cmake -D command=<the compiler with its options, parted by |>
#         -D source=<the file> -D object=<the object file to try for>
#         -D said=<where to write what the compiler said>
#         -P tools/refused.cmake
#
# What the compiler said is kept, so that the reason on the list can be
# held against it: a file may come to be refused for something else than
# the list says, and this does not notice.

string(REPLACE "|" ";" command "${command}")
execute_process(
    COMMAND ${command} -fo=${object} ${source}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output)
file(REMOVE ${object})
if(NOT result MATCHES "^[0-9]+$")
    message(FATAL_ERROR "The compiler did not run for ${source}: ${result}")
endif()
if(result EQUAL 0)
    message(FATAL_ERROR
        "${source} compiles for 32 bits now: take it off the list of what "
        "does not, in add_portable_c32 of CMakeLists.txt")
endif()
file(WRITE ${said} "${output}")
