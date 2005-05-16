;
; Clock tick code for Contiki (Apple2 version)
;

        .constructor    inittick
	.export		_tick

	.segment	"INIT"

inittick:
	; Switch in ROM
	bit	$C082

	; Check for IIgs
	sec
	jsr     $FE1F
	bcs     done

	; Check for fast speed
	bit	$C036
	bpl	done

	; Adjust tick (5/14 = 1.0MHz/2.8MHz)
	lda	#5
	sta	_tick

	; Switch in LC bank 2 for R/O
done:	bit	$C080

	rts

        .data

_tick:	.byte	14
