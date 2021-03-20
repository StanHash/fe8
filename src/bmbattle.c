#include "global.h"

#include "constants/iids.h"
#include "constants/jids.h"
#include "constants/pids.h"
#include "constants/terrains.h"

#include "random.h"
#include "bmitem.h"
#include "unit.h"
#include "bmmap.h"
#include "bmreliance.h"
#include "chapterdata.h"
#include "bmtrick.h"
#include "m4a.h"
#include "sound.h"
#include "hardware.h"
#include "proc.h"
#include "mu.h"
#include "bmbattle.h"

struct WeaponTriangleRule {
    s8 attackerWeaponType;
    s8 defenderWeaponType;
    s8 hitBonus;
    s8 atkBonus;
};

static CONST_DATA struct WeaponTriangleRule sWeaponTriangleRules[] = {
    { ITYPE_SWORD, ITYPE_LANCE, -15, -1 },
    { ITYPE_SWORD, ITYPE_AXE,   +15, +1 },

    { ITYPE_LANCE, ITYPE_AXE,   -15, -1 },
    { ITYPE_LANCE, ITYPE_SWORD, +15, +1 },

    { ITYPE_AXE,   ITYPE_SWORD, -15, -1 },
    { ITYPE_AXE,   ITYPE_LANCE, +15, +1 },

    { ITYPE_ANIMA, ITYPE_DARK,  -15, -1 },
    { ITYPE_ANIMA, ITYPE_LIGHT, +15, +1 },

    { ITYPE_LIGHT, ITYPE_ANIMA, -15, -1 },
    { ITYPE_LIGHT, ITYPE_DARK,  +15, +1 },

    { ITYPE_DARK,  ITYPE_LIGHT, -15, -1 },
    { ITYPE_DARK,  ITYPE_ANIMA, +15, +1 },

    { -1 },
};

static void UpdateActorFromBattle(void);

static CONST_DATA struct ProcScr sProcScr_BattleAnimSimpleLock[] = {
    PROC_SLEEP(1),
    PROC_CALL(UpdateActorFromBattle),
    PROC_END
};

EWRAM_DATA struct BattleStats gBattleStats = {};

EWRAM_DATA struct BattleUnit gBattleUnitA = {};
EWRAM_DATA struct BattleUnit gBattleUnitB = {};

EWRAM_DATA struct BattleHit gBattleHitArray[BATTLE_HIT_MAX] = {};
EWRAM_DATA struct BattleHit* gBattleHitIterator = 0;

static EWRAM_DATA struct {
    u8 unk00;
    u8 unk01;
    u8 unk02;
} sUnknown_0203A60C = {};

static void BattleGenerateSimulationInternal(struct Unit* actor, struct Unit* target, int x, int y, int actorWpnSlot);
static void BattleGenerateRealInternal(struct Unit* actor, struct Unit* target);

static s8 BattleRandRoll(u16 threshold, s8 simResult);
static s8 BattleRandRoll2Rn(u16 threshold, s8 simResult);

static void ComputeBattleUnitStats(struct BattleUnit* attacker, struct BattleUnit* defender);
static void ComputeBattleUnitEffectiveStats(struct BattleUnit* attacker, struct BattleUnit* defender);
static void ComputeBattleUnitSupportBonuses(struct BattleUnit* attacker, struct BattleUnit* defender);
static void ComputeBattleUnitDefense(struct BattleUnit* attacker, struct BattleUnit* defender);
static void ComputeBattleUnitBaseDefense(struct BattleUnit* bu);
static void ComputeBattleUnitAttack(struct BattleUnit* attacker, struct BattleUnit* defender);
static void ComputeBattleUnitSpeed(struct BattleUnit* bu);
static void ComputeBattleUnitHitRate(struct BattleUnit* bu);
static void ComputeBattleUnitAvoidRate(struct BattleUnit* bu);
static void ComputeBattleUnitCritRate(struct BattleUnit* bu);
static void ComputeBattleUnitDodgeRate(struct BattleUnit* bu);
static void ComputeBattleUnitEffectiveHitRate(struct BattleUnit* attacker, struct BattleUnit* defender);
static void ComputeBattleUnitEffectiveCritRate(struct BattleUnit* attacker, struct BattleUnit* defender);
static void ComputeBattleUnitSilencerRate(struct BattleUnit* attacker, struct BattleUnit* defender);
static void ComputeBattleUnitWeaponRankBonuses(struct BattleUnit* bu);
static void ComputeBattleUnitStatusBonuses(struct BattleUnit* bu);
static void ComputeBattleUnitSpecialWeaponStats(struct BattleUnit* attacker, struct BattleUnit* defender);

static s8 BattleGenerateRoundHits(struct BattleUnit* attacker, struct BattleUnit* defender);
static int GetBattleUnitHitCount(struct BattleUnit* attacker);
static int BattleCheckBraveEffect(struct BattleUnit* bu);

static s8 BattleCheckTriangleAttack(struct BattleUnit* attacker, struct BattleUnit* defender);
static void BattleUpdateBattleStats(struct BattleUnit* attacker, struct BattleUnit* defender);
static void BattleCheckSureShot(struct BattleUnit* attacker);
static void BattleCheckPierce(struct BattleUnit* attacker, struct BattleUnit* defender);
static void BattleCheckGreatShield(struct BattleUnit* attacker, struct BattleUnit* defender);
static s8 BattleCheckSilencer(struct BattleUnit* attacker, struct BattleUnit* defender);
static void BattleCheckPetrify(struct BattleUnit* attacker, struct BattleUnit* defender);
static void BattleGenerateHitAttributes(struct BattleUnit* attacker, struct BattleUnit* defender);
static void BattleGenerateHitTriangleAttack(struct BattleUnit* attacker, struct BattleUnit* defender);
static void BattleGenerateHitEffects(struct BattleUnit* attacker, struct BattleUnit* defender);
static s8 BattleGenerateHit(struct BattleUnit* attacker, struct BattleUnit* defender);

static int GetStatIncrease(int growth);

static int GetBattleUnitUpdatedWeaponExp(struct BattleUnit* bu);

static int GetUnitExpLevel(struct Unit* unit);
static int GetUnitRoundExp(struct Unit* actor, struct Unit* target);
static int GetUnitPowerLevel(struct Unit* unit);
static int GetUnitClassKillExpBonus(struct Unit* actor, struct Unit* target);
static int GetUnitExpMultiplier(struct Unit* actor, struct Unit* target);
static int GetUnitKillExpBonus(struct Unit* actor, struct Unit* target);
static void ModifyUnitSpecialExp(struct Unit* actor, struct Unit* target, int* exp);
static int GetBattleUnitExpGain(struct BattleUnit* actor, struct BattleUnit* target);
static void BattleApplyItemExpGains(void);
static int GetBattleUnitStaffExp(struct BattleUnit* bu);
static void BattleApplyMiscActionExpGains(void);

static void BattleApplyReaverEffect(struct BattleUnit* attacker, struct BattleUnit* defender);

static void ComputeBattleObstacleStats(void);

static void BattlePrintDebugUnitInfo(struct BattleUnit* actor, struct BattleUnit* target);
static void BattlePrintDebugHitInfo(void);

static void BattleGenerateHitScriptedDamage(struct BattleUnit* bu);
static void BattleUnwindScripted(void);

void BattleGenerateSimulationInternal(struct Unit* actor, struct Unit* target, int x, int y, int actorWpnSlot) {
    InitBattleUnit(&gBattleUnitA, actor);
    InitBattleUnit(&gBattleUnitB, target);

    gBattleUnitA.unit.x = x;
    gBattleUnitA.unit.y = y;

    gBattleStats.range = RECT_DISTANCE(
        gBattleUnitA.unit.x, gBattleUnitA.unit.y,
        gBattleUnitB.unit.x, gBattleUnitB.unit.y
    );

    if (gBattleStats.config & BATTLE_CONFIG_BALLISTA)
        SetBattleUnitWeaponBallista(&gBattleUnitA);
    else
        SetBattleUnitWeapon(&gBattleUnitA, actorWpnSlot);

    SetBattleUnitWeapon(&gBattleUnitB, BU_ISLOT_AUTO);

    BattleInitTargetCanCounter();
    BattleApplyWeaponTriangleEffect(&gBattleUnitA, &gBattleUnitB);

    BattleSomethingTrapChangeTerrain();

    SetBattleUnitTerrainBonusesAuto(&gBattleUnitA);
    SetBattleUnitTerrainBonusesAuto(&gBattleUnitB);

    BattleGenerate(actor, target);

    NullAllLightRunesTerrain();
}

void BattleGenerateRealInternal(struct Unit* actor, struct Unit* target) {
    InitBattleUnit(&gBattleUnitA, actor);
    InitBattleUnit(&gBattleUnitB, target);

    gBattleStats.range = RECT_DISTANCE(
        gBattleUnitA.unit.x, gBattleUnitA.unit.y,
        gBattleUnitB.unit.x, gBattleUnitB.unit.y
    );

    if (gBattleStats.config & BATTLE_CONFIG_BALLISTA)
        SetBattleUnitWeaponBallista(&gBattleUnitA);
    else
        SetBattleUnitWeapon(&gBattleUnitA, BU_ISLOT_AUTO);

    SetBattleUnitWeapon(&gBattleUnitB, BU_ISLOT_AUTO);

    BattleInitTargetCanCounter();
    BattleApplyWeaponTriangleEffect(&gBattleUnitA, &gBattleUnitB);

    BattleSomethingTrapChangeTerrain();

    SetBattleUnitTerrainBonusesAuto(&gBattleUnitA);
    SetBattleUnitTerrainBonusesAuto(&gBattleUnitB);

    BattleGenerate(actor, target);

    NullAllLightRunesTerrain();

    BattleUnitTargetCheckCanCounter(&gBattleUnitB);
    BattleUnitTargetSetEquippedWeapon(&gBattleUnitB);

    if (gBattleUnitB.unit.id != 0)
    {
        BattleApplyExpGains();
        sub_80A4AA4();

        BWL_AddBattle(actor);
        BWL_AddBattle(target);
    }
}

void BattleApplyGameStateUpdates(void) {
    BattleApplyUnitUpdates();
    BattleApplyBallistaUpdates();

    BattlePrintDebugUnitInfo(&gBattleUnitA, &gBattleUnitB);
    BattlePrintDebugHitInfo();
}

void BattleGenerateSimulation(struct Unit* actor, struct Unit* target, int x, int y, int actorWpnSlot) {
    if (x < 0 && y < 0) {
        x = actor->x;
        y = actor->y;
    }

    gBattleStats.config = BATTLE_CONFIG_SIMULATE;
    BattleGenerateSimulationInternal(actor, target, x, y, actorWpnSlot);
}

void BattleGenerateReal(struct Unit* actor, struct Unit* target) {
    gBattleStats.config = BATTLE_CONFIG_REAL;
    BattleGenerateRealInternal(actor, target);
}

void BattleGenerateBallistaSimulation(struct Unit* actor, struct Unit* target, int x, int y) {
    gBattleStats.config = BATTLE_CONFIG_SIMULATE | BATTLE_CONFIG_BALLISTA;
    BattleGenerateSimulationInternal(actor, target, x, y, 0);
}

void BattleGenerateBallistaReal(struct Unit* actor, struct Unit* target) {
    gBattleStats.config = BATTLE_CONFIG_REAL | BATTLE_CONFIG_BALLISTA;
    BattleGenerateRealInternal(actor, target);
}

void BattleGenerate(struct Unit* actor, struct Unit* target) {
    ComputeBattleUnitStats(&gBattleUnitA, &gBattleUnitB);
    ComputeBattleUnitStats(&gBattleUnitB, &gBattleUnitA);

    ComputeBattleUnitEffectiveStats(&gBattleUnitA, &gBattleUnitB);
    ComputeBattleUnitEffectiveStats(&gBattleUnitB, &gBattleUnitA);

    if (target == NULL)
        ComputeBattleObstacleStats();

    if ((gBattleStats.config & BATTLE_CONFIG_REAL) && (gAction.scriptedBattleHits))
        BattleUnwindScripted();
    else
        BattleUnwind();
}

