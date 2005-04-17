;
; Contiki kernel file system (Apple2 version)
;
; int __fastcall__ kfs_open(const char* name);
; int __fastcall__ kfs_read(int fd, void* buf, unsigned count);
; int __fastcall__ kfs_close(int fd);
; char* kfs_getdir(void);
;
; Be sure to keep the value priority of closeallfiles lower than that of
; closeallstreams (which is the high level C file I/O counterpart and must be
; called before closeallfiles).

	.constructor	initprefix
        .destructor     closeallfiles, 17
	.export		_kfs_open, _kfs_read, _kfs_close, _kfs_getdir
	.import		_uip_buf, popax
	.importzp 	ptr1

PATHNAME	:= $0280
MLI		:= $BF00

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

initprefix:
	lda	PATHNAME
	sec
	sbc	#.strlen("CONTIKI")
	sta	PREFIX
	rts

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

closeallfiles:
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
