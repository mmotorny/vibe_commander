#!/bin/sh
# Build a toy of the trial into build/trial32/ (see README.md):
#
#   build.sh NAME d32 [MORE.c ...]   NAME.c as NAME-d32.exe, with DOS/32A
#   build.sh NAME cw [MORE.c ...]    as NAME-cw.exe, with CauseWay
#   build.sh hello d32|cw            hello.c with Open Watcom's C library
#   build.sh NAME com                NAME.asm as NAME.com, for real mode
#   build.sh driver                  DRIVERX.ASM as DRIVERX.COM
#   build.sh sys 16                  sys.c as sys-16.exe, with the dos.c
#                                    of the portable C as the build made it
#   build.sh sys 32                  as sys-32.exe, with its dos32.c and
#                                    DOS/32A
#
# It needs the project built once, for the compiler and the assembler
# that the build fetches, and uses Open Watcom's tools for Linux on
# x86-64. A file of MORE.c is taken from here or from build/trial32/.
set -e
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)
W=$root/build/_deps/open_watcom-src
bin=$W/binl64
jwasm=$root/build/tools/jwasm
out=$root/build/trial32
export WATCOM=$W
mkdir -p "$out"

if [ "$1" = driver ]; then
    "$jwasm" -q -bin -Fo "$out/DRIVERX.COM" "$here/DRIVERX.ASM"
    ls -l "$out/DRIVERX.COM"
    exit
fi
name=$1; kind=$2; shift 2
if [ "$kind" = com ]; then
    "$jwasm" -q -bin -Fo "$out/$name.com" "$here/$name.asm"
    ls -l "$out/$name.com"
    exit
fi

# The toy of the portable C's sys.h, which is built both ways of the
# tree's own files (see sys.c). In 16 bits it is linked with what the
# project's build compiled for the program: the functions, dos.obj, the
# stack, and the two routines for arithmetic on 32 bits; with the options
# and the order of segments that CMakeLists.txt has for the program. In
# 32 bits the tree's files are compiled here, as the plan of TODO.md has
# them compiled; the compiler's own headers are named for stddef.h, which
# vc.h reads there.
if [ "$name" = sys ]; then
    tree=$root/src/portable_c
    made=$root/build/portable_c
    exe=$out/sys-$kind.exe
    case $kind in
    16)
        "$bin/wcc" -q -0 -mm -os -s -zu -zl -zls -wx -we -nt=CTEXT \
            -nc=CCODE -i="$tree" -fo="$out/sys16.obj" "$here/sys.c"
        "$bin/wlink" option quiet format dos name "$exe" \
            order clname MAIN clname DATA clname BSS \
                  clname CCODE segment CTEXT segment CTEXT2 \
                  clname FAR_DATA clname STACK \
            file "$out/sys16.obj" file "$made/dos.obj" \
            file "$made/arith.obj" file "$made/stack.obj" \
            option map="$out/sys-16.map" option start=Alloc0_;;
    32)
        files=""
        for source in "$here/sys.c" "$tree/dos32.c" "$tree/stack.c"; do
            object=$out/$(basename "$source" .c)32.obj
            "$bin/wcc386" -q -3r -mf -os -s -zl -zls -wx -we -i="$tree" \
                -i="$W/h" -fo="$object" "$source"
            "$bin/wdis" -l="${object%.obj}.lst" "$object"
            files="$files file $object"
        done
        "$bin/wlink" option quiet format os2 le name "$exe" $files \
            option start=Start_ option stub="$W/binw/dos32a.exe" \
            option map="$out/sys-32.map" \
            order clname CODE clname DATA clname STACK clname BSS
        python3 "$here/quiet.py" "$exe";;
    *) echo "build.sh: sys 16 or sys 32, not $kind" >&2; exit 1;;
    esac
    ls -l "$exe"
    exit
fi

# The extender is the stub of the file. The linker's file of systems for
# Linux, binl64/wlink.lnk, has neither `causeway` nor `dos32a`, which
# binw/wlink.lnk has: so the format and the stub are said here.
case $kind in
    d32) stub=$W/binw/dos32a.exe;;
    cw) stub=$W/binw/cwstub.exe;;
    *) echo "build.sh: d32, cw or com, not $kind" >&2; exit 1;;
esac
exe=$out/$name-$kind.exe

if [ "$name" = hello ]; then
    "$bin/wcc386" -q -4r -mf -os -i="$W/h" -fo="$out/hello.obj" \
        "$here/hello.c"
    "$bin/wlink" option quiet format os2 le \
        libpath "$W/lib386" libpath "$W/lib386/dos" \
        option stub="$stub" name "$exe" file "$out/hello.obj"
else
    files=""
    for source in "$name.c" lib.c "$@"; do
        [ -f "$here/$source" ] || [ ! -f "$out/$source" ] \
            && path=$here/$source || path=$out/$source
        object=$out/$(basename "$source" .c).obj
        # -zl -zls: no C library is asked for. -s: no check of the stack,
        # which would call one.
        "$bin/wcc386" -q -4r -mf -os -s -zl -zls -wx -we -i="$here" \
            -fo="$object" "$path"
        files="$files file $object"
    done
    # The stack ahead of what has no value: behind it, all that has none
    # would be in the file, as zeros.
    "$bin/wlink" option quiet format os2 le name "$exe" $files \
        option start=Start_ option stack=16k option stub="$stub" \
        option map="$out/$name-$kind.map" \
        order clname CODE clname DATA clname STACK clname BSS
fi
[ "$kind" = d32 ] && python3 "$here/quiet.py" "$exe"
ls -l "$exe"
