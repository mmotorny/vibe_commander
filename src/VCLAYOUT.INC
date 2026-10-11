;-------------------------------------------------------
;	      Layout of the shareware binaries
;-------------------------------------------------------
;
; The binaries shipped in 2000 were shareware. They hold registration
; code, tables and strings that this source does not have. When
; SHAREWARE_LAYOUT is defined, the macros below reserve exactly the room
; which that material takes in the shipped binaries, so that everything
; else lands on the same address and can be compared with them byte for
; byte. Code is filled with NOPs and data with zeros. Without
; SHAREWARE_LAYOUT the macros generate nothing.

ShwCode	MACRO	Bytes			; Shareware-only code
IFDEF	SHAREWARE_LAYOUT
	DB	Bytes DUP(90h)
ENDIF
	ENDM

ShwData	MACRO	Bytes			; Shareware-only data
IFDEF	SHAREWARE_LAYOUT
	DB	Bytes DUP(0)
ENDIF
	ENDM
