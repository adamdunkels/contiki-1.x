;; File: crt0-dsc.s

    .module crt0
	.globl	_main

	.area _HEADER (REL)
;;	.org 	0x100 ;; Start from address &100

init:
_main:
	;; Ordering of segments for the linker.
	.area	_HOME (REL)
	.area	_CODE (REL)
    .area   _GSINIT (REL)
    .area   _GSFINAL (REL)
        
	.area	_DATA (REL)
    .area   _BSS (REL)
    .area   _HEAP (REL)

   .area   _CODE (REL)
	
_exit::
	.area   _GSINIT (REL)
gsinit::	
    .area   _GSFINAL (REL)
