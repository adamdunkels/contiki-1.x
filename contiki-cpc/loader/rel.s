	.area _CODE
	.globl _get_file_length
	.globl _load_file
;;----------------------------------------------------------------------------
;; get length of file on disc. Assumption file has a AMSDOS header
;;
;; int get_file_length(const char *filename);

_get_file_length::
	ld hl,#2
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a

	;; HL = address of null terminated string
	call count_string_length
	ld de,#0x0c000		;; points to unused 2k buffer
	call 0x0bc77		;; cas in open
	push bc			;; BC = length of file
	call 0x0bc7d		;; cas in abandon
	pop hl
	ret

;;---------------------------------------------------------------------

count_string_length:
	push hl
	ld b,#0
csl:	ld a,(hl)
	or a
	jr z,csl2
	inc hl
	inc b
	jr csl
csl2:
	pop hl
	ret

;;---------------------------------------------------------------------------
;; void load_file(const char *filename, void *addr)

_load_file::
	ld hl,#5
	add hl,sp
	ld d,(hl)
	dec hl
	ld e,(hl)
	dec hl

	push de
	ld a,(hl)
	dec hl
	ld l,(hl)
	ld h,a

	call count_string_length
	ld de,#0x0c000
	call 0x0bc77		;; cas in open
	pop hl			;; load address
	call 0x0bc83		;; cas in direct
	call 0x0bc7a		;; cas in close
	ret

;; void relocate(void *addr,void *base)

;; IX = address of relocate data
_relocate::
	ld hl,#5
	add hl,sp
	push ix
	ld b,(hl)			;; base address
	dec hl
	ld c,(hl)
	dec hl
	ld a,(hl)
	.db #0x0dd
	ld h,a	
	dec hl
	ld a,(hl)
	.db #0x0dd
	ld l,a				;; IX is offset of table from start of loaded file
	add ix,bc			;; relocate IX to give absolute address of table.

	push bc
	pop hl
	call relocate_16bit
	push bc
	pop hl
	call relocate_8bitl	;; lower byte
	push bc
	pop hl
	call relocate_8bith	;; upper byte
	pop ix
	ret

;;--------------------------------------------------------------------------
;; Relocate 8-bit values (e.g. where low and high parts of an address
;; are loaded seperatly into registers)
;;
;; IX = list of 16-bit addresses. Each address identifies an 8-bit
;; value
;; 
relocate_8bith:
ld a,0(ix)
inc ix
or a
ret z
cp #0x0ff
jr nz,r8bh
ld e,0(ix)
inc ix
ld d,0(ix)
inc ix
add hl,de
jr relocate_8bith

r8bh:
;; add offset
add a,l
ld l,a
ld a,h
adc a,#0x0
ld h,a

;; get low byte of address
ld e,0(ix)
inc ix

;; get high byte to relocate
ld d,(hl)
ex de,hl
add hl,bc
ex de,hl
ld (hl),d
jr relocate_8bith

relocate_8bitl:
ld a,0(ix)
inc ix
or a
ret z
cp #0x0ff
jr nz,r8bl
ld e,0(ix)
inc ix
ld d,0(ix)
inc ix
add hl,de
jr relocate_8bitl

r8bl:
add a,l
ld l,a
ld a,h
adc a,#0x0
ld h,a

ld e,(hl)
ld d,#0x0
ex de,hl
add hl,bc
ex de,hl
ld (hl),e
jr relocate_8bitl

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
or a
ret z
cp #0x0ff
jr nz,r16
ld e,0(ix)
inc ix
ld d,0(ix)
inc ix
add hl,de
jr relocate_16bit

r16:
;; add offset
add a,l
ld l,a
ld a,h
adc a,#0x0
ld h,a

;; get the 16-bit value
ld e,(hl)
inc hl
ld d,(hl)


;; add base address; therefore relocating it.
ex de,hl
add hl,bc
ex de,hl
;; write relocated value
ld (hl),d
dec hl
ld (hl),e
jr relocate_16bit

