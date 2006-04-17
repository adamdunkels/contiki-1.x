;; File: crt0-dsc.s

    .module crt0
	.area _HEADER (ABS)
	.area _HOME (REL)	
	.area _CODE (REL)
	;; relocation data
	.dw 0
     .area _DATA (REL)
    .area   _BSS (REL)
    .area   _HEAP (REL)

	.area   _GSINIT (REL)
gsinit::	
    .area   _GSFINAL (REL)
	ret
