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
	
; Switch in $2000-$3FFF from aux memory

	lda	$C018		; 80Store ?
	sta	save80
	sta	$C001		; 80Store On
	
	bit	$C055		; Page2
	bit	$C057		; HiRes

; Call the cleanup code in the "UIP" segment

	jsr	cleanup

; Switch in $2000-$3FFF from main memory

	bit	$C056		; LoRes
	bit	$C054		; Page1
	
	lda	save80
	bmi	:+
	sta	$C000		; 80Store Off

.endif

; Switch in ROM

:	bit	$C082

; Back to DOS

	jmp	$03D0

.ifdef  __APPLE2ENH__
; ------------------------------------------------------------------------
; There's space left in the "UIP" segment so we move all cleanup code here

.segment	"UIP"

.proc	cleanup

; Deallocate interrupt vector table entry

	dec     intrpt		; Adjust parameter count
	jsr     $BF00
	.byte   $41		; Dealloc interrupt
	.addr   intrpt

; Save last used device number

	lda	$BF30		; DEVNUM
	sta	devnum

; Read ProDOS dispatcher from /RAM

	jsr     $BF00
	.byte   $C8		; Open file
	.addr   open
	
	lda	o_ref
	sta	r_ref
	sta	c_ref

	jsr     $BF00
	.byte   $CA		; Read file
	.addr   read

	jsr     $BF00
	.byte   $CC		; Close file
	.addr   close
	
; Destroy /RAM files

	jsr     $BF00
	.byte   $C1		; Destroy file
	.addr   destr

	dec	n_ext		; CONTIKI.2 -> CONTIKI.1

	jsr     $BF00
	.byte   $C1		; Destroy file
	.addr   destr

; Restore last used device number

	lda	devnum
	sta	$BF30		; DEVNUM

; Switch in LC bank 2 for W/O

	bit	$C081
	bit	$C081

; Set source start

	lda	#<__BSS_RUN__
	ldx	#>__BSS_RUN__
	sta	$3C
	stx	$3D

; Set source end

	lda	#<(__BSS_RUN__ + $0300)
	ldx	#>(__BSS_RUN__ + $0300)
	sta	$3E
	stx	$3F

; Set destination

	lda	#<$D100
	ldx	#>$D100
	sta	$42
	stx	$43

; Reset index and call MOVE to copy ProDOS dispatcher into LC

	ldy	#$00
	jmp	$FE2C

open:	.byte	$03		; Parameter count
	.addr	name		; Pathname
	.addr	_uip_buf	; IO buffer
o_ref:	.byte	$00		; Reference number

read:	.byte	$04		; Parameter count
r_ref:	.byte	$00		; Reference number
	.addr	__BSS_RUN__	; Data buffer
	.word	$0300		; Requested count
	.word	$0000		; Transfered count

close:	.byte	$01		; Parameter count
c_ref:	.byte	$00		; Reference number

destr:	.byte	$01		; Parameter count
	.addr	name		; Pathname

name:	.byte	.strlen("/RAM/CONTIKI.2")
	.byte	"/RAM/CONTIKI."
n_ext:	.byte	'2'

devnum:	.res	1

.endproc
.endif

; ------------------------------------------------------------------------
; The linker doesn't understand that the "BOOT" segment overlays the "BSS"
; segment so the automatic fixup of references to the "BOOT" segment does
; not work. Therefore references to this segment have to be expressed as
; offsets to __BSS_RUN__.

.segment	"BOOT"

.ifdef  __APPLE2ENH__

; Save potential command line arguments from being overwritten by
; a slot-based realtime clock being called by ProDOS on file I/O

	ldx	#$7F
:	lda	$0200,x
	sta	$0100,x
	dex
	bpl	:-

; ProDOS TechRefMan, chapter 5.2.2.1:
; "... you may protect those areas of auxiliary memory. If you save a
;  dummy 8K file as the first entry in /RAM, it will always be saved
;  at $2000 to $3FFF."
;
; Read /RAM directory

	jsr     $BF00
	.byte   $C8		; Open file
	.addr   open
	
	lda	o_ref
	sta	r_ref
	sta	c_ref

	jsr     $BF00
	.byte   $CA		; Read file
	.addr   read

	jsr     $BF00
	.byte   $CC		; Close file
	.addr   close
	
