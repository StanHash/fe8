
#include "global.h"

#include "proc.h"
#include "unit.h"
#include "bmmap.h"
#include "bmtrick.h"

#include "constants/jids.h"

#include "cp_common.h"

static void CpDecide_Suspend(ProcPtr proc);
static void CpDecide_Main(ProcPtr proc);

static void DecideHealOrEscape(void);
static void DecideScriptA(void);
static void DecideScriptB(void);
static void DecideSpecialItems(void);

typedef void(*DecideFunc)(void);

static DecideFunc CONST_DATA sDecideFuncList[] =
{
    DecideHealOrEscape,
    DecideScriptA,
    DecideScriptB,
    DecideSpecialItems,
    NULL, NULL,
};

static DecideFunc CONST_DATA sUnused_085A7FB4[] =
{
    DecideSpecialItems,
    DecideScriptA,
    DecideHealOrEscape,
    DecideScriptB,
    NULL, NULL,
};

struct ProcScr CONST_DATA gProcScr_CpDecide[] =
{
    PROC_NAME("E_CPDECIDE"),

PROC_LABEL(0),
    PROC_CALL(CpDecide_Main),
    PROC_SLEEP(0),

    PROC_CALL(CpDecide_Suspend),

    PROC_GOTO(0),

    PROC_END,
};

void CpDecide_Suspend(ProcPtr proc)
{
    if (UNIT_FACTION(gActiveUnit) == FACTION_BLUE)
        gAction.suspendPointType = SUSPEND_POINT_BSKPHASE;
    else
        gAction.suspendPointType = SUSPEND_POINT_CPPHASE;

    SaveSuspendedGame(SAVE_BLOCK_SUSPEND_BASE);
}

#if NONMATCHING

void CpDecide_Main(ProcPtr proc)
{
next_unit:
    gAiSt.decideState = 0;

    if (*gAiSt.unitIt)
    {
        gAiSt.unk7C = 0;

        gActiveUnitId = *gAiSt.unitIt;
        gActiveUnit = GetUnit(gActiveUnitId);

        if (gActiveUnit->flags & (UNIT_FLAG_DEAD | UNIT_FLAG_TURN_ENDED) || !gActiveUnit->pinfo)
        {
            gAiSt.unitIt++;
            goto next_unit;
        }

        RefreshEntityBmMaps();
        RenderBmMap();
        RefreshUnitSprites();

        AiUpdateNoMoveFlag(gActiveUnit);

        gAiSt.combatWeightTableId = (gActiveUnit->aiConfig & 0xF8) >> 3;

        gAiSt.dangerMapFilled = FALSE;
        SetupAiDangerMap();

        ClearAiDecision();
        AiDecideMainFunc();

        gActiveUnit->flags |= UNIT_FLAG_AI_PROCESSED;

        if (!gAiDecision.actionPerformed ||
            (gActiveUnit->x == gAiDecision.xMove && gActiveUnit->y == gAiDecision.yMove && gAiDecision.actionId == AI_ACTION_NONE))
        {
            // Ignoring actions that are just moving to the same square

            gAiSt.unitIt++;
            Proc_Goto(proc, 0);
        }
        else
        {
            gAiSt.unitIt++;
            SpawnProcLocking(gProcScr_CpPerform, proc);
        }
    }
    else
    {
        Proc_End(proc);
    }
}

#else // if !NONMATCHING

