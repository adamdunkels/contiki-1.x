;; Hires: 
;;
;; Mode 0, 40 columns. Each character is 4 pixels wide and 8 pixels tall.
;; Each character is two bytes wide and 8 lines tall.
;;
;; The graphics for each character is a bitmap defined using
;; pen 0 and pen 15.
;;
;; The bitmap is processed at runtime to convert the font to the 
;; appropiate pixel colours.
		.area _CODE
;; This table converts between pen index (0-15) and pixel bitmap.
.pen_table
defb #00	;; pen 0  (%00000000)
defb #c0	;; pen 1  (%11000000)
defb #0c	;; pen 2  (%00001100)
defb #cc	;; pen 3  (%11001100)
defb #30	;; pen 4  (%00110000)
defb #f0	;; pen 5  (%11110000)
defb #3c	;; pen 6  (%00111100)
defb #fc	;; pen 7  (%11111100)
defb #03	;; pen 8  (%00000011)
defb #c3	;; pen 9  (%11000011)
defb #0f	;; pen 10 (%00001111)
defb #cf	;; pen 11 (%11001111)
defb #33	;; pen 12 (%00110011)
defb #f3	;; pen 13 (%11110011)
defb #3f	;; pen 14 (%00111111)
defb #ff	;; pen 15 (%11111111)

;; A = pen
.get_pen_mask
ld hl,pen_table
add a,l
ld l,a
ld a,h
adc a,0
ld h,a
ld a,(hl)
ret

.set_pen
call get_pen_mask
ld (pen_mask+1),a
ld (pen_mask2+1),a
ret

.set_paper
call get_pen_mask
ld (paper_mask+1),a
ld (paper_mask2+1),a
ret

;; enter:
;; HL = current memory address
;; exit:
;; HL = memory address of byte immediatly below
;; AF corrupt.
.scr_next_line
ld a,h
add a,8
ld h,a
ret nc
ld a,l
add a,&50
ld l,a
ld a,h
adc a,&c0
ld h,a
ret

;; enter:
;; HL = screen address
;; DE = character pixel graphics
;; exit:
;; AF, BC, HL, DE corrupt
.plot_char
ld b,8
.pc1
ld a,(de)		;; convert 'on' pixels
.pen_mask and 1
ld c,a
ld a,(de)
cpl 
.paper_mask and 1
or c
ld (hl),a
inc l
ld a,(de)
.pen_mask2 and 1
ld c,a
ld a,(de)
cpl
.paper_mask2 and 1
or c
ld (hl),a
dec l
call scr_next_line
djnz pc1
ret

