;; File: crt0.s
;; Generic crt0.s for a Z80
;; From SDCC..
;; Modified to suit execution on the Amstrad CPC!
;; by H. Hansen 2003

    .module crt0
	.globl	_main
	.globl _progend
	.globl __jpbc
	.area	_HEADER (ABS)
	;; Ordering of segments for the linker.
	.area	_CODE
init:

;; Initialise global variables
    call    gsinit
	call	_main

_exit::
	ret
__jpbc:
	push bc
	ret

	.area	_DATA (REL)
	 .area   _BSS (REL)
	
	.area   _GSINIT (REL)
gsinit::	

    .area   _GSFINAL (REL)
    ret
    	.area   _HEAP (REL)
_progend::
