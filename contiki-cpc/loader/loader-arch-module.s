;; This is the header for Contiki program files
;;
;; This must always be at at the start of the file
;; The order of the data must not change.
;;
.globl _loader_appinit
		.area _CODE

_loader_arch_loadaddr::
arch:		.byte	0,0,0,0,0,1,1,1
version:	.byte	0,0,0,0,0,1,1,1

;; The position of the init function can be anywhere in the file.
;; This jump is always in the same place, and points to the actual init function.
	call gsinit
	jp _loader_appinit
