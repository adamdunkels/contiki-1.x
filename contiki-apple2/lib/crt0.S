;
; Startup code for Contiki (Apple2 version)
;
; This must be the *first* file on the linker command line
;

	.export		_exit, __Exit
	.export		__heaporg, __heapptr, __heapend
	.export		__heapfirst, __heaplast
	.import	   	initlib, donelib
	.import	   	zerobss
       	.import	       	__RAM_START__, __RAM_LAST__	; Linker generated
	.import		callmain
	.importzp	sp

; ------------------------------------------------------------------------
; The executable header

.segment	"EXEHDR"

       	.word  	__RAM_START__			; Start address
       	.word  	__RAM_LAST__ - __RAM_START__	; Size

; ------------------------------------------------------------------------
; Place the startup code in a special segment.

.segment       	"STARTUP"

; ProDOS TechRefMan, chapter 5.2.1:
; "For maximum interrupt efficiency, a system program should not use more
;  than the upper 3/4 of the stack."

	ldx	#$FF
	txs	       		; Init stack pointer

; Switch to 80 column mode

	.ifdef __APPLE2ENH__
	lda	#$0D
	jsr	$C300
	.endif

; Save the original RESET vector

	ldx	#$02
:	lda	$03F2,x
	sta	rvsave,x
	dex
	bpl	:-

; ProDOS TechRefMan, chapter 5.3.5:
; "Your system program should place in the RESET vector the address of a
;  routine that ... closes the files."

	lda	#<_exit
	sta	$03F2
	lda	#>_exit
	sta	$03F3
	eor	#$A5
	sta	$03F4
		
; Switch in LC bank 2 for R/W

	bit	$C083
	bit	$C083

; Clear the BSS data

	jsr	zerobss

; Setup the stack in LC bank 2

	lda    	#$00
	sta	sp
	lda	#$E0
       	sta	sp+1

; Setup the heap end (aligned down to a page boundary)

	lda	$73+1		; HIMEM
	sec
	sbc	#$04		; 1kB ProDOS8 I/O buffer
	sta	__heapend+1

; Call module constructors

	jsr	initlib

; Push arguments and call main()

	jsr	callmain

; Restore the original RESET vector. This is also the _exit entry

_exit:	ldx	#$02
:	lda	rvsave,x
	sta	$03F2,x
	dex
	bpl	:-

; Call module destructors

	jsr	donelib

; ProDOS TechRefMan, chapter 5.2.1:
; "System programs should set the stack pointer to $FF at the warm-start
;  entry point." This is also the __Exit entry

__Exit:	ldx	#$FF
	txs			; Re-init stack pointer

; Back to DOS

	jmp	$03D0

; ------------------------------------------------------------------------
; Data

.data

rvsave:	.res	3

__heaporg:
       	.word  	__RAM_LAST__	; Linker calculates this symbol
__heapptr:
      	.word	__RAM_LAST__	; Dito
__heapend:
       	.word	0
__heapfirst:
      	.word	0
__heaplast:
      	.word	0
