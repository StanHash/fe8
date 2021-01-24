    .INCLUDE "macro.inc"

    .SYNTAX unified

	THUMB_FUNC_START ClearOam_t
ClearOam_t: @ 0x080D7498
	bx pc
	nop
	.ARM
	b ClearOam

	THUMB_FUNC_END ClearOam_t

	THUMB_FUNC_START TmApplyTsa_t
TmApplyTsa_t: @ 0x080D74A0
	bx pc
	nop
	.ARM
	b TmApplyTsa

	THUMB_FUNC_END TmApplyTsa_t

	THUMB_FUNC_START TmFillRect_t
TmFillRect_t: @ 0x080D74A8
	bx pc
	nop
	.ARM
	b TmFillRect

	THUMB_FUNC_END TmFillRect_t

	THUMB_FUNC_START sub_8000234_t
sub_8000234_t: @ 0x080D74B0
	bx pc
	nop
	.ARM
	b sub_8000234

	THUMB_FUNC_END sub_8000234_t

	THUMB_FUNC_START TmCopyRect_t
TmCopyRect_t: @ 0x080D74B8
	bx pc
	nop
	.ARM
	b TmCopyRect

	THUMB_FUNC_END TmCopyRect_t

	THUMB_FUNC_START GetChecksum32_t
GetChecksum32_t: @ 0x080D74C0
	bx pc
	nop
	.ARM
	b GetChecksum32

	THUMB_FUNC_END GetChecksum32_t

	.align 2, 0
