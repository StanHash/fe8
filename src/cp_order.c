
#include "global.h"

#include "hardware.h"
#include "proc.h"
#include "bmitem.h"
#include "unit.h"

#include "cp_common.h"

static void CpOrderMain(ProcPtr proc);
static void CpOrderBerserkInit(ProcPtr proc);
static void CpOrderFunc_BeginDecide(ProcPtr proc);
static int GetUnitBattleAiPriority(struct Unit* unit);
static int GetUnitAiPriority(struct Unit* unit);
static int BuildAiUnitList(void);
static void SortAiUnitList(int count);
static void CpOrderFunc_End(ProcPtr proc);

static
u32* CONST_DATA sUnitPriorityArray = (void*) gBuf;

struct ProcScr CONST_DATA gProcScr_CpOrder[] =
{
    PROC_NAME("E_CPORDER"),

    PROC_REPEAT(CpOrderMain),

    PROC_END,
};

struct ProcScr CONST_DATA gProcScr_BerserkCpOrder[] =
{
    PROC_NAME("E_BSKORDER"),

    PROC_CALL(CpOrderBerserkInit),
    PROC_REPEAT(CpOrderFunc_End),

    PROC_END,
};

static
ProcFunc CONST_DATA sCpOrderFuncList[] =
{
    CpOrderFunc_BeginDecide,
    CpOrderFunc_End,
};

void CpOrderMain(ProcPtr proc)
{
    sCpOrderFuncList[gAiSt.orderState++](proc);
}

void CpOrderBerserkInit(ProcPtr proc)
{
    int i, aiNum = 0;

    u32 faction = gPlaySt.chapterPhaseIndex;

    int factionUnitCountLut[3] = { 62, 20, 50 }; // TODO: named constant for those

    for (i = 0; i < factionUnitCountLut[faction >> 6]; ++i)
    {
        struct Unit* unit = GetUnit(faction + i + 1);

        if (!unit->pinfo)
            continue;

        if (unit->statusId != UNIT_STATUS_BERSERK)
            continue;

        if (unit->flags & (UNIT_FLAG_HIDDEN | UNIT_FLAG_TURN_ENDED | UNIT_FLAG_DEAD | UNIT_FLAG_RESCUED | UNIT_FLAG_AI_PROCESSED))
            continue;

        gAiSt.units[aiNum++] = faction + i + 1;
    }

    if (aiNum != 0)
    {
        gAiSt.units[aiNum] = 0;
        gAiSt.unitIt = gAiSt.units;

        AiDecideMainFunc = AiMasterDecisionMaker;

        SpawnProcLocking(gProcScr_CpDecide, proc);
    }
}

void CpOrderFunc_BeginDecide(ProcPtr proc)
{
    int unitAmt = BuildAiUnitList();

    if (unitAmt != 0)
    {
        SortAiUnitList(unitAmt);

        gAiSt.units[unitAmt] = 0;
        gAiSt.unitIt = gAiSt.units;

        AiDecideMainFunc = AiMasterDecisionMaker;

        SpawnProcLocking(gProcScr_CpDecide, proc);
    }
}

int GetUnitBattleAiPriority(struct Unit* unit)
{
    int i, item;

    u8 rangedAmt = 0;
    u8 meleeAmt = 0;

    for (i = 0; (i < UNIT_ITEM_COUNT) && !!(item = unit->items[i]); ++i)
    {
        if (!CanUnitUseWeapon(unit, item) && !CanUnitUseStaff(unit, item))
            continue;

        if (GetItemAttributes(item) & IA_STAFF)
        {
            return 72;
        }

        if (GetItemAttributes(item) & IA_WEAPON)
        {
            int range = GetItemMaxRange(item);

            if (range > 1)
                rangedAmt++;
            else
                meleeAmt++;
        }
    }

    if (rangedAmt != 0)
        return 40;

    if (meleeAmt != 0)
        return 20;

    return 87;
}

int GetUnitAiPriority(struct Unit* unit)
{
    int priority = UNIT_MOV(unit);

    u16 lead = GetUnitLeaderPid(unit);

    if (UNIT_ATTRIBUTES(unit) & (UNIT_ATTR_DANCE | UNIT_ATTR_PLAY))
        return priority - 149;

    if (!(unit->aiFlags & AI_UNIT_FLAG_0))
    {
        priority += lead << 8;

        if (UNIT_ATTRIBUTES(unit) & UNIT_ATTR_2)
            return priority + 60;

        if ((unit->pinfo->id == lead) || (UNIT_ATTRIBUTES(unit) & UNIT_ATTR_LORD))
            return priority + 87;

        priority = priority + GetUnitBattleAiPriority(unit);
    }

    return priority;
}

int BuildAiUnitList(void)
{
    int i, aiNum = 0;

    u32 faction = gPlaySt.chapterPhaseIndex;
    u32* prioIt = sUnitPriorityArray;

    int factionUnitCountLut[3] = { 62, 20, 50 }; // TODO: named constant for those

    for (i = 0; i < factionUnitCountLut[faction >> 6]; ++i)
    {
        struct Unit* unit = GetUnit(faction + i + 1);

        if (!unit->pinfo)
            continue;

        if (unit->statusId == UNIT_STATUS_SLEEP)
            continue;

        if (unit->statusId == UNIT_STATUS_BERSERK)
            continue;

        if (unit->flags & (UNIT_FLAG_HIDDEN | UNIT_FLAG_TURN_ENDED | UNIT_FLAG_DEAD | UNIT_FLAG_RESCUED | UNIT_FLAG_AI_PROCESSED))
            continue;

        gAiSt.units[aiNum] = faction + i + 1;
        *prioIt++ = GetUnitAiPriority(unit);

        aiNum++;
    }

    return aiNum;
}

void SortAiUnitList(int count)
{
    int i, j;

    if (count <= 1) // Redundant check
        return;
	++count; --count;

    // this is a bubble sort, I think

    for (i = 0; i <= count - 2; ++i)
    {
        for (j = count - 2; j >= i; --j)
        {
            if (sUnitPriorityArray[j] > sUnitPriorityArray[j+1])
            {
                // swap

                int tmp;

                tmp = sUnitPriorityArray[j];
                sUnitPriorityArray[j] = sUnitPriorityArray[j+1];
                sUnitPriorityArray[j+1] = tmp;

                tmp = gAiSt.units[j];
                gAiSt.units[j] = gAiSt.units[j+1];
                gAiSt.units[j+1] = tmp;
            }
        }
    }
}

void CpOrderFunc_End(ProcPtr proc)
{
    Proc_Break(proc);
}
