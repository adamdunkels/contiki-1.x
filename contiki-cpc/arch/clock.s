	.module clock
	.area GSINIT
	call __clock_init

	.globl __clock_val
	.globl __clock_init

	.area _CODE
__clock_init::
	ld hl,#event_block
	ld b,#0x01
	ld c,#0x00
	ld de,#event_routine
	call #0xbce0		;; KL NEW FAST TICKER
	ret

event_routine::
	push hl
	ld hl,(__clock_val)
	inc hl
	ld (__clock_val),hl
	pop hl
	ret

__clock_val:: .dw 0

event_block::
	.ds 9

