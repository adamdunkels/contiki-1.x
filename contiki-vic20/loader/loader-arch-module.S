.segment        "JUMPTABLE"
	.import _init
	.export _loader_arch_loadaddr
_loader_arch_loadaddr:
	jmp _init