; ProDOS TechRefMan, chapter B.2.5:
; "FileCount := ThisBlock[$25] + (256 * ThisBlock[$26]);"
;
; Make sure /RAM is empty

	lda	END + $25
	ora	END + $26
	beq	:++
	
	ldx	#.strlen("Fatal error: /RAM not empty !")
:	lda	error,x
	jsr	$FDED		; COUT
	dex
	bpl     :-
	
	jsr	$FD0C		; RDKEY
	jmp	$03D0		; DOSWARM

; Switch to 80 column mode here to show some progress

:	lda	#$0D
	jsr	$C300
	
; Relocate the "UIP" segment by writing it to /RAM as the first file

	lda	#.strlen("/RAM/CONTIKI.1")
	sta	name
	jsr	ramfile
	
; Switch in LC bank 2 for R/O

	bit	$C080

; Copy ProDOS dispatcher out of LC

	ldx	#$00
l_mod	= __BSS_RUN__ + *
:	lda	$D100,x
s_mod	= __BSS_RUN__ + *
	sta	UIP,x
	inx
	bne	:-
	inc	l_mod+2
	inc	s_mod+2
	lda	l_mod+2
	cmp	#>$D400
	bne	:-

; Write ProDOS dispatcher to /RAM

	lda	#>$0300
	sta	w_cnt+1
	inc	n_ext		; CONTIKI.1 -> CONTIKI.2
	jsr	ramfile
	
; Restore potential command line arguments

	ldx	#$7F
:	lda	$0100,x
	sta	$0200,x
	dex
	bpl	:-

; Allocate interrupt vector table entry

	jsr     $BF00
	.byte   $40		; Alloc interrupt
	.addr   intrpt

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
	
.ifdef  __APPLE2ENH__

; Create a temporary file in /RAM

ramfile	= __BSS_RUN__ + *
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
	
	rts

UIP	= __BSS_RUN__ + __BOOT_SIZE__ + __INIT_SIZE__ + __CODE_SIZE__
END	= __BSS_RUN__ + __BOOT_SIZE__ + __INIT_SIZE__ + __CODE_SIZE__ + __UIP_SIZE__

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

read	= __BSS_RUN__ + *
	.byte	$04		; Parameter count
r_ref	= __BSS_RUN__ + *
	.byte	$00		; Reference number
	.addr	END		; Data buffer
	.word	$0200		; Requested count
	.word	$0000		; Transfered count

write	= __BSS_RUN__ + *
	.byte	$04		; Parameter count
w_ref	= __BSS_RUN__ + *
	.byte	$00		; Reference number
	.addr	UIP		; Data buffer
w_cnt	= __BSS_RUN__ + *
	.word	$2000		; Requested count
	.word	$0000		; Transfered count

close	= __BSS_RUN__ + *
	.byte	$01		; Parameter count
c_ref	= __BSS_RUN__ + *
	.byte	$00		; Reference number

name	= __BSS_RUN__ + *
	.byte	.strlen("/RAM")
	.byte	"/RAM/CONTIKI."
n_ext	= __BSS_RUN__ + *
	.byte	'1'

error	= __BSS_RUN__ + *
	.byte	$8D,     '!'|$80, ' '|$80, 'y'|$80, 't'|$80, 'p'|$80
	.byte	'm'|$80, 'e'|$80, ' '|$80, 't'|$80, 'o'|$80, 'n'|$80
	.byte	' '|$80, 'M'|$80, 'A'|$80, 'R'|$80, '\'|$80, ' '|$80
	.byte	':'|$80, 'r'|$80, 'o'|$80, 'r'|$80, 'r'|$80, 'e'|$80
	.byte	' '|$80, 'l'|$80, 'a'|$80, 't'|$80, 'a'|$80, 'F'|$80

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

save80:	.res	1

intrpt: .byte   $02		; Parameter count
	.byte   $00		; Interrupt number
	.addr   irq		; Interrupt handler

.endif