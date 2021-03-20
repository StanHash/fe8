
#include "global.h"

#include "m4a.h"
#include "sound.h"
#include "hardware.h"
#include "oam.h"
#include "sprite.h"
#include "text.h"
#include "ap.h"
#include "bmitem.h"
#include "unit.h"
#include "bmmap.h"
#include "bmbattle.h"
#include "bmtrick.h"
#include "uiutils.h"
#include "uimenu.h"
#include "mapselect.h"

#include "constants/pids.h"
#include "constants/iids.h"
#include "constants/terrains.h"

#include "bmitemuse.h"

struct WarpSelectProc
{
    /* 00 */ PROC_HEADER;

    /* 29 */ u8 pad29[0x4A - 0x29];
    /* 4A */ short prevWarpAllowed;
    /* 4C */ u8 pad4C[0x54 - 0x4C];
    /* 54 */ struct APHandle* ap;
};

void sub_8034FFC(ProcPtr proc);
void sub_803501C(struct Unit* unit);
void sub_8035090(ProcPtr proc);
void sub_80350A4(struct Unit* unit);
void sub_80350FC(ProcPtr proc);
void sub_803511C(struct Unit* unit, int number);
void NewUnitInfoWindow_WithAllLines(ProcPtr proc);
void DrawHammerneUnitInfoWindow(struct Unit* unit);

void StartBottomHelpText(ProcPtr parent, const char* string);
void EndBottomHelpText(void);

ProcPtr StartTargetSelectionExt(const struct SelectInfo* selectInfo, int(*onSelect)(ProcPtr, struct SelectTarget*));
ProcPtr StartTargetSelection(const struct SelectInfo* selectInfo);
void EndTargetSelection(ProcPtr proc);

void ForceMenuItemPanel(ProcPtr parent, struct Unit* unit, int x, int y);
void UpdateMenuItemPanel(int number);

void EnsureCameraOntoPosition(ProcPtr parent, int x, int y);
void FillWarpRangeMap(struct Unit* caster, struct Unit* target);
void DisplayMoveRangeGraphics(int config);

void MakeTargetListForAdjacentHeal(struct Unit* unit);
void MakeTargetListForRangedHeal(struct Unit* unit);
void MakeTargetListForRestore(struct Unit* unit);
void MakeTargetListForRescueStaff(struct Unit* unit);
void MakeTargetListForBarrier(struct Unit* unit);
void MakeTargetListForSilence(struct Unit* unit);
void MakeTargetListForSleep(struct Unit* unit);
void MakeTargetListForBerserk(struct Unit* unit);
void MakeTargetListForWarp(struct Unit* unit);
void MakeTargetListForHammerne(struct Unit* unit);
void MakeTargetListForUnlock(struct Unit* unit);
void MakeTargetListForLatona(struct Unit* unit);
void MakeTargetListForMine(struct Unit* unit);
void MakeTargetListForLightRune(struct Unit* unit);
void MakeTargetListForDanceRing(struct Unit* unit);
void MakeTargetListForDoorAndBridges(struct Unit* unit, int terrain);

static s8 HasSelectTarget(struct Unit* unit, void(*func)(struct Unit*));

static void SetStaffUseAction(struct Unit* unit);
static void SetItemUseAction(struct Unit* unit);

static void DoUseRescueStaff(struct Unit* unit, void(*func)(struct Unit*));
static void DoUseSpecialDance(struct Unit* unit, void(*func)(struct Unit*), int msgHelp);
static void DoUseWarpStaff(struct Unit* unit);
static void DoUsePutTrap(struct Unit* unit, void(*func)(struct Unit*), int msgHelp);
static void DoUseRepairStaff(struct Unit* unit);
static void DoUseHealStaff(struct Unit* unit, void(*func)(struct Unit*));
static void DoUseRestoreStaff(struct Unit* unit, void(*func)(struct Unit*));
static void DoUseBarrierStaff(struct Unit* unit);
static void DoUseAttackStaff(struct Unit* unit, void(*func)(struct Unit*));
static void DoUseTorchStaff(struct Unit* unit);

extern struct Unit gStatGainSimUnit;

extern struct ProcScr CONST_DATA gProcScr_0859B600[]; // go back to unit menu proc

extern const struct MenuDef gMenuInfo_RepairItems;

extern struct SelectInfo CONST_DATA gSelectInfo_OffensiveStaff;
extern struct SelectInfo CONST_DATA gSelectInfo_Barrier;
extern struct SelectInfo CONST_DATA gSelectInfo_Restore;
extern struct SelectInfo CONST_DATA gSelectInfo_Heal;
extern struct SelectInfo CONST_DATA gSelectInfo_PutTrap;
extern struct SelectInfo CONST_DATA gSelectInfo_0859D2F8;
extern struct SelectInfo CONST_DATA gSelectInfo_Repair;

