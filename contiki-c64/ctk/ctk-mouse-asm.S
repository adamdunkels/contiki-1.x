
;---------------------------------------------------------------------
	;; The 1351 mouse code was taken from the CC65 libraries,
	;; written by Ullrich von Bassewitz 
;---------------------------------------------------------------------	

        .macpack        generic

	.import		_ctk_mouse_joyx, _ctk_mouse_joyy
	.import		_ctk_mouse_firebutton
	.export		_ctk_mouse_asm_irq
;---------------------------------------------------------------------
.bss
lastjoydy:	.res 1
lastjoydx:	.res 1
joydycnt:	.res 1
joydxcnt:	.res 1
OldValue:       .res    1               ; Temp for MoveCheck routine
NewValue:       .res    1               ; Temp for MoveCheck routine
YCorr:          .res    1               ; Correction for Y coordinate
	
OldPotX:        .res    1               ; Old hw counter values
OldPotY:        .res    1
 
SID_ADConv1     = $D419
SID_ADConv2     = $D41A
CIA1_PRA        = $DC00
CIA1_PRB        = $DC01
CIA1_DDRB       = $DC03	

mousebutton:	.res 1
joybutton:	.res 1		
;---------------------------------------------------------------------
.data
XMin:           .word    0               ; X1 value of bounding box
YMin:           .word    0               ; Y1 value of bounding box
XMax:           .word    319             ; X2 value of bounding box
YMax:           .word    199             ; Y2 value of bounding box
	
;---------------------------------------------------------------------
.code

;---------------------------------------------------------------------
joystick:	
	  lda $dc00
	  ldy #0
	  ldx #0
	  lsr
	  bcs nodey
	  dey
nodey:
	  lsr
	  bcs noiny
	  iny
noiny:
	  lsr
	  bcs nodex
	  dex
nodex:
	  lsr
	  bcs noinx
	  inx
noinx:
	  and #1
	  eor #1
	  sta joybutton

	  cpy lastjoydy
	  sty lastjoydy
	  bne noydy
	  tya
	  pha
	  inc joydycnt
	  lda joydycnt
          cmp #$1f
          bmi nostajoydycnt
          lda #$20
	  sta joydycnt
nostajoydycnt:
	  lsr
	  lsr
	  lsr
	  lsr
	  tay
	  pla
asldeyloop:
	  asl
	  dey
	  bpl asldeyloop
	  tay
	  jmp ydy
noydy:
	  lda #0
	  sta joydycnt
ydy:

	  tya
	  clc
	  adc _ctk_mouse_joyy

	  cmp #240
	  bcc :+
	  lda #0
:	
		
 	  cmp #200
 	  bcc :+
 	  lda #199		
:
	  sta _ctk_mouse_joyy
	
	  cpx lastjoydx
	  stx lastjoydx
	  bne noxdx
	  txa
	  pha
	  inc joydxcnt
	  lda joydxcnt
          cmp #$1f
          bmi nostajoydxcnt
          lda #$20
	  sta joydxcnt
nostajoydxcnt:
	  lsr
	  lsr
	  lsr
	  lsr
	  tax
	  pla
asldexloop:
	  asl
	  dex
	  bpl asldexloop
	  tax
	  jmp xdx
noxdx:
	  lda #0
	  sta joydxcnt
xdx:
	                                                                                       
	  txa
	  clc
	  adc _ctk_mouse_joyx
	  sta _ctk_mouse_joyx
	  php
	  lda #$ff
	  cpx #0
	  bmi nolda02
	  lda #0
nolda02:
	  plp
	  adc _ctk_mouse_joyx+1
	  sta _ctk_mouse_joyx+1
	  rts	                                                                                       
;---------------------------------------------------------------------
	;; Most of the mouse code is taken from the CC65 libraries written by
	;; Ullrich von Bassewitz
MoveCheck:
        sty     OldValue
        sta     NewValue
        ldx     #$00
 
        sub     OldValue                ; a = mod64 (new - old)
        and     #%01111111
        cmp     #%01000000              ; if (a > 0)
        bcs     @L1                     ;
        lsr     a                       ;   a /= 2;
        beq     @L2                     ;   if (a != 0)
        ldy     NewValue                ;     y = NewValue
        rts                             ;   return
 
@L1:    ora     #%11000000              ; else or in high order bits
        cmp     #$FF                    ; if (a != -1)
        beq     @L2
        sec
        ror     a                       ;   a /= 2
        dex                             ;   high byte = -1 (X = $FF)
        ldy     NewValue
        rts
                                                                               
@L2:    txa                             ; A = $00
        rts
                                                                               
;---------------------------------------------------------------------	
mouse:
        lda     SID_ADConv1             ; Get mouse X movement
        ldy     OldPotX
        jsr     MoveCheck               ; Calculate movement vector
        sty     OldPotX
 
; Calculate the new X coordinate (--> a/y)
 
        add     _ctk_mouse_joyx
        tay                             ; Remember low byte
        txa
        adc     _ctk_mouse_joyx+1
        tax
 
; Limit the X coordinate to the bounding box
 
        cpy     XMin
        sbc     XMin+1
        bpl     @L1
        ldy     XMin
        ldx     XMin+1
        jmp     @L2
@L1:    txa
 
        cpy     XMax
        sbc     XMax+1
        bmi     @L2
        ldy     XMax
        ldx     XMax+1
@L2:    sty     _ctk_mouse_joyx
        stx     _ctk_mouse_joyx+1
; Calculate the Y movement vector
 
        lda     SID_ADConv2             ; Get mouse Y movement
        ldy     OldPotY
        jsr     MoveCheck               ; Calculate movement
        sty     OldPotY
 
; Calculate the new Y coordinate (--> a/y)
 
        sta     OldValue
        lda     _ctk_mouse_joyy
        sub     OldValue
        tay
        stx     OldValue
        lda     _ctk_mouse_joyy+1
        sbc     OldValue
        tax
 
        cpy     YMin
        sbc     YMin+1
        bpl     @L3
        ldy     YMin
        ldx     YMin+1
        jmp     @L4
@L3:    txa
                                                                               
        cpy     YMax
        sbc     YMax+1
        bmi     @L4
        ldy     YMax
        ldx     YMax+1
@L4:    sty     _ctk_mouse_joyy
        stx     _ctk_mouse_joyy+1

	;; Get mouse button


	lda	#$11
	sta	CIA1_DDRB
	ldx	#0
	lda	CIA1_PRB
	lsr
	lsr
	lsr
	lsr
	sta $d020
	and	#$10
	bne	@NOLEFTB
	ldx	#1
@NOLEFTB:	
	stx	mousebutton
	
	lda	#0
	sta	CIA1_DDRB	
	rts	
;---------------------------------------------------------------------			
_ctk_mouse_asm_irq:
	jsr joystick
	; 	jsr mouse	

	lda mousebutton
	ora joybutton
	sta _ctk_mouse_firebutton
	
	  lda _ctk_mouse_joyy
	  clc
	  adc #$32
	  sta $d001
	  sta $d003	

	  lda _ctk_mouse_joyx
	  clc
	  adc #$18
	  sta $d000
	  sta $d002	
	  lda #0
	  adc _ctk_mouse_joyx+1
	  and #1
	  beq :+
	  lda #3
:		
	  sta $d010                                                                                       
	  jmp $ea31