void BattleGenerateUiStats(struct Unit* unit, s8 itemSlot) {
    gBattleStats.config = BATTLE_CONFIG_BIT2;

    gBattleUnitB.weapon = 0;
    gBattleUnitB.weaponAttributes = IA_NONE;
    gBattleUnitB.weaponType = 0xFF;

    gBattleUnitB.unit.jinfo = NULL;

    gBattleUnitA.wTriangleHitBonus = 0;
    gBattleUnitA.wTriangleDmgBonus = 0;

    if ((itemSlot >= 0) && (itemSlot < UNIT_ITEM_COUNT)) {
        struct Unit tmpUnit = *unit;

        EquipUnitItemSlot(&tmpUnit, itemSlot);
        itemSlot = 0;

        InitBattleUnit(&gBattleUnitA, &tmpUnit);
    } else
        InitBattleUnit(&gBattleUnitA, unit);

    if (gGmData.state & GMAP_STATE_BIT0)
        SetBattleUnitTerrainBonuses(&gBattleUnitA, 0); // TODO: TERRAIN ID DEFINITIONS
    else
        SetBattleUnitTerrainBonusesAuto(&gBattleUnitA);

    SetBattleUnitWeapon(&gBattleUnitA, itemSlot);
    ComputeBattleUnitStats(&gBattleUnitA, &gBattleUnitB);

    if (GetItemIndex(gBattleUnitA.weapon) == IID_RUNESWORD) {
        gBattleUnitA.battleAttack -= gBattleUnitA.unit.pow / 2;

        gBattleUnitA.battleCritRate = 0;
        gBattleUnitA.battleEffectiveCritRate = 0;
    }

    if (!gBattleUnitA.weapon) {
        gBattleUnitA.battleAttack = 0xFF;
        gBattleUnitA.battleHitRate = 0xFF;
        gBattleUnitA.battleCritRate = 0xFF;
    }

    if (GetItemWeaponEffect(gBattleUnitA.weapon) == WPN_EFFECT_HPHALVE)
        gBattleUnitA.battleAttack = 0xFF;

    if (GetItemIndex(gBattleUnitA.weapon) == IID_MONSTER_STONE) {
        gBattleUnitA.battleAttack = 0xFF;
        gBattleUnitA.battleCritRate = 0xFF;
    }
}

s8 BattleRandRoll(u16 threshold, s8 simResult) {
    if (gBattleStats.config & BATTLE_CONFIG_SIMULATE)
        return simResult;

    return RandRoll(threshold);
}

s8 BattleRandRoll2Rn(u16 threshold, s8 simResult) {
    if (gBattleStats.config & BATTLE_CONFIG_SIMULATE)
        return simResult;

    return RandRoll2Rn(threshold);
}

void InitBattleUnit(struct BattleUnit* bu, struct Unit* unit) {
    if (!unit)
        return;

    bu->unit = *unit;

    bu->unit.maxHP = GetUnitMaxHp(unit);
    bu->unit.pow = GetUnitPower(unit);
    bu->unit.skl = GetUnitSkill(unit);
    bu->unit.spd = GetUnitSpeed(unit);
    bu->unit.def = GetUnitDefense(unit);
    bu->unit.lck = GetUnitLuck(unit);
    bu->unit.res = GetUnitResistance(unit);
    bu->unit.conBonus = UNIT_CON(unit);
    bu->unit.movBonus = UNIT_MOV(unit);

    bu->levelPrevious = bu->unit.level;
    bu->expPrevious = bu->unit.exp;

    bu->hpInitial = bu->unit.curHP;
    bu->statusOut = 0xFF;

    bu->changeHP = 0;
    bu->changePow = 0;
    bu->changeSkl = 0;
    bu->changeSpd = 0;
    bu->changeDef = 0;
    bu->changeRes = 0;
    bu->changeLck = 0;
    bu->changeCon = 0;

    gBattleUnitA.wexpMultiplier = 0;
    gBattleUnitB.wexpMultiplier = 0;

    bu->wTriangleHitBonus = 0;
    bu->wTriangleDmgBonus = 0;

    bu->nonZeroDamage = FALSE;

    gBattleUnitA.weaponBroke = FALSE;
    gBattleUnitB.weaponBroke = FALSE;

    gBattleUnitA.expGain = 0;
    gBattleUnitB.expGain = 0;
}

void InitBattleUnitWithoutBonuses(struct BattleUnit* bu, struct Unit* unit) {
    InitBattleUnit(bu, unit);

    bu->unit.maxHP = unit->maxHP;
    bu->unit.pow = unit->pow;
    bu->unit.skl = unit->skl;
    bu->unit.spd = unit->spd;
    bu->unit.def = unit->def;
    bu->unit.lck = unit->lck;
    bu->unit.res = unit->res;

    bu->unit.conBonus = UNIT_CON_BASE(unit);
}

void SetBattleUnitTerrainBonuses(struct BattleUnit* bu, int terrain) {
    bu->terrainId = terrain;

    bu->terrainAvoid      = bu->unit.jinfo->pTerrainAvoidLookup[bu->terrainId];
    bu->terrainDefense    = bu->unit.jinfo->pTerrainDefenseLookup[bu->terrainId];
    bu->terrainResistance = bu->unit.jinfo->pTerrainResistanceLookup[bu->terrainId];
}

void SetBattleUnitTerrainBonusesAuto(struct BattleUnit* bu) {
    bu->terrainId = gMapTerrain[bu->unit.y][bu->unit.x];

    bu->terrainAvoid      = bu->unit.jinfo->pTerrainAvoidLookup[bu->terrainId];
    bu->terrainDefense    = bu->unit.jinfo->pTerrainDefenseLookup[bu->terrainId];
    bu->terrainResistance = bu->unit.jinfo->pTerrainResistanceLookup[bu->terrainId];
}

void SetBattleUnitWeapon(struct BattleUnit* bu, int itemSlot) {
    if (itemSlot == BU_ISLOT_AUTO)
        itemSlot = GetUnitEquippedWeaponSlot(&bu->unit);

    if (bu->unit.flags & UNIT_FLAG_IN_BALLISTA)
        itemSlot = BU_ISLOT_BALLISTA;

    bu->canCounter = TRUE;

    switch (itemSlot) {

    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        // regular item slots

        bu->weaponSlotIndex = itemSlot;
        bu->weapon = bu->unit.items[bu->weaponSlotIndex];

        break;

    case BU_ISLOT_5:
        // borrowed item?

        bu->weaponSlotIndex = 0xFF;
        bu->weapon = gBmSt.itemUnk2C;

        break;

    case BU_ISLOT_ARENA_PLAYER:
        // arena player weapon

        bu->weaponSlotIndex = 0;

        bu->weapon = gArenaSt.playerWeapon;
        bu->canCounter = FALSE;

        break;

    case BU_ISLOT_ARENA_OPPONENT:
        // arena opponent weapon

        bu->weaponSlotIndex = 0;

        bu->weapon = gArenaSt.opponentWeapon;
        bu->canCounter = FALSE;

        break;

    case BU_ISLOT_BALLISTA:
        // riding ballista

        bu->weaponSlotIndex = 0xFF;

        bu->weapon = GetBallistaItemAt(bu->unit.x, bu->unit.y);
        bu->canCounter = FALSE;

        break;

    default:
        bu->weaponSlotIndex = 0xFF;

        bu->weapon = 0;
        bu->canCounter = FALSE;

        break;

    } // switch (itemSlot)

    bu->weaponBefore = bu->weapon;
    bu->weaponAttributes = GetItemAttributes(bu->weapon);
    bu->weaponType = GetItemType(bu->weapon);

    if (!(gBattleStats.config & BATTLE_CONFIG_BIT2)) {
        if (bu->weaponAttributes & IA_MAGICDAMAGE) {
            switch (GetItemIndex(bu->weapon)) {

            case IID_WIND_SWORD:
                if (gBattleStats.range == 2)
                    bu->weaponType = ITYPE_ANIMA;
                else
                    bu->weaponAttributes = bu->weaponAttributes &~ IA_MAGICDAMAGE;

                break;

            case IID_LIGHTBRAND:
                if (gBattleStats.range == 2)
                    bu->weaponType = ITYPE_LIGHT;
                else
                    bu->weaponAttributes = bu->weaponAttributes &~ IA_MAGICDAMAGE;

                break;

            case IID_RUNESWORD:
                bu->weaponType = ITYPE_DARK;
                break;

            } // switch (GetItemIndex(bu->weapon))
        } // if (bu->weaponAttributes & IA_MAGICDAMAGE)

        if (!IsItemCoveringRange(bu->weapon, gBattleStats.range) || bu->weaponSlotIndex == 0xFF) {
            bu->weapon = 0;
            bu->canCounter = FALSE;
        }

        switch (bu->unit.statusId) {

        case UNIT_STATUS_SLEEP:
        case UNIT_STATUS_PETRIFY:
        case UNIT_STATUS_PETRIFY_2:
            bu->weapon = 0;
            bu->canCounter = FALSE;

            break;

        } // switch (bu->unit.statusId)
    }
}

void SetBattleUnitWeaponBallista(struct BattleUnit* bu) {
    bu->weapon = GetBallistaItemAt(bu->unit.x, bu->unit.y);

    bu->weaponBefore = bu->weapon;
    bu->weaponAttributes = GetItemAttributes(bu->weapon);
    bu->weaponType = GetItemType(bu->weapon);

    bu->canCounter = FALSE;
}

void sub_802A958(void) {} // unused

void ComputeBattleUnitStats(struct BattleUnit* attacker, struct BattleUnit* defender) {
    ComputeBattleUnitDefense(attacker, defender);
    ComputeBattleUnitAttack(attacker, defender);
    ComputeBattleUnitSpeed(attacker);
    ComputeBattleUnitHitRate(attacker);
    ComputeBattleUnitAvoidRate(attacker);
    ComputeBattleUnitCritRate(attacker);
    ComputeBattleUnitDodgeRate(attacker);
    ComputeBattleUnitSupportBonuses(attacker, defender);
    ComputeBattleUnitWeaponRankBonuses(attacker);
    ComputeBattleUnitStatusBonuses(attacker);
}

void ComputeBattleUnitEffectiveStats(struct BattleUnit* attacker, struct BattleUnit* defender) {
    ComputeBattleUnitEffectiveHitRate(attacker, defender);
    ComputeBattleUnitEffectiveCritRate(attacker, defender);
    ComputeBattleUnitSilencerRate(attacker, defender);
    ComputeBattleUnitSpecialWeaponStats(attacker, defender);
}

void ComputeBattleUnitSupportBonuses(struct BattleUnit* attacker, struct BattleUnit* defender) {
    if (!(gBattleStats.config & BATTLE_CONFIG_ARENA) || gPlaySt.chapterWeatherId) {
        struct SupportBonuses tmpBonuses;

        GetUnitSupportBonuses(&attacker->unit, &tmpBonuses);

        attacker->battleAttack    += tmpBonuses.bonusAttack;
        attacker->battleDefense   += tmpBonuses.bonusDefense;
        attacker->battleHitRate   += tmpBonuses.bonusHit;
        attacker->battleAvoidRate += tmpBonuses.bonusAvoid;
        attacker->battleCritRate  += tmpBonuses.bonusCrit;
        attacker->battleDodgeRate += tmpBonuses.bonusDodge;
    }
}

void ComputeBattleUnitDefense(struct BattleUnit* attacker, struct BattleUnit* defender) {
    if (GetItemAttributes(defender->weapon) & IA_MAGICDAMAGE)
        attacker->battleDefense = attacker->terrainResistance + attacker->unit.res;
    else if (GetItemAttributes(defender->weapon) & IA_MAGIC)
        attacker->battleDefense = attacker->terrainResistance + attacker->unit.res;
    else
        attacker->battleDefense = attacker->terrainDefense + attacker->unit.def;
}

void ComputeBattleUnitBaseDefense(struct BattleUnit* bu) {
    bu->battleDefense = bu->terrainDefense + bu->unit.def;
}

