;---------------------------------------------------------------------
	.importzp ptr1
	.export _lc_get
	.export _lc_resume
;---------------------------------------------------------------------
.code
_lc_get:
	pla
	tay
	pla
	tax
	
	pha
	tya
	pha

	sec
	sbc #2
	bcs :+
	dex
:
	rts
;---------------------------------------------------------------------
_lc_resume:	
	sta ptr1
	stx ptr1+1

	ora ptr1+1		; Don't resume a NULL continuation
	bne :+
	rts
:		
	pla
	pla
	jmp (ptr1)
;---------------------------------------------------------------------	


	