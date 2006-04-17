;; File: crt0.s
;; Generic crt0.s for a Z80
;; From SDCC..
;; Modified to suit execution on the Amstrad CPC!
;; by H. Hansen 2003

    .module crt0
	.globl	_main
	.globl _progend
	.area _HEADER (ABS)
	.org 	0x100 ;; Start from address &100
init:

;; Initialise global variables
    call    gsinit
	call	_main

_exit::
	ret

	;; Ordering of segments for the linker.
	.area	_HOME
	.area	_CODE
	.area	_DATA
	 .area   _BSS
	
	.area   _GSINIT
gsinit::	

    .area   _GSFINAL
    ret
    	.area   _HEAP
_progend::