extern u16 CONST_DATA gUnknown_085A0EA0[]; // ap

static void WarpSelect_OnEnd(struct WarpSelectProc* proc);
static void WarpSelect_OnInit(struct WarpSelectProc* proc);
static void WarpSelect_OnIdle(struct WarpSelectProc* proc);
static void WarpSelect_OnConfirm(struct WarpSelectProc* proc);
static void WarpSelect_OnCancel(struct WarpSelectProc* proc);

static void TorchSelect_OnInit(struct WarpSelectProc* proc);
static void TorchSelect_OnIdle(struct WarpSelectProc* proc);

struct ProcScr CONST_DATA gProcScr_SquareSelectWarp[] =
{
    PROC_ONEND(WarpSelect_OnEnd),

    PROC_CALL(LockGameLogic),

    PROC_WHILE_EXISTS(gUnknown_0859A548),

    PROC_CALL(WarpSelect_OnInit),
    PROC_WHILE_EXISTS(gUnknown_0859A548),

    PROC_REPEAT(WarpSelect_OnIdle),

    PROC_CALL(WarpSelect_OnConfirm),
    PROC_SLEEP(0),

    PROC_CALL(UnlockGameLogic),

    PROC_GOTO(100),

PROC_LABEL(99),
    PROC_CALL(WarpSelect_OnCancel),
    PROC_SLEEP(0),

    PROC_CALL(UnlockGameLogic),

PROC_LABEL(100),
    PROC_END,
};

struct ProcScr CONST_DATA gProcScr_SquareSelectTorch[] =
{
    PROC_CALL(LockGameLogic),

    PROC_CALL(TorchSelect_OnInit),
    PROC_WHILE_EXISTS(gUnknown_0859A548),

    PROC_REPEAT(TorchSelect_OnIdle),

    PROC_CALL(WarpSelect_OnConfirm),

    PROC_GOTO(100),

PROC_LABEL(99),
    PROC_CALL(WarpSelect_OnCancel),

PROC_LABEL(100),
    PROC_CALL(UnlockGameLogic),

    PROC_END,
};

s8 CanUnitUseItem(struct Unit* unit, int item)
{
    if ((GetItemAttributes(item) & IA_STAFF) && !CanUnitUseStaff(unit, item))
        return FALSE;

    switch (GetItemIndex(item))
    {

    case IID_HEAL_STAFF:
    case IID_MEND_STAFF:
    case IID_RECOVER_STAFF:
        return HasSelectTarget(unit, MakeTargetListForAdjacentHeal);

    case IID_PHYSIC_STAFF:
        return HasSelectTarget(unit, MakeTargetListForRangedHeal);

    case IID_FORTIFY_STAFF:
        return HasSelectTarget(unit, MakeTargetListForRangedHeal);

    case IID_RESTORE_STAFF:
        return HasSelectTarget(unit, MakeTargetListForRestore);

    case IID_RESCUE_STAFF:
        return HasSelectTarget(unit, MakeTargetListForRescueStaff);

    case IID_BARRIER_STAFF:
        return HasSelectTarget(unit, MakeTargetListForBarrier);

    case IID_SILENCE_STAFF:
        return HasSelectTarget(unit, MakeTargetListForSilence);

    case IID_SLEEP_STAFF:
        return HasSelectTarget(unit, MakeTargetListForSleep);

    case IID_BERSERK_STAFF:
        return HasSelectTarget(unit, MakeTargetListForBerserk);

    case IID_WARP_STAFF:
        return HasSelectTarget(unit, MakeTargetListForWarp);

    case IID_REPAIR_STAFF:
        return HasSelectTarget(unit, MakeTargetListForHammerne);

    case IID_UNLOCK_STAFF:
        return HasSelectTarget(unit, MakeTargetListForUnlock);

    case IID_BOOSTER_HP:
    case IID_BOOSTER_POW:
    case IID_BOOSTER_SKL:
    case IID_BOOSTER_SPD:
    case IID_BOOSTER_LCK:
    case IID_BOOSTER_DEF:
    case IID_BOOSTER_RES:
    case IID_BOOSTER_MOV:
    case IID_BOOSTER_CON:
        return CanUseStatBooster(unit, item);

    case IID_HEROCREST:
    case IID_KNIGHTCREST:
    case IID_ORIONSBOLT:
    case IID_ELYSIANWHIP:
    case IID_GUIDINGRING:
    case IID_MASTERSEAL:
    case IID_HEAVENSEAL:
    case IID_OCEANSEAL:
    case IID_LUNARBRACE:
    case IID_SOLARBRACE:
    case IID_UNK_C1:
        return CanUsePromotionItem(unit, item);

    case IID_VULNERARY:
    case IID_ELIXIR:
    case IID_VULNERARY_2:
        return CanUseHealingItem(unit);

    case IID_PUREWATER:
        return CanUsePureWater(unit);

    case IID_TORCH:
        return CanUseTorch(unit);

    case IID_ANTITOXIN:
        return CanUseAntidote(unit);

    case IID_CHESTKEY:
    case IID_CHESTKEY_BUNDLE:
        return CanUseChestKey(unit);

    case IID_DOORKEY:
        return CanUseDoorKey(unit);

    case IID_LOCKPICK:
        return CanUseLockpick(unit);

    case IID_LATONA_STAFF:
        return HasSelectTarget(unit, MakeTargetListForLatona);

    case IID_MINE:
        return HasSelectTarget(unit, MakeTargetListForMine);

    case IID_LIGHTRUNE:
        return HasSelectTarget(unit, MakeTargetListForLightRune);

    case IID_TORCH_STAFF:
        return gPlaySt.chapterVisionRange != 0;

    case IID_FILLAS_MIGHT:
    case IID_NINISS_GRACE:
    case IID_THORS_IRE:
    case IID_SETS_LITANY:
        return HasSelectTarget(unit, MakeTargetListForDanceRing);

    case IID_METISSTOME:
        if (unit->flags & UNIT_FLAG_GROWTH_BOOST)
            return FALSE;

        return TRUE;

    case IID_JUNAFRUIT:
        return CanUseJunaFruit(unit);

    default:
        return FALSE;

    }
}

