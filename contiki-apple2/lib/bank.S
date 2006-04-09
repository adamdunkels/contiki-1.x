;
; Bank switching code for Contiki (Apple2 version)
;

	.export		VTABZ

	.segment	"CONTIKI"

VTABZ:
	; Switch in ROM and call VTABZ
	bit	$C082
	jsr	$FC24

	; Switch in LC bank 2 for R/O and return
	bit	$C080
	rts
