;
; Contiki kernel file system (Apple2 version)
;
; int __fastcall__ kfs_open(const char* name);
; int __fastcall__ kfs_read(int fd, void* buf, unsigned count);
; int __fastcall__ kfs_close(int fd);
; char* kfs_getdir(void);
;

	.constructor	init_kfs
        .destructor     done_kfs
	.export		_kfs_open, _kfs_read, _kfs_close, _kfs_getdir
	.import		_uip_buf, popax
	.importzp 	ptr1

PATHNAME	:= $0280
MLI		:= $BF00

GET_PREFIX_CALL	= $C7
OPEN_CALL	= $C8
READ_CALL	= $CA
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

READ_PARAM:
		.byte	$04		;PARAM_COUNT
READ_FD:	.byte	$00		;REF_NUM
READ_BUFFER:	.addr	$0000		;DATA_BUFFER
READ_COUNT_IN:	.word	$0000		;REQUEST_COUNT
READ_COUNT_OUT:	.word	$0000		;TRANS_COUNT

CLOSE_PARAM:
		.byte	$01		;PARAM_COUNT
CLOSE_FD:	.byte	$00		;REF_NUM

; ------------------------------------------------------------------------

	.segment	"INIT"

init_kfs:
        ; Check for full pathname
	lda	PATHNAME+1
	cmp	#'/'
	beq	prefix

	; Save name incl. length
	ldx	PATHNAME
:	lda	PATHNAME,x
	sta	_uip_buf,x
	dex
	bpl	:-

	jsr	MLI
	.byte	GET_PREFIX_CALL
	.addr	GET_PREFIX_PARAM

	; Append name to prefix
	lda	PATHNAME
	clc
	adc	_uip_buf
	sta	PATHNAME
	tax
	ldy	_uip_buf
:	lda	_uip_buf,y
	sta	PATHNAME,x
	dex
	dey
	bne	:-

prefix:	lda	PATHNAME
	sec
	sbc	#.strlen("CONTIKI")
	sta	PREFIX
	rts

GET_PREFIX_PARAM:
		.byte	$01		; PARAM_COUNT
		.addr	PATHNAME	; DATA_BUFFER

; ------------------------------------------------------------------------

	.segment	"CONTIKI"

_kfs_open:
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

_kfs_read:
	; Store count
	sta	READ_COUNT_IN
	stx	READ_COUNT_IN+1

	; Pop and store buf
	jsr	popax
	sta	READ_BUFFER
	stx	READ_BUFFER+1

	; Pop and store fd
	jsr     popax
	sta	READ_FD

	jsr	MLI
	.byte	READ_CALL
	.addr	READ_PARAM
	bcs	ERROR

	; Return bytes transfered
	lda	READ_COUNT_OUT
	ldx	READ_COUNT_OUT+1
	rts

done_kfs:
	lda	#$00

_kfs_close:
	; Store	fd
	sta	CLOSE_FD

	jsr	MLI
	.byte	CLOSE_CALL
	.addr	CLOSE_PARAM
	bcs	ERROR

	; Return 0
	tax
	rts

_kfs_getdir:
	; Append '\0' to prefix
	lda	#$00
	ldx	PREFIX
	sta	PATHNAME+1,x

	; Return prefix
	lda	#<(PATHNAME+1)
	ldx	#>(PATHNAME+1)
	rts

ERROR:
	; Return -1
	lda     #$FF
        tax
        rts