int GetItemCantUseMsgid(struct Unit* unit, int item)
{
    switch (GetItemIndex(item))
    {

    case IID_TORCH_STAFF:
    case IID_BOOSTER_HP:
    case IID_BOOSTER_POW:
    case IID_BOOSTER_SKL:
    case IID_BOOSTER_SPD:
    case IID_BOOSTER_LCK:
    case IID_BOOSTER_DEF:
    case IID_BOOSTER_RES:
    case IID_BOOSTER_MOV:
    case IID_BOOSTER_CON:
    case IID_VULNERARY:
    case IID_ELIXIR:
    case IID_PUREWATER:
    case IID_ANTITOXIN:
    case IID_TORCH:
    case IID_VULNERARY_2:
        return 0x859; // TODO: msgid "There's no need for that."

    case IID_CHESTKEY:
    case IID_CHESTKEY_BUNDLE:
        return 0x85E; // TODO: msgid "There's no chest."

    case IID_DOORKEY:
        return 0x85D; // TODO: msgid "There's no door."

    case IID_LOCKPICK:
        if (UNIT_ATTRIBUTES(gActiveUnit) & UNIT_ATTR_THIEF)
            return 0x861; // TODO: msgid "No doors or chests."

        return 0x85F; // TODO: msgid "For thieves only."

    case IID_HEROCREST:
    case IID_KNIGHTCREST:
    case IID_ORIONSBOLT:
    case IID_ELYSIANWHIP:
    case IID_GUIDINGRING:
    case IID_MASTERSEAL:
    case IID_HEAVENSEAL:
    case IID_OCEANSEAL:
    case IID_LUNARBRACE:
    case IID_SOLARBRACE:
    case IID_UNK_C1:
    {
        int level = gActiveUnit->level;
        s8 boolval;

        gActiveUnit->level = 10;
        boolval = CanUsePromotionItem(gActiveUnit, item);
        gActiveUnit->level = level;

        if (boolval)
            return 0x85B; // TODO: msgid "Must be above level 10."

        return 0x85A; // TODO: msgid "This can't be used."
    }

    case IID_JUNAFRUIT:
        return 0x85C; // TODO: msgid "Must be above level 10."

    default:
        return 0x85A; // TODO: msgid "This can't be used."

    }
}

