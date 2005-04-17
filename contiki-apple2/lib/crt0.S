;
; Startup code for Contiki (Apple2 version)
;
; This must be the *first* file on the linker command line
;

	.export		_exit
	.import	   	zerobss, initlib, callmain, donelib
       	.import	       	__STARTUP_RUN__, __BOOT_SIZE__	; Linker generated
       	.import	       	__BSS_RUN__, __BSS_SIZE__	; Linker generated
       	.import	       	__INIT_RUN__, __INIT_SIZE__	; Linker generated
       	.import	       	__CODE_RUN__, __CODE_SIZE__	; Linker generated
	.importzp	sp
	.linecont	+

; ------------------------------------------------------------------------
; The executable header

.segment	"EXEHDR"

       	.word  	__STARTUP_RUN__				; Start address
       	.word  	__BSS_RUN__   + __BOOT_SIZE__ +		\
		__INIT_SIZE__ + __CODE_SIZE__ -		\
		__STARTUP_RUN__				; Size

; ------------------------------------------------------------------------
; Place the startup code in a special segment.

.segment       	"STARTUP"

; Forward control to the code in the "BOOT" segment

	jmp	__BSS_RUN__

; Avoid re-entrance of donelib. This is also the _exit entry

_exit:	ldx	#$02
:	lda	rvsave,x
	sta	$03F2,x
	dex
	bpl	:-

; Switch in LC bank 2 for R/O

	bit	$C080

; Call module destructors

	jsr	donelib

; Switch in ROM

	bit	$C082

; Back to DOS

	jmp	$03D0

; ------------------------------------------------------------------------
; The linker doesn't calculate where this code runs so it has to be PIC

.segment	"BOOT"

; Switch to 80 column mode

	.ifdef __APPLE2ENH__
	lda	#$0D
	jsr	$C300
	.endif

; Setup the stack at HIMEM

	lda    	$73
	ldx	$73+1
	sta	sp
       	stx	sp+1

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
		
; Switch in LC bank 2 for W/O

	bit	$C081
	bit	$C081

; Set source start

	lda	#<(__BSS_RUN__ + __BOOT_SIZE__ + __INIT_SIZE__)
	ldx	#>(__BSS_RUN__ + __BOOT_SIZE__ + __INIT_SIZE__)
	sta	$3C
	stx	$3D

; Set source end

	lda	#<(__BSS_RUN__ + __BOOT_SIZE__ + __INIT_SIZE__ + __CODE_SIZE__)
	ldx	#>(__BSS_RUN__ + __BOOT_SIZE__ + __INIT_SIZE__ + __CODE_SIZE__)
	sta	$3E
	stx	$3F

; Set destination

	lda	#<__CODE_RUN__
	ldx	#>__CODE_RUN__
	sta	$42
	stx	$43

; Reset index and call MOVE to relocate the "CODE" segment

	ldy	#$00
	jsr	$FE2C

; Set source start

	lda	#<(__BSS_RUN__ + __BOOT_SIZE__)
	ldx	#>(__BSS_RUN__ + __BOOT_SIZE__)
	sta	$3C
	stx	$3D

; Set source end

	lda	#<(__BSS_RUN__ + __BOOT_SIZE__ + __INIT_SIZE__)
	ldx	#>(__BSS_RUN__ + __BOOT_SIZE__ + __INIT_SIZE__)
	sta	$3E
	stx	$3F

; Set destination

	lda	#<__INIT_RUN__
	ldx	#>__INIT_RUN__
	sta	$42
	stx	$43

; Reset index and call MOVE to relocate the "INIT" segment

	ldy	#$00
	jsr	$FE2C

; Switch in LC bank 2 for R/O

	bit	$C080

; Forward control to code in the "INIT" segment

	jmp	init

; ------------------------------------------------------------------------
; The "INIT" segment will be overwritten by the heap

.segment	"INIT"

; Clear the BSS data (and thus overwrite the "BOOT" segment)

init:	jsr	zerobss

; Call module constructors

	jsr	initlib

; Push arguments and call main()

	jmp	callmain

; ------------------------------------------------------------------------
; Data

.data

rvsave:	.res	3