void ComputeBattleUnitAttack(struct BattleUnit* attacker, struct BattleUnit* defender) {
    short attack;

    attacker->battleAttack = GetItemMight(attacker->weapon) + attacker->wTriangleDmgBonus;
    attack = attacker->battleAttack;

    if (IsUnitEffectiveAgainst(&attacker->unit, &defender->unit) == TRUE)
        attack = attacker->battleAttack * 3;

    if (IsItemEffectiveAgainst(attacker->weapon, &defender->unit) == TRUE) {
        attack = attacker->battleAttack;

        switch (GetItemIndex(attacker->weapon)) {

        case IID_AUDHULMA:
        case IID_VIDOFNIR:
        case IID_GARM:
        case IID_NIDHOGG:
        case IID_EXCALIBUR:
        case IID_IVALDI:
        case IID_SIEGLINDE:
        case IID_SIEGMUND:
            attack *= 2;
            break;

        default:
            attack *= 3;
            break;

        } // switch (GetItemIndex(attacker->weapon))
    }

    attacker->battleAttack = attack;
    attacker->battleAttack += attacker->unit.pow;

    if (GetItemIndex(attacker->weapon) == IID_MONSTER_STONE)
        attacker->battleAttack = 0;
}

void ComputeBattleUnitSpeed(struct BattleUnit* bu) {
    int effWt = GetItemWeight(bu->weaponBefore);

    effWt -= bu->unit.conBonus;

    if (effWt < 0)
        effWt = 0;

    bu->battleSpeed = bu->unit.spd - effWt;

    if (bu->battleSpeed < 0)
        bu->battleSpeed = 0;
}

void ComputeBattleUnitHitRate(struct BattleUnit* bu) {
    bu->battleHitRate = (bu->unit.skl * 2) + GetItemHit(bu->weapon) + (bu->unit.lck / 2) + bu->wTriangleHitBonus;
}

void ComputeBattleUnitAvoidRate(struct BattleUnit* bu) {
    bu->battleAvoidRate = (bu->battleSpeed * 2) + bu->terrainAvoid + (bu->unit.lck);

    if (bu->battleAvoidRate < 0)
        bu->battleAvoidRate = 0;
}

void ComputeBattleUnitCritRate(struct BattleUnit* bu) {
    bu->battleCritRate = GetItemCrit(bu->weapon) + (bu->unit.skl / 2);

    if (UNIT_ATTRIBUTES(&bu->unit) & UNIT_ATTR_CRITBONUS)
        bu->battleCritRate += 15;
}

void ComputeBattleUnitDodgeRate(struct BattleUnit* bu) {
    bu->battleDodgeRate = bu->unit.lck;
}

void ComputeBattleUnitEffectiveHitRate(struct BattleUnit* attacker, struct BattleUnit* defender) {
    attacker->battleEffectiveHitRate = attacker->battleHitRate - defender->battleAvoidRate;

    if (attacker->battleEffectiveHitRate > 100)
        attacker->battleEffectiveHitRate = 100;

    if (attacker->battleEffectiveHitRate < 0)
        attacker->battleEffectiveHitRate = 0;
}

void ComputeBattleUnitEffectiveCritRate(struct BattleUnit* attacker, struct BattleUnit* defender) {
    int item, i;

    attacker->battleEffectiveCritRate = attacker->battleCritRate - defender->battleDodgeRate;

    if (GetItemIndex(attacker->weapon) == IID_MONSTER_STONE)
        attacker->battleEffectiveCritRate = 0;

    if (attacker->battleEffectiveCritRate < 0)
        attacker->battleEffectiveCritRate = 0;

    for (i = 0; (i < UNIT_ITEM_COUNT) && (item = defender->unit.items[i]); ++i) {
        if (GetItemAttributes(item) & IA_NEGATE_CRIT) {
            attacker->battleEffectiveCritRate = 0;
            break;
        }
    }
}

void ComputeBattleUnitSilencerRate(struct BattleUnit* attacker, struct BattleUnit* defender) {
    if (!(UNIT_ATTRIBUTES(&attacker->unit) & UNIT_ATTR_ASSASSIN))
        attacker->battleSilencerRate = 0;
    else {
        attacker->battleSilencerRate = 50;

        if (UNIT_ATTRIBUTES(&defender->unit) & UNIT_ATTR_BOSS)
            attacker->battleSilencerRate = 25;

        if (UNIT_ATTRIBUTES(&defender->unit) & UNIT_ATTR_FINAL_BOSS)
            attacker->battleSilencerRate = 0;
    }
}

void ComputeBattleUnitWeaponRankBonuses(struct BattleUnit* bu) {
    if (bu->weapon) {
        int wType = GetItemType(bu->weapon);

        if (wType < 8 && bu->unit.wexp[wType] >= WPN_EXP_S) {
            bu->battleHitRate += 5;
            bu->battleCritRate += 5;
        }
    }
}

void ComputeBattleUnitStatusBonuses(struct BattleUnit* bu) {
    switch (bu->unit.statusId) {

    case UNIT_STATUS_ATTACK:
        bu->battleAttack += 10;
        break;

    case UNIT_STATUS_DEFENSE:
        bu->battleDefense += 10;
        break;

    case UNIT_STATUS_CRIT:
        bu->battleCritRate += 10;
        break;

    case UNIT_STATUS_AVOID:
        bu->battleAvoidRate += 10;
        break;

    } // switch (bu->unit.statusId)
}

void ComputeBattleUnitSpecialWeaponStats(struct BattleUnit* attacker, struct BattleUnit* defender) {
    if (attacker->weaponAttributes & IA_MAGICDAMAGE) {
        switch (GetItemIndex(attacker->weapon)) {

        case IID_LIGHTBRAND:
        case IID_RUNESWORD:
        case IID_WIND_SWORD:
            attacker->battleAttack -= attacker->unit.pow / 2;

            attacker->battleCritRate = 0;
            attacker->battleEffectiveCritRate = 0;

            break;

        } // switch (GetItemIndex(attacker->weapon))
    } else {
        if (GetItemWeaponEffect(attacker->weaponBefore) == WPN_EFFECT_HPHALVE) {
            attacker->battleAttack = (defender->unit.curHP + 1) >> 1;

            if (attacker->battleAttack == 0)
                attacker->battleAttack = 1;

            defender->battleDefense = 0;

            attacker->battleCritRate = 0;
            attacker->battleEffectiveCritRate = 0;
        }

        if (attacker->weaponAttributes & IA_NEGATE_DEFENSE)
            defender->battleDefense = 0;

        if (defender->unit.statusId == UNIT_STATUS_PETRIFY || defender->unit.statusId == UNIT_STATUS_PETRIFY_2) {
            attacker->battleEffectiveHitRate = 100;

            attacker->battleEffectiveCritRate += 30;

            if (attacker->battleEffectiveCritRate > 100)
                attacker->battleEffectiveCritRate = 100;
        }
    }
}

void ClearBattleHits(void) {
    int i;

    for (i = 0; i < BATTLE_HIT_MAX; ++i) {
        gBattleHitArray[i].attributes = 0;
        gBattleHitArray[i].info = 0;
        gBattleHitArray[i].hpChange = 0;
    }

    gBattleHitIterator = gBattleHitArray;
}

void BattleUnwind(void) {
    ClearBattleHits();

    // this do { ... } while (0); is required for match
    // which is kind of neat because it implies scrapped plans for supporting some accost kind of thing

    do {
        struct BattleUnit* attacker;
        struct BattleUnit* defender;

        BattleGetBattleUnitOrder(&attacker, &defender);

        gBattleHitIterator->info |= BATTLE_HIT_INFO_BEGIN;

        if (!BattleGenerateRoundHits(attacker, defender)) {
            gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_RETALIATE;

            if (!BattleGenerateRoundHits(defender, attacker) && BattleGetFollowUpOrder(&attacker, &defender)) {
                gBattleHitIterator->attributes = BATTLE_HIT_ATTR_FOLLOWUP;

                BattleGenerateRoundHits(attacker, defender);
            }
        }
    } while (FALSE);

    gBattleHitIterator->info |= BATTLE_HIT_INFO_END;
}

void BattleGetBattleUnitOrder(struct BattleUnit** outAttacker, struct BattleUnit** outDefender) {
    *outAttacker = &gBattleUnitA;
    *outDefender = &gBattleUnitB;
}

s8 BattleGetFollowUpOrder(struct BattleUnit** outAttacker, struct BattleUnit** outDefender) {
    if (gBattleUnitB.battleSpeed > 250)
        return FALSE;

    if (ABS(gBattleUnitA.battleSpeed - gBattleUnitB.battleSpeed) < BATTLE_FOLLOWUP_SPEED_THRESHOLD)
        return FALSE;

    if (gBattleUnitA.battleSpeed > gBattleUnitB.battleSpeed) {
        *outAttacker = &gBattleUnitA;
        *outDefender = &gBattleUnitB;
    } else {
        *outAttacker = &gBattleUnitB;
        *outDefender = &gBattleUnitA;
    }

    if (GetItemWeaponEffect((*outAttacker)->weaponBefore) == WPN_EFFECT_HPHALVE)
        return FALSE;

    if (GetItemIndex((*outAttacker)->weapon) == IID_MONSTER_STONE)
        return FALSE;

    return TRUE;
}

s8 BattleGenerateRoundHits(struct BattleUnit* attacker, struct BattleUnit* defender) {
    int i, count;
    u16 attrs; // NOTE: this is a bug! attrs are 19 bits in FE8 (they're 16 bits in previous games)

    if (!attacker->weapon)
        return FALSE;

    attrs = gBattleHitIterator->attributes;
    count = GetBattleUnitHitCount(attacker);

    for (i = 0; i < count; ++i) {
        gBattleHitIterator->attributes |= attrs;

        if (BattleGenerateHit(attacker, defender))
            return TRUE;
    }

    return FALSE;
}

int GetBattleUnitHitCount(struct BattleUnit* attacker) {
    int result = 1;

    result <<= BattleCheckBraveEffect(attacker);

    return result;
}

int BattleCheckBraveEffect(struct BattleUnit* attacker) {
    if (!(attacker->weaponAttributes & IA_BRAVE))
        return FALSE;

    gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_BRAVE;
    return TRUE;
}

s8 BattleCheckTriangleAttack(struct BattleUnit* attacker, struct BattleUnit* defender) {
    s8 adjacentLookup[] = {
        -1, 0,
        0, -1,
        +1, 0,
        0, +1
    };

    int i, count = 0;

    int triangleAttackAttr = UNIT_ATTRS_TRIANGLEATTACK_ANY & UNIT_ATTRIBUTES(&attacker->unit);

    int x = defender->unit.x;
    int y = defender->unit.y;

    int faction = UNIT_FACTION(&attacker->unit);

    gBattleStats.taUnitA = NULL;
    gBattleStats.taUnitB = NULL;

    for (i = 0; i < 4; ++i) {
        int uId = gMapUnit[adjacentLookup[i*2 + 1] + y][adjacentLookup[i*2 + 0] + x];
        struct Unit* unit;

        if (!uId)
            continue;

        unit = GetUnit(uId);

        if ((uId & 0xC0) != faction)
            continue;

        if (unit->statusId == UNIT_STATUS_SLEEP)
            continue;

        if (unit->statusId == UNIT_STATUS_PETRIFY)
            continue;

        if (unit->statusId == UNIT_STATUS_PETRIFY_2)
            continue;

        if (unit->jinfo->id == JID_WYVERN_KNIGHT_F)
            continue;

        if (UNIT_ATTRIBUTES(unit) & triangleAttackAttr) {
            ++count;

            if (!gBattleStats.taUnitA)
                gBattleStats.taUnitA = unit;
            else if (!gBattleStats.taUnitB)
                gBattleStats.taUnitB = unit;
        }
    }

    return count >= 2 ? TRUE : FALSE;
}

void BattleUpdateBattleStats(struct BattleUnit* attacker, struct BattleUnit* defender) {
    gBattleStats.attack = attacker->battleAttack;
    gBattleStats.defense = defender->battleDefense;
    gBattleStats.hitRate = attacker->battleEffectiveHitRate;
    gBattleStats.critRate = attacker->battleEffectiveCritRate;
    gBattleStats.silencerRate = attacker->battleSilencerRate;
}