void ItemEffect_Call(struct Unit* unit, int item)
{
    ClearBg0Bg1();
    EndFaceById(0);

    switch (GetItemIndex(item))
    {

    case IID_HEAL_STAFF:
    case IID_MEND_STAFF:
    case IID_RECOVER_STAFF:
        DoUseHealStaff(unit, MakeTargetListForAdjacentHeal);
        break;

    case IID_PHYSIC_STAFF:
        DoUseHealStaff(unit, MakeTargetListForRangedHeal);
        break;

    case IID_RESCUE_STAFF:
        DoUseRescueStaff(unit, MakeTargetListForRescueStaff);
        break;

    case IID_RESTORE_STAFF:
        DoUseRestoreStaff(unit, MakeTargetListForRestore);
        break;

    case IID_SILENCE_STAFF:
        DoUseAttackStaff(unit, MakeTargetListForSilence);
        break;

    case IID_SLEEP_STAFF:
        DoUseAttackStaff(unit, MakeTargetListForSleep);
        break;

    case IID_BERSERK_STAFF:
        DoUseAttackStaff(unit, MakeTargetListForBerserk);
        break;

    case IID_BARRIER_STAFF:
        DoUseBarrierStaff(unit);
        break;

    case IID_UNLOCK_STAFF:
        DoUsePutTrap(unit, MakeTargetListForUnlock, 0x87A); // TODO: msgid "Select a door to open."
        break;

    case IID_WARP_STAFF:
        DoUseWarpStaff(unit);
        break;

    case IID_REPAIR_STAFF:
        DoUseRepairStaff(unit);
        break;

    case IID_FORTIFY_STAFF:
    case IID_LATONA_STAFF:
        SetStaffUseAction(unit);
        break;

    case IID_MINE:
        DoUsePutTrap(unit, MakeTargetListForMine, 0x87D); // TODO: msgid "Select an area to trap."
        break;

    case IID_LIGHTRUNE:
        DoUsePutTrap(unit, MakeTargetListForLightRune, 0x87E); // TODO: msgid "Select an area to trap."
        break;

    case IID_TORCH_STAFF:
        DoUseTorchStaff(unit);
        break;

    case IID_FILLAS_MIGHT:
    case IID_NINISS_GRACE:
    case IID_THORS_IRE:
    case IID_SETS_LITANY:
        DoUseSpecialDance(unit, MakeTargetListForDanceRing, 0x87F); // TODO: msgid "Select a character to bless."
        break;

    default:
        SetItemUseAction(unit);
        break;

    }
}

s8 HasSelectTarget(struct Unit* unit, void(*func)(struct Unit*))
{
    func(unit);

    return GetTargetListSize() != 0;
}

s8 CanUseHealingItem(struct Unit* unit)
{
    if (GetUnitCurrentHp(unit) == GetUnitMaxHp(unit))
        return FALSE;

    return TRUE;
}

s8 sub_802909C(struct Unit* unit)
{
    return FALSE;
}

s8 CanUsePureWater(struct Unit* unit)
{
    if (unit->barrierDuration == 7)
        return FALSE;

    return TRUE;
}

s8 CanUseTorch(struct Unit* unit)
{
    if (gPlaySt.chapterVisionRange != 0 && unit->torchDuration != 4)
        return TRUE;

    return FALSE;
}

s8 CanUseAntidote(struct Unit* unit)
{
    if (unit->statusId != UNIT_STATUS_POISON)
        return FALSE;

    return TRUE;
}

s8 CanUseChestKey(struct Unit* unit)
{
    if (gMapTerrain[unit->y][unit->x] != TERRAIN_CHEST_21)
        return FALSE;

    if (!IsThereClosedChestAt(unit->x, unit->y))
        return FALSE;

    return TRUE;
}

s8 CanUseDoorKey(struct Unit* unit)
{
    MakeTargetListForDoorAndBridges(unit, TERRAIN_DOOR);
    return GetTargetListSize();
}

s8 CanOpenBridge(struct Unit* unit)
{
    MakeTargetListForDoorAndBridges(unit, TERRAIN_BRIDGE_14);
    return GetTargetListSize();
}

s8 CanUseLockpick(struct Unit* unit)
{
    if (!(UNIT_ATTRIBUTES(unit) & UNIT_ATTR_THIEF))
        return FALSE;

    if (!CanUseChestKey(unit) && !CanUseDoorKey(unit) && !CanOpenBridge(unit))
        return FALSE;

    return TRUE;
}

s8 CanUsePromotionItem(struct Unit* unit, int item)
{
    #define CHECK_LIST(it)              \
        while (*it)                     \
        {                               \
            if (unit->jinfo->id == *it) \
                return TRUE;            \
            it++;                       \
        }                               \

    const u8* classList = NULL;

    if (unit->pinfo->id == PID_EIRIKA || unit->pinfo->id == PID_EPHRAIM)
    {
        switch (GetItemIndex(item))
        {

        case IID_LUNARBRACE:
            classList = gUnknown_088ADFA4;
            break;

        case IID_SOLARBRACE:
            classList = gUnknown_088ADFA6;
            break;

        } // switch (GetItemIndex(item))

        if (classList)
            CHECK_LIST(classList);
    }

    if (unit->level < 10)
        return FALSE;

    switch (GetItemIndex(item))
    {

    case IID_HEROCREST:
        classList = gUnknown_088ADF57;
        break;

    case IID_KNIGHTCREST:
        classList = gUnknown_088ADF5E;
        break;

    case IID_ORIONSBOLT:
        classList = gUnknown_088ADF64;
        break;

    case IID_ELYSIANWHIP:
        classList = gUnknown_088ADF67;
        break;

    case IID_GUIDINGRING:
        classList = gUnknown_088ADF6B;
        break;

    case IID_MASTERSEAL:
        classList = gUnknown_088ADF76;
        break;

    case IID_LUNARBRACE:
        classList = gUnknown_088ADFA4;
        break;

    case IID_SOLARBRACE:
        classList = gUnknown_088ADFA6;
        break;

    case IID_HEAVENSEAL:
        classList = gUnknown_088ADF96;
        break;

    case IID_UNK_C1:
        classList = gUnknown_088ADFA3;
        break;

    case IID_OCEANSEAL:
        classList = gUnknown_088ADF9E;
        break;

    } // switch (GetItemIndex(item))

    CHECK_LIST(classList);

    return FALSE;

    #undef CHECK_LIST
}

