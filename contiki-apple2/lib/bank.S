;
; Bank switching code for Contiki (Apple2 version)
;

	.export		SETWND, BASCALC

SETWND:
	.ifdef __APPLE2__
	sei
	.endif

	; Switch in ROM and call SETWND
	bit	$C082
	jsr	$FB4B

	; Switch in LC bank 2 for R/W
	bit	$C083
	bit	$C083

	.ifdef __APPLE2__
	cli
	.endif
	rts

BASCALC:
	.ifdef __APPLE2__
	sei
	.endif

	; Switch in ROM and call BASCALC
	bit	$C082
	jsr	$FBC1

	; Switch in LC bank 2 for R/W
	bit	$C083
	bit	$C083

	.ifdef __APPLE2__
	cli
	.endif
	rts
