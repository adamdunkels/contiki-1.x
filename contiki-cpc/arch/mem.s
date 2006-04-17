	.globl _calc_free_ram
	.globl _get_ram_start
	.globl _get_ram_end
	.globl __sdcc_heap_init

	.area _GSINIT
	call __sdcc_heap_init

	.area _CODE

_calc_free_ram::
	push bc
        ;; get information about BASIC ROM
        ld c,#0x0
        call  #0xb915
        cp #0x80
        jr nz,gfr
        ;; must be built in ROM

        ;; v1.0 (BASIC 1.0 in CPC464)
        ;; v1.1 (BASIC 1.1 in CPC664)
        ;; v1.2 (BASIC 1.1 in CPC6128 and KC Compact)
        ;; v1.4 (BASIC 1.1 in CPC+)

        ld a,h
        cp #0x1
        ret z
        ld a,l
        or a
        jr z,basic10       

        ;; basic 1.1       
  
        ld hl,(#0xb073)
        inc hl
        ld bc,(#0xb736)
        or a
        sbc hl,bc
        jr nz,after_himem11

        ld hl,(#0xae6c)
        ld (ram_start),hl

        ld hl,(#0xb071)
        ld (ram_end),hl
gfr:
	pop bc
        ret

after_himem11:

        ;; after himem
        ld hl,(#0xb073)
        inc hl
        ld (ram_start),hl
        
        ;; start of UDG
        ld hl,(#0xb736)
        dec hl
        ld (ram_end),hl
	pop bc
        ret


basic10:
        ld hl,(#0xb08f)
        inc hl
        ld bc,(#0xb296)
        or a
        sbc hl,bc
        jr nz,after_himem10

        ld hl,(#0xae89)
        ld (ram_start),hl

        ld hl,(#0xb08d)
        ld (ram_end),hl
	pop bc
        ret

after_himem10:

        ;; after himem
        ld hl,(#0xb08f)
        inc hl
        ld (ram_start),hl

        ;; start of UDG
        ld hl,(#0xb296)
        dec hl
        ld (ram_end),hl
	pop bc
        ret

_get_ram_start:: 
	ld hl,(ram_start)
	ret
_get_ram_end::
	ld hl,(ram_end)
	ret
	
ram_start:: 
	.dw 0
ram_end:: 
	.dw 0
