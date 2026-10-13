/* What the toys of the trial share (see README.md): a flat 32-bit program
   for DOS, compiled by wcc386 and started by a DOS extender, with no C
   library. lib.c has the functions. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

/* The registers of a software interrupt made from protected mode, and
   the flags it came back with. */
struct R { u32 eax, ebx, ecx, edx, esi, edi, flags; };
#define CARRY 1
#define ZERO 0x40

void Int10(struct R *r);
void Int16(struct R *r);
void Int21(struct R *r);
void Int28(struct R *r);
void Int2F(struct R *r);
void Int31(struct R *r);
void Int33(struct R *r);
#define INTERRUPT(instruction) \
    "push ebp" "push esi" \
    "mov eax,[esi]" "mov ebx,[esi+4]" "mov ecx,[esi+8]" "mov edx,[esi+12]" \
    "mov edi,[esi+20]" "mov esi,[esi+16]" \
    instruction \
    "xchg esi,[esp]" \
    "mov [esi],eax" "mov [esi+4],ebx" "mov [esi+8],ecx" "mov [esi+12],edx" \
    "mov [esi+20],edi" "pop eax" "mov [esi+16],eax" \
    "pushfd" "pop eax" "mov [esi+24],eax" "pop ebp" "cld" \
    parm [esi] modify [eax ebx ecx edx esi edi]
#pragma aux Int10 = INTERRUPT("int 10h");
#pragma aux Int16 = INTERRUPT("int 16h");
#pragma aux Int21 = INTERRUPT("int 21h");
#pragma aux Int28 = INTERRUPT("int 28h");
#pragma aux Int2F = INTERRUPT("int 2Fh");
#pragma aux Int31 = INTERRUPT("int 31h");
#pragma aux Int33 = INTERRUPT("int 33h");

u16 GetCS(void);
u16 GetDS(void);
u16 GetES(void);
u32 GetESP(void);
#pragma aux GetCS = "mov ax,cs" value [ax] modify exact [ax];
#pragma aux GetDS = "mov ax,ds" value [ax] modify exact [ax];
#pragma aux GetES = "mov ax,es" value [ax] modify exact [ax];
#pragma aux GetESP = "mov eax,esp" value [eax] modify exact [eax];

/* What the extender handed over where it entered the program, kept by
   Start of lib.c: ESP, CS, DS, ES, SS, FS, GS, the ACPI timer, EBX, the
   flags, the BIOS's ticks, and the timer's counter 0 latched with its
   status in the byte above the count. */
extern u32 Entry[12];
enum { E_ESP, E_CS, E_DS, E_ES, E_SS, E_FS, E_GS, E_ACPI, E_EBX, E_FLAGS,
       E_TICKS, E_TIMER };
extern u8 Stack[16];            /* where the stack begins */

/* Texts go to the screen's memory at B8000h, from Row and Col on, and
   move no cursor; with ViaDos set they go to DOS, which scrolls. */
extern u32 Row, Col;
extern int ViaDos;
void Put(const char *text);
void Hex(u32 value, int digits);
void Dec(u32 value);
void Line(void);

void Exit(int code);
void Zero(void *at, u32 size);
u32 Ticks(void);                /* the BIOS's count of timer ticks */
u32 SelBase(u16 selector);      /* where a selector's segment begins */
/* The ACPI timer, which nothing in the machine sets: 3,579,545 counts to
   a second, 24 bits. A count is 279.4 ns, an instruction of the bench's
   machine 256 ns. */
u32 AcpiNow(void);

/* The registers of a call in real mode, as DPMI's functions 0300h and
   0301h take them. */
struct Real { u32 edi, esi, ebp, reserved, ebx, edx, ecx, eax;
              u16 flags, es, ds, fs, gs, ip, cs, sp, ss; };
/* An interrupt made in real mode, on a stack of the extender's. Nonzero
   if the extender refused. */
int RealInt(int number, struct Real *real);
/* A block of DOS's memory, below the megabyte: its segment, 0 if none. */
u16 LowBlock(u32 paragraphs);