s8 CanUseStatBooster(struct Unit* unit, int item)
{
    s8 result;

    const struct ItemStatBonuses* bonuses = GetItemStatBonuses(item);

    ClearUnit(&gStatGainSimUnit);

    gStatGainSimUnit.pinfo = unit->pinfo;
    gStatGainSimUnit.jinfo     = unit->jinfo;

    gStatGainSimUnit.maxHP = unit->maxHP + bonuses->hpBonus;
    gStatGainSimUnit.pow = unit->pow + bonuses->powBonus;
    gStatGainSimUnit.skl = unit->skl + bonuses->sklBonus;
    gStatGainSimUnit.spd = unit->spd + bonuses->spdBonus;
    gStatGainSimUnit.def = unit->def + bonuses->defBonus;
    gStatGainSimUnit.res = unit->res + bonuses->resBonus;
    gStatGainSimUnit.lck = unit->lck + bonuses->lckBonus;
    gStatGainSimUnit.movBonus = unit->movBonus + bonuses->movBonus;
    gStatGainSimUnit.conBonus = unit->conBonus + bonuses->conBonus;

    UnitCheckStatOverflow(&gStatGainSimUnit);

    result = gStatGainSimUnit.maxHP != unit->maxHP;

    if (gStatGainSimUnit.pow != unit->pow)
        result = TRUE;

    if (gStatGainSimUnit.skl != unit->skl)
        result = TRUE;

    if (gStatGainSimUnit.spd != unit->spd)
        result = TRUE;

    if (gStatGainSimUnit.def != unit->def)
        result = TRUE;

    if (gStatGainSimUnit.res != unit->res)
        result = TRUE;

    if (gStatGainSimUnit.lck != unit->lck)
        result = TRUE;

    if (gStatGainSimUnit.movBonus != unit->movBonus)
        result = TRUE;

    if (gStatGainSimUnit.conBonus != unit->conBonus)
        result = TRUE;

    return result;
}

s8 CanUseJunaFruit(struct Unit* unit)
{
    return (unit->level >= 10)
        ? TRUE
        : FALSE;
}

void SetStaffUseAction(struct Unit* unit)
{
    HideMoveRangeGraphics();

    TmFill(gBg2Tm, 0);
    EnableBgSync(BG2_SYNC_BIT);

    gAction.unitActionType = UNIT_ACTION_STAFF;
}

void SetItemUseAction(struct Unit* unit)
{
    gAction.unitActionType = UNIT_ACTION_USE_ITEM;
}

static int StaffSelectOnSelect(ProcPtr proc, struct SelectTarget* target)
{
    gAction.targetIndex = target->uid;
    SetStaffUseAction(NULL);

    return 0x17; // TODO: Select Return Constants
}

void DoUseRescueStaff(struct Unit* unit, void(*func)(struct Unit*))
{
    func(unit);

    BmMapFill(gMapMovement, -1);

    StartBottomHelpText(
        StartTargetSelectionExt(&gSelectInfo_0859D2F8, StaffSelectOnSelect),
        GetMsg(0x876)); // TODO: msgid "Select which character to bring next to you."
}

void DoUseSpecialDance(struct Unit* unit, void(*func)(struct Unit*), int msgHelp)
{
    func(unit);

    BmMapFill(gMapMovement, -1);

    StartBottomHelpText(
        StartTargetSelectionExt(&gSelectInfo_0859D2F8, StaffSelectOnSelect),
        GetMsg(msgHelp));
}

void WarpSelect_OnInit(struct WarpSelectProc* proc)
{
    struct APHandle* ap;

    StartBottomHelpText(proc, GetMsg(0x871)); // TODO: msgid "Select warp point."

    EnsureCameraOntoPosition(proc,
        GetUnit(gAction.targetIndex)->x,
        GetUnit(gAction.targetIndex)->y);

    HideMoveRangeGraphics();

    FillWarpRangeMap(gActiveUnit, GetUnit(gAction.targetIndex));

    gBmSt.gameStateBits &= ~GMAP_STATE_BIT1;

    DisplayMoveRangeGraphics(1);

    SetCursorMapPosition(
        GetUnit(gAction.targetIndex)->x,
        GetUnit(gAction.targetIndex)->y);

    ap = StartAnim(gUnknown_085A0EA0, 0);

    ap->tileBase = OAM2_CHR(0) + OAM2_PAL(0);
    Anim_SetAnimId(ap, 0);

    proc->ap = ap;
    proc->prevWarpAllowed = 2; // neither TRUE nor FALSE
}