void BattleCheckSureShot(struct BattleUnit* attacker) {
    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_SURESHOT)
        return;

    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_PIERCE)
        return;

    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_GREATSHLD)
        return;

    switch (attacker->unit.jinfo->id) {

    case JID_SNIPER:
    case JID_SNIPER_F:
        switch (GetItemIndex(attacker->weapon)) {

        case IID_BALLISTA:
        case IID_BALLISTA_LONG:
        case IID_BALLISTA_KILLER:
            break;

        default:
            if (BattleRandRoll(attacker->unit.level, FALSE) == TRUE)
                gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_SURESHOT;

            break;

        } // switch (GetItemIndex(attacker->weapon))

        break;

    } // switch (attacker->unit.jinfo->id)
}

void BattleCheckPierce(struct BattleUnit* attacker, struct BattleUnit* defender) {
    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_SURESHOT)
        return;

    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_PIERCE)
        return;

    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_GREATSHLD)
        return;

    switch (attacker->unit.jinfo->id) {

    case JID_WYVERN_KNIGHT:
    case JID_WYVERN_KNIGHT_F:
        if (BattleRandRoll(attacker->unit.level, FALSE) == TRUE)
            gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_PIERCE;

        break;

    } // switch (attacker->unit.jinfo->id)
}

void BattleCheckGreatShield(struct BattleUnit* attacker, struct BattleUnit* defender) {
    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_MISS)
        return;

    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_SURESHOT)
        return;

    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_PIERCE)
        return;

    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_GREATSHLD)
        return;

    if (GetItemWeaponEffect(attacker->weapon) == WPN_EFFECT_POISON)
        return;

    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_MISS)
        return;

    switch (defender->unit.jinfo->id) {

    case JID_GENERAL:
    case JID_GENERAL_F:
        if (BattleRandRoll(attacker->unit.level, FALSE) == TRUE)
            gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_GREATSHLD;

        break;

    } // switch (defender->unit.jinfo->id)
}

s8 BattleCheckSilencer(struct BattleUnit* attacker, struct BattleUnit* defender) {
    switch (defender->unit.jinfo->id) {

    case JID_DEMON_KING:
        return FALSE;

    case JID_NECROMANCER:
        if (gPlaySt.chapter == 0x15) // TODO: CHAPTER ID CONSTANTS
            return FALSE;

        if (gPlaySt.chapter == 0x22) // TODO: CHAPTER ID CONSTANTS
            return FALSE;

    } // switch (defender->unit.jinfo->id)

    if (BattleRandRoll(gBattleStats.silencerRate, FALSE) == TRUE)
        return TRUE;

    return FALSE;
}

void BattleCheckPetrify(struct BattleUnit* attacker, struct BattleUnit* defender) {
    if (GetItemIndex(attacker->weapon) == IID_MONSTER_STONE)
        gBattleStats.damage = 0;
}

void BattleGenerateHitAttributes(struct BattleUnit* attacker, struct BattleUnit* defender) {
    short attack, defense;

    gBattleStats.damage = 0;

    BattleCheckSureShot(attacker);

    if (!(gBattleHitIterator->attributes & BATTLE_HIT_ATTR_SURESHOT)) {
        if (!BattleRandRoll2Rn(gBattleStats.hitRate, TRUE)) {
            gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_MISS;
            return;
        }
    }

    attack = gBattleStats.attack;
    defense = gBattleStats.defense;

    BattleCheckGreatShield(attacker, defender);

    if (!(gBattleHitIterator->attributes & BATTLE_HIT_ATTR_GREATSHLD))
        BattleCheckPierce(attacker, defender);

    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_PIERCE)
        defense = 0;

    gBattleStats.damage = attack - defense;

    if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_GREATSHLD)
        gBattleStats.damage = 0;

    if (BattleRandRoll(gBattleStats.critRate, FALSE) == TRUE) {
        if (BattleCheckSilencer(attacker, defender)) {
            gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_SILENCER;

            gBattleStats.damage = BATTLE_MAX_DAMAGE;

            gBattleHitIterator->attributes = gBattleHitIterator->attributes &~ BATTLE_HIT_ATTR_GREATSHLD;
        } else {
            gBattleHitIterator->attributes = gBattleHitIterator->attributes | BATTLE_HIT_ATTR_CRIT;
            gBattleStats.damage = gBattleStats.damage * 3;
        }
    }

    if (gBattleStats.damage > BATTLE_MAX_DAMAGE)
        gBattleStats.damage = BATTLE_MAX_DAMAGE;

    if (gBattleStats.damage < 0)
        gBattleStats.damage = 0;

    BattleCheckPetrify(attacker, defender);

    if (gBattleStats.damage != 0)
        attacker->nonZeroDamage = TRUE;
}

void BattleGenerateHitTriangleAttack(struct BattleUnit* attacker, struct BattleUnit* defender) {
    if (!(UNIT_ATTRIBUTES(&attacker->unit) & UNIT_ATTRS_TRIANGLEATTACK_ANY))
        return;

    if (gBattleStats.range != 1)
        return;

    if (!(gBattleHitIterator->info & BATTLE_HIT_INFO_BEGIN))
        return;

    if (attacker->unit.statusId == UNIT_STATUS_BERSERK)
        return;

    if (gBattleStats.config & BATTLE_CONFIG_ARENA)
        return;

    if (!BattleCheckTriangleAttack(attacker, defender))
        return;

    gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_TATTACK;

    gBattleStats.critRate = 100;
    gBattleStats.hitRate  = 100;
}

void BattleGenerateHitEffects(struct BattleUnit* attacker, struct BattleUnit* defender) {
    attacker->wexpMultiplier++;

    if (!(gBattleHitIterator->attributes & BATTLE_HIT_ATTR_MISS)) {
        if (defender->unit.jinfo->id != JID_DEMON_KING) {
            switch (GetItemWeaponEffect(attacker->weapon)) {

            case WPN_EFFECT_POISON:
                // Poison defender

                defender->statusOut = UNIT_STATUS_POISON;
                gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_POISON;

                // "Ungray" defender if it was petrified (as it won't be anymore)
                if (defender->unit.statusId == UNIT_STATUS_PETRIFY || defender->unit.statusId == UNIT_STATUS_PETRIFY_2)
                    defender->unit.flags = defender->unit.flags &~ UNIT_FLAG_TURN_ENDED;

                break;

            case WPN_EFFECT_HPHALVE:
                gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_HPHALVE;
                break;

            } // switch (GetItemWeaponEffect(attacker->weapon))
        }

        if ((GetItemWeaponEffect(attacker->weapon) == WPN_EFFECT_DEVIL) && (BattleRandRoll(31 - attacker->unit.lck, FALSE))) {
            gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_DEVIL;

            attacker->unit.curHP -= gBattleStats.damage;

            if (attacker->unit.curHP < 0)
                attacker->unit.curHP = 0;
        } else {
            if (gBattleStats.damage > defender->unit.curHP)
                gBattleStats.damage = defender->unit.curHP;

            defender->unit.curHP -= gBattleStats.damage;

            if (defender->unit.curHP < 0)
                defender->unit.curHP = 0;
        }

        if (GetItemWeaponEffect(attacker->weapon) == WPN_EFFECT_HPDRAIN) {
            if (attacker->unit.maxHP < (attacker->unit.curHP + gBattleStats.damage))
                attacker->unit.curHP = attacker->unit.maxHP;
            else
                attacker->unit.curHP += gBattleStats.damage;

            gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_HPSTEAL;
        }

        if (defender->unit.jinfo->id != JID_DEMON_KING) {
            if (GetItemWeaponEffect(attacker->weapon) == WPN_EFFECT_PETRIFY) {
                switch (gPlaySt.chapterPhaseIndex) {

                case FACTION_BLUE:
                    if (UNIT_FACTION(&defender->unit) == FACTION_BLUE)
                        defender->statusOut = UNIT_STATUS_PETRIFY_2;
                    else
                        defender->statusOut = UNIT_STATUS_PETRIFY;

                    break;

                case FACTION_RED:
                    if (UNIT_FACTION(&defender->unit) == FACTION_RED)
                        defender->statusOut = UNIT_STATUS_PETRIFY_2;
                    else
                        defender->statusOut = UNIT_STATUS_PETRIFY;

                    break;

                case FACTION_GREEN:
                    if (UNIT_FACTION(&defender->unit) == FACTION_GREEN)
                        defender->statusOut = UNIT_STATUS_PETRIFY_2;
                    else
                        defender->statusOut = UNIT_STATUS_PETRIFY;

                    break;

                } // switch (gPlaySt.chapterPhaseIndex)

                gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_PETRIFY;
            }
        }
    }

    gBattleHitIterator->hpChange = gBattleStats.damage;

    if (!(gBattleHitIterator->attributes & BATTLE_HIT_ATTR_MISS) || attacker->weaponAttributes & (IA_UNCOUNTERABLE | IA_MAGIC)) {
        attacker->weapon = GetItemAfterUse(attacker->weapon);

        if (!attacker->weapon)
            attacker->weaponBroke = TRUE;
    }
}

s8 BattleGenerateHit(struct BattleUnit* attacker, struct BattleUnit* defender) {
    if (attacker == &gBattleUnitB)
        gBattleHitIterator->info |= BATTLE_HIT_INFO_RETALIATION;

    BattleUpdateBattleStats(attacker, defender);

    BattleGenerateHitTriangleAttack(attacker, defender);
    BattleGenerateHitAttributes(attacker, defender);
    BattleGenerateHitEffects(attacker, defender);

    if (attacker->unit.curHP == 0 || defender->unit.curHP == 0) {
        attacker->wexpMultiplier++;

        gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;

        if (gBattleUnitB.unit.curHP != 0) {
            gBattleHitIterator++;
            return TRUE;
        }

        gBattleHitIterator->info |= BATTLE_HIT_INFO_KILLS_TARGET;

        gBattleHitIterator++;
        return TRUE;
    } else if (defender->statusOut == UNIT_STATUS_PETRIFY || defender->statusOut == UNIT_STATUS_PETRIFY_2) {
        gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;

        gBattleHitIterator++;
        return TRUE;
    }

    gBattleHitIterator++;
    return FALSE;
}

void BattleApplyExpGains(void) {
    if ((UNIT_FACTION(&gBattleUnitA.unit) != FACTION_BLUE) || (UNIT_FACTION(&gBattleUnitB.unit) != FACTION_BLUE)) {
        if (!(gPlaySt.chapterStateBits & CHAPTER_FLAG_7)) {
            gBattleUnitA.expGain  = GetBattleUnitExpGain(&gBattleUnitA, &gBattleUnitB);
            gBattleUnitB.expGain = GetBattleUnitExpGain(&gBattleUnitB, &gBattleUnitA);

            gBattleUnitA.unit.exp  += gBattleUnitA.expGain;
            gBattleUnitB.unit.exp += gBattleUnitB.expGain;

            CheckBattleUnitLevelUp(&gBattleUnitA);
            CheckBattleUnitLevelUp(&gBattleUnitB);
        }
    }
}

int GetStatIncrease(int growth) {
    int result = 0;

    while (growth > 100) {
        result++;
        growth -= 100;
    }

    if (RandRoll(growth))
        result++;

    return result;
}

int GetAutoleveledStatIncrease(int growth, int levelCount) {
    return GetStatIncrease((growth * levelCount) + (RandNext((growth * levelCount) / 4) - (growth * levelCount) / 8));
}

s8 CanBattleUnitGainLevels(struct BattleUnit* bu) {
    if (gBmSt.gameStateBits & 0x40)
        return TRUE;

    if (bu->unit.exp == UNIT_EXP_DISABLED)
        return FALSE;

    if (UNIT_FACTION(&bu->unit) != FACTION_BLUE)
        return FALSE;

    return TRUE;
}

