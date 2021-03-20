	.INCLUDE "macro.inc"

	.SYNTAX UNIFIED

	@ Computer/AI Phase (and berserk) action performing & display proc(s) (I think)

	THUMB_FUNC_START AiTargetCursor_Main
AiTargetCursor_Main: @ 0x08039E88
	push {r4, lr}
	adds r4, r0, #0
	ldr r0, [r4, #0x2c]
	ldr r1, [r4, #0x30]
	ldr r2, [r4, #0x58]
	bl DisplayCursor
	ldr r0, _08039EC8  @ gKeySt
	ldr r0, [r0]
	ldrh r1, [r0, #4]
	movs r0, #9
	ands r0, r1
	cmp r0, #0
	bne _08039EB2
	adds r0, r4, #0
	adds r0, #0x64
	movs r2, #0
	ldrsh r1, [r0, r2]
	adds r2, r0, #0
	cmp r1, #0x2d
	ble _08039EBC
_08039EB2:
	adds r0, r4, #0
	bl Proc_Break
	adds r2, r4, #0
	adds r2, #0x64
_08039EBC:
	ldrh r0, [r2]
	adds r0, #1
	strh r0, [r2]
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_08039EC8: .4byte gKeySt

	THUMB_FUNC_END AiTargetCursor_Main

	THUMB_FUNC_START StartAiTargetCursor
StartAiTargetCursor: @ 0x08039ECC
	push {r4, r5, r6, lr}
	adds r4, r0, #0
	adds r5, r1, #0
	adds r6, r2, #0
	adds r1, r3, #0
	ldr r0, _08039EF0  @ gUnknown_085A8004
	bl SpawnProcLocking
	str r4, [r0, #0x2c]
	str r5, [r0, #0x30]
	str r6, [r0, #0x58]
	adds r0, #0x64
	movs r1, #0
	strh r1, [r0]
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.align 2, 0
_08039EF0: .4byte gUnknown_085A8004

	THUMB_FUNC_END StartAiTargetCursor

	THUMB_FUNC_START CpPerform_UpdateMapMusic
CpPerform_UpdateMapMusic: @ 0x08039EF4
	push {lr}
	ldr r0, _08039F08  @ ProcScr_DelaySong
	bl FindProc
	cmp r0, #0
	bne _08039F04
	bl UpdatePlayMapMusic
_08039F04:
	pop {r0}
	bx r0
	.align 2, 0
_08039F08: .4byte ProcScr_DelaySong

	THUMB_FUNC_END CpPerform_UpdateMapMusic

	THUMB_FUNC_START CpPerform_MoveCameraOntoUnit
CpPerform_MoveCameraOntoUnit: @ 0x08039F0C
	push {r4, r5, r6, lr}
	adds r4, r0, #0
	movs r0, #0x31
	adds r0, r0, r4
	mov ip, r0
	movs r0, #1
	mov r1, ip
	strb r0, [r1]
	ldr r1, _08039F68  @ gPlaySt
	ldrb r0, [r1, #0xd]
	cmp r0, #0
	beq _08039F8E
	ldrb r0, [r1, #0xf]
	cmp r0, #0x80
	bne _08039F8E
	ldr r0, _08039F6C  @ gActiveUnit
	ldr r1, [r0]
	movs r6, #0x11
	ldrsb r6, [r1, r6]
	ldr r0, _08039F70  @ gMapFog
	ldr r2, [r0]
	lsls r0, r6, #2
	adds r0, r0, r2
	movs r5, #0x10
	ldrsb r5, [r1, r5]
	ldr r0, [r0]
	adds r0, r0, r5
	ldrb r0, [r0]
	cmp r0, #0
	bne _08039F5C
	ldr r3, _08039F74  @ gAiDecision
	ldrb r0, [r3, #3]
	lsls r0, r0, #2
	adds r0, r0, r2
	ldrb r1, [r3, #2]
	ldr r0, [r0]
	adds r0, r0, r1
	ldrb r0, [r0]
	cmp r0, #0
	beq _08039F78
_08039F5C:
	adds r0, r4, #0
	adds r1, r5, #0
	adds r2, r6, #0
	bl EnsureCameraOntoPosition
	b _08039FA0
	.align 2, 0
_08039F68: .4byte gPlaySt
_08039F6C: .4byte gActiveUnit
_08039F70: .4byte gMapFog
_08039F74: .4byte gAiDecision
_08039F78:
	mov r1, ip
	strb r0, [r1]
	ldrb r0, [r3]
	cmp r0, #4
	bne _08039FA0
	ldrb r1, [r3, #2]
	ldrb r2, [r3, #3]
	adds r0, r4, #0
	bl EnsureCameraOntoPosition
	b _08039FA0
_08039F8E:
	ldr r0, _08039FA8  @ gActiveUnit
	ldr r0, [r0]
	movs r1, #0x10
	ldrsb r1, [r0, r1]
	movs r2, #0x11
	ldrsb r2, [r0, r2]
	adds r0, r4, #0
	bl EnsureCameraOntoPosition
_08039FA0:
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.align 2, 0
_08039FA8: .4byte gActiveUnit

	THUMB_FUNC_END CpPerform_MoveCameraOntoUnit

	THUMB_FUNC_START CpPerform_BeginUnitMovement
CpPerform_BeginUnitMovement: @ 0x08039FAC
	push {r4, r5, r6, r7, lr}
	adds r5, r0, #0
	ldr r6, _0803A010  @ gActiveUnit
	ldr r0, [r6]
	bl UnitBeginAction
	ldr r0, [r6]
	bl HideUnitSprite
	ldr r0, [r6]
	bl FillMovementMapForUnit
	ldr r0, _0803A014  @ gMapMovement
	ldr r0, [r0]
	bl SetSubjectMap
	ldr r4, _0803A018  @ gAiDecision
	ldrb r0, [r4, #2]
	ldrb r1, [r4, #3]
	ldr r7, _0803A01C  @ gUnitMoveBuffer
	adds r2, r7, #0
	bl GenerateMovementInstructionsToPoint
	ldr r0, [r6]
	movs r1, #0x10
	ldrsb r1, [r0, r1]
	movs r2, #0x11
	ldrsb r2, [r0, r2]
	bl ProcessUnitMovement
	ldr r1, _0803A020  @ gAction
	ldrb r0, [r1, #0xe]
	strb r0, [r4, #2]
	ldrb r0, [r1, #0xf]
	strb r0, [r4, #3]
	adds r5, #0x31
	ldrb r0, [r5]
	cmp r0, #0
	beq _0803A00A
	ldr r0, [r6]
	bl MU_Create
	bl MU_SetDefaultFacing_Auto
	adds r0, r7, #0
	bl MU_StartMoveScript_Auto
_0803A00A:
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0
	.align 2, 0
_0803A010: .4byte gActiveUnit
_0803A014: .4byte gMapMovement
_0803A018: .4byte gAiDecision
_0803A01C: .4byte gUnitMoveBuffer
_0803A020: .4byte gAction

	THUMB_FUNC_END CpPerform_BeginUnitMovement

	THUMB_FUNC_START AiRefreshMap
AiRefreshMap: @ 0x0803A024
	push {r4, r5, lr}
	ldr r0, _0803A070  @ gAction
	ldrb r0, [r0, #0xc]
	bl GetUnit
	ldr r5, _0803A074  @ gActiveUnit
	str r0, [r5]
	ldr r4, _0803A078  @ gAiDecision
	ldrb r0, [r4, #2]
	ldrb r1, [r4, #3]
	bl SetCursorMapPosition
	bl RenderBmMapOnBg2
	ldrb r0, [r4, #2]
	ldrb r1, [r4, #3]
	bl MoveActiveUnit
	bl RefreshEntityBmMaps
	bl RenderBmMap
	movs r0, #1
	bl StartBMXFADE
	bl MU_EndAll
	bl RefreshEntityBmMaps
	ldr r0, [r5]
	bl ShowUnitSprite
	bl RefreshUnitSprites
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0
_0803A070: .4byte gAction
_0803A074: .4byte gActiveUnit
_0803A078: .4byte gAiDecision

	THUMB_FUNC_END AiRefreshMap

	THUMB_FUNC_START CpPerform_PerformCombat
CpPerform_PerformCombat: @ 0x0803A07C
	push {r4, r5, r6, r7, lr}
	adds r7, r0, #0
	ldr r5, _0803A0D4  @ gAction
	ldr r0, _0803A0D8  @ gActiveUnitId
	ldrb r0, [r0]
	strb r0, [r5, #0xc]
	movs r0, #2
	strb r0, [r5, #0x11]
	ldr r4, _0803A0DC  @ gAiDecision
	ldrb r0, [r4, #6]
	strb r0, [r5, #0xd]
	ldr r6, _0803A0E0  @ gActiveUnit
	ldr r1, [r6]
	ldrb r0, [r4, #2]
	strb r0, [r1, #0x10]
	ldr r1, [r6]
	ldrb r0, [r4, #3]
	strb r0, [r1, #0x11]
	ldrb r0, [r4, #6]
	cmp r0, #0
	bne _0803A0BA
	ldrb r0, [r4, #8]
	ldrb r1, [r4, #9]
	bl GetTrapAt
	ldrb r1, [r4, #8]
	strb r1, [r5, #0x13]
	ldrb r1, [r4, #9]
	strb r1, [r5, #0x14]
	ldrb r0, [r0, #3]
	strb r0, [r5, #0x15]
_0803A0BA:
	movs r1, #7
	ldrsb r1, [r4, r1]
	movs r0, #1
	negs r0, r0
	cmp r1, r0
	beq _0803A0E4
	ldr r0, [r6]
	ldrb r1, [r4, #7]
	bl EquipUnitItemSlot
	movs r0, #0
	b _0803A0E6
	.align 2, 0
_0803A0D4: .4byte gAction
_0803A0D8: .4byte gActiveUnitId
_0803A0DC: .4byte gAiDecision
_0803A0E0: .4byte gActiveUnit
_0803A0E4:
	movs r0, #8
_0803A0E6:
	strb r0, [r5, #0x12]
	adds r0, r7, #0
	bl ApplyUnitAction
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0

	THUMB_FUNC_END CpPerform_PerformCombat

	THUMB_FUNC_START CpPerform_PerformEscape
CpPerform_PerformEscape: @ 0x0803A0F4
	push {r4, lr}
	sub sp, #0xc
	adds r4, r0, #0
	ldr r1, _0803A12C  @ gUnknown_080D80E8
	mov r0, sp
	movs r2, #0xc
	bl memcpy
	ldr r1, _0803A130  @ gAiDecision
	ldrb r0, [r1, #8]
	cmp r0, #5
	beq _0803A124
	adds r0, r4, #0
	adds r0, #0x31
	ldrb r0, [r0]
	cmp r0, #0
	beq _0803A124
	ldrb r0, [r1, #8]
	lsls r1, r0, #1
	adds r1, r1, r0
	mov r2, sp
	adds r0, r2, r1
	bl MU_StartMoveScript_Auto
_0803A124:
	add sp, #0xc
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_0803A12C: .4byte gUnknown_080D80E8
_0803A130: .4byte gAiDecision

	THUMB_FUNC_END CpPerform_PerformEscape

	THUMB_FUNC_START CpPerform_PerformSteal
CpPerform_PerformSteal: @ 0x0803A134
	push {r4, r5, r6, lr}
	mov r6, r8
	push {r6}
	mov r8, r0
	ldr r4, _0803A174  @ gAiDecision
	ldrb r0, [r4, #6]
	bl GetUnit
	adds r6, r0, #0
	ldrb r1, [r4, #7]
	lsls r1, r1, #1
	adds r0, #0x1e
	adds r0, r0, r1
	ldrh r5, [r0]
	ldr r0, _0803A178  @ gActiveUnit
	ldr r0, [r0]
	adds r1, r5, #0
	bl UnitAddItem
	ldrb r1, [r4, #7]
	adds r0, r6, #0
	bl UnitRemoveItem
	adds r0, r5, #0
	mov r1, r8
	bl CreateItemStealingPopup
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.align 2, 0
_0803A174: .4byte gAiDecision
_0803A178: .4byte gActiveUnit

	THUMB_FUNC_END CpPerform_PerformSteal

	THUMB_FUNC_START CpPerform_LootWait
CpPerform_LootWait: @ 0x0803A17C
	push {r4, r5, lr}
	adds r5, r0, #0
	ldr r3, _0803A1B8  @ gAiDecision
	ldrb r2, [r3, #2]
	ldrb r4, [r3, #3]
	ldr r0, _0803A1BC  @ gMapTerrain
	ldr r1, [r0]
	lsls r0, r4, #2
	adds r0, r0, r1
	ldr r0, [r0]
	adds r0, r0, r2
	ldrb r0, [r0]
	cmp r0, #0x21
	bne _0803A1C8
	ldr r1, _0803A1C0  @ gActiveUnit
	ldr r0, [r1]
	strb r2, [r0, #0x10]
	ldr r1, [r1]
	ldrb r0, [r3, #3]
	strb r0, [r1, #0x11]
	ldr r1, _0803A1C4  @ gAction
	movs r0, #0x1a
	strb r0, [r1, #0x11]
	ldrb r0, [r3, #7]
	strb r0, [r1, #0x12]
	adds r0, r5, #0
	bl ActionStaffDoorChestUseItem
	b _0803A1F4
	.align 2, 0
_0803A1B8: .4byte gAiDecision
_0803A1BC: .4byte gMapTerrain
_0803A1C0: .4byte gActiveUnit
_0803A1C4: .4byte gAction
_0803A1C8:
	subs r1, r4, #1
	lsls r0, r2, #0x18
	asrs r0, r0, #0x18
	lsls r1, r1, #0x18
	asrs r1, r1, #0x18
	bl RunLocationEvents
	ldr r0, _0803A1FC  @ gPlaySt
	adds r0, #0x41
	ldrb r0, [r0]
	lsls r0, r0, #0x1e
	cmp r0, #0
	blt _0803A1E8
	movs r0, #0xab
	bl m4aSongNumStart
_0803A1E8:
	ldr r0, _0803A200  @ gUnknown_085A80A4
	movs r1, #0x60
	movs r2, #0
	adds r3, r5, #0
	bl Popup_Create
_0803A1F4:
	movs r0, #1
	pop {r4, r5}
	pop {r1}
	bx r1
	.align 2, 0
_0803A1FC: .4byte gPlaySt
_0803A200: .4byte gUnknown_085A80A4

	THUMB_FUNC_END CpPerform_LootWait

	THUMB_FUNC_START CpPerform_StaffWait
CpPerform_StaffWait: @ 0x0803A204
	push {r4, lr}
	ldr r4, _0803A230  @ gActiveUnit
	ldr r2, [r4]
	ldr r3, _0803A234  @ gAiDecision
	ldrb r1, [r3, #2]
	strb r1, [r2, #0x10]
	ldr r2, [r4]
	ldrb r1, [r3, #3]
	strb r1, [r2, #0x11]
	ldr r2, _0803A238  @ gAction
	movs r1, #3
	strb r1, [r2, #0x11]
	ldrb r1, [r3, #6]
	strb r1, [r2, #0xd]
	ldrb r1, [r3, #7]
	strb r1, [r2, #0x12]
	bl ActionStaffDoorChestUseItem
	movs r0, #1
	pop {r4}
	pop {r1}
	bx r1
	.align 2, 0
_0803A230: .4byte gActiveUnit
_0803A234: .4byte gAiDecision
_0803A238: .4byte gAction

	THUMB_FUNC_END CpPerform_StaffWait

	THUMB_FUNC_START CpPerform_ChestWait
CpPerform_ChestWait: @ 0x0803A23C
	push {r4, lr}
	ldr r4, _0803A264  @ gActiveUnit
	ldr r2, [r4]
	ldr r3, _0803A268  @ gAiDecision
	ldrb r1, [r3, #2]
	strb r1, [r2, #0x10]
	ldr r2, [r4]
	ldrb r1, [r3, #3]
	strb r1, [r2, #0x11]
	ldr r2, _0803A26C  @ gAction
	movs r1, #0x1a
	strb r1, [r2, #0x11]
	ldrb r1, [r3, #7]
	strb r1, [r2, #0x12]
	bl ActionStaffDoorChestUseItem
	movs r0, #1
	pop {r4}
	pop {r1}
	bx r1
	.align 2, 0
_0803A264: .4byte gActiveUnit
_0803A268: .4byte gAiDecision
_0803A26C: .4byte gAction

	THUMB_FUNC_END CpPerform_ChestWait

	THUMB_FUNC_START CpPerform_DanceWait
CpPerform_DanceWait: @ 0x0803A270
	movs r0, #1
	bx lr

	THUMB_FUNC_END CpPerform_DanceWait

	THUMB_FUNC_START CpPerform_TalkWait
CpPerform_TalkWait: @ 0x0803A274
	push {r4, r5, lr}
	ldr r2, _0803A2B0  @ gActiveUnit
	ldr r1, [r2]
	ldr r5, _0803A2B4  @ gAiDecision
	ldrb r0, [r5, #2]
	strb r0, [r1, #0x10]
	ldr r1, [r2]
	ldrb r0, [r5, #3]
	strb r0, [r1, #0x11]
	ldrb r0, [r5, #6]
	cmp r0, #0
	bne _0803A2A6
	ldrb r0, [r5, #7]
	bl GetUnit
	ldr r0, [r0]
	ldrb r4, [r0, #4]
	ldrb r0, [r5, #8]
	bl GetUnit
	ldr r0, [r0]
	ldrb r1, [r0, #4]
	adds r0, r4, #0
	bl RunCharacterEvents
_0803A2A6:
	movs r0, #1
	pop {r4, r5}
	pop {r1}
	bx r1
	.align 2, 0
_0803A2B0: .4byte gActiveUnit
_0803A2B4: .4byte gAiDecision

	THUMB_FUNC_END CpPerform_TalkWait

	THUMB_FUNC_START CpPerform_RideBallistaWait
CpPerform_RideBallistaWait: @ 0x0803A2B8
	push {lr}
	ldr r1, _0803A2D8  @ gActiveUnit
	ldr r2, [r1]
	ldr r3, _0803A2DC  @ gAiDecision
	ldrb r0, [r3, #2]
	strb r0, [r2, #0x10]
	ldr r2, [r1]
	ldrb r0, [r3, #3]
	strb r0, [r2, #0x11]
	ldr r0, [r1]
	bl RideBallista
	movs r0, #1
	pop {r1}
	bx r1
	.align 2, 0
_0803A2D8: .4byte gActiveUnit
_0803A2DC: .4byte gAiDecision

	THUMB_FUNC_END CpPerform_RideBallistaWait

	THUMB_FUNC_START CpPerform_ExitBallistaWait
CpPerform_ExitBallistaWait: @ 0x0803A2E0
	push {lr}
	ldr r1, _0803A300  @ gActiveUnit
	ldr r2, [r1]
	ldr r3, _0803A304  @ gAiDecision
	ldrb r0, [r3, #2]
	strb r0, [r2, #0x10]
	ldr r2, [r1]
	ldrb r0, [r3, #3]
	strb r0, [r2, #0x11]
	ldr r0, [r1]
	bl TryRemoveUnitFromBallista
	movs r0, #1
	pop {r1}
	bx r1
	.align 2, 0
_0803A300: .4byte gActiveUnit
_0803A304: .4byte gAiDecision

	THUMB_FUNC_END CpPerform_ExitBallistaWait

	THUMB_FUNC_START CpPerform_PerformSomeOtherCombat
CpPerform_PerformSomeOtherCombat: @ 0x0803A308
	push {r4, r5, r6, lr}
	adds r6, r0, #0
	ldr r4, _0803A348  @ gAction
	ldr r0, _0803A34C  @ gActiveUnitId
	ldrb r0, [r0]
	movs r5, #0
	strb r0, [r4, #0xc]
	movs r0, #2
	strb r0, [r4, #0x11]
	ldr r3, _0803A350  @ gAiDecision
	ldrb r0, [r3, #6]
	strb r0, [r4, #0xd]
	ldr r1, _0803A354  @ gActiveUnit
	ldr r2, [r1]
	ldrb r0, [r3, #2]
	strb r0, [r2, #0x10]
	ldr r2, [r1]
	ldrb r0, [r3, #3]
	strb r0, [r2, #0x11]
	ldr r0, [r1]
	ldrb r1, [r3, #7]
	bl EquipUnitItemSlot
	strb r5, [r4, #0x12]
	adds r0, r6, #0
	bl ApplyUnitAction
	movs r0, #1
	pop {r4, r5, r6}
	pop {r1}
	bx r1
	.align 2, 0
_0803A348: .4byte gAction
_0803A34C: .4byte gActiveUnitId
_0803A350: .4byte gAiDecision
_0803A354: .4byte gActiveUnit

	THUMB_FUNC_END CpPerform_PerformSomeOtherCombat

	THUMB_FUNC_START CpPerform_PerformDkSummon
CpPerform_PerformDkSummon: @ 0x0803A358
	push {r4, lr}
	ldr r2, _0803A380  @ gAction
	ldr r1, _0803A384  @ gActiveUnitId
	ldrb r1, [r1]
	strb r1, [r2, #0xc]
	movs r1, #8
	strb r1, [r2, #0x11]
	ldr r4, _0803A388  @ gActiveUnit
	ldr r2, [r4]
	ldr r3, _0803A38C  @ gAiDecision
	ldrb r1, [r3, #2]
	strb r1, [r2, #0x10]
	ldr r2, [r4]
	ldrb r1, [r3, #3]
	strb r1, [r2, #0x11]
	bl ApplyUnitAction
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_0803A380: .4byte gAction
_0803A384: .4byte gActiveUnitId
_0803A388: .4byte gActiveUnit
_0803A38C: .4byte gAiDecision

	THUMB_FUNC_END CpPerform_PerformDkSummon

	THUMB_FUNC_START CpPerform_PickWait
CpPerform_PickWait: @ 0x0803A390
	push {r4, lr}
	ldr r4, _0803A3BC  @ gActiveUnit
	ldr r2, [r4]
	ldr r3, _0803A3C0  @ gAiDecision
	ldrb r1, [r3, #2]
	strb r1, [r2, #0x10]
	ldr r2, [r4]
	ldrb r1, [r3, #3]
	strb r1, [r2, #0x11]
	ldr r2, _0803A3C4  @ gAction
	ldrb r1, [r3, #8]
	strb r1, [r2, #0x13]
	ldrb r1, [r3, #9]
	strb r1, [r2, #0x14]
	movs r1, #0x15
	strb r1, [r2, #0x11]
	bl ApplyUnitAction
	movs r0, #1
	pop {r4}
	pop {r1}
	bx r1
	.align 2, 0
_0803A3BC: .4byte gActiveUnit
_0803A3C0: .4byte gAiDecision
_0803A3C4: .4byte gAction

	THUMB_FUNC_END CpPerform_PickWait

	THUMB_FUNC_START CpPerform_MoveCameraOntoTarget
CpPerform_MoveCameraOntoTarget: @ 0x0803A3C8
	push {r4, r5, r6, r7, lr}
	mov r7, r8
	push {r7}
	mov r8, r0
	movs r6, #0
	movs r5, #0
	ldr r0, _0803A3F4  @ gAction
	ldrb r0, [r0, #0x11]
	cmp r0, #0x1e
	bne _0803A3DE
	b _0803A4D8
_0803A3DE:
	ldr r0, _0803A3F8  @ gAiDecision
	ldrb r1, [r0]
	adds r2, r0, #0
	cmp r1, #0xd
	bhi _0803A4C2
	lsls r0, r1, #2
	ldr r1, _0803A3FC  @ _0803A400
	adds r0, r0, r1
	ldr r0, [r0]
	mov pc, r0
	.align 2, 0
_0803A3F4: .4byte gAction
_0803A3F8: .4byte gAiDecision
_0803A3FC: .4byte _0803A400
_0803A400: @ jump table
	.4byte _0803A4D8 @ case 0
	.4byte _0803A438 @ case 1
	.4byte _0803A4D8 @ case 2
	.4byte _0803A4A4 @ case 3
	.4byte _0803A4D8 @ case 4
	.4byte _0803A4B0 @ case 5
	.4byte _0803A4D8 @ case 6
	.4byte _0803A4A8 @ case 7
	.4byte _0803A4AC @ case 8
	.4byte _0803A4D8 @ case 9
	.4byte _0803A4D8 @ case 10
	.4byte _0803A4D8 @ case 11
	.4byte _0803A4D8 @ case 12
	.4byte _0803A4D8 @ case 13
_0803A438:
	ldr r1, _0803A448  @ gAiDecision
	ldrb r0, [r1, #6]
	cmp r0, #0
	bne _0803A44C
	ldrb r6, [r1, #8]
	ldrb r5, [r1, #9]
	b _0803A45A
	.align 2, 0
_0803A448: .4byte gAiDecision
_0803A44C:
	ldrb r0, [r1, #6]
	bl GetUnit
	movs r6, #0x10
	ldrsb r6, [r0, r6]
	movs r5, #0x11
	ldrsb r5, [r0, r5]
_0803A45A:
	ldr r7, _0803A49C  @ gAiDecision
	movs r1, #7
	ldrsb r1, [r7, r1]
	movs r0, #1
	negs r0, r0
	cmp r1, r0
	bne _0803A4C2
	ldr r4, _0803A4A0  @ gActiveUnit
	ldr r0, [r4]
	ldr r0, [r0, #0xc]
	movs r1, #0x80
	lsls r1, r1, #4
	ands r0, r1
	cmp r0, #0
	bne _0803A4C2
	bl MU_EndAll
	ldr r1, [r4]
	ldrb r0, [r7, #2]
	strb r0, [r1, #0x10]
	ldr r1, [r4]
	ldrb r0, [r7, #3]
	strb r0, [r1, #0x11]
	ldr r0, [r4]
	bl RideBallista
	ldr r0, [r4]
	bl MU_Create
	bl MU_SetDefaultFacing_Auto
	b _0803A4C2
	.align 2, 0
_0803A49C: .4byte gAiDecision
_0803A4A0: .4byte gActiveUnit
_0803A4A4:
	ldrb r0, [r2, #6]
	b _0803A4B6
_0803A4A8:
	ldrb r0, [r2, #6]
	b _0803A4B6
_0803A4AC:
	ldrb r0, [r2, #9]
	b _0803A4B6
_0803A4B0:
	ldrb r0, [r2, #6]
	cmp r0, #0
	beq _0803A4D8
_0803A4B6:
	bl GetUnit
	movs r6, #0x10
	ldrsb r6, [r0, r6]
	movs r5, #0x11
	ldrsb r5, [r0, r5]
_0803A4C2:
	mov r0, r8
	adds r1, r6, #0
	adds r2, r5, #0
	bl EnsureCameraOntoPosition
	lsls r0, r6, #4
	lsls r1, r5, #4
	movs r2, #2
	mov r3, r8
	bl StartAiTargetCursor
_0803A4D8:
	pop {r3}
	mov r8, r3
	pop {r4, r5, r6, r7}
	pop {r0}
	bx r0

	THUMB_FUNC_END CpPerform_MoveCameraOntoTarget

	THUMB_FUNC_START CpPerform_PerformAction
CpPerform_PerformAction: @ 0x0803A4E4
	push {r4, lr}
	adds r4, r0, #0
	adds r1, r4, #0
	adds r1, #0x30
	movs r0, #0
	strb r0, [r1]
	ldr r0, _0803A4FC  @ gAction
	ldrb r0, [r0, #0x11]
	cmp r0, #0x1e
	bne _0803A504
	ldr r0, _0803A500  @ CpPerform_DummyWait
	b _0803A5EA
	.align 2, 0
_0803A4FC: .4byte gAction
_0803A500: .4byte CpPerform_DummyWait
_0803A504:
	ldr r0, _0803A518  @ gAiDecision
	ldrb r0, [r0]
	cmp r0, #0xd
	bls _0803A50E
	b _0803A5EC
_0803A50E:
	lsls r0, r0, #2
	ldr r1, _0803A51C  @ _0803A520
	adds r0, r0, r1
	ldr r0, [r0]
	mov pc, r0
	.align 2, 0
_0803A518: .4byte gAiDecision
_0803A51C: .4byte _0803A520
_0803A520: @ jump table
	.4byte _0803A558 @ case 0
	.4byte _0803A560 @ case 1
	.4byte _0803A570 @ case 2
	.4byte _0803A580 @ case 3
	.4byte _0803A590 @ case 4
	.4byte _0803A598 @ case 5
	.4byte _0803A5A0 @ case 6
	.4byte _0803A5A8 @ case 7
	.4byte _0803A5B0 @ case 8
	.4byte _0803A5B8 @ case 9
	.4byte _0803A5C0 @ case 10
	.4byte _0803A5C8 @ case 11
	.4byte _0803A5D8 @ case 12
	.4byte _0803A5E8 @ case 13
_0803A558:
	ldr r0, _0803A55C  @ CpPerform_DummyWait
	b _0803A5EA
	.align 2, 0
_0803A55C: .4byte CpPerform_DummyWait
_0803A560:
	ldr r0, _0803A56C  @ CpPerform_DummyWait
	str r0, [r4, #0x2c]
	adds r0, r4, #0
	bl CpPerform_PerformCombat
	b _0803A5EC
	.align 2, 0
_0803A56C: .4byte CpPerform_DummyWait
_0803A570:
	adds r0, r4, #0
	bl CpPerform_PerformEscape
	ldr r0, _0803A57C  @ CpPerform_EscapeWait
	b _0803A5EA
	.align 2, 0
_0803A57C: .4byte CpPerform_EscapeWait
_0803A580:
	adds r0, r4, #0
	bl CpPerform_PerformSteal
	ldr r0, _0803A58C  @ CpPerform_StealWait
	b _0803A5EA
	.align 2, 0
_0803A58C: .4byte CpPerform_StealWait
_0803A590:
	ldr r0, _0803A594  @ CpPerform_LootWait
	b _0803A5EA
	.align 2, 0
_0803A594: .4byte CpPerform_LootWait
_0803A598:
	ldr r0, _0803A59C  @ CpPerform_StaffWait
	b _0803A5EA
	.align 2, 0
_0803A59C: .4byte CpPerform_StaffWait
_0803A5A0:
	ldr r0, _0803A5A4  @ CpPerform_ChestWait
	b _0803A5EA
	.align 2, 0
_0803A5A4: .4byte CpPerform_ChestWait
_0803A5A8:
	ldr r0, _0803A5AC  @ CpPerform_DanceWait
	b _0803A5EA
	.align 2, 0
_0803A5AC: .4byte CpPerform_DanceWait
_0803A5B0:
	ldr r0, _0803A5B4  @ CpPerform_TalkWait
	b _0803A5EA
	.align 2, 0
_0803A5B4: .4byte CpPerform_TalkWait
_0803A5B8:
	ldr r0, _0803A5BC  @ CpPerform_RideBallistaWait
	b _0803A5EA
	.align 2, 0
_0803A5BC: .4byte CpPerform_RideBallistaWait
_0803A5C0:
	ldr r0, _0803A5C4  @ CpPerform_ExitBallistaWait
	b _0803A5EA
	.align 2, 0
_0803A5C4: .4byte CpPerform_ExitBallistaWait
_0803A5C8:
	ldr r0, _0803A5D4  @ CpPerform_DummyWait
	str r0, [r4, #0x2c]
	adds r0, r4, #0
	bl CpPerform_PerformSomeOtherCombat
	b _0803A5EC
	.align 2, 0
_0803A5D4: .4byte CpPerform_DummyWait
_0803A5D8:
	ldr r0, _0803A5E4  @ CpPerform_DummyWait
	str r0, [r4, #0x2c]
	adds r0, r4, #0
	bl CpPerform_PerformDkSummon
	b _0803A5EC
	.align 2, 0
_0803A5E4: .4byte CpPerform_DummyWait
_0803A5E8:
	ldr r0, _0803A5F4  @ CpPerform_PickWait
_0803A5EA:
	str r0, [r4, #0x2c]
_0803A5EC:
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_0803A5F4: .4byte CpPerform_PickWait

	THUMB_FUNC_END CpPerform_PerformAction

	THUMB_FUNC_START CpPerform_WaitAction
CpPerform_WaitAction: @ 0x0803A5F8
	push {r4, lr}
	adds r4, r0, #0
	adds r1, r4, #0
	adds r1, #0x30
	ldrb r0, [r1]
	adds r0, #1
	strb r0, [r1]
	ldr r1, [r4, #0x2c]
	adds r0, r4, #0
	bl _call_via_r1
	lsls r0, r0, #0x18
	asrs r0, r0, #0x18
	cmp r0, #1
	bne _0803A61C
	adds r0, r4, #0
	bl Proc_Break
_0803A61C:
	ldr r3, _0803A634  @ gActiveUnit
	ldr r1, [r3]
	ldr r2, _0803A638  @ gAiDecision
	ldrb r0, [r2, #2]
	strb r0, [r1, #0x10]
	ldr r1, [r3]
	ldrb r0, [r2, #3]
	strb r0, [r1, #0x11]
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_0803A634: .4byte gActiveUnit
_0803A638: .4byte gAiDecision

	THUMB_FUNC_END CpPerform_WaitAction

	THUMB_FUNC_START sub_803A63C
sub_803A63C: @ 0x0803A63C
	push {r4, lr}
	adds r4, r0, #0
	bl AiUpdateAllUnitHealAi
	bl AiRefreshMap
	ldr r0, _0803A66C  @ gActiveUnit
	ldr r1, [r0]
	ldr r0, [r1]
	cmp r0, #0
	beq _0803A65C
	ldr r0, [r1, #0xc]
	ldr r1, _0803A670  @ 0x00010005
	ands r0, r1
	cmp r0, #0
	beq _0803A664
_0803A65C:
	adds r0, r4, #0
	movs r1, #1
	bl Proc_Goto
_0803A664:
	pop {r4}
	pop {r0}
	bx r0
	.align 2, 0
_0803A66C: .4byte gActiveUnit
_0803A670: .4byte 0x00010005

	THUMB_FUNC_END sub_803A63C

	THUMB_FUNC_START CpPerform_DummyWait
CpPerform_DummyWait: @ 0x0803A674
	movs r0, #1
	bx lr

	THUMB_FUNC_END CpPerform_DummyWait

	THUMB_FUNC_START CpPerform_EscapeWait
CpPerform_EscapeWait: @ 0x0803A678
	push {lr}
	bl MU_IsAnyActive
	lsls r0, r0, #0x18
	asrs r1, r0, #0x18
	cmp r1, #0
	beq _0803A68A
	movs r0, #0
	b _0803A692
_0803A68A:
	ldr r0, _0803A698  @ gActiveUnit
	ldr r0, [r0]
	str r1, [r0]
	movs r0, #1
_0803A692:
	pop {r1}
	bx r1
	.align 2, 0
_0803A698: .4byte gActiveUnit

	THUMB_FUNC_END CpPerform_EscapeWait

	THUMB_FUNC_START CpPerform_StealWait
CpPerform_StealWait: @ 0x0803A69C
	push {lr}
	adds r0, #0x30
	ldrb r0, [r0]
	cmp r0, #4
	bhi _0803A6AA
	movs r0, #0
	b _0803A6C2
_0803A6AA:
	ldr r0, _0803A6C8  @ gBg0Tm
	movs r1, #0
	bl TmFill
	ldr r0, _0803A6CC  @ gBg1Tm
	movs r1, #0
	bl TmFill
	movs r0, #3
	bl EnableBgSync
	movs r0, #1
_0803A6C2:
	pop {r1}
	bx r1
	.align 2, 0
_0803A6C8: .4byte gBg0Tm
_0803A6CC: .4byte gBg1Tm

	THUMB_FUNC_END CpPerform_StealWait

	THUMB_FUNC_START sub_803A6D0
sub_803A6D0: @ 0x0803A6D0
	push {r4, r5, r6, lr}
	sub sp, #0x18
	bl sub_803E900
	lsls r0, r0, #0x18
	cmp r0, #0
	beq _0803A70E
	add r0, sp, #4
	bl sub_803E93C
	lsls r0, r0, #0x18
	cmp r0, #0
	beq _0803A70E
	ldr r1, _0803A718  @ gAiDecision
	ldrb r0, [r1, #2]
	ldrb r1, [r1, #3]
	add r4, sp, #0x10
	mov r5, sp
	adds r5, #0x12
	add r6, sp, #0x14
	str r6, [sp]
	adds r2, r4, #0
	adds r3, r5, #0
	bl sub_803EA58
	ldrh r0, [r4]
	ldrh r1, [r5]
	ldrh r2, [r6]
	add r3, sp, #4
	bl sub_803EBF0
_0803A70E:
	add sp, #0x18
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.align 2, 0
_0803A718: .4byte gAiDecision

	THUMB_FUNC_END sub_803A6D0

.align 2, 0
