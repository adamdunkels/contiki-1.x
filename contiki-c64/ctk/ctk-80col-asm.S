
;---------------------------------------------------------------------       
	.importzp       ptr1, ptr2, ptr3, ptr4, sreg
	.importzp	sp

	.import		incsp2

	.import		_ctk_80col_lefttab
	.import		_ctk_80col_righttab
	
	.import		_ctk_80col_cursx
	.import		_ctk_80col_cursy
	.import		_ctk_80col_color
	.import		_ctk_80col_underline
	.import		_ctk_80col_reversed

	.import		_ctk_80col_yhiresaddr

	.import		_ctk_80col_windowparams
	
	.import		_ctk_80col_bitmapptr

	.import		_ctk_80col_theme
			
	.export		_ctk_80col_cclear
	.export		_ctk_80col_chline
	.export		_ctk_80col_cputc
	.export		_ctk_80col_cputsn
	
	.export		_ctk_80col_clear_line
	.export		_ctk_80col_draw_bitmapline

;---------------------------------------------------------------------

ptr5 = sreg

	;; Offsets in the theme struct

	ulcornerpattern =	8
	titlebarpattern =	ulcornerpattern+8
	urcornerpattern =	titlebarpattern+8
	rborderpattern  =	urcornerpattern+8
	lrcornerpattern =	rborderpattern+8
	lowerborderpattern =	lrcornerpattern+8
	llcornerpattern =	lowerborderpattern+8
	lborderpattern =	llcornerpattern+8

	
	buttonleftpattern =	lborderpattern+8
	buttonrightpattern =	buttonleftpattern+8

				
.bss
tmp01:	.res 1
tmpl:	.res 1
tmpf:	.res 1
tmpc:	.res 1
tmpw:	.res 1	
tmpptr:	.res 2

tmpy:	.res 1
tmph:	.res 1

	;; Window border parameters	
w:	.res 1
wpw          = 0
h:	.res 1
wph          = 1	
clipy1:	.res 1
wpclipy1     = 2
clipy2:	.res 1
wpclipy2     = 3	
color1:	.res 1
wpcolor1     = 4	
color2:	.res 1
wpcolor2     = 5	
titlelen:	.res 1
wptitlelen   = 6
title:	.res 2
wptitle      = 7	
	
;---------------------------------------------------------------------
.data
;---------------------------------------------------------------------
.code
setupptrs:	
	;; Find hires address.	
	lda _ctk_80col_cursy
	asl
	tax
	lda _ctk_80col_yhiresaddr,x
	sta ptr2
	lda _ctk_80col_yhiresaddr+1,x
	sta ptr2+1

	;; Add X coordinate to the hires address. 
	lda #0
	sta ptr1+1
	lda _ctk_80col_cursx
	asl
	rol ptr1+1
; 	asl	
; 	rol ptr1+1	
	asl
	rol ptr1+1
	clc
	adc ptr2
	and #$f8
	sta ptr2
	lda ptr2+1
	adc ptr1+1
	sta ptr2+1


	;; Turn off interrupts, prepare $01 to store color data in RAM
	;; under I/O area.		
	sei
	lda $01
	sta tmp01
	and #$f8
	sta $01
	rts
;---------------------------------------------------------------------	
restore01:	
	lda tmp01
	sta $01
	cli
	rts		
;---------------------------------------------------------------------
_ctk_80col_cclear:
	sta tmpl
	cmp #0
	bne @norts
	rts
@norts:		
		
	jsr setupptrs		

	;; Store either $00 or $ff in hires memory, depending on the
	;; reversed flag		
	
	ldx tmpl
	lda _ctk_80col_reversed
	beq @noff
	lda #$ff
@noff:		
	sta tmpf

		
@loop1:
	ldy #7
	lda tmpf
@loop2:
	sta (ptr2),y
	dey
	bpl @loop2

	lda ptr2
	clc
	adc #8
	sta ptr2
	bcc @noinc
	inc ptr2+1
@noinc:		

	inc _ctk_80col_cursx

	dex
	bne @loop1

	jmp restore01
;---------------------------------------------------------------------
_ctk_80col_chline:
	sta tmpl
	cmp #0
	bne @norts
	rts
@norts:	
			
	jsr setupptrs		

	;; Store horizontal line pattern in hires memory
	ldx tmpl
@loop1:
	ldy #0
	lda #$00
	sta (ptr2),y
	iny
	lda #$00
	sta (ptr2),y
	iny
	lda #$00
	sta (ptr2),y
	iny
	lda #$ff
	sta (ptr2),y
	iny
	lda #$ff
	sta (ptr2),y
	iny
	lda #$00
	sta (ptr2),y
	iny
	lda #$00
	sta (ptr2),y
	iny
	lda #$00
	sta (ptr2),y


	lda ptr2
	clc
	adc #8
	sta ptr2
	bcc @noinc
	inc ptr2+1
@noinc:	

	inc _ctk_80col_cursx

	dex
	bne @loop1

	jmp restore01
;---------------------------------------------------------------------	
_ctk_80col_cputc:
	sta tmpc

	jsr setupptrs
	
	;; Turn on font ROM
	lda tmp01
	and #$f8
	ora #$01
	sta $01

	lda #0
	sta ptr1+1		  
	
	;; Convert from PETSCII to screen encoding
	lda tmpc
	eor #$e0
	clc
	adc #$20
	bpl @cont
	adc #$40
	bpl @cont
	eor #$a0
@cont:
	ldx _ctk_80col_reversed
	beq :+
	eor #$80