__attribute__((naked))
void CpDecide_Main(ProcPtr proc)
{
    asm("\n\
        .syntax unified\n\
        push {r4, r5, r6, r7, lr}\n\
        adds r7, r0, #0\n\
    _08039B04:\n\
        ldr r4, _08039B48  @ gAiSt\n\
        adds r0, r4, #0\n\
        adds r0, #0x79\n\
        movs r1, #0\n\
        strb r1, [r0]\n\
        ldr r2, [r4, #0x74]\n\
        ldrb r0, [r2]\n\
        cmp r0, #0\n\
        beq _08039BF4\n\
        adds r0, r4, #0\n\
        adds r0, #0x7c\n\
        strb r1, [r0]\n\
        ldr r1, _08039B4C  @ gActiveUnitId\n\
        ldrb r0, [r2]\n\
        strb r0, [r1]\n\
        ldrb r0, [r1]\n\
        bl GetUnit\n\
        adds r1, r0, #0\n\
        ldr r6, _08039B50  @ gActiveUnit\n\
        str r1, [r6]\n\
        ldr r5, [r1, #0xc]\n\
        movs r0, #6\n\
        ands r5, r0\n\
        cmp r5, #0\n\
        bne _08039B3E\n\
        ldr r0, [r1]\n\
        cmp r0, #0\n\
        bne _08039B54\n\
    _08039B3E:\n\
        ldr r0, [r4, #0x74]\n\
        adds r0, #1\n\
        str r0, [r4, #0x74]\n\
        b _08039B04\n\
        .align 2, 0\n\
    _08039B48: .4byte gAiSt\n\
    _08039B4C: .4byte gActiveUnitId\n\
    _08039B50: .4byte gActiveUnit\n\
    _08039B54:\n\
        bl RefreshEntityBmMaps\n\
        bl RenderBmMap\n\
        bl RefreshUnitSprites\n\
        ldr r0, [r6]\n\
        bl AiUpdateNoMoveFlag\n\
        ldr r0, [r6]\n\
        adds r0, #0x40\n\
        ldrh r1, [r0]\n\
        movs r0, #0xf8\n\
        ands r0, r1\n\
        lsrs r0, r0, #3\n\
        adds r1, r4, #0\n\
        adds r1, #0x7d\n\
        strb r0, [r1]\n\
        adds r0, r4, #0\n\
        adds r0, #0x7a\n\
        strb r5, [r0]\n\
        bl SetupAiDangerMap\n\
        bl ClearAiDecision\n\
        ldr r0, _08039BD0  @ AiDecideMainFunc\n\
        ldr r0, [r0]\n\
        bl _call_via_r0\n\
        ldr r2, [r6]\n\
        ldr r0, [r2, #0xc]\n\
        movs r1, #0x80\n\
        lsls r1, r1, #3\n\
        orrs r0, r1\n\
        str r0, [r2, #0xc]\n\
        ldr r1, _08039BD4  @ gAiDecision\n\
        movs r0, #0xa\n\
        ldrsb r0, [r1, r0]\n\
        cmp r0, #0\n\
        beq _08039BBE\n\
        movs r0, #0x10\n\
        ldrsb r0, [r2, r0]\n\
        ldrb r3, [r1, #2]\n\
        cmp r0, r3\n\
        bne _08039BD8\n\
        movs r0, #0x11\n\
        ldrsb r0, [r2, r0]\n\
        ldrb r2, [r1, #3]\n\
        cmp r0, r2\n\
        bne _08039BD8\n\
        ldrb r0, [r1]\n\
        cmp r0, #0\n\
        bne _08039BD8\n\
    _08039BBE:\n\
        ldr r0, [r4, #0x74]\n\
        adds r0, #1\n\
        str r0, [r4, #0x74]\n\
        adds r0, r7, #0\n\
        movs r1, #0\n\
        bl Proc_Goto\n\
        b _08039BFA\n\
        .align 2, 0\n\
    _08039BD0: .4byte AiDecideMainFunc\n\
    _08039BD4: .4byte gAiDecision\n\
    _08039BD8:\n\
        ldr r0, _08039BEC  @ gAiSt\n\
        ldr r1, [r0, #0x74]\n\
        adds r1, #1\n\
        str r1, [r0, #0x74]\n\
        ldr r0, _08039BF0  @ gProcScr_CpPerform\n\
        adds r1, r7, #0\n\
        bl SpawnProcLocking\n\
        b _08039BFA\n\
        .align 2, 0\n\
    _08039BEC: .4byte gAiSt\n\
    _08039BF0: .4byte gProcScr_CpPerform\n\
    _08039BF4:\n\
        adds r0, r7, #0\n\
        bl Proc_End\n\
    _08039BFA:\n\
        pop {r4, r5, r6, r7}\n\
        pop {r0}\n\
        bx r0\n\
        .syntax divided\n\
    ");
}

#endif // NONMATCHING

