.segment        "JUMPTABLE"
	.import _init
	.export _loader_arch_loadaddr
_loader_arch_loadaddr:
arch:		.byte 0,0,0,0,0,0,0,0
version:	.byte 0,0,0,0,0,0,0,0	
	jmp _init