void WarpSelect_OnIdle(struct WarpSelectProc* proc)
{
    s8 warpAllowed = ((s8**) gMapMovement)[gBmSt.playerCursor.y][gBmSt.playerCursor.x] != -1;

    HandlePlayerCursorMovement();

    if (gKeySt->pressed & A_BUTTON)
    {
        if (warpAllowed)
        {
            Proc_Break(proc);

            gAction.xOther = gBmSt.playerCursor.x;
            gAction.yOther = gBmSt.playerCursor.y;

            SetStaffUseAction(gActiveUnit);

            TmFill(gBg2Tm, 0);
            EnableBgSync(BG2_SYNC_BIT);

            PlaySe(0x6A); // TODO: song ids

            return;
        }
        else
        {
            PlaySe(0x6C); // TODO: song ids
        }
    }

    if (gKeySt->pressed & B_BUTTON)
    {
        Proc_Goto(proc, 99);

        TmFill(gBg2Tm, 0);
        EnableBgSync(BG2_SYNC_BIT);

        PlaySe(0x6B); // TODO: song ids
    }

    if (warpAllowed != proc->prevWarpAllowed)
    {
        Anim_SetAnimId(proc->ap, warpAllowed ? 0 : 1);
    }

    Anim_Display(proc->ap,
        gBmSt.playerCursorDisplay.x - gBmSt.camera.x,
        gBmSt.playerCursorDisplay.y - gBmSt.camera.y);

    proc->prevWarpAllowed = warpAllowed;
}

void WarpSelect_OnConfirm(struct WarpSelectProc* proc)
{
    ResetTextFont();
    HideMoveRangeGraphics();
    EndBottomHelpText();

    SetCursorMapPosition(
        gActiveUnit->x,
        gActiveUnit->y);

    EnsureCameraOntoPosition(proc,
        gActiveUnit->x,
        gActiveUnit->y);
}

void WarpSelect_OnCancel(struct WarpSelectProc* proc)
{
    ResetTextFont();
    HideMoveRangeGraphics();
    EndBottomHelpText();

    SetCursorMapPosition(
        gActiveUnit->x,
        gActiveUnit->y);

    SpawnProc(gProcScr_0859B600, PROC_TREE_3);
}

void WarpSelect_OnEnd(struct WarpSelectProc* proc)
{
    HideMoveRangeGraphics();
    Anim_End(proc->ap);
}

static int WarpOnSelectTarget(ProcPtr proc, struct SelectTarget* target)
{
    EndTargetSelection(proc);

    gAction.targetIndex = target->uid;

    SpawnProc(gProcScr_SquareSelectWarp, PROC_TREE_3);

    return 4; // TODO: Map Select Return Constants
}

void DoUseWarpStaff(struct Unit* unit)
{
    MakeTargetListForWarp(unit);

    BmMapFill(gMapMovement, -1);

    StartBottomHelpText(
        StartTargetSelectionExt(&gSelectInfo_0859D2F8, WarpOnSelectTarget),
        GetMsg(0x875)); // TODO: msgid "Select character to warp."

    PlaySe(0x6A); // TODO: song ids
}

static int OnSelectPutTrap(ProcPtr proc, struct SelectTarget* target)
{
    gAction.xOther = target->x;
    gAction.yOther = target->y;

    SetStaffUseAction(NULL);

    return 0x17; // TODO: Map Select Return Constants
}

void DoUsePutTrap(struct Unit* unit, void(*func)(struct Unit*), int msgHelp)
{
    func(unit);

    BmMapFill(gMapMovement, -1);

    StartBottomHelpText(
        StartTargetSelectionExt(&gSelectInfo_PutTrap, OnSelectPutTrap),
        GetMsg(msgHelp));

    PlaySe(0x6A); // TODO: song ids
}

int HammerneTargetSelection_OnSelect(ProcPtr proc, struct SelectTarget* target)
{
    ResetTextFont();

    gAction.targetIndex = target->uid;

    ForceMenuItemPanel(
        StartMenu(&gMenuInfo_RepairItems),
        GetUnit(gAction.targetIndex),
        16, 11);

    // TODO: UNIT_HAS_PORTRAIT macro?
    if (GetFaceInfo(GetUnitFid(GetUnit(gAction.targetIndex)))->img)
    {
        StartFace(0, GetUnitFid(GetUnit(gAction.targetIndex)), 184, 12, 2);
        SetFaceBlinkControlById(0, 5);
    }

    return 0x17; // TODO: Map Select Return Constants
}