void CheckBattleUnitLevelUp(struct BattleUnit* bu) {
    if (CanBattleUnitGainLevels(bu) && bu->unit.exp >= 100) {
        int growthBonus, statGainTotal;

        bu->unit.exp -= 100;
        bu->unit.level++;

        if (UNIT_ATTRIBUTES(&bu->unit) & UNIT_ATTR_TRAINEE) {
            if (bu->unit.level == 10) {
                bu->expGain -= bu->unit.exp;
                bu->unit.exp = UNIT_EXP_DISABLED;
            }
        } else if (bu->unit.level == 20) {
            bu->expGain -= bu->unit.exp;
            bu->unit.exp = UNIT_EXP_DISABLED;
        }

        growthBonus = (bu->unit.flags & UNIT_FLAG_GROWTH_BOOST) ? 5: 0;
        statGainTotal = 0;

        bu->changeHP  = GetStatIncrease(bu->unit.pinfo->growthHP + growthBonus);
        statGainTotal += bu->changeHP;

        bu->changePow = GetStatIncrease(bu->unit.pinfo->growthPow + growthBonus);
        statGainTotal += bu->changePow;

        bu->changeSkl = GetStatIncrease(bu->unit.pinfo->growthSkl + growthBonus);
        statGainTotal += bu->changeSkl;

        bu->changeSpd = GetStatIncrease(bu->unit.pinfo->growthSpd + growthBonus);
        statGainTotal += bu->changeSpd;

        bu->changeDef = GetStatIncrease(bu->unit.pinfo->growthDef + growthBonus);
        statGainTotal += bu->changeDef;

        bu->changeRes = GetStatIncrease(bu->unit.pinfo->growthRes + growthBonus);
        statGainTotal += bu->changeRes;

        bu->changeLck = GetStatIncrease(bu->unit.pinfo->growthLck + growthBonus);
        statGainTotal += bu->changeLck;

        if (statGainTotal == 0) {
            for (statGainTotal = 0; statGainTotal < 2; ++statGainTotal) {
                bu->changeHP = GetStatIncrease(bu->unit.pinfo->growthHP);

                if (bu->changeHP)
                    break;

                bu->changePow = GetStatIncrease(bu->unit.pinfo->growthPow);

                if (bu->changePow)
                    break;

                bu->changeSkl = GetStatIncrease(bu->unit.pinfo->growthSkl);

                if (bu->changeSkl)
                    break;

                bu->changeSpd = GetStatIncrease(bu->unit.pinfo->growthSpd);

                if (bu->changeSpd)
                    break;

                bu->changeDef = GetStatIncrease(bu->unit.pinfo->growthDef);

                if (bu->changeDef)
                    break;

                bu->changeRes = GetStatIncrease(bu->unit.pinfo->growthRes);

                if (bu->changeRes)
                    break;

                bu->changeLck = GetStatIncrease(bu->unit.pinfo->growthLck);

                if (bu->changeLck)
                    break;
            }
        }

        CheckBattleUnitStatCaps(GetUnit(bu->unit.id), bu);
    }
}

void ApplyUnitDefaultPromotion(struct Unit* unit) {
    const struct JInfo* promotedClass = GetJInfo(unit->jinfo->jidPromotion);

    int baseClassId = unit->jinfo->id;
    int promClassId = promotedClass->id;

    int i;

    // Apply stat ups

    unit->maxHP += promotedClass->promotionHp;

    if (unit->maxHP > promotedClass->maxHP)
        unit->maxHP = promotedClass->maxHP;

    unit->pow += promotedClass->promotionPow;

    if (unit->pow > promotedClass->maxPow)
        unit->pow = promotedClass->maxPow;

    unit->skl += promotedClass->promotionSkl;

    if (unit->skl > promotedClass->maxSkl)
        unit->skl = promotedClass->maxSkl;

    unit->spd += promotedClass->promotionSpd;

    if (unit->spd > promotedClass->maxSpd)
        unit->spd = promotedClass->maxSpd;

    unit->def += promotedClass->promotionDef;

    if (unit->def > promotedClass->maxDef)
        unit->def = promotedClass->maxDef;

    unit->res += promotedClass->promotionRes;

    if (unit->res > promotedClass->maxRes)
        unit->res = promotedClass->maxRes;

    // Remove base class' base wexp from unit wexp
    for (i = 0; i < 8; ++i)
        unit->wexp[i] -= unit->jinfo->baseWexp[i];

    // Update unit class
    unit->jinfo = promotedClass;

    // Add promoted class' base wexp to unit wexp
    for (i = 0; i < 8; ++i) {
        int wexp = unit->wexp[i];

        wexp += unit->jinfo->baseWexp[i];

        if (wexp > WPN_EXP_S)
            wexp = WPN_EXP_S;

        unit->wexp[i] = wexp;
    }

    // If Pupil -> Shaman jidPromotion, set Anima rank to 0
    if (baseClassId == JID_PUPIL && promClassId == JID_SHAMAN)
        unit->wexp[ITYPE_ANIMA] = 0;

    unit->level = 1;
    unit->exp   = 0;

    unit->curHP += promotedClass->promotionHp;

    if (unit->curHP > GetUnitMaxHp(unit))
        unit->curHP = GetUnitMaxHp(unit);
}

void ApplyUnitPromotion(struct Unit* unit, u8 jid) {
    const struct JInfo* promotedClass = GetJInfo(jid);

    int baseClassId = unit->jinfo->id;
    int promClassId = promotedClass->id;

    int i;

    // Apply stat ups

    unit->maxHP += promotedClass->promotionHp;

    if (unit->maxHP > promotedClass->maxHP)
        unit->maxHP = promotedClass->maxHP;

    unit->pow += promotedClass->promotionPow;

    if (unit->pow > promotedClass->maxPow)
        unit->pow = promotedClass->maxPow;

    unit->skl += promotedClass->promotionSkl;

    if (unit->skl > promotedClass->maxSkl)
        unit->skl = promotedClass->maxSkl;

    unit->spd += promotedClass->promotionSpd;

    if (unit->spd > promotedClass->maxSpd)
        unit->spd = promotedClass->maxSpd;

    unit->def += promotedClass->promotionDef;

    if (unit->def > promotedClass->maxDef)
        unit->def = promotedClass->maxDef;

    unit->res += promotedClass->promotionRes;

    if (unit->res > promotedClass->maxRes)
        unit->res = promotedClass->maxRes;

    // Remove base class' base wexp from unit wexp
    for (i = 0; i < 8; ++i)
        unit->wexp[i] -= unit->jinfo->baseWexp[i];

    // Update unit class
    unit->jinfo = promotedClass;

    // Add promoted class' base wexp to unit wexp
    for (i = 0; i < 8; ++i) {
        int wexp = unit->wexp[i];

        wexp += unit->jinfo->baseWexp[i];

        if (wexp > WPN_EXP_S)
            wexp = WPN_EXP_S;

        unit->wexp[i] = wexp;
    }

    // If Pupil -> Shaman jidPromotion, set Anima rank to 0
    if (baseClassId == JID_PUPIL && promClassId == JID_SHAMAN)
        unit->wexp[ITYPE_ANIMA] = 0;

    unit->level = 1;
    unit->exp   = 0;

    unit->curHP += promotedClass->promotionHp;

    if (unit->curHP > GetUnitMaxHp(unit))
        unit->curHP = GetUnitMaxHp(unit);
}

void MakeBattleUnitPromoteGains(struct BattleUnit* bu, struct Unit* unit) {
    bu->changeHP  = bu->unit.maxHP - unit->maxHP;
    bu->changePow = bu->unit.pow   - unit->pow;
    bu->changeSkl = bu->unit.skl   - unit->skl;
    bu->changeSpd = bu->unit.spd   - unit->spd;
    bu->changeDef = bu->unit.def   - unit->def;
    bu->changeRes = bu->unit.res   - unit->res;
    bu->changeLck = bu->unit.lck   - unit->lck;

    if (bu->unit.conBonus != unit->conBonus)
        bu->changeCon = bu->unit.conBonus - unit->conBonus;
    else {
        bu->changeCon = 0;
        bu->unit.conBonus = unit->conBonus;
    }
}

void CheckBattleUnitStatCaps(struct Unit* unit, struct BattleUnit* bu) {
    if ((unit->maxHP + bu->changeHP) > UNIT_MHP_MAX(unit))
        bu->changeHP = UNIT_MHP_MAX(unit) - unit->maxHP;

    if ((unit->pow + bu->changePow) > UNIT_POW_MAX(unit))
        bu->changePow = UNIT_POW_MAX(unit) - unit->pow;

    if ((unit->skl + bu->changeSkl) > UNIT_SKL_MAX(unit))
        bu->changeSkl = UNIT_SKL_MAX(unit) - unit->skl;

    if ((unit->spd + bu->changeSpd) > UNIT_SPD_MAX(unit))
        bu->changeSpd = UNIT_SPD_MAX(unit) - unit->spd;

    if ((unit->def + bu->changeDef) > UNIT_DEF_MAX(unit))
        bu->changeDef = UNIT_DEF_MAX(unit) - unit->def;

    if ((unit->res + bu->changeRes) > UNIT_RES_MAX(unit))
        bu->changeRes = UNIT_RES_MAX(unit) - unit->res;

    if ((unit->lck + bu->changeLck) > UNIT_LCK_MAX(unit))
        bu->changeLck = UNIT_LCK_MAX(unit) - unit->lck;
}

void BattleApplyUnitUpdates(void) {
    struct Unit* actor  = GetUnit(gBattleUnitA.unit.id);
    struct Unit* target = GetUnit(gBattleUnitB.unit.id);

    if (gBattleUnitA.canCounter)
        gBattleUnitA.unit.items[gBattleUnitA.weaponSlotIndex] = gBattleUnitA.weapon;

    if (gBattleUnitB.canCounter)
        gBattleUnitB.unit.items[gBattleUnitB.weaponSlotIndex] = gBattleUnitB.weapon;

    UpdateUnitFromBattle(actor, &gBattleUnitA);

    if (target)
        UpdateUnitFromBattle(target, &gBattleUnitB);
    else
        UpdateObstacleFromBattle(&gBattleUnitB);
}

// unused?
s8 sub_802C0B0(void) {
    return TRUE;
}

int GetBattleUnitUpdatedWeaponExp(struct BattleUnit* bu) {
    int i, result;

    if (UNIT_FACTION(&bu->unit) != FACTION_BLUE)
        return -1;

    if (bu->unit.curHP == 0)
        return -1;

    if (gPlaySt.chapterStateBits & CHAPTER_FLAG_7)
        return -1;

    if (gBmSt.gameStateBits & 0x40) // TODO: GAME STATE BITS CONSTANTS
        return -1;

    if (!(gBattleStats.config & BATTLE_CONFIG_ARENA)) {
        if (!bu->canCounter)
            return -1;

        if (!(bu->weaponAttributes & IA_REQUIRES_WEXP))
            return -1;

        if (bu->weaponAttributes & (IA_MAGICDAMAGE | IA_LOCK_3))
            return -1;
    }

    result = bu->unit.wexp[bu->weaponType];
    result += GetItemAwardedExp(bu->weapon) * bu->wexpMultiplier;

    for (i = 0; i < 8; ++i) {
        if (i == bu->weaponType)
            continue;

        if (bu->unit.jinfo->baseWexp[i] == WPN_EXP_S)
            continue;

        if (bu->unit.wexp[i] < WPN_EXP_S)
            continue;

        if (result >= WPN_EXP_S)
            result = (WPN_EXP_S - 1);

        break;
    }

    if (UNIT_ATTRIBUTES(&bu->unit) & UNIT_ATTR_PROMOTED) {
        if (result > WPN_EXP_S)
            result = WPN_EXP_S;
    } else if (UNIT_ATTRIBUTES(&bu->unit) & UNIT_ATTR_TRAINEE) {
        if (result > WPN_EXP_C)
            result = WPN_EXP_C;
    } else {
        if (result > WPN_EXP_A)
            result = WPN_EXP_A;
    }

    return result;
}

s8 HasBattleUnitGainedWeaponLevel(struct BattleUnit* bu) {
    int oldWexp = bu->unit.wexp[bu->weaponType];
    int newWexp = GetBattleUnitUpdatedWeaponExp(bu);

    if (newWexp < 0)
        return FALSE;

    return GetWeaponLevelFromExp(oldWexp) != GetWeaponLevelFromExp(newWexp);
}