void ClearAiDecision(void)
{
    gAiDecision.actionId = 0;

    gAiDecision.unitId = 0;
    gAiDecision.xMove = 0;
    gAiDecision.yMove = 0;
    gAiDecision.unk04 = 0;
    gAiDecision.unk05 = 0;
    gAiDecision.targetId = 0;
    gAiDecision.itemSlot = 0;
    gAiDecision.xTarget = 0;
    gAiDecision.yTarget = 0;

    gAiDecision.actionPerformed = FALSE;
}

void AiSetDecision(u8 xMove, u8 yMove, u8 actionId, u8 targetId, u8 itemSlot, u8 xTarget, u8 yTarget)
{
    gAiDecision.unitId = gActiveUnitId;
    gAiDecision.xMove = xMove;
    gAiDecision.yMove = yMove;

    gAiDecision.actionId = actionId;

    gAiDecision.targetId = targetId;
    gAiDecision.itemSlot = itemSlot;
    gAiDecision.xTarget = xTarget;
    gAiDecision.yTarget = yTarget;

    gAiDecision.actionPerformed = TRUE;
}

void AiUpdateDecision(u8 actionId, u8 targetId, u8 itemSlot, u8 xTarget, u8 yTarget)
{
    if (actionId != 0xFF)
        gAiDecision.actionId = actionId;

    if (targetId != 0xFF)
        gAiDecision.targetId = targetId;

    if (itemSlot != 0xFF)
        gAiDecision.itemSlot = itemSlot;

    if (xTarget != 0xFF)
        gAiDecision.xTarget = xTarget;

    if (yTarget != 0xFF)
        gAiDecision.yTarget = yTarget;

    gAiDecision.actionPerformed = TRUE;
}

void AiMasterDecisionMaker(void)
{
    while (sDecideFuncList[gAiSt.decideState] && !gAiDecision.actionPerformed)
    {
        sDecideFuncList[gAiSt.decideState++]();
    }
}

void DecideHealOrEscape(void)
{
    if (gAiSt.flags & AI_FLAG_BERSERKED)
        return;

    if (AiUnitUpdateGetHealMode(gActiveUnit) == TRUE)
    {
        struct Vec2 vec2;

        if (AiUnitTryHealSelf() == TRUE)
            return;

        if ((gActiveUnit->aiFlags & AI_UNIT_FLAG_3) && (AiUnitTryMoveTowardsEscapePoint() == TRUE))
        {
            AiTryDanceOrStealInPlace();
            return;
        }

        if (AiTryGetNearestHealPoint(&vec2) != TRUE)
            return;

        AiTryMoveTowards(vec2.x, vec2.y, 0, 0, 1);

        if (gAiDecision.actionPerformed == TRUE)
            AiTryActionAfterMove();
    }
    else
    {
        if ((gActiveUnit->aiFlags & AI_UNIT_FLAG_3) && (AiUnitTryMoveTowardsEscapePoint() == TRUE))
            AiTryDanceOrStealInPlace();
    }
}

void DecideSpecialItems(void)
{
    if (gAiSt.flags & AI_FLAG_BERSERKED)
        return;

    AiTryDoSpecialItems();
}

void DecideScriptA(void)
{
    int i = 0;

    if (UNIT_IS_GORGON_EGG(gActiveUnit))
        return;

    if (gAiSt.flags & AI_FLAG_BERSERKED)
    {
        AiDoBerserkAction();
        return;
    }

    for (i = 0; i < 0x100; ++i)
    {
        if (AiExecAi1() == TRUE)
            return;
    }

    AiExecFallbackAi1();
}

void DecideScriptB(void)
{
    int i = 0;

    if ((gActiveUnit->flags & UNIT_FLAG_IN_BALLISTA) && (GetNonEmptyBallistaAt(gActiveUnit->x, gActiveUnit->y) != NULL))
        return;

    if (gAiSt.flags & AI_FLAG_BERSERKED)
    {
        AiDoBerserkMove();
        return;
    }

    for (i = 0; i < 0x100; ++i)
    {
        if (AiExecAi2() == TRUE)
            return;
    }

    AiExecFallbackAi2();
}
