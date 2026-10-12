/*
 * DOS, the BIOS, the mouse driver and the machine: every place where the
 * program asks something of them that C has no words for.
 *
 * DOS, the BIOS and the mouse driver are called by software interrupts,
 * and an interrupt is an instruction: each function here that calls one
 * is a few lines of C around the instruction, which is written as inline
 * assembly, in a #pragma aux of its own, with as little beside it as
 * will do. The registers go into the instruction and come out of it as
 * arguments and as what the function returns; the compiler loads them.
 * Nothing else in the C translation has inline assembly, and no other file
 * calls an interrupt: the rest of the program calls these functions in
 * plain C. The ports of the machine and its interrupt flag are here too.
 * They need no assembly, the compiler has intrinsic functions for them, but
 * they are the machine all the same.
 *
 * What is in the memory of the machine is not here: the BIOS's data and
 * the screen are read and written through far pointers, in plain C,
 * where the program does so (see system.c and window.c).
 *
 * This is where the program will meet another operating system. Until
 * the translation is done nothing here hides DOS: a function is the call
 * of an interrupt and no more, and what the program makes of it is in
 * the routine that calls it. What each function is for is said with it,
 * and which of the services of an interrupt the program uses.
 */
#include "vc.h"

/* The compiler's own functions for a port and for the interrupt flag:
   an instruction each, put where they are called. */
unsigned inp(unsigned port);
void _disable(void);
void _enable(void);
#pragma intrinsic(inp, _disable, _enable)

/* The interrupts. Each of these is put in place where it is called, as
   the instructions between the quotes. */

/* INT 21h with the registers of `regs`, which is on the stack, ES among
   them; the carry flag is set first if `carry` is not 0, since some
   calls of DOS leave it as they find it. AX, BX, CX, DX and ES go back
   into `regs` as DOS leaves them, and what is returned is not 0 if the
   carry flag is set then. PUSH, POP, MOV and LDS leave the flags
   alone. */
static u16 Dos(struct DosRegs STACK *regs, u16 carry);
#pragma aux Dos = \
    "neg ax" \
    "push bp" \
    "push ds" \
    "mov bp,bx" \
    "mov es,[bp+14]" \
    "mov ax,[bp]" \
    "mov bx,[bp+2]" \
    "mov cx,[bp+4]" \
    "mov si,[bp+10]" \
    "mov di,[bp+12]" \
    "lds dx,[bp+6]" \
    "int 21h" \
    "mov [bp],ax" \
    "mov [bp+2],bx" \
    "mov [bp+4],cx" \
    "mov [bp+6],dx" \
    "mov [bp+14],es" \
    "sbb ax,ax" \
    "pop ds" \
    "pop bp" \
    parm [bx] [ax] value [ax] modify exact [ax bx cx dx si di es]

/* INT 10h with AX, BX, CX and DX, which go back into the four words at
   `regs`, on the stack, as the BIOS leaves them: more than a function
   can return. Some of the BIOSes that the program was written for lose
   BP in it, and the program saved BP around the calls that did; one call
   answers in ES and BP besides, which the program does not read. */
static void Video(u16 ax, u16 bx, u16 cx, u16 dx,
                  struct BiosRegs STACK *regs);
#pragma aux Video = \
    "push bp" \
    "int 10h" \
    "pop bp" \
    "mov ss:[si],ax" \
    "mov ss:[si+2],bx" \
    "mov ss:[si+4],cx" \
    "mov ss:[si+6],dx" \
    parm [ax] [bx] [cx] [dx] [si] modify exact [ax bx cx dx es]

/* INT 10h with an address in ES:DI, and the address that is there
   afterwards. */
static u16 __far *VideoBuffer(u16 ax, u16 __far *cells, u16 count);
#pragma aux VideoBuffer = \
    "push bp" \
    "int 10h" \
    "pop bp" \
    parm [ax] [es di] [cx] value [es di] modify exact [ax bx cx dx si di es]

/* INT 16h, with AX as it leaves it and, above that, the flags. */
static u32 Keyboard(u16 ax);
#pragma aux Keyboard = \
    "int 16h" \
    "pushf" \
    "pop dx" \
    parm [ax] value [dx ax] modify exact [ax dx]

/* INT 2Fh, with AX and BX as it leaves them. */
static u32 Multiplex(u16 ax, u16 bx, u16 cx);
#pragma aux Multiplex = \
    "int 2Fh" \
    parm [ax] [bx] [cx] value [bx ax] modify exact [ax bx]

/* INT 28h, which leaves every register as it finds it. */
static void DosIdle(void);
#pragma aux DosIdle = \
    "int 28h" \
    modify exact []

/* INT 15h three times, for DESQview: its calls 101Ah, 1000h and 1025h.
   The first has DESQview take the program off its stack onto one of
   DESQview's own and the third puts it back, so nothing that the
   compiler makes, which may use the stack, can stand between them: the
   three calls are one piece. */
static void DesqviewPause(void);
#pragma aux DesqviewPause = \
    "mov ax,101Ah" \
    "int 15h" \
    "mov ax,1000h" \
    "int 15h" \
    "mov ax,1025h" \
    "int 15h" \
    modify exact [ax]

/* INT 33h, with AX as it leaves it. */
static u16 MouseDriver(u16 ax, u16 bx, u16 cx, u16 dx);
#pragma aux MouseDriver = \
    "int 33h" \
    parm [ax] [bx] [cx] [dx] value [ax] modify exact [ax bx cx dx]

/* INT 33h, with BX, CX and DX as it leaves them, in the three words at
   `state`, which is on the stack: more than a function can return. */