void DoUseRepairStaff(struct Unit* unit)
{
    MakeTargetListForHammerne(unit);

    BmMapFill(gMapMovement, -1);

    StartBottomHelpText(
        StartTargetSelection(&gSelectInfo_Repair),
        GetMsg(0x878)); // TODO: msgid "Select the character whose weapon needs repair."

    PlaySe(0x6A); // TODO: song ids
}

int HammerneTargetSelection_OnChange(ProcPtr proc, struct SelectTarget* target)
{
    ChangeActiveUnitFacing(target->x, target->y);
    DrawHammerneUnitInfoWindow(GetUnit(target->uid));
}

void HammerneTargetSelection_OnInit(ProcPtr proc)
{
    NewUnitInfoWindow_WithAllLines(proc);
}

int RepairMenuItemOnChange(struct MenuProc* menu, struct MenuItemProc* item)
{
    UpdateMenuItemPanel(item->itemNumber);
}

int nullsub_24(struct MenuProc* menu, struct MenuItemProc* item)
{
}

u8 RepairMenuItemIsAvailable(const struct MenuItemDef* def, int number)
{
    int item = GetUnit(gAction.targetIndex)->items[number];

    if (!item)
        return MENU_NOTSHOWN;

    if (!IsItemHammernable(item))
        return MENU_DISABLED;

    return MENU_ENABLED;
}

int RepairMenuItemDraw(struct MenuProc* menu, struct MenuItemProc* menuItem)
{
    int item = GetUnit(gAction.targetIndex)->items[menuItem->itemNumber];
    int isRepairable = IsItemHammernable(item);

    DrawItemMenuLineLong(
        &menuItem->text, item, isRepairable,
        gBg0Tm + TM_OFFSET(menuItem->xTile, menuItem->yTile));

    EnableBgSync(BG0_SYNC_BIT);

    return 0;
}

u8 RepairMenuItemSelect(struct MenuProc* menu, struct MenuItemProc* menuItem)
{
    if (menuItem->availability == MENU_DISABLED)
    {
        int msgId = 0;

        int item = GetUnit(gAction.targetIndex)->items[menuItem->itemNumber];

        if (GetItemAttributes(item) & (IA_UNBREAKABLE | IA_HAMMERNE | IA_LOCK_3))
            msgId = 0x863; // TODO: msgid "Item cannot be repaired."
        else if (!(GetItemAttributes(item) & (IA_STAFF | IA_WEAPON)))
            msgId = 0x857; // TODO: msgid "Only weapons, tomes, and[N]staves can be repaired."
        else if (GetItemUses(item) == GetItemMaxUses(item))
            msgId = 0x856; // TODO: msgid "There's nothing to repair."

        if (msgId != 0)
            MenuCallHelpBox(menu, msgId);

        return MENU_ACT_SND6B;
    }

    gAction.trapType = menuItem->itemNumber;
    SetStaffUseAction(gActiveUnit);

    return MENU_ACT_SKIPCURSOR + MENU_ACT_END + MENU_ACT_SND6A + MENU_ACT_CLEAR + MENU_ACT_ENDFACE;
}

void DoUseHealStaff(struct Unit* unit, void(*func)(struct Unit*))
{
    func(unit);

    BmMapFill(gMapMovement, -1);

    StartBottomHelpText(
        StartTargetSelection(&gSelectInfo_Heal),
        GetMsg(0x874)); // TODO: msgid "Select a character to restore HP to."
}

void DoUseRestoreStaff(struct Unit* unit, void(*func)(struct Unit*))
{
    func(unit);

    BmMapFill(gMapMovement, -1);

    StartBottomHelpText(
        StartTargetSelection(&gSelectInfo_Restore),
        GetMsg(0x877)); // TODO: msgid "Select a character to restore to normal."
}

int BarrierSelectOnInit(ProcPtr proc)
{
    sub_8034FFC(proc);
}

int BarrierSelectOnChange(ProcPtr proc, struct SelectTarget* target)
{
    ChangeActiveUnitFacing(target->x, target->y);
    sub_803501C(GetUnit(target->uid));
}

void DoUseBarrierStaff(struct Unit* unit)
{
    MakeTargetListForBarrier(unit);

    BmMapFill(gMapMovement, -1);

    StartBottomHelpText(
        StartTargetSelection(&gSelectInfo_Barrier),
        GetMsg(0x879)); // TODO: msgid "Select a character to restore to normal."
}

int AttackStaffSelectOnInit(ProcPtr proc)
{
    sub_8035090(proc);
}

int AttackStaffSelectOnChange(ProcPtr proc, struct SelectTarget* target)
{
    ChangeActiveUnitFacing(target->x, target->y);
    sub_80350A4(GetUnit(target->uid));
}

