;
; Bank switching code for Contiki (Apple2 version)
;

	.export		VTABZ

	.segment	"CONTIKI"

VTABZ:
	.ifdef __APPLE2__
	sei
	.endif

	; Switch in ROM and call VTABZ
	bit	$C082
	jsr	$FC24

	; Switch in LC bank 2 for R/O
	bit	$C080

	.ifdef __APPLE2__
	cli
	.endif
	rts
