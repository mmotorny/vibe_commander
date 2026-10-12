;-------------------------------------------------------
;		Version :	07.06.2000
;-------------------------------------------------------

; The map of the memory is C: port/memory.c. What is left here are the
; buttons of its window and their colours, which MinMnu reads in this
; segment, the command, and what the C code asks of DOS and of the part
; of the program that stays in the memory. Counter, PrgList and Command
; are of that part, in VC.ASM.
	PUBLIC	Memo1_P, Memo_D, Counter, PrgList, Command

	.DATA
Memo1_P	DB	Conf_C, CnfRe_C, CnfRe_C
Memo_D	DB	11,  0
	DB	10, 14
	DB	 0

	.CODE
F65_00:	CallC	F65_00_c		; Alt F5 - memory info
	JMP	Func04

; Whether the upper memory is linked into the chain of blocks of DOS:
; AX is 0 or 1, or 0FFFFh if DOS does not know of it.
	PUBLIC	Dos5802_a, Dos5803_a, Dos52_a, Dos4452_a, Dos2523_a
Dos5802_a PROC	FAR
	MOV	AX,5802h
	INT	21h
	MOV	AH,0
	JNC	Dos58A
	MOV	AX,0FFFFh
Dos58A:	RET
Dos5802_a ENDP

; Links it or not: BX - 1 or 0.
Dos5803_a PROC	FAR
	MOV	AX,5803h
	INT	21h
	RET
Dos5803_a ENDP

; The segment of the first header of that chain, in AX.
Dos52_a	PROC	FAR
	PUSH	ES
	MOV	AH,52h
	INT	21h
	MOV	AX,ES:[BX-2]
	POP	ES
	RET
Dos52_a	ENDP

; What DR DOS says its version is, in AX: 0 if it is another DOS.
Dos4452_a PROC	FAR
	PUSH	DX
	MOV	AX,4452h
	INT	21h
	POP	DX
	MOV	AH,0
	JNC	Dos44A
	MOV	AX,0
Dos44A:	RET
Dos4452_a ENDP

; Ctrl-C of DOS to a routine of the part of the program that stays in the
; memory: DX - its address there.
Dos2523_a PROC	FAR
	PUSH	DS
	PUSH	SS
	POP	DS
	MOV	AX,2523h
	INT	21h
	POP	DS
	RET
Dos2523_a ENDP

; The segment of the part of the program that goes while a command runs,
; in AX, and that of the part that stays.
	PUBLIC	MemRun_a, ResSeg_a, Release_a
MemRun_a PROC	FAR
	MOV	AX,SS:RunPar
	ADD	AX,TrnPar
	RET
MemRun_a ENDP

ResSeg_a PROC	FAR
	MOV	AX,SS
	RET
ResSeg_a ENDP

; Release a program that has stayed in the memory: Release, which is of
; the part that stays. AX - what PrgList has of the program.
Release_a PROC	FAR
	PUSH	BP
	PUSH	DS
	PUSH	DS
	POP	ES
	MOV	DS,AX
	MOV	AX,OFFSET Release
	CALL	CallPSP
	POP	DS
	POP	BP
	RET
Release_a ENDP