:		

	;; Convert to address in character ROM
	asl
	rol ptr1+1
	asl
	rol ptr1+1
	asl
	rol ptr1+1
	sta ptr1
	lda #$d8
	adc ptr1+1
	sta ptr1+1

	lda _ctk_80col_cursx
	and #1
	beq @left

	;; Print on hires screen, bottom-up. If we should underline,
	;; we do this by eor:ing the bottom line with #$ff
	ldy #7
	lda (ptr1),y
	ldx _ctk_80col_underline
	beq @rightnounderline
	eor #$ff
@rightnounderline:
	tax
	lda (ptr2),y
	and #$f0
	ora _ctk_80col_righttab,x	
 	sta (ptr2),y
	dey
@rightloop:
	lda (ptr1),y
	tax
	lda (ptr2),y
	and #$f0
	ora _ctk_80col_righttab,x	
 	sta (ptr2),y
	dey
	bpl @rightloop	
	
	inc _ctk_80col_cursx	
	
	jmp restore01		
	
@left:		
	;; Print on hires screen, bottom-up. If we should underline,
	;; we do this by eor:ing the bottom line with #$ff
	ldy #7
	lda (ptr1),y
	ldx _ctk_80col_underline
	beq @nounderline
	eor #$ff
@nounderline:
	tax
	lda (ptr2),y
	and #$0f
	ora _ctk_80col_lefttab,x	
 	sta (ptr2),y
	dey
@loop:
	lda (ptr1),y
	tax
	lda (ptr2),y
	and #$0f
	ora _ctk_80col_lefttab,x	
 	sta (ptr2),y
	dey
	bpl @loop

	
	
	inc _ctk_80col_cursx	
	
	jmp restore01		
;---------------------------------------------------------------------
_ctk_80col_cputsn:
	sta tmpl
	cmp #0
	bne :+
	rts
:		

	jsr setupptrs
	
	;; Turn on font ROM
	lda tmp01
	and #$f8
	ora #$01
	sta $01

	;; Output each character from the string
	lda #0
	sta tmpc
	ldy #0
	lda (sp),y
	sta ptr3
	iny
	lda (sp),y
	sta ptr3+1	
	
@loop1:		

	
	lda #0
	sta ptr1+1		  	

	ldy tmpc
	lda (ptr3),y
	bne :+
	jmp @end
:	
			
	;; Convert from PETSCII to screen encoding
	eor #$e0
	clc
	adc #$20
	bpl @cont
	adc #$40
	bpl @cont
	eor #$a0
@cont:
	ldx _ctk_80col_reversed
	beq :+
	eor #$80
:		

	;; Convert to address in character ROM
	asl
	rol ptr1+1
	asl
	rol ptr1+1
	asl
	rol ptr1+1
	sta ptr1
	lda #$d8
	adc ptr1+1
	sta ptr1+1

	lda _ctk_80col_cursx
	and #1
	beq @left

	;; Print on hires screen, bottom-up. If we should underline,
	;; we do this by eor:ing the bottom line with #$ff
	ldy #7
	lda (ptr1),y
	ldx _ctk_80col_underline
	beq @rightnounderline
	eor #$ff
@rightnounderline:
	tax
	lda (ptr2),y
	and #$f0
	ora _ctk_80col_righttab,x	
 	sta (ptr2),y
	dey
@rightloop:
	lda (ptr1),y
	tax
	lda (ptr2),y
	and #$f0
	ora _ctk_80col_righttab,x	
 	sta (ptr2),y
	dey
	bpl @rightloop	


	lda ptr2
	clc
	adc #8
	sta ptr2
	bcc :+
	inc ptr2+1
:	
	
	inc _ctk_80col_cursx	
	
	jmp @loopend
	
@left:		
	;; Print on hires screen, bottom-up. If we should underline,
	;; we do this by eor:ing the bottom line with #$ff
	ldy #7
	lda (ptr1),y
	ldx _ctk_80col_underline
	beq @nounderline
	eor #$ff
@nounderline:
	tax
	lda (ptr2),y
	and #$0f
	ora _ctk_80col_lefttab,x	
 	sta (ptr2),y
	dey
@loop:
	lda (ptr1),y
	tax
	lda (ptr2),y
	and #$0f
	ora _ctk_80col_lefttab,x	
 	sta (ptr2),y
	dey
	bpl @loop
			
	inc _ctk_80col_cursx	
@loopend:
	inc tmpc
	lda tmpc
	cmp tmpl
	beq :+
	jmp @loop1
:	
	
@end:
	jsr restore01
	
        jmp incsp2	
;---------------------------------------------------------------------
_ctk_80col_clear_line:
	asl
	tax
	lda _ctk_80col_yhiresaddr,x
	sta ptr2
	lda _ctk_80col_yhiresaddr+1,x
	sta ptr2+1
	
	sei
	lda $01
	sta tmp01
	and #$f8
	sta $01

	ldy #0
:	
	lda #$55
	sta (ptr2),y
	iny
	lda #$aa
	sta (ptr2),y
	iny
	bne :-

	inc ptr2+1
	
:	
	lda #$55
	sta (ptr2),y
	iny
	lda #$aa
	sta (ptr2),y
	iny
	cpy #$40
	bne :-
		
	
	jmp restore01	
;---------------------------------------------------------------------	
_ctk_80col_draw_bitmapline:
	sta tmpl
	cmp #0
	bne :+
	rts
:		

	jsr setupptrs
	
	ldy #0	
	lda _ctk_80col_color
:	
	;  	sta (ptr1),y	
	iny
	cpy tmpl
	bne :-

	lda _ctk_80col_bitmapptr
	sta ptr1
	lda _ctk_80col_bitmapptr+1
	sta ptr1+1
	
	ldy #0
	ldx #0
:		
	lda (ptr1),y
 	sta (ptr2),y		
	iny
	inx
	cpx #8
	bne :-

	ldx #0
	
	dec tmpl
	bne :-
	
	jmp restore01	
;---------------------------------------------------------------------
	