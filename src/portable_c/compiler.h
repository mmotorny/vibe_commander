/*
 * The words that each compiler has its own way of saying: what the
 * language has no word for, or has none for in C of 1999, and the
 * program needs said. Each is defined once for Open Watcom, which
 * builds the program for DOS, once for gcc and clang, and as nothing
 * for any other compiler, which is then told nothing and compiles the
 * program all the same. vc.h includes this, and every file has it so.
 */
#ifndef COMPILER_H
#define COMPILER_H

/* A #pragma written by a macro: _Pragma of C99 takes the line as a
   string, which this makes of the words it is given. */
#define PRAGMA(words) _Pragma(#words)

/* A function that does not return, written in front of it where it is
   declared: Open Watcom then ends its caller at the call, with nothing
   behind it, the same instructions as `#pragma aux NAME aborts` made
   before. Another compiler is told nothing here: _Noreturn of C11 is
   its word, and the functions of sys.h that end the program would want
   it too. */
#ifdef __WATCOMC__
#define NO_RETURN __declspec(aborts)
#else
#define NO_RETURN
#endif

/* What is defined between the two words is in a segment of the name
   and of the class that the first names, where Open Watcom has it in
   the segments of its own: `#pragma data_seg`. The linker's command
   says where a class stands (see CMakeLists.txt). The main data is of
   the class MAIN, and the stack of stack_dos.c of the class STACK, which
   the linker takes for the stack. Another compiler is told nothing,
   and has the object where it has every other. */
#ifdef __WATCOMC__
#define SEGMENT_BEGIN(name, class) PRAGMA(data_seg(name, class))
#define SEGMENT_END PRAGMA(data_seg())
#else
#define SEGMENT_BEGIN(name, class)
#define SEGMENT_END
#endif

/* An array of characters that is as long as its text and has no room
   for the zero byte behind it, on purpose: a text that is read by its
   count, or that runs on into what follows it, as the original has it.
   Written behind the array's name where it is defined; gcc and clang
   then know that no zero was meant, and say nothing of it. Open Watcom
   says nothing of such an array in any case. */
#ifdef __GNUC__
#define NONSTRING __attribute__((nonstring))
#else
#define NONSTRING
#endif

/* A case of a switch that goes on into the next one on purpose, as the
   original runs on from one label into the next: written as a statement,
   `FALLTHROUGH;`, the last thing in front of the next label. gcc and
   clang then know that it was meant, and say nothing of it
   (-Wimplicit-fallthrough). Open Watcom says nothing of it in any case,
   and has an empty statement there, which is no code. */
#ifdef __GNUC__
#define FALLTHROUGH __attribute__((fallthrough))
#else
#define FALLTHROUGH
#endif

#endif