void DoUseAttackStaff(struct Unit* unit, void(*func)(struct Unit*))
{
    func(unit);

    BmMapFill(gMapMovement, -1);

    StartBottomHelpText(
        StartTargetSelection(&gSelectInfo_OffensiveStaff),
        GetMsg(0x87B)); // TODO: msgid "Select a unit to use the staff on."
}

int sub_8029CDC(ProcPtr proc)
{
    sub_80350FC(proc);
}

int sub_8029CE8(ProcPtr proc, struct SelectTarget* target)
{
    ChangeActiveUnitFacing(target->x, target->y);

    sub_803511C(
        GetUnit(target->uid),
        GetOffensiveStaffAccuracy(gActiveUnit, GetUnit(target->uid)));
}

void GenericSelection_DeleteBBAndBG(ProcPtr proc)
{
    EndBottomHelpText();
    ClearBg0Bg1();
}

int sub_8029D38(struct Unit* unit)
{
    if ((UNIT_ATTRIBUTES(unit) & UNIT_ATTR_ASSASSIN) && GetTrapAt(unit->x, unit->y) == NULL)
        return TRUE;

    return FALSE;
}

void sub_8029D6C(void)
{
    StartBottomHelpText(
        StartTargetSelectionExt(&gSelectInfo_0859D2F8, StaffSelectOnSelect),
        GetMsg(0x876)); // TODO: msgid "Select which character to bring next to you."
}

void TorchSelect_OnInit(struct WarpSelectProc* proc)
{
    gBmSt.gameStateBits |= GMAP_STATE_BIT0;

    StartBottomHelpText(proc,
        GetMsg(0x87C)); // TODO: msgid "Select an area to light up."

    if (ShouldMoveCameraPosSomething(gActiveUnit->x, gActiveUnit->y))
        EnsureCameraOntoPosition(proc, gActiveUnit->x, gActiveUnit->y);
}

void TorchSelect_OnIdle(struct WarpSelectProc* proc)
{
    int xTorch = gBmSt.playerCursor.x;
    int yTorch = gBmSt.playerCursor.y;

    s8 canTorch = ((s8**) gMapRange)[yTorch][xTorch];

    HandlePlayerCursorMovement();

    if (gKeySt->pressed & A_BUTTON)
    {
        if (canTorch)
        {
            PlaySe(0x6A); // TODO: song ids

            Proc_Break(proc);

            gAction.xOther = gBmSt.playerCursor.x;
            gAction.yOther = gBmSt.playerCursor.y;

            SetStaffUseAction(gActiveUnit);

            return;
        }
        else
        {
            PlaySe(0x6C); // TODO: song ids
        }
    }

    if (gKeySt->pressed & B_BUTTON)
    {
        TmFill(gBg2Tm, 0);
        EnableBgSync(BG2_SYNC_BIT);

        Proc_Goto(proc, 99);

        PlaySe(0x6B); // TODO: song ids
    }

    DisplayCursor(
        gBmSt.playerCursorDisplay.x,
        gBmSt.playerCursorDisplay.y,
        TRUE);
}

void DoUseTorchStaff(struct Unit* unit)
{
    SpawnProc(gProcScr_SquareSelectTorch, PROC_TREE_3);
    PlaySe(0x6A); // TODO: song ids
}

s8 CanUnitUseItemPrepScreen(struct Unit* unit, int item)
{
    if (GetItemAttributes(item) & IA_STAFF)
        return FALSE;

    switch (GetItemIndex(item))
    {

    case IID_BOOSTER_HP:
    case IID_BOOSTER_POW:
    case IID_BOOSTER_SKL:
    case IID_BOOSTER_SPD:
    case IID_BOOSTER_LCK:
    case IID_BOOSTER_DEF:
    case IID_BOOSTER_RES:
    case IID_BOOSTER_MOV:
    case IID_BOOSTER_CON:
        return CanUseStatBooster(unit, item);

    case IID_HEROCREST:
    case IID_KNIGHTCREST:
    case IID_ORIONSBOLT:
    case IID_ELYSIANWHIP:
    case IID_GUIDINGRING:
    case IID_MASTERSEAL:
    case IID_HEAVENSEAL:
    case IID_OCEANSEAL:
    case IID_LUNARBRACE:
    case IID_SOLARBRACE:
    case IID_UNK_C1:
        return CanUsePromotionItem(unit, item);

    case IID_METISSTOME:
        if (unit->flags & UNIT_FLAG_GROWTH_BOOST)
            return FALSE;

        return TRUE;

    case IID_JUNAFRUIT:
        return CanUseJunaFruit(unit);

    default:
        return FALSE;

    }
}

s8 sub_802A108(struct Unit* unit)
{
    int i, count = GetUnitItemCount(unit);

    for (i = 0; i < count; ++i)
    {
        if (GetItemIndex(unit->items[i]) == IID_UNK_CC)
            return TRUE;
    }

    return FALSE;
}
