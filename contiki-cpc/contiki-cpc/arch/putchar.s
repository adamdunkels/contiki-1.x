;; File: putchar.s
;; Modified to suit execution on the Amstrad CPC
;; by H. Hansen 2003
;; Original lines has been marked out!

		.area _CODE
_putchar::       
_putchar_rr_s:: 
        ld      hl,#2
        add     hl,sp
        
        ld      a,(hl)
;;      ld      a,#1
;;      rst     0x08
        call	0xBB5A
        ret
           
_putchar_rr_dbs::

;;      ld      l,e
;;		ld		a,#1
;;      rst     0x08

        ld		a,e
        call	0xBB5A
        ret
			