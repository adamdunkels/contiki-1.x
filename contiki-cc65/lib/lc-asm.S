;---------------------------------------------------------------------
	.importzp ptr1
	.export _lc_get
	.export _lc_resume
	.export _lc_tmp
	
;---------------------------------------------------------------------
.data
_lc_tmp:	.byte 2		
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
	cpx #0
	bne :+
	rts
:		
	pla
	pla
	jmp (ptr1)
;---------------------------------------------------------------------	


	