;
; MLI code for Contiki (Apple2 version)
;
; int open (const char* name, int flags);
; int read (int fd, void* buf, unsigned count);
; int write (int fd, const void* buf, unsigned count);
; int close (int fd);
;
; Be sure to keep the value priority of closeallfiles lower than that of
; closeallstreams (which is the high level C file I/O counterpart and must be
; called before closeallfiles).

	.constructor	initprefix
        .destructor     closeallfiles, 17
	.export		_open, _read, _write, _close
	.import		_uip_buf, popax
	.importzp 	ptr1

PATHNAME	:= $0280
MLI		:= $BF00

OPEN_CALL	= $C8
READ_CALL	= $CA
WRITE_CALL	= $CB
CLOSE_CALL	= $CC

; ------------------------------------------------------------------------

	.bss

PREFIX:		.res	1

; ------------------------------------------------------------------------

	.data

OPEN_PARAM:
		.byte	$03		;PARAM_COUNT
		.addr	PATHNAME	;PATHNAME
		.addr	_uip_buf	;IO_BUFFER
OPEN_FD:	.byte	$00		;REF_NUM

RW_PARAM:
		.byte	$04		;PARAM_COUNT
RW_FD:		.byte	$00		;REF_NUM
RW_BUFFER:	.addr	$0000		;DATA_BUFFER
RW_COUNT_IN:	.word	$0000		;REQUEST_COUNT
RW_COUNT_OUT:	.word	$0000		;TRANS_COUNT

CLOSE_PARAM:
		.byte	$01		;PARAM_COUNT
CLOSE_FD:	.byte	$00		;REF_NUM

; ------------------------------------------------------------------------

	.segment	"CONTIKI"

initprefix:
	lda	PATHNAME
	sec
	sbc	#.strlen("CONTIKI")
	sta	PREFIX
	rts

_open:
	; Pop flags and name
	jsr     popax
	jsr	popax

	; Append name to prefix
	sta	ptr1
	stx	ptr1+1
	ldy	#$00
	ldx	PREFIX
:	lda	(ptr1),y
	beq	:+
	sta	PATHNAME+1,x
	iny
	inx
	bne	:-	; bra
:	stx	PATHNAME

	jsr	MLI
	.byte	OPEN_CALL
	.addr	OPEN_PARAM
	bcs	ERROR

	; Return fd
	lda	OPEN_FD
	ldx	#$00
	rts

_write:
	ldy	#WRITE_CALL
	bne	:+	; bra
_read:
	ldy	#READ_CALL
:	sty	RW_CMD

	; Store count
	sta	RW_COUNT_IN
	stx	RW_COUNT_IN+1

	; Pop and store buf
	jsr	popax
	sta	RW_BUFFER
	stx	RW_BUFFER+1

	; Pop and store fd
	jsr     popax
	sta	RW_FD

	jsr	MLI
RW_CMD:	.byte	$00
	.addr	RW_PARAM
	bcs	ERROR

	; Return bytes transfered
	lda	RW_COUNT_OUT
	ldx	RW_COUNT_OUT+1
	rts

closeallfiles:
	lda	#$00
_close:
	; Store	fd
	sta	CLOSE_FD

	jsr	MLI
	.byte	CLOSE_CALL
	.addr	CLOSE_PARAM
	bcs	ERROR

	; Return 0
	tax
	rts

ERROR:
	; Return -1
	lda     #$FF
        tax
        rts