void UpdateUnitFromBattle(struct Unit* unit, struct BattleUnit* bu) {
    int tmp;

    unit->level = bu->unit.level;
    unit->exp   = bu->unit.exp;
    unit->curHP = bu->unit.curHP;
    unit->flags = bu->unit.flags;

    gUnknown_03003060 = UNIT_ARENA_LEVEL(unit);

    if (bu->statusOut >= 0)
        SetUnitStatus(unit, bu->statusOut);

    unit->maxHP += bu->changeHP;
    unit->pow   += bu->changePow;
    unit->skl   += bu->changeSkl;
    unit->spd   += bu->changeSpd;
    unit->def   += bu->changeDef;
    unit->res   += bu->changeRes;
    unit->lck   += bu->changeLck;

    UnitCheckStatOverflow(unit);

    tmp = GetBattleUnitUpdatedWeaponExp(bu);

    if (tmp > 0)
        unit->wexp[bu->weaponType] = tmp;

    for (tmp = 0; tmp < UNIT_ITEM_COUNT; ++tmp)
        unit->items[tmp] = bu->unit.items[tmp];

    UnitRemoveInvalidItems(unit);

    if (bu->expGain)
        BWL_AddExpGained(unit->pinfo->id, bu->expGain);
}

void UpdateUnitDuringBattle(struct Unit* unit, struct BattleUnit* bu) {
    int wexp;

    unit->curHP = bu->unit.curHP;

    wexp = GetBattleUnitUpdatedWeaponExp(bu);

    if (wexp > 0)
        unit->wexp[bu->weaponType] = wexp;
}

void BattleApplyBallistaUpdates(void) {
    if (gBattleStats.config & BATTLE_CONFIG_BALLISTA) {
        int uses = GetItemUses(gBattleUnitA.weapon);
        GetTrap(gBattleUnitA.unit.ballistaId)->data[TRAP_EXTDATA_BLST_ITEMUSES] = uses;
    }
}

// ???
void sub_802C334(void) {
    sUnknown_0203A60C.unk00 = 0;
    sUnknown_0203A60C.unk01 = 0;
    sUnknown_0203A60C.unk02 = 0;
}

int GetUnitExpLevel(struct Unit* unit) {
    int result = unit->level;

    if (UNIT_ATTRIBUTES(unit) & UNIT_ATTR_PROMOTED)
        result += 20;

    return result;
}

int GetUnitRoundExp(struct Unit* actor, struct Unit* target) {
    int expLevel;

    expLevel = GetUnitExpLevel(actor);
    expLevel = expLevel - GetUnitExpLevel(target);
    expLevel = 31 - expLevel;

    if (expLevel < 0)
        expLevel = 0;

    return expLevel / actor->jinfo->classRelativePower;
}

int GetUnitPowerLevel(struct Unit* unit) {
    int result = unit->level * unit->jinfo->classRelativePower;

    if ((UNIT_ATTRIBUTES(unit) & UNIT_ATTR_PROMOTED) && unit->jinfo->jidPromotion)
        result = result + 20 * GetJInfo(unit->jinfo->jidPromotion)->classRelativePower;

    return result;
}

int GetUnitClassKillExpBonus(struct Unit* actor, struct Unit* target) {
    int result = 0;

    if (UNIT_ATTRIBUTES(target) & UNIT_ATTR_THIEF)
        result += 20;

    if (UNIT_ATTRIBUTES(target) & UNIT_ATTR_BOSS)
        result += 40;

    if (target->jinfo->id == JID_ENTOUMBED)
        result += 40;

    return result;
}

int GetUnitExpMultiplier(struct Unit* actor, struct Unit* target) {
    int i;

    if (!(UNIT_ATTRIBUTES(actor) & UNIT_ATTR_ASSASSIN))
        return 1;

    for (i = 0; i < BATTLE_HIT_MAX; ++i)
        if (gBattleHitArray[i].attributes & BATTLE_HIT_ATTR_SILENCER)
            return 2;

    return 1;
}

int GetUnitKillExpBonus(struct Unit* actor, struct Unit* target) {
    int result;

    if (target->curHP != 0)
        return 0;

    result = 20;

    // TODO: All the definitions
    if ((gBmSt.gameStateBits & 0x40) || (gPlaySt.chapterModeIndex != 1)) {
        result = GetUnitPowerLevel(target);

        result += 20;
        result -= GetUnitPowerLevel(actor);
    } else {
        int local = GetUnitPowerLevel(target);

        if (local <= GetUnitPowerLevel(actor))
            local = GetUnitPowerLevel(target) - GetUnitPowerLevel(actor) / 2;
        else
            local = GetUnitPowerLevel(target) - GetUnitPowerLevel(actor);

        result += local;
    }

    result += GetUnitClassKillExpBonus(actor, target);
    result *= GetUnitExpMultiplier(actor, target);

    if (result < 0)
        result = 0;

    return result;
}

void ModifyUnitSpecialExp(struct Unit* actor, struct Unit* target, int* exp) {
    if (UNIT_IS_GORGON_EGG(target)) {
        if (target->curHP == 0)
            *exp = 50;
        else
            *exp = 0;
    }

    if (target->jinfo->id == JID_DEMON_KING)
        if (target->curHP == 0)
            *exp = 0;

    if (actor->jinfo->id == JID_PHANTOM)
        *exp = 0;
}

int GetBattleUnitExpGain(struct BattleUnit* actor, struct BattleUnit* target) {
    int result;

    if (!CanBattleUnitGainLevels(actor) || (actor->unit.curHP == 0) || UNIT_ATTRIBUTES(&target->unit) & UNIT_ATTR_FINAL_BOSS)
        return 0;

    if (!actor->nonZeroDamage)
        return 1;

    result = GetUnitRoundExp(&actor->unit, &target->unit);
    result += GetUnitKillExpBonus(&actor->unit, &target->unit);

    if (result > 100)
        result = 100;

    if (result < 1)
        result = 1;

    ModifyUnitSpecialExp(&actor->unit, &target->unit, &result);

    return result;
}

void BattleApplyItemExpGains(void) {
    if (!(gPlaySt.chapterStateBits & CHAPTER_FLAG_7)) {
        if (gBattleUnitA.weaponAttributes & IA_STAFF) {
            if (UNIT_FACTION(&gBattleUnitA.unit) == FACTION_BLUE)
                gBattleUnitA.wexpMultiplier++;

            gBattleUnitA.expGain = GetBattleUnitStaffExp(&gBattleUnitA);
            gBattleUnitA.unit.exp += gBattleUnitA.expGain;

            CheckBattleUnitLevelUp(&gBattleUnitA);
        } else if ((gBattleUnitA.weaponType == ITYPE_12) && (gBattleUnitA.unit.exp != UNIT_EXP_DISABLED)) {
            gBattleUnitA.expGain = 20;
            gBattleUnitA.unit.exp += 20;

            CheckBattleUnitLevelUp(&gBattleUnitA);
        }
    }
}

int GetBattleUnitStaffExp(struct BattleUnit* bu) {
    int result;

    if (!CanBattleUnitGainLevels(bu))
        return 0;

    if (gBattleHitArray->attributes & BATTLE_HIT_ATTR_MISS)
        return 1;

    result = 10 + GetItemCostPerUse(bu->weapon) / 20;

    if (UNIT_ATTRIBUTES(&bu->unit) & UNIT_ATTR_PROMOTED)
        result = result / 2;

    if (result > 100)
        result = 100;

    return result;
}

void BattleApplyMiscActionExpGains(void) {
    if (UNIT_FACTION(&gBattleUnitA.unit) != FACTION_BLUE)
        return;

    if (!CanBattleUnitGainLevels(&gBattleUnitA))
        return;

    if (gPlaySt.chapterStateBits & CHAPTER_FLAG_7)
        return;

    gBattleUnitA.expGain = 10;
    gBattleUnitA.unit.exp += 10;

    CheckBattleUnitLevelUp(&gBattleUnitA);
}

void BattleUnitTargetSetEquippedWeapon(struct BattleUnit* bu) {
    int i;

    if (bu->weaponBefore)
        return;

    bu->weaponBefore = GetUnitEquippedWeapon(&bu->unit);

    if (bu->weaponBefore)
        return;

    if (!UnitKnowsMagic(&bu->unit))
        return;

    for (i = 0; i < UNIT_ITEM_COUNT; ++i) {
        if (CanUnitUseStaff(&bu->unit, bu->unit.items[i]) == TRUE) {
            bu->weaponBefore = bu->unit.items[i];
            break;
        }
    }
}

void BattleUnitTargetCheckCanCounter(struct BattleUnit* bu) {
    if (!bu->canCounter) {
        bu->battleAttack = 0xFF;
        bu->battleHitRate = 0xFF;
        bu->battleEffectiveHitRate = 0xFF;
        bu->battleCritRate = 0xFF;
        bu->battleEffectiveCritRate = 0xFF;
    }
}

void BattleApplyReaverEffect(struct BattleUnit* attacker, struct BattleUnit* defender) {
    if (!(attacker->weaponAttributes & IA_REVERTTRIANGLE) || !(defender->weaponAttributes & IA_REVERTTRIANGLE)) {
        attacker->wTriangleHitBonus = -(attacker->wTriangleHitBonus * 2);
        attacker->wTriangleDmgBonus = -(attacker->wTriangleDmgBonus * 2);
        defender->wTriangleHitBonus = -(defender->wTriangleHitBonus * 2);
        defender->wTriangleDmgBonus = -(defender->wTriangleDmgBonus * 2);
    }
}

void BattleApplyWeaponTriangleEffect(struct BattleUnit* attacker, struct BattleUnit* defender) {
    const struct WeaponTriangleRule* it;

    for (it = sWeaponTriangleRules; it->attackerWeaponType >= 0; ++it) {
        if ((attacker->weaponType == it->attackerWeaponType) && (defender->weaponType == it->defenderWeaponType)) {
            attacker->wTriangleHitBonus = it->hitBonus;
            attacker->wTriangleDmgBonus = it->atkBonus;

            defender->wTriangleHitBonus = -it->hitBonus;
            defender->wTriangleDmgBonus = -it->atkBonus;

            break;
        }
    }

    if (attacker->weaponAttributes & IA_REVERTTRIANGLE)
        BattleApplyReaverEffect(attacker, defender);

    if (defender->weaponAttributes & IA_REVERTTRIANGLE)
        BattleApplyReaverEffect(attacker, defender);
}

void BattleInitTargetCanCounter(void) {
    // Target cannot counter if it is a gorgon egg

    if (UNIT_IS_GORGON_EGG(&gBattleUnitB.unit)) {
        gBattleUnitB.weapon = 0;
        gBattleUnitB.canCounter = FALSE;
    }

    // Target cannot counter if either units are using "uncounterable" weapons

    if ((gBattleUnitA.weaponAttributes | gBattleUnitB.weaponAttributes) & IA_UNCOUNTERABLE) {
        gBattleUnitB.weapon = 0;
        gBattleUnitB.canCounter = FALSE;
    }

    // Target cannot counter if a berserked player unit is attacking another player unit

    if (gBattleUnitA.unit.statusId == UNIT_STATUS_BERSERK) {
        if ((UNIT_FACTION(&gBattleUnitA.unit) == FACTION_BLUE) && (UNIT_FACTION(&gBattleUnitB.unit) == FACTION_BLUE)) {
            gBattleUnitB.weapon = 0;
            gBattleUnitB.canCounter = FALSE;
        }
    }
}

void InitObstacleBattleUnit(void) {
    ClearUnit(&gBattleUnitB.unit);

    gBattleUnitB.unit.id = 0;

    gBattleUnitB.unit.jinfo = GetJInfo(JID_OBSTACLE);

    gBattleUnitB.unit.maxHP = GetChapterInfo(gPlaySt.chapter)->mapCrackedWallHeath;
    gBattleUnitB.unit.curHP = gAction.trapType; // TODO: better

    gBattleUnitB.unit.x  = gAction.xOther;
    gBattleUnitB.unit.y  = gAction.yOther;

    switch (gMapTerrain[gBattleUnitB.unit.y][gBattleUnitB.unit.x]) {

    case TERRAIN_WALL_1B:
        gBattleUnitB.unit.pinfo = GetPInfo(PID_WALL);

        break;

    case TERRAIN_SNAG:
        gBattleUnitB.unit.pinfo = GetPInfo(PID_SNAG);
        gBattleUnitB.unit.maxHP = 20;

        break;

    } // switch (gMapTerrain[gBattleUnitB.unit.y][gBattleUnitB.unit.x])
}