static void MouseState(u16 ax, struct MouseAt STACK *state);
#pragma aux MouseState = \
    "int 33h" \
    "mov ss:[di],bx" \
    "mov ss:[di+2],cx" \
    "mov ss:[di+4],dx" \
    parm [ax] [di] modify exact [ax bx cx dx]

/* DOS's routine that makes a capital of a letter beyond ASCII, which it
   names in what it tells of the country: called far, the letter in AL
   and back there, and every other register left alone. */
typedef u16 __far CaseMapFn(u16 letter);
#pragma aux CaseMapFn parm [ax] value [ax] modify exact [ax]

/* DOS: any of its calls, with the registers of `regs` and their answer
   there (see struct DosRegs in vc.h), and ES on the segment that DS is
   on, the main data. Not 0 if DOS set the carry flag, which is clear
   when DOS is called. The program calls DOS like this where it does not
   look for the box of a drive that is not ready, and through Intr21 of
   system.c where it does. */
u16 Int21d(struct DosRegs STACK *regs)
{
    regs->es = MAIN_SEG;
    return Dos(regs, 0);
}

/* The same with the carry flag set when DOS is called, if `carry` is not
   0: for a call of DOS that leaves the flag as it finds it. */
u16 Int21f(struct DosRegs STACK *regs, u16 carry)
{
    regs->es = MAIN_SEG;
    return Dos(regs, carry);
}

/* The same as Int21d with ES as `regs` has it: for the calls of DOS that
   take a block of the memory there which is not the main data. The
   program makes the block that its code is in another size like this
   (AH 4Ah) when it starts. */
u16 Int21e(struct DosRegs STACK *regs)
{
    return Dos(regs, 0);
}

/* The BIOS, for the screen: its call with the registers of `regs`, which
   come back there as the BIOS leaves them. The program has it say what
   mode and page the screen has (AH 0Fh) and set a mode (0), place the
   cursor (2), say where the cursor is and what shape it has (3), give it
   a shape (1), scroll a window up or down (6, 7), write a character with
   its colour (9), sound the bell and feed a line (0Eh), and for an EGA
   or a VGA say how high its font is and how many rows that makes (1130h),
   load the font of 8 lines (1112h) and take over printing the screen
   (12h). */
void Int10(struct BiosRegs STACK *regs)
{
    Video(regs->ax, regs->bx, regs->cx, regs->dx, regs);
}

/* Tell TopView or DESQview that `count` cells of the screen's buffer
   have changed, from `cells` on (AH 0FFh): under them the program draws
   into a buffer that they show. */
void TopView(u16 __far *cells, u16 count)
{
    VideoBuffer(0xFF00, cells, count);
}

/* Ask TopView or DESQview where the buffer is that they show for the
   display's memory at `screen` (AH 0FEh). The BIOS itself leaves the
   address as it is: `screen` comes back where neither runs. */
u16 __far *TopScr(u16 __far *screen)
{
    return VideoBuffer(0xFE00, screen, 0);
}

/* The BIOS, for the keyboard: its call with AX. Returns AX as the BIOS
   leaves it, and above it 1 if the zero flag is set then. The program
   has it take a key, waiting for one (AH 0), say whether a key waits
   and which, the zero flag set if none does (1), and say which of
   Shift, Ctrl and Alt are held (2, and 12h). */
u32 Int16(u16 ax)
{
    u32 pair = Keyboard(ax);
    u16 flags = (u16)(pair >> 16);

    return (u32)(flags >> 6 & 1) << 16 | (u16)pair;
}

/* The multiplex interrupt, by which resident programs are asked: its
   call with these registers. Returns AX in the low word and BX in the
   high one. The program asks the CD-ROM extensions whether a drive is
   theirs (AX 150Bh), and tells Windows, or whatever else shares the
   machine among programs, that it can do without the rest of its turn
   (1680h), which leaves AL as it is if nobody listens. */
u32 Int2F(u16 ax, u16 bx, u16 cx)
{
    return Multiplex(ax, bx, cx);
}

/* Tell DOS that the program is idle: the interrupt that DOS itself
   calls while it waits for a key, and that programs which stay in the
   memory take to do their work in. */
void Int28(void)
{
    DosIdle();
}

/* Give the rest of the program's turn to the other programs of
   DESQview: on DESQview's stack (INT 15h, AX 101Ah), pause (1000h), and
   back on the program's own (1025h). */
void DvPause(void)
{
    DesqviewPause();
}

/* The mouse driver: its call with these registers. Returns AX as the
   driver leaves it. The program has it say whether it is there (AX 21h,
   and 0), show and hide the pointer (1, 2), put the pointer somewhere
   (4), set the rows it may go to (8) and what it looks like (0Ah). */
u16 Int33(u16 ax, u16 bx, u16 cx, u16 dx)
{
    return MouseDriver(ax, bx, cx, dx);
}

/* Ask the mouse driver where the pointer is and which buttons are held
   (AX 3). */
void MousAt(struct MouseAt STACK *state)
{
    MouseState(3, state);
}

/* The capital of a letter as DOS has it for the country: only letters
   beyond ASCII change. The low byte is the letter, and the high byte
   comes back as it went. */
u16 CaseMap(u16 letter)
{
    return ((CaseMapFn *)md.Country.CaseMap)(letter);
}

/* A byte from a port of the machine. The program reads the status of a
   CGA there, 3DAh, to write to its memory when that makes no snow. */
u16 InPort(u16 port)
{
    return inp(port);
}

/* Keep the machine from interrupting the program, and let it again. */
void IntOff(void)
{
    _disable();
}

void IntOn(void)
{
    _enable();
}
