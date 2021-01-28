	.INCLUDE "macro.inc"

	.SYNTAX UNIFIED

	@ Background effect for the game over screen

	THUMB_FUNC_START sub_80211C8
sub_80211C8: @ 0x080211C8
	adds r2, r0, #0
	movs r0, #0x2e
	str r0, [r2, #0x34]
	subs r0, #0x88
	str r0, [r2, #0x38]
	adds r0, #0x4a
	str r0, [r2, #0x3c]
	subs r0, #0x25
	str r0, [r2, #0x40]
	adds r1, r2, #0
	adds r1, #0x64
	ldr r0, _080211F8  @ 0x000004D2
	strh r0, [r1]
	adds r1, #2
	ldr r0, _080211FC  @ 0x0000162E
	strh r0, [r1]
	adds r1, #2
	ldr r0, _08021200  @ 0x000018CA
	strh r0, [r1]
	adds r1, #2
	ldr r0, _08021204  @ 0x00002158
	strh r0, [r1]
	bx lr
	.align 2, 0
_080211F8: .4byte 0x000004D2
_080211FC: .4byte 0x0000162E
_08021200: .4byte 0x000018CA
_08021204: .4byte 0x00002158

	THUMB_FUNC_END sub_80211C8

	THUMB_FUNC_START sub_8021208
sub_8021208: @ 0x08021208
	push {r4, r5, lr}
	adds r3, r0, #0
	adds r2, r3, #0
	adds r2, #0x64
	ldr r1, [r3, #0x34]
	ldrh r0, [r2]
	adds r0, r0, r1
	strh r0, [r2]
	movs r0, #0x66
	adds r0, r0, r3
	mov ip, r0
	ldr r1, [r3, #0x38]
	ldrh r0, [r0]
	adds r0, r0, r1
	mov r1, ip
	strh r0, [r1]
	adds r4, r3, #0
	adds r4, #0x68
	ldr r1, [r3, #0x3c]
	ldrh r0, [r4]
	adds r0, r0, r1
	strh r0, [r4]
	adds r5, r3, #0
	adds r5, #0x6a
	ldr r1, [r3, #0x40]
	ldrh r0, [r5]
	adds r0, r0, r1
	strh r0, [r5]
	movs r3, #0
	ldrsh r1, [r2, r3]
	negs r1, r1
	lsls r1, r1, #8
	lsrs r1, r1, #0x10
	mov r0, ip
	movs r3, #0
	ldrsh r2, [r0, r3]
	negs r2, r2
	lsls r2, r2, #8
	lsrs r2, r2, #0x10
	movs r0, #2
	bl SetBgOffset
	movs r0, #0
	ldrsh r1, [r4, r0]
	negs r1, r1
	lsls r1, r1, #8
	lsrs r1, r1, #0x10
	movs r3, #0
	ldrsh r2, [r5, r3]
	negs r2, r2
	lsls r2, r2, #8
	lsrs r2, r2, #0x10
	movs r0, #3
	bl SetBgOffset
	pop {r4, r5}
	pop {r0}
	bx r0

	THUMB_FUNC_END sub_8021208

	THUMB_FUNC_START sub_802127C
sub_802127C: @ 0x0802127C
	push {lr}
	ldr r0, _080212B8  @ 0x04000006
	ldrh r0, [r0]
	adds r0, #1
	lsls r0, r0, #0x10
	lsrs r1, r0, #0x10
	cmp r1, #0xa0
	bls _0802128E
	movs r1, #0
_0802128E:
	cmp r1, #0x50
	bls _0802129A
	movs r0, #0xa0
	subs r0, r0, r1
	lsls r0, r0, #0x10
	lsrs r1, r0, #0x10
_0802129A:
	adds r0, r1, #0
	movs r1, #3
	bl __udivsi3
	lsls r0, r0, #0x10
	lsrs r1, r0, #0x10
	cmp r1, #0x10
	bls _080212AC
	movs r1, #0x10
_080212AC:
	ldr r0, _080212BC  @ 0x04000052
	strb r1, [r0]
	adds r0, #1
	strb r1, [r0]
	pop {r0}
	bx r0
	.align 2, 0
_080212B8: .4byte 0x04000006
_080212BC: .4byte 0x04000052

	THUMB_FUNC_END sub_802127C

	THUMB_FUNC_START sub_80212C0
sub_80212C0: @ 0x080212C0
	push {r4, r5, lr}
	sub sp, #4
	adds r5, r0, #0
	bl BMapDispSuspend
	movs r0, #0x3e
	movs r1, #0
	bl StartBgm
	ldr r3, _080213C8  @ gDispIo
	ldrb r2, [r3, #0xc]
	movs r1, #4
	negs r1, r1
	adds r0, r1, #0
	ands r0, r2
	strb r0, [r3, #0xc]
	ldrb r2, [r3, #0x10]
	adds r0, r1, #0
	ands r0, r2
	movs r2, #1
	orrs r0, r2
	strb r0, [r3, #0x10]
	ldrb r0, [r3, #0x14]
	ands r1, r0
	movs r0, #2
	orrs r1, r0
	strb r1, [r3, #0x14]
	ldrb r0, [r3, #0x18]
	movs r1, #3
	orrs r0, r1
	strb r0, [r3, #0x18]
	movs r0, #2
	movs r1, #0
	bl SetBgChrOffset
	movs r0, #3
	movs r1, #0
	bl SetBgChrOffset
	ldr r0, _080213CC  @ gUnknown_08A0AB0C
	ldr r1, _080213D0  @ 0x06001000
	bl Decompress
	ldr r0, _080213D4  @ gUnknown_08A0AE64
	movs r1, #0x80
	movs r2, #0x20
	bl ApplyPaletteExt
	ldr r0, _080213D8  @ gUnknown_08A07DD8
	ldr r1, _080213DC  @ 0x06002000
	bl Decompress
	ldr r0, _080213E0  @ gUnknown_08A0AE44
	movs r1, #0
	movs r2, #0x20
	bl ApplyPaletteExt
	movs r0, #0
	movs r1, #0
	movs r2, #0
	bl SetBgOffset
	bl ClearBg0Bg1
	ldr r0, _080213E4  @ gBg0Tm+0x24E
	ldr r1, _080213E8  @ gUnknown_08A0AE84
	movs r2, #0x80
	bl TmApplyTsa_t
	bl sub_801FEE8
	bl sub_801FE14
	movs r0, #0xc
	bl EnableBgSync
	ldr r0, _080213EC  @ sub_802127C
	bl SetOnHBlankA
	movs r0, #1
	movs r1, #0xe
	movs r2, #0xe
	movs r3, #0
	bl SetBlendConfig
	movs r4, #0
	str r4, [sp]
	movs r0, #0
	movs r1, #0
	movs r2, #1
	movs r3, #0
	bl SetBlendTargetA
	str r4, [sp]
	movs r0, #0
	movs r1, #0
	movs r2, #0
	movs r3, #1
	bl SetBlendTargetB
	bl sub_8001710
	ldr r4, _080213F0  @ gPal
	adds r0, r4, #0
	movs r1, #0
	movs r2, #1
	movs r3, #1
	bl sub_800172C
	adds r4, #0x80
	adds r0, r4, #0
	movs r1, #4
	movs r2, #1
	movs r3, #1
	bl sub_800172C
	adds r5, #0x4c
	movs r0, #0x15
	strh r0, [r5]
	movs r4, #9
_080213B0:
	bl sub_8000234_t
	subs r4, #1
	cmp r4, #0
	bge _080213B0
	bl EnablePalSync
	add sp, #4
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0
_080213C8: .4byte gDispIo
_080213CC: .4byte gUnknown_08A0AB0C
_080213D0: .4byte 0x06001000
_080213D4: .4byte gUnknown_08A0AE64
_080213D8: .4byte gUnknown_08A07DD8
_080213DC: .4byte 0x06002000
_080213E0: .4byte gUnknown_08A0AE44
_080213E4: .4byte gBg0Tm+0x24E
_080213E8: .4byte gUnknown_08A0AE84
_080213EC: .4byte sub_802127C
_080213F0: .4byte gPal

	THUMB_FUNC_END sub_80212C0

	THUMB_FUNC_START sub_80213F4
sub_80213F4: @ 0x080213F4
	push {r4, lr}
	adds r4, r0, #0
	bl GetGameTime
	movs r1, #7
	ands r1, r0
	cmp r1, #0
	bne _08021422
	bl sub_8000234_t
	bl EnablePalSync
	adds r1, r4, #0
	adds r1, #0x4c
	ldrh r0, [r1]
	subs r0, #1
	strh r0, [r1]
	lsls r0, r0, #0x10
	cmp r0, #0
	bge _08021422
	adds r0, r4, #0
	bl Proc_Break
_08021422:
	pop {r4}
	pop {r0}
	bx r0

	THUMB_FUNC_END sub_80213F4

	THUMB_FUNC_START sub_8021428
sub_8021428: @ 0x08021428
	adds r0, #0x4e
	ldr r1, _08021430  @ 0x000005DC
	strh r1, [r0]
	bx lr
	.align 2, 0
_08021430: .4byte 0x000005DC

	THUMB_FUNC_END sub_8021428

	THUMB_FUNC_START sub_8021434
sub_8021434: @ 0x08021434
	push {r4, lr}
	adds r4, r0, #0
	adds r1, r4, #0
	adds r1, #0x4e
	ldrh r0, [r1]
	subs r0, #1
	strh r0, [r1]
	lsls r0, r0, #0x10
	cmp r0, #0
	bge _08021450
	adds r0, r4, #0
	movs r1, #0x63
	bl Proc_Goto
_08021450:
	ldr r0, _0802146C  @ gKeySt
	ldr r0, [r0]
	ldrh r1, [r0, #8]
	movs r0, #0xb
	ands r0, r1
	cmp r0, #0
	beq _08021466
	adds r0, r4, #0
	movs r1, #0x63
	bl Proc_Goto
_08021466:
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_0802146C: .4byte gKeySt

	THUMB_FUNC_END sub_8021434

	THUMB_FUNC_START sub_8021470
sub_8021470: @ 0x08021470
	push {r4, r5, lr}
	bl sub_8001710
	ldr r4, _080214A4  @ gPal
	movs r5, #1
	negs r5, r5
	adds r0, r4, #0
	movs r1, #0
	movs r2, #1
	adds r3, r5, #0
	bl sub_800172C
	adds r4, #0x80
	adds r0, r4, #0
	movs r1, #4
	movs r2, #1
	adds r3, r5, #0
	bl sub_800172C
	movs r0, #4
	bl FadeBgmOut
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0
_080214A4: .4byte gPal

	THUMB_FUNC_END sub_8021470

	THUMB_FUNC_START sub_80214A8
sub_80214A8: @ 0x080214A8
	push {r4, lr}
	adds r4, r0, #0
	bl sub_8000234_t
	bl EnablePalSync
	adds r1, r4, #0
	adds r1, #0x4c
	ldrh r0, [r1]
	adds r0, #1
	strh r0, [r1]
	lsls r0, r0, #0x10
	asrs r0, r0, #0x10
	cmp r0, #0x20
	bne _080214CC
	adds r0, r4, #0
	bl Proc_Break
_080214CC:
	pop {r4}
	pop {r0}
	bx r0

	THUMB_FUNC_END sub_80214A8

	THUMB_FUNC_START sub_80214D4
sub_80214D4: @ 0x080214D4
	push {lr}
	movs r0, #0
	bl SetOnHBlankA
	movs r0, #0
	bl SetOnHBlankB
	ldr r2, _08021510  @ gDispIo
	ldrb r1, [r2, #1]
	movs r0, #2
	negs r0, r0
	ands r0, r1
	movs r1, #3
	negs r1, r1
	ands r0, r1
	subs r1, #2
	ands r0, r1
	subs r1, #4
	ands r0, r1
	subs r1, #8
	ands r0, r1
	strb r0, [r2, #1]
	ldr r1, _08021514  @ gPal
	movs r0, #0
	strh r0, [r1]
	bl EnablePalSync
	pop {r0}
	bx r0
	.align 2, 0
_08021510: .4byte gDispIo
_08021514: .4byte gPal

	THUMB_FUNC_END sub_80214D4

	THUMB_FUNC_START sub_8021518
sub_8021518: @ 0x08021518
	push {lr}
	adds r1, r0, #0
	cmp r1, #0
	beq _0802152C
	ldr r0, _08021528  @ gUnknown_0859B358
	bl SpawnProcLocking
	b _08021534
	.align 2, 0
_08021528: .4byte gUnknown_0859B358
_0802152C:
	ldr r0, _08021538  @ gUnknown_0859B358
	movs r1, #3
	bl SpawnProc
_08021534:
	pop {r0}
	bx r0
	.align 2, 0
_08021538: .4byte gUnknown_0859B358

	THUMB_FUNC_END sub_8021518

.align 2, 0 @ align with 0