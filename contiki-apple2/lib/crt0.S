;
; Startup code for Contiki (Apple2 version)
;
; This must be the *first* file on the linker command line
;

	.export		_exit
	.import	   	zerobss, initlib, callmain, donelib
.ifdef  __APPLE2ENH__
	.import		_uip_buf, callirq
.endif
	.import	       	__STARTUP_RUN__			; Linker generated
	.import	       	__BSS_RUN__, __BSS_SIZE__	; Linker generated
	.import	       	__INIT_RUN__, __INIT_SIZE__	; Linker generated
	.import	       	__CODE_RUN__, __CODE_SIZE__	; Linker generated
	.import	       	__BOOT_SIZE__, __UIP_SIZE__	; Linker generated
	.importzp	sp
	.linecont	+

; ------------------------------------------------------------------------
; The executable header

.segment	"EXEHDR"

	.word  	__STARTUP_RUN__				; Start address
	.word  	__BSS_RUN__   + __BOOT_SIZE__ +		\
		__INIT_SIZE__ + __CODE_SIZE__ +		\
		__UIP_SIZE__ - __STARTUP_RUN__		; Size

; ------------------------------------------------------------------------
; Place the startup code in a special segment

.segment       	"STARTUP"

; Forward control to the code in the "BOOT" segment

	jmp	__BSS_RUN__

.ifdef  __APPLE2ENH__

; ProDOS TechRefMan, chapter 6.2:
; "Each installed routine must begin with a CLD instruction."

irq:	cld

; Switch in LC bank 2 for R/O

	bit	$C080

; Call interruptors

	jsr     callirq
	
; Switch in LC bank 1 for R/W

	bit	$C08B
	bit	$C08B

; Check for success

	bcc	:+

; ProDOS TechRefMan, chapter 6.2:
; "When the routine that can process the interrupt is called, it
;  should ... return (via an RTS) with the carry flag clear."

	clc
	rts

; ProDOS TechRefMan, chapter 6.2:
; "When a routine that cannot process the interrupt is called,
;  it should return (via an RTS) with the cary flag set ..."

:       sec
	rts

.endif

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

.ifdef  __APPLE2ENH__

; Deallocate interrupt vector table entry

	dec     intrpt		; Adjust parameter count
	jsr     $BF00
	.byte   $41		; Dealloc interrupt
	.addr   intrpt

.endif

; Switch in ROM

	bit	$C082

; Back to DOS

	jmp	$03D0

; ------------------------------------------------------------------------
; The linker doesn't understand that the "BOOT" segment overlays the "BSS"
; segment so the automatic fixup of references to the "BOOT" segment does
; not work. Therefore references to this segment have to be expressed as
; offsets to __BSS_RUN__.

.segment	"BOOT"

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
		
.ifdef  __APPLE2ENH__

; Allocate interrupt vector table entry

	jsr     $BF00
	.byte   $40		; Alloc interrupt
	.addr   intrpt

; Switch to 80 column mode here to show some progress

	lda	#$0D
	jsr	$C300
	
; Save potential command line arguments from being overwritten by
; slot-based realtime clock being called by ProDOS on file I/O

	ldx	#$7F
:	lda	$200,x
	sta	$100,x
	dex
	bpl	:-

; ProDOS TechRefMan, chapter 5.2.2.1:
; "... you may protect those areas of auxiliary memory. If you save a
;  dummy 8K file as the first entry in /RAM, it will always be saved
;  at $2000 to $3FFF."
;
; Relocate the "UIP" segment by writing it to /RAM as the first file

	jsr     $BF00
	.byte   $C0		; Create file
	.addr   create

	jsr     $BF00
	.byte   $C8		; Open file
	.addr   open
	
	lda	o_ref
	sta	w_ref
	sta	c_ref

	jsr     $BF00
	.byte   $CB		; Write file
	.addr   write

	jsr     $BF00
	.byte   $CC		; Close file
	.addr   close

; Restore potential command line arguments

	ldx	#$7F
:	lda	$100,x
	sta	$200,x
	dex
	bpl	:-

.endif

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
	
.ifdef  __APPLE2ENH__

create	= __BSS_RUN__ + *
	.byte	$07		; Parameter count
	.addr	name		; Pathname
	.byte	%11000011	; Access:	Standard full access
	.byte	$00		; File type:	Typeless file
	.word	$0000		; Aux type:	N/A
	.byte	$01		; Storage type:	Standard seedling file
	.word	$0000		; Create date:	Current date
	.word	$0000		; Create time:	Current time

open	= __BSS_RUN__ + *
	.byte	$03		; Parameter count
	.addr	name		; Pathname
	.addr	_uip_buf	; IO buffer
o_ref	= __BSS_RUN__ + *
	.byte	$00		; Reference number

write	= __BSS_RUN__ + *
	.byte	$04		; Parameter count
w_ref	= __BSS_RUN__ + *
	.byte	$00		; Reference number
	.addr	__BSS_RUN__ + \
		__BOOT_SIZE__ + \
		__INIT_SIZE__ + \
		__CODE_SIZE__	; Data buffer
	.word	__UIP_SIZE__	; Requested count
	.word	$0000		; Transfered count

close	= __BSS_RUN__ + *
	.byte	$01		; Parameter count
c_ref	= __BSS_RUN__ + *
	.byte	$00		; Reference number

name	= __BSS_RUN__ + *
	.byte	14
	.byte	"/RAM/CONTIKI.1"

.endif

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

.ifdef  __APPLE2ENH__

intrpt: .byte   $02		; Parameter count
	.byte   $00		; Interrupt number
	.addr   irq		; Interrupt handler

.endif