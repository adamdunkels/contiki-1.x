;
; cfs-MLI code for Contiki (Apple2 version)
;
; int creat (const char* name, unsigned mode);
;

	.export		_creat
	.import		_open, PREFIX, ldax0sp, pushax
	.importzp 	ptr1

PATHNAME	:= $0280
MLI		:= $BF00

CREATE_CALL	= $C0

; ------------------------------------------------------------------------

	.data

CREATE_PARAM:
		.byte	$07		;PARAM_COUNT
		.addr	PATHNAME	;PATHNAME
		.byte	%11000011	;ACCESS:	Standard full access
		.byte	$06		;FILE_TYPE:	Standard binary file
		.word	$0000		;AUX_TYPE:	Load address N/A
		.byte	$01		;STORAGE_TYPE:	Standard seedling file
		.word	$0000		;CREATE_DATE:	Current date
		.word	$0000		;CREATE_TIME:	Current time

; ------------------------------------------------------------------------

	.code

_creat:
	; Get name
	jsr	ldax0sp

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
	.byte	CREATE_CALL
	.addr	CREATE_PARAM

	; Push flags dummy
	jsr	pushax
	jmp	_open
