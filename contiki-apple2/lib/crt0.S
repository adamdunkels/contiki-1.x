;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Custom startup code for Contiki on the Apple ][ (Oliver Schmidt)
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


.export 	_exit, SETWND, BASCALC
.importzp	sp
.import 	zerobss, initlib, callmain, donelib
.import 	__CONF_MEM_START__, __CONF_MEM_LAST__
.import 	__CODE_MEM_START__, __CODE_MEM_LAST__
.import 	__DATA_MEM_START__, __DATA_MEM_LAST__
.import		__RODATA_LOAD__, __RODATA_RUN__, __RODATA_SIZE__
.import		__DATA_SIZE__, __BSS_SIZE__, __STACKLAST__


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


.segment	"CONF_HDR"

	.word	__CONF_MEM_START__
	.word	__CONF_MEM_LAST__ - __CONF_MEM_START__


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


.segment	"CODE_HDR"

	.word	__CODE_MEM_START__
	.word	__CODE_MEM_LAST__ - __CODE_MEM_START__


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


.segment	"CODE_1ST"

	; Reset CPU stack
	ldx	#$FF
	txs

	; Init operand stack
	lda	#<__STACKLAST__
	ldx	#>__STACKLAST__
	sta	sp
	stx	sp + 1

	; Make RESET reboot
	lda	$03F3
	sta	$03F4

.ifdef __APPLE2__

	; Switch in LC bank 1 for R/W
	bit	$C08B
	bit	$C08B

	; Get addr of RESET handler
	lda	#<reset
	ldx	#>reset

	; Set RESET vector into LC
	sta	$FFFC
	stx	$FFFD
	
	; Get addr of IRQ handler
	lda	#<irq_init
	ldx	#>irq_init

.else

	; Switch to 80 column mode
	lda	#$00
	jsr	$C300

	; Get IRQ vector from ROM
	lda	$FFFE
	ldx	$FFFF

	; Switch in LC bank 1 for R/W
	bit	$C08B
	bit	$C08B

.endif

	; Set IRQ vector into LC
	sta	$FFFE
	stx	$FFFF

	; Set BBS to zero
	jsr	zerobss

	; Go for it ...
	jsr	initlib
	jsr	callmain
_exit:	jsr	donelib

	; Switch in ROM and do RESET
reset:	bit	$C08A
	jmp	($FFFC)


SETWND:

.ifdef __APPLE2__
	sei
.endif
	; Switch in ROM and call SETWND
	bit	$C08A
	jsr	$FB4B

	; Switch in LC bank 1 for R/W
	bit	$C08B
	bit	$C08B

.ifdef __APPLE2__
	cli
.endif
	rts


BASCALC:

.ifdef __APPLE2__
	sei
.endif
	; Switch in ROM and call BASCALC
	bit	$C08A
	jsr	$FBC1

	; Switch in LC bank 1 for R/W
	bit	$C08B
	bit	$C08B

.ifdef __APPLE2__
	cli
.endif
	rts


.ifref irq_init
irq_init:
	; Save registers on CPU stack
	pha
	txa
	pha
	tya
	pha

	; Create another interrupt frame
	lda	#>irq_exit
	pha
	lda	#<irq_exit
	pha
	php

	; Execute user IRQ handler
	jmp	($03FE)
.endif


.ifref irq_exit
irq_exit:
	; Restore registers and return
	pla
	tay
	pla
	tax
	pla
	rti
.endif


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


.segment	"DATA_HDR"

	.word	__DATA_MEM_START__
	.word	__DATA_MEM_LAST__ - __DATA_MEM_START__


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


.segment	"DATA_1ST"

	; Switch in LC bank 1 for W/O
	bit	$C089
	bit	$C089

	; Set source start
	lda	#<__RODATA_LOAD__
	ldx	#>__RODATA_LOAD__
	sta	$3C
	stx	$3D

	; Set source end
	lda	#<(__RODATA_LOAD__ + __RODATA_SIZE__ + __DATA_SIZE__)
	ldx	#>(__RODATA_LOAD__ + __RODATA_SIZE__ + __DATA_SIZE__)
	sta	$3E
	stx	$3F

	; Set destination
	lda	#<__RODATA_RUN__
	ldx	#>__RODATA_RUN__
	sta	$42
	stx	$43

	; Reset index and call MOVE
	ldy	#$00
	jsr	$FE2C

	; Switch in ROM and return
	bit	$C08A
	rts


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
