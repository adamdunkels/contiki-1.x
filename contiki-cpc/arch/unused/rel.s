	.globl _relocate
	.area _CODE

;; IX = address of relocate data
_relocate::
	call relocate_16bit
	call relocate_8bit	;; lower byte
	ld c,b
	call relocate_8bit	;; upper byte
	ret

;;--------------------------------------------------------------------------
;; Relocate 8-bit values (e.g. where low and high parts of an address
;; are loaded seperatly into registers)
;;
;; IX = list of 16-bit addresses. Each address identifies an 8-bit
;; value
;; 
relocate_8bit:
	ld a,0(ix)
	inc ix
rel8bit: push af
	ld e,0(ix)
	inc ix
	ld d,0(ix)
inc ix

ld a,(de)
add c
ld (de),a
pop af
dec a
jr nz,rel8bit
ret


;;--------------------------------------------------------------------------
;; Relocate 16-bit values
;;
;; Entry conditions:
;;
;; IX = list of 16-bit addresses. Each address identifies a 16-bit 
;; value to relocate. 
;;
;; BC = base address
;; 
;; NOTE: 
;; - Relocatable 16-bit values come from CALL and JP instructions and
;; loading a 16-bit register.

relocate_16bit:
ld a,0(ix)		;; number of items to relocate
inc ix

rel16bit:
push af
;; get address of 16-bit value to relocate
ld e,0(ix)
inc ix
ld d,0(ix)
inc ix

;; get the 16-bit value
ld a,(de)
ld l,a
inc de
ld a,(de)
ld h,a

;; add base address; therefore relocating it.
add hl,bc

;; write relocated value
ld a,h
ld (de),a
dec de
ld a,l
ld (de),a
pop af
dec a
jr nz,rel16bit
ret

;;--------------------------------------------------------------------------

	.area	_DATA
 	.area   _BSS
	.area   _GSINIT
        .area   _GSFINAL

_relocate_data::