void ComputeBattleObstacleStats(void) {
    gBattleUnitA.battleEffectiveHitRate = 100;
    gBattleUnitA.battleEffectiveCritRate = 0;

    gBattleUnitB.battleSpeed = 0xFF;
    gBattleUnitB.hpInitial = gBattleUnitB.unit.curHP;

    gBattleUnitB.wTriangleHitBonus = 0;
    gBattleUnitB.wTriangleDmgBonus = 0;
}

void UpdateObstacleFromBattle(struct BattleUnit* bu) {
    struct Trap* trap = GetTrapAt(bu->unit.x, bu->unit.y);

    trap->extra = bu->unit.curHP;

    if (trap->extra == 0) {
        int mapChangeId = GetMapChangesIdAt(bu->unit.x, bu->unit.y);

        if (gMapTerrain[bu->unit.y][bu->unit.x] == TERRAIN_SNAG)
            PlaySe(0x2D7); // TODO: Sound id constants

        RenderBmMapOnBg2();

        ApplyMapChangesById(mapChangeId);

        // This is kind of jank: it sets trap type to 0 (TRAP_NONE) which should be the terminating id
        // But then immediately calls the map change trap adding routine, which would effectively replace
        // the 0-id trap with the new map change trap, even if it is not actually the end of the trap array

        trap->type = TRAP_NONE;
        AddMapChange(mapChangeId);

        RefreshTerrainBmMap();
        UpdateUnitsUnderRoof();
        RenderBmMap();

        StartBMXFADE(FALSE);
    }
}

void BeginBattleAnimations(void) {
    TmFill(gBg2Tm, 0);
    EnableBgSync(BG2_SYNC_BIT);

    SetBackdropColor(0);

    RenderBmMap();

    if (sub_8055BC4()) {
        SetBattleAnimLinkArenaFlag(0);
        BeginAnimsOnBattleAnimations();
    } else {
        MU_EndAll();
        RenderBmMap();
        BeginBattleMapAnims();

        gBattleStats.config |= BATTLE_CONFIG_MAPANIMS;
    }
}

int GetUnitSoloBattleAnimType(struct Unit* unit) {
    // TODO: battle anim type constants

    if (unit->flags & UNIT_FLAG_SOLOANIM_A)
        return 0;

    if (unit->flags & UNIT_FLAG_SOLOANIM_B)
        return 3;

    return 1;
}

int GetBattleAnimType(void) {
    // TODO: battle anim type constants

    // If not solo anim, return global type
    if (gPlaySt.configBattleAnim != 2)
        return gPlaySt.configBattleAnim;

    // If both units are players, use actor solo anim type
    if (UNIT_FACTION(&gBattleUnitA.unit) == FACTION_BLUE)
        if (UNIT_FACTION(&gBattleUnitB.unit) == FACTION_BLUE)
            return GetUnitSoloBattleAnimType(&gBattleUnitA.unit);

    // If neither are players, return 1
    if (UNIT_FACTION(&gBattleUnitA.unit) != FACTION_BLUE)
        if (UNIT_FACTION(&gBattleUnitB.unit) != FACTION_BLUE)
            return 1;

    // Return solo anim type for the one that is a player unit
    if (UNIT_FACTION(&gBattleUnitA.unit) == FACTION_BLUE)
        return GetUnitSoloBattleAnimType(&gBattleUnitA.unit);
    else
        return GetUnitSoloBattleAnimType(&gBattleUnitB.unit);
}

void BattlePrintDebugUnitInfo(struct BattleUnit* actor, struct BattleUnit* target) {
    // prints battle unit information to debug output
}

void BattlePrintDebugHitInfo(void) {
    struct BattleHit* it;

    for (it = gBattleHitArray; !(it->info & BATTLE_HIT_INFO_END); ++it) {
        // prints battle rounds information to debug output
    }
}

void BattleInitItemEffect(struct Unit* actor, int itemSlot) {
    int item = actor->items[itemSlot];

    if (itemSlot < 0)
        item = 0;

    gBattleStats.config = 0;

    InitBattleUnit(&gBattleUnitA, actor);

    SetBattleUnitTerrainBonusesAuto(&gBattleUnitA);
    ComputeBattleUnitBaseDefense(&gBattleUnitA);
    ComputeBattleUnitSupportBonuses(&gBattleUnitA, NULL);

    gBattleUnitA.battleAttack = 0xFF;
    gBattleUnitA.battleEffectiveHitRate = 100;
    gBattleUnitA.battleEffectiveCritRate = 0xFF;

    gBattleUnitA.weapon = item;
    gBattleUnitA.weaponBefore = item;
    gBattleUnitA.weaponSlotIndex = itemSlot;
    gBattleUnitA.weaponType = GetItemType(item);
    gBattleUnitA.weaponAttributes = GetItemAttributes(item);

    gBattleUnitA.canCounter = TRUE;
    gBattleUnitA.hasItemEffectTarget = FALSE;

    gBattleUnitA.statusOut = -1;
    gBattleUnitB.statusOut = -1;

    ClearBattleHits();
}

void BattleInitItemEffectTarget(struct Unit* unit) {
    InitBattleUnit(&gBattleUnitB, unit);

    SetBattleUnitTerrainBonusesAuto(&gBattleUnitB);
    ComputeBattleUnitBaseDefense(&gBattleUnitB);
    ComputeBattleUnitSupportBonuses(&gBattleUnitB, NULL);

    gBattleUnitB.battleAttack = 0xFF;
    gBattleUnitB.battleEffectiveHitRate = 0xFF;
    gBattleUnitB.battleEffectiveCritRate = 0xFF;

    gBattleUnitB.weaponBefore = 0;

    BattleUnitTargetSetEquippedWeapon(&gBattleUnitB);

    gBattleUnitA.hasItemEffectTarget = TRUE;
}

void UpdateActorFromBattle(void) {
    UpdateUnitFromBattle(GetUnit(gBattleUnitA.unit.id), &gBattleUnitA);
}

void BattleApplyMiscAction(struct Proc* proc) {
    BattleApplyMiscActionExpGains();
    SpawnProcLocking(sProcScr_BattleAnimSimpleLock, proc);
}

void BattleApplyItemEffect(struct Proc* proc) {
    (++gBattleHitIterator)->info = BATTLE_HIT_INFO_END;

    BattleApplyItemExpGains();

    if (gBattleUnitA.canCounter) {
        if (GetItemAttributes(gBattleUnitA.weapon) & IA_STAFF)
            gBattleUnitA.weaponBroke = TRUE;

        gBattleUnitA.weapon = GetItemAfterUse(gBattleUnitA.weapon);
        gBattleUnitA.unit.items[gBattleUnitA.weaponSlotIndex] = gBattleUnitA.weapon;

        if (gBattleUnitA.weapon)
            gBattleUnitA.weaponBroke = FALSE;
    }

    SpawnProcLocking(sProcScr_BattleAnimSimpleLock, proc);
}

int GetOffensiveStaffAccuracy(struct Unit* actor, struct Unit* target) {
    int baseAccuracy = (GetUnitPower(actor) - GetUnitResistance(target)) * 5;
    int unitSkill = GetUnitSkill(actor);
    int distance = RECT_DISTANCE(actor->x, actor->y, target->x, target->y);

    int result;

    if (actor->jinfo->id == JID_DEMON_KING)
        result = (baseAccuracy + unitSkill) - distance * 2;
    else
        result = (baseAccuracy + 30 + unitSkill) - distance * 2;

    if ((target->jinfo->id == JID_DEMON_KING) || (target->pinfo->id == PID_LYON) || (target->pinfo->id == PID_LYON_FINAL))
        return 0;

    if (result < 0)
        result = 0;

    if (result > 100)
        result = 100;

    return result;
}

void BattleGenerateArena(struct Unit* actor) {
    struct Unit* target = gArenaSt.opponentUnit;
    int something = gBmSt.unk3C;

    gBattleStats.config = BATTLE_CONFIG_REAL | BATTLE_CONFIG_ARENA;

    InitBattleUnit(&gBattleUnitA, actor);
    InitBattleUnit(&gBattleUnitB, target);

    if (gAction.trapType) {
        gBattleUnitB.unit.curHP = gAction.trapType;
        gBattleUnitB.hpInitial = gAction.trapType;
    }

    gBattleStats.range = gArenaSt.range;

    gBattleUnitB.unit.x = gBattleUnitA.unit.x + gArenaSt.range;
    gBattleUnitB.unit.y = gBattleUnitA.unit.y;

    SetBattleUnitWeapon(&gBattleUnitA, BU_ISLOT_ARENA_PLAYER);
    SetBattleUnitWeapon(&gBattleUnitB, BU_ISLOT_ARENA_OPPONENT);

    BattleApplyWeaponTriangleEffect(&gBattleUnitA, &gBattleUnitB);

    gAction.suspendPointType = SUSPEND_POINT_DURINGARENA;
    SaveSuspendedGame(SAVE_BLOCK_SUSPEND_BASE);

    SetBattleUnitTerrainBonusesAuto(&gBattleUnitA);
    SetBattleUnitTerrainBonuses(&gBattleUnitB, 8); // TODO: terrain id constants

    BattleGenerate(actor, target);

    if (gBattleUnitB.unit.curHP == 0)
        BattleApplyExpGains();

    UpdateUnitDuringBattle(actor, &gBattleUnitA);

    if (!something || (gBattleUnitB.unit.curHP == 0)) {
        sub_80A4AA4();

        actor->flags = (actor->flags &~ (UNIT_FLAG_ARENA_A | UNIT_FLAG_ARENA_B | UNIT_FLAG_ARENA_C))
            + ((((UNIT_ARENA_LEVEL(actor) + 1) <= 7) ? (UNIT_ARENA_LEVEL(actor) + 1) << 17 : 7 << 17));

        gUnknown_03003060 = UNIT_ARENA_LEVEL(actor);
    }

    BattlePrintDebugUnitInfo(&gBattleUnitA, &gBattleUnitB);
}

s8 BattleIsTriangleAttack(void) {
    return (gBattleHitArray->attributes & BATTLE_HIT_ATTR_TATTACK) != 0;
}

s8 DidBattleUnitBreakWeapon(struct BattleUnit* bu) {
    if (bu->unit.curHP == 0)
        return FALSE;

    return bu->weaponBroke;
}

void SetScriptedBattle(struct BattleHit* hits) {
    gAction.scriptedBattleHits = hits;
}

void BattleGenerateHitScriptedDamage(struct BattleUnit* bu) {
    gBattleStats.damage = 0;

    if (!(gBattleHitIterator->attributes & BATTLE_HIT_ATTR_MISS)) {
        if (gBattleHitIterator->hpChange == 0) {
            gBattleStats.damage = gBattleStats.attack - gBattleStats.defense;

            if (gBattleHitIterator->attributes & BATTLE_HIT_ATTR_CRIT)
                gBattleStats.damage = 3 * gBattleStats.damage;
        } else
            gBattleStats.damage = gBattleHitIterator->hpChange;

        if (gBattleStats.damage > BATTLE_MAX_DAMAGE)
            gBattleStats.damage = BATTLE_MAX_DAMAGE;

        if (gBattleStats.damage < 0)
            gBattleStats.damage = 0;

        if (gBattleStats.damage != 0)
            bu->nonZeroDamage = TRUE;
    }
}

#if NONMATCHING

