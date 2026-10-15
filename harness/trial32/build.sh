#!/bin/sh
# Build a toy of the trial into build/trial32/ (see README.md):
#
#   build.sh NAME d32 [MORE.c ...]   NAME.c as NAME-d32.exe, with DOS/32A
#   build.sh NAME cw [MORE.c ...]    as NAME-cw.exe, with CauseWay
#   build.sh hello d32|cw            hello.c with Open Watcom's C library
#   build.sh NAME com                NAME.asm as NAME.com, for real mode
#   build.sh driver                  DRIVERX.ASM as DRIVERX.COM
#   build.sh sys 32                  sys.c as sys-32.exe, with the
#                                    sys_dos.c of the portable C and DOS/32A
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

# The toy of the portable C's sys.h, which is built of the tree's own
# files (see sys.c): they are compiled here, as the plan of TODO.md has
# them compiled; the compiler's own headers are named for stddef.h, which
# vc.h reads. It was built for 16 bits too, as sys-16.exe, with the dos.c
# that the portable C had while it was a program of 16 bits.
if [ "$name" = sys ]; then
    tree=$root/src/portable_c
    exe=$out/sys-$kind.exe
    case $kind in
    32)
        files=""
        for source in "$here/sys.c" "$tree/sys_dos.c" "$tree/stack_dos.c"; do
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
    *) echo "build.sh: sys 32, not $kind" >&2; exit 1;;
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
