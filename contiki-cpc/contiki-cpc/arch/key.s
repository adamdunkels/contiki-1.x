; unsigned char kbhit (void);
; Return true if there's a key waiting, return false if not 
; TESTED

.globl _kbhit
		.area _CODE
_kbhit::
		call	0xBB09	; KM READ KEY
		ld (key),a
		ld		l,#1
		ret		c
		ld		l,#0
		ret

; char cgetc (void);
; Return a character from the keyboard. If there is no character available,
; the functions waits until the user does press a key. If cursor is set to
; 1 (see below), a blinking cursor is displayed while waiting.
; TESTED

.globl _cgetc

_cgetc::	
		ld a,(key)
		ld l,a
		ret

;;		call	0xBB09
;;		jr nc,_cgetc
;;		ld		l,a
;;		ret

key:		.db 0