void BattleUnwindScripted(void) {
    struct BattleUnit* attacker;
    struct BattleUnit* defender;

    struct BattleHit* itIn;
    struct BattleHit* itOut;

    itIn = gAction.scriptedBattleHits;
    itOut = gBattleHitArray;

    while (!(itIn->info & BATTLE_HIT_INFO_END))
        *itOut++ = *itIn++;

    *itOut = *itIn;

    for (gBattleHitIterator = gBattleHitArray; !(gBattleHitIterator->info & BATTLE_HIT_INFO_END); ++gBattleHitIterator) {
        if (gBattleHitIterator->info & BATTLE_HIT_INFO_RETALIATION) {
            attacker = &gBattleUnitB;
            defender = &gBattleUnitA;
        } else {
            attacker = &gBattleUnitA;
            defender = &gBattleUnitB;
        }

        BattleUpdateBattleStats(attacker, defender);
        BattleGenerateHitScriptedDamage(attacker);
        BattleGenerateHitEffects(attacker, defender);

        if ((attacker->unit.curHP == 0) || (defender->unit.curHP == 0)) {
            attacker->wexpMultiplier++;

            gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;

            if (gBattleUnitB.unit.curHP == 0)
                gBattleHitIterator->info |= BATTLE_HIT_INFO_KILLS_TARGET;

            (gBattleHitIterator + 1)->info = BATTLE_HIT_INFO_END;

            break;
        }

        if (
            (defender->unit.statusId == UNIT_STATUS_PETRIFY) ||
            (defender->unit.statusId == UNIT_STATUS_PETRIFY_2) ||
            (defender->statusOut == UNIT_STATUS_PETRIFY) ||
            (defender->statusOut == UNIT_STATUS_PETRIFY_2)
        ) {
            attacker->wexpMultiplier++;

            gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;

            (gBattleHitIterator + 1)->info = BATTLE_HIT_INFO_END;

            break;
        }
    }

    gAction.scriptedBattleHits = NULL;
}

#else // if !NONMATCHING

__attribute__((naked))
void BattleUnwindScripted(void) {
    /* :( */

    asm("\n\
        .syntax unified\n\
        push {r4, r5, r6, r7, lr}\n\
        ldr r0, _0802CFA4  @ gAction\n\
        ldr r3, [r0, #0x18]\n\
        ldr r4, _0802CFA8  @ gBattleHitArray\n\
        ldr r2, [r3]\n\
        lsls r0, r2, #8\n\
        lsrs r0, r0, #0x1b\n\
        movs r1, #0x10\n\
        ands r0, r1\n\
        adds r5, r4, #0\n\
        ldr r6, _0802CFAC  @ gBattleHitIterator\n\
        cmp r0, #0\n\
        bne _0802CF76\n\
    _0802CF66:\n\
        stm r4!, {r2}\n\
        adds r3, #4\n\
        ldr r2, [r3]\n\
        lsls r0, r2, #8\n\
        lsrs r0, r0, #0x1b\n\
        ands r0, r1\n\
        cmp r0, #0\n\
        beq _0802CF66\n\
    _0802CF76:\n\
        ldr r0, [r3]\n\
        str r0, [r4]\n\
        str r5, [r6]\n\
        ldr r0, [r5]\n\
        lsls r0, r0, #8\n\
        lsrs r0, r0, #0x1b\n\
        movs r1, #0x10\n\
        ands r0, r1\n\
        cmp r0, #0\n\
        beq _0802CF8C\n\
        b _0802D0AC\n\
    _0802CF8C:\n\
        movs r7, #7\n\
    _0802CF8E:\n\
        ldr r0, [r6]\n\
        ldr r0, [r0]\n\
        lsls r0, r0, #8\n\
        lsrs r0, r0, #0x1b\n\
        movs r1, #8\n\
        ands r0, r1\n\
        cmp r0, #0\n\
        beq _0802CFB8\n\
        ldr r4, _0802CFB0  @ gBattleUnitB\n\
        ldr r5, _0802CFB4  @ gBattleUnitA\n\
        b _0802CFBC\n\
        .align 2, 0\n\
    _0802CFA4: .4byte gAction\n\
    _0802CFA8: .4byte gBattleHitArray\n\
    _0802CFAC: .4byte gBattleHitIterator\n\
    _0802CFB0: .4byte gBattleUnitB\n\
    _0802CFB4: .4byte gBattleUnitA\n\
    _0802CFB8:\n\
        ldr r4, _0802D034  @ gBattleUnitA\n\
        ldr r5, _0802D038  @ gBattleUnitB\n\
    _0802CFBC:\n\
        adds r0, r4, #0\n\
        adds r1, r5, #0\n\
        bl BattleUpdateBattleStats\n\
        adds r0, r4, #0\n\
        bl BattleGenerateHitScriptedDamage\n\
        adds r0, r4, #0\n\
        adds r1, r5, #0\n\
        bl BattleGenerateHitEffects\n\
        movs r0, #0x13\n\
        ldrsb r0, [r4, r0]\n\
        cmp r0, #0\n\
        beq _0802CFE2\n\
        movs r0, #0x13\n\
        ldrsb r0, [r5, r0]\n\
        cmp r0, #0\n\
        bne _0802D040\n\
    _0802CFE2:\n\
        adds r1, r4, #0\n\
        adds r1, #0x7b\n\
        ldrb r0, [r1]\n\
        adds r0, #1\n\
        strb r0, [r1]\n\
        ldr r5, _0802D03C  @ gBattleHitIterator\n\
        ldr r3, [r5]\n\
        ldr r1, [r3]\n\
        lsls r1, r1, #8\n\
        lsrs r1, r1, #0x1b\n\
        movs r0, #2\n\
        orrs r1, r0\n\
        lsls r1, r1, #3\n\
        ldrb r2, [r3, #2]\n\
        movs r4, #7\n\
        adds r0, r4, #0\n\
        ands r0, r2\n\
        orrs r0, r1\n\
        strb r0, [r3, #2]\n\
        ldr r0, _0802D038  @ gBattleUnitB\n\
        ldrb r0, [r0, #0x13]\n\
        lsls r0, r0, #0x18\n\
        asrs r0, r0, #0x18\n\
        cmp r0, #0\n\
        bne _0802D02C\n\
        ldr r3, [r5]\n\
        ldr r1, [r3]\n\
        lsls r1, r1, #8\n\
        lsrs r1, r1, #0x1b\n\
        movs r0, #4\n\
        orrs r1, r0\n\
        lsls r1, r1, #3\n\
        ldrb r2, [r3, #2]\n\
        adds r0, r4, #0\n\
        ands r0, r2\n\
        orrs r0, r1\n\
        strb r0, [r3, #2]\n\
    _0802D02C:\n\
        ldr r2, [r5]\n\
        ldrb r1, [r2, #6]\n\
        adds r0, r4, #0\n\
        b _0802D08C\n\
        .align 2, 0\n\
    _0802D034: .4byte gBattleUnitA\n\
    _0802D038: .4byte gBattleUnitB\n\
    _0802D03C: .4byte gBattleHitIterator\n\
    _0802D040:\n\
        adds r0, r5, #0\n\
        adds r0, #0x30\n\
        ldrb r0, [r0]\n\
        movs r1, #0xf\n\
        ands r1, r0\n\
        cmp r1, #0xb\n\
        beq _0802D064\n\
        cmp r1, #0xd\n\
        beq _0802D064\n\
        adds r0, r5, #0\n\
        adds r0, #0x6f\n\
        ldrb r0, [r0]\n\
        lsls r0, r0, #0x18\n\
        asrs r0, r0, #0x18\n\
        cmp r0, #0xb\n\
        beq _0802D064\n\
        cmp r0, #0xd\n\
        bne _0802D096\n\
    _0802D064:\n\
        adds r1, r4, #0\n\
        adds r1, #0x7b\n\
        ldrb r0, [r1]\n\
        adds r0, #1\n\
        strb r0, [r1]\n\
        ldr r3, [r6]\n\
        ldr r1, [r3]\n\
        lsls r1, r1, #8\n\
        lsrs r1, r1, #0x1b\n\
        movs r0, #2\n\
        orrs r1, r0\n\
        lsls r1, r1, #3\n\
        ldrb r2, [r3, #2]\n\
        adds r0, r7, #0\n\
        ands r0, r2\n\
        orrs r0, r1\n\
        strb r0, [r3, #2]\n\
        ldr r2, [r6]\n\
        ldrb r1, [r2, #6]\n\
        adds r0, r7, #0\n\
    _0802D08C:\n\
        ands r0, r1\n\
        movs r1, #0x80\n\
        orrs r0, r1\n\
        strb r0, [r2, #6]\n\
        b _0802D0AC\n\
    _0802D096:\n\
        ldr r1, [r6]\n\
        adds r0, r1, #4\n\
        str r0, [r6]\n\
        ldr r0, [r1, #4]\n\
        lsls r0, r0, #8\n\
        lsrs r0, r0, #0x1b\n\
        movs r1, #0x10\n\
        ands r0, r1\n\
        cmp r0, #0\n\
        bne _0802D0AC\n\
        b _0802CF8E\n\
    _0802D0AC:\n\
        ldr r1, _0802D0B8  @ gAction\n\
        movs r0, #0\n\
        str r0, [r1, #0x18]\n\
        pop {r4, r5, r6, r7}\n\
        pop {r0}\n\
        bx r0\n\
        .align 2, 0\n\
    _0802D0B8: .4byte gAction\n\
        .syntax divided\n\
    ");
}

#endif // !NONMATCHING

void UnitLevelUp_unused(struct Unit* unit) {
    if (unit->level != 20) {
        int hpGain, powGain, sklGain, spdGain, defGain, resGain, lckGain;
        int growthBonus;
        int totalGain;

        unit->exp = 0;
        unit->level++;

        if (unit->level == 20)
            unit->exp = UNIT_EXP_DISABLED;

        growthBonus = (unit->flags & UNIT_FLAG_GROWTH_BOOST) ? 5: 0;
        totalGain = 0;

        hpGain  = GetStatIncrease(growthBonus + unit->pinfo->growthHP);
        totalGain += hpGain;

        powGain = GetStatIncrease(growthBonus + unit->pinfo->growthPow);
        totalGain += powGain;

        sklGain = GetStatIncrease(growthBonus + unit->pinfo->growthSkl);
        totalGain += sklGain;

        spdGain = GetStatIncrease(growthBonus + unit->pinfo->growthSpd);
        totalGain += spdGain;

        defGain = GetStatIncrease(growthBonus + unit->pinfo->growthDef);
        totalGain += defGain;

        resGain = GetStatIncrease(growthBonus + unit->pinfo->growthRes);
        totalGain += resGain;

        lckGain = GetStatIncrease(growthBonus + unit->pinfo->growthLck);
        totalGain += lckGain;

        if (totalGain == 0) {
            for (totalGain = 0; totalGain < 2; ++totalGain) {
                hpGain = GetStatIncrease(unit->pinfo->growthHP);

                if (hpGain)
                    break;

                powGain = GetStatIncrease(unit->pinfo->growthPow);

                if (powGain)
                    break;

                sklGain = GetStatIncrease(unit->pinfo->growthSkl);

                if (sklGain)
                    break;

                spdGain = GetStatIncrease(unit->pinfo->growthSpd);

                if (spdGain)
                    break;

                defGain = GetStatIncrease(unit->pinfo->growthDef);

                if (defGain)
                    break;

                resGain = GetStatIncrease(unit->pinfo->growthRes);

                if (resGain)
                    break;

                lckGain = GetStatIncrease(unit->pinfo->growthLck);

                if (lckGain)
                    break;
            }
        }

        if ((unit->maxHP + hpGain) > UNIT_MHP_MAX(unit))
            hpGain = UNIT_MHP_MAX(unit) - unit->maxHP;

        if ((unit->pow + powGain) > UNIT_POW_MAX(unit))
            powGain = UNIT_POW_MAX(unit) - unit->pow;

        if ((unit->skl + sklGain) > UNIT_SKL_MAX(unit))
            sklGain = UNIT_SKL_MAX(unit) - unit->skl;

        if ((unit->spd + spdGain) > UNIT_SPD_MAX(unit))
            spdGain = UNIT_SPD_MAX(unit) - unit->spd;

        if ((unit->def + defGain) > UNIT_DEF_MAX(unit))
            defGain = UNIT_DEF_MAX(unit) - unit->def;

        if ((unit->res + resGain) > UNIT_RES_MAX(unit))
            resGain = UNIT_RES_MAX(unit) - unit->res;

        if ((unit->lck + lckGain) > UNIT_LCK_MAX(unit))
            lckGain = UNIT_LCK_MAX(unit) - unit->lck;

        unit->maxHP += hpGain;
        unit->pow += powGain;
        unit->skl += sklGain;
        unit->spd += spdGain;
        unit->def += defGain;
        unit->res += resGain;
        unit->lck += lckGain;
    }
}

void BattleHitAdvance(void) {
    gBattleHitIterator++;
}

void BattleHitTerminate(void) {
    gBattleHitIterator++;
    gBattleHitIterator->info = BATTLE_HIT_INFO_END;
}
