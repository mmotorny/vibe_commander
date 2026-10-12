;-------------------------------------------------------
;		Version :	07.06.2000
;-------------------------------------------------------

; The map of the memory is C: port/memory.c, with the buttons of its
; window and their colours in port/memory_t.c. What is left here is what
; the C code asks of the part of the program that stays in the memory.

	.CODE
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