;-------------------------------------------------------
;		Version :	09.05.95
;-------------------------------------------------------

HlpData	STRUC
	DB	HelpBuf DUP(?)
HlpHndl	DW	?
HlpPrms	DB	0 DUP(?)
HlpMode	DW	?
HlpSize	DW	?
HlpLins	DW	?
	DW	?
HlpBkLn	DW	?
HlpOffs	DD	?
HlpFrst	DW	?
HlpCur	DW	?
	DB	10 DUP(?)
HlpBkFr	DW	?
HlpInfo	DB	0 DUP(?)
HlpData	ENDS

	.CONST
Help_S	DB	' Help ',0
HlEr1_S	DB	'There was error reading data',0
HlEr2_S	DB	'from the help file.',0

	.DATA
HlpMn_P	DB	Help_C, HlpRe_C, HlpRe_C
HlMn1_D	DB	 8, 14
	DB	10, 25
	DB	 0
HlMn2_D	DB	 8,  0
	DB	12, 11
	DB	 9, 26
	DB	10, 38
	DB	 0
HlpEr_P	DB	Err_C, ErInv_C, ErInv_C, 0
	DB	4, 0
	DW	HlpInfo
	DW	Help_S
	DW	HlEr1_S
	DW	HlEr2_S
	DW	Ok_S
HlpEr_B	DW	HlpEr_P, 928h, 0
	DB	2, 0FFh, 0FFh, 2
	DB	4
	DB	0, 0, 0, 0
	DW	426h, TmpErr, OkMn_D
	DB	0

	.CODE
; The help is C: Help00_c of port/help.c, with the routines that were its
; own. It saves the registers, as Help did.
Help	PROC	NEAR
	PUSH	AX
	CallC	Help00_c
	POP	AX
	RET
Help	ENDP
