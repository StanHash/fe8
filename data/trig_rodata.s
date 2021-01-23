	.section .rodata

	.align 2

	.global gSinLut
gSinLut:  @ 0x080D751C
	.incbin "baserom.gba", 0xD751C, 0x280
