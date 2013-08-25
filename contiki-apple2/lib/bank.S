;
; Bank switching code for Contiki (Apple2 version)
;

	.segment	"CONTIKI"

	.export		VTABZ

VTABZ:
	; Switch in ROM and call VTABZ
	bit	$C082
	jsr	$FC24

	; Switch in LC bank 2 for R/O and return
	bit	$C080
	rts

.ifdef  __APPLE2ENH__

	.code

main2aux:
	; Switch in $2000-$3FFF from aux memory
	bit	$C055		; Page2
	bit	$C057		; HiRes
	rts
	
aux2main:
	; Switch in $2000-$3FFF from main memory
	bit	$C056		; LoRes
	bit	$C054		; Page1
	rts

	.export		_tcpip_init
	.import		__tcpip_init

_tcpip_init:
	jsr	main2aux
	jsr	__tcpip_init
	jmp	aux2main
	
	.export		_tcpip_eventhandler
	.import		__tcpip_eventhandler

_tcpip_eventhandler:
	jsr	main2aux
	jsr	__tcpip_eventhandler
	jmp	aux2main
	
	.export		_tcpip_pollhandler
	.import		__tcpip_pollhandler

_tcpip_pollhandler:
	jsr	main2aux
	jsr	__tcpip_pollhandler
	jmp	aux2main
	
	.export		_htons
	.import		__htons

_htons:
	jsr	main2aux
	jsr	__htons
	jmp	aux2main
	
	.export		_uiplib_ipaddrconv
	.import		__uiplib_ipaddrconv

_uiplib_ipaddrconv:
	jsr	main2aux
	jsr	__uiplib_ipaddrconv
	jmp	aux2main
	
	.export		_tcp_markconn
	.import		__tcp_markconn

_tcp_markconn:
	jsr	main2aux
	jsr	__tcp_markconn
	jmp	aux2main

	.export		_tcp_listen
	.import		__tcp_listen

_tcp_listen:
	jsr	main2aux
	jsr	__tcp_listen
	jmp	aux2main

	.export		_tcp_unlisten
	.import		__tcp_unlisten

_tcp_unlisten:
	jsr	main2aux
	jsr	__tcp_unlisten
	jmp	aux2main

	.export		_tcp_connect
	.import		__tcp_connect

_tcp_connect:
	jsr	main2aux
	jsr	__tcp_connect
	jmp	aux2main

	.export		_udp_new
	.import		__udp_new

_udp_new:
	jsr	main2aux
	jsr	__udp_new
	jmp	aux2main

	.export		_tcpip_input
	.import		__tcpip_input

_tcpip_input:
	jsr	main2aux
	jsr	__tcpip_input
	jmp	aux2main

	.export		_tcpip_poll_udp
	.import		__tcpip_poll_udp

_tcpip_poll_udp:
	jsr	main2aux
	jsr	__tcpip_poll_udp
	jmp	aux2main
	
	.export		__ek_post_synch
	.import		_ek_post_synch

__ek_post_synch:
	jsr	aux2main
	jsr	_ek_post_synch
	jmp	main2aux

.endif
