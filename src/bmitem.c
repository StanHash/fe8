#include "global.h"

#include "constants/iids.h"

#include "text.h"
#include "icon.h"
#include "chapterdata.h"
#include "gold.h"
#include "unit.h"
#include "uimenu.h"
#include "bmitemuse.h"

#include "bmitem.h"

#define IID_INDEX(aItem) ((aItem) & 0xFF)
#define IID_USES(aItem) ((aItem) >> 8)

// TODO: figure out those two inline functions and where they belong

static inline void SetChapterUnk1C(int arg, u8 val) {
    gPlaySt.unk1C[arg] = val;
}

static inline int GetChapterUnk1C(int arg) {
    return gPlaySt.unk1C[arg];
}

char* GetItemNameWithArticle(int item, s8 capitalize) {
    switch (GetItemIndex(item)) {

    case IID_REGINLEIF:
    case IID_FIMBULVETR:
    case IID_AIRCALIBUR:
    case IID_FENRIR:

    case IID_SIEGLINDE:
    case IID_SIEGMUND:
    case IID_NAGLFAR:

    case IID_AUDHULMA:
    case IID_VIDOFNIR:
    case IID_GARM:
    case IID_NIDHOGG:
    case IID_EXCALIBUR:
    case IID_IVALDI:
    case IID_GLEIPNIR:
    case IID_LATONA_STAFF:

    case IID_GOLD:
    case IID_METISSTOME:

    case IID_1G:
    case IID_5G:
    case IID_10G:
    case IID_50G:
    case IID_100G:
    case IID_150G:
    case IID_200G:
    case IID_3000G:
    case IID_5000G:
        // no article
        return GetItemName(item);

    default: {
        char* result;
        const char* article;

        // check for article
        if (GetItemIndex(item) == IID_JUNAFRUIT)
            article = (capitalize == TRUE) ? "Some " : "some ";
        else
            article = NULL;

        result = GetItemName(item);
        PrependArticleToString(result, article, capitalize);

        return result;
    }

    } // switch (GetItemIndex(item))
}

inline const struct ItemData* GetItemData(int itemIndex) {
    return gItemData + itemIndex;
}

inline int GetItemIndex(int item) {
    return IID_INDEX(item);
}

inline char* GetItemName(int item) {
    char* result;

    result = GetMsg(GetItemData(IID_INDEX(item))->msgName);
    result = StrExpandTact();

    return result;
}

inline int GetItemDescId(int item) {
    return GetItemData(IID_INDEX(item))->msgDesc;
}

inline int GetItemUseDescId(int item) {
    return GetItemData(IID_INDEX(item))->useDescTextId;
}

inline int GetItemType(int item) {
    if (!item)
        return 0xFF;

    return GetItemData(IID_INDEX(item))->weaponType;
}

inline int GetItemAttributes(int item) {
    return GetItemData(IID_INDEX(item))->attributes;
}

inline int GetItemUses(int item) {
    if (GetItemAttributes(item) & IA_UNBREAKABLE)
        return 0xFF;
    else
        return IID_USES(item);
}

inline int GetItemMaxUses(int item) {
    if (GetItemAttributes(item) & IA_UNBREAKABLE)
        return 0xFF;
    else
        return GetItemData(IID_INDEX(item))->maxUses;
}

inline int GetItemMight(int item) {
    return GetItemData(IID_INDEX(item))->might;
}

inline int GetItemHit(int item) {
    return GetItemData(IID_INDEX(item))->hit;
}

inline int GetItemWeight(int item) {
    return GetItemData(IID_INDEX(item))->weight;
}

inline int GetItemCrit(int item) {
    return GetItemData(IID_INDEX(item))->crit;
}

inline int GetItemCost(int item) {
    if (GetItemAttributes(item) & IA_UNBREAKABLE)
        return GetItemData(IID_INDEX(item))->costPerUse;
    else
        return GetItemData(IID_INDEX(item))->costPerUse * GetItemUses(item);
}

inline int GetItemMinRange(int item) {
    return GetItemData(IID_INDEX(item))->encodedRange >> 4;
}

inline int GetItemMaxRange(int item) {
    return GetItemData(IID_INDEX(item))->encodedRange & 0xF;
}

inline int GetItemEncodedRange(int item) {
    return GetItemData(IID_INDEX(item))->encodedRange;
}

inline int GetItemRequiredExp(int item) {
    return GetItemData(IID_INDEX(item))->weaponRank;
}

inline const u8* GetItemEffectiveness(int item) {
    return GetItemData(IID_INDEX(item))->pEffectiveness;
}

inline const struct ItemStatBonuses* GetItemStatBonuses(int item) {
    return GetItemData(IID_INDEX(item))->pStatBonuses;
}

inline int GetItemIconId(int item) {
    if (!item)
        return -1;

    return GetItemData(IID_INDEX(item))->iconId;
}

inline int GetItemWeaponEffect(int item) {
    return GetItemData(IID_INDEX(item))->weaponEffectId;
}

inline int GetItemUseEffect(int item) {
    return GetItemData(IID_INDEX(item))->useEffectId;
}

inline int GetItemCostPerUse(int item) {
    return GetItemData(IID_INDEX(item))->costPerUse;
}

inline int GetItemMaxCost(int item) {
    return GetItemData(IID_INDEX(item))->costPerUse * GetItemMaxUses(item);
}

inline int GetItemAwardedExp(int item) {
    return GetItemData(IID_INDEX(item))->weaponExp;
}

int GetItemHpBonus(int item) {
    if (!item)
        return 0;
    else {
        const struct ItemStatBonuses* statBonuses = GetItemStatBonuses(item);

        if (statBonuses)
            return statBonuses->hpBonus;
    }

    return 0;
}

int GetItemPowBonus(int item) {
    if (!item)
        return 0;
    else {
        const struct ItemStatBonuses* statBonuses = GetItemStatBonuses(item);

        if (statBonuses)
            return statBonuses->powBonus;
    }

    return 0;
}

int GetItemSklBonus(int item) {
    if (!item)
        return 0;
    else {
        const struct ItemStatBonuses* statBonuses = GetItemStatBonuses(item);

        if (statBonuses)
            return statBonuses->sklBonus;
    }

    return 0;
}

int GetItemSpdBonus(int item) {
    if (!item)
        return 0;
    else {
        const struct ItemStatBonuses* statBonuses = GetItemStatBonuses(item);

        if (statBonuses)
            return statBonuses->spdBonus;
    }

    return 0;
}

int GetItemDefBonus(int item) {
    if (!item)
        return 0;
    else {
        const struct ItemStatBonuses* statBonuses = GetItemStatBonuses(item);

        if (statBonuses)
            return statBonuses->defBonus;
    }

    return 0;
}

int GetItemResBonus(int item) {
    if (!item)
        return 0;
    else {
        const struct ItemStatBonuses* statBonuses = GetItemStatBonuses(item);

        if (statBonuses)
            return statBonuses->resBonus;
    }

    return 0;
}

int GetItemLckBonus(int item) {
    if (!item)
        return 0;
    else {
        const struct ItemStatBonuses* statBonuses = GetItemStatBonuses(item);

        if (statBonuses)
            return statBonuses->lckBonus;
    }

    return 0;
}

int MakeNewItem(int item) {
    int uses = GetItemMaxUses(item);

    if (GetItemAttributes(item) & IA_UNBREAKABLE)
        uses = 0;

    return (uses << 8) + GetItemIndex(item);
}

s8 CanUnitUseWeapon(struct Unit* unit, int item) {
    if (item == 0)
        return FALSE;

    if (!(GetItemAttributes(item) & IA_WEAPON))
        return FALSE;

    if (GetItemAttributes(item) & IA_LOCK_ANY) {
        // Check for item locks

        if ((GetItemAttributes(item) & IA_LOCK_1) && !(UNIT_ATTRIBUTES(unit) & UNIT_ATTR_LOCK_1))
            return FALSE;

        if ((GetItemAttributes(item) & IA_LOCK_4) && !(UNIT_ATTRIBUTES(unit) & UNIT_ATTR_LOCK_3))
            return FALSE;

        if ((GetItemAttributes(item) & IA_LOCK_5) && !(UNIT_ATTRIBUTES(unit) & UNIT_ATTR_LOCK_4))
            return FALSE;

        if ((GetItemAttributes(item) & IA_LOCK_6) && !(UNIT_ATTRIBUTES(unit) & UNIT_ATTR_LOCK_5))
            return FALSE;

        if ((GetItemAttributes(item) & IA_LOCK_7) && !(UNIT_ATTRIBUTES(unit) & UNIT_ATTR_LOCK_6))
            return FALSE;

        if ((GetItemAttributes(item) & IA_LOCK_2) && !(UNIT_ATTRIBUTES(unit) & UNIT_ATTR_LOCK_2))
            return FALSE;

        // Monster lock is special
        if (GetItemAttributes(item) & IA_LOCK_3) {
            if (!(UNIT_ATTRIBUTES(unit) & UNIT_ATTR_MONSTER))
                return FALSE;

            return TRUE;
        }

        if (GetItemAttributes(item) & IA_UNUSABLE)
            if (!(IsItemUnsealedForUnit(unit, item)))
                return FALSE;
    }

    if ((unit->statusId == UNIT_STATUS_SILENCED) && (GetItemAttributes(item) & IA_MAGIC))
        return FALSE;

    {
        int wRank = GetItemRequiredExp(item);
        int uRank = (unit->wexp[GetItemType(item)]);

        return (uRank >= wRank) ? TRUE : FALSE;
    }
}

s8 CanUnitUseWeaponNow(struct Unit* unit, int item) {
    if (item == 0)
        return FALSE;

    if (!(GetItemAttributes(item) & IA_WEAPON))
        return FALSE;

    if ((GetItemAttributes(item) & IA_MAGIC) && IsUnitMagicSealed(unit))
        return FALSE;

    return CanUnitUseWeapon(unit, item);
}

s8 CanUnitUseStaff(struct Unit* unit, int item) {
    if (item == 0)
        return FALSE;

    if (!(GetItemAttributes(item) & IA_STAFF))
        return FALSE;

    if (unit->statusId == UNIT_STATUS_SLEEP)
        return FALSE;

    if (unit->statusId == UNIT_STATUS_BERSERK)
        return FALSE;

    if (unit->statusId == UNIT_STATUS_SILENCED)
        return FALSE;

    {
        int wRank = GetItemRequiredExp(item);
        int uRank = unit->wexp[GetItemType(item)];

        return (uRank >= wRank) ? TRUE : FALSE;
    }
}

// unused?
s8 CanUnitUseStaffNow(struct Unit* unit, int item) {
    if (item == 0)
        return FALSE;

    if (!(GetItemAttributes(item) & IA_STAFF))
        return FALSE;

    if (IsUnitMagicSealed(unit))
        return FALSE;

    return CanUnitUseStaff(unit, item);
}

// TODO: special character codes

void DrawItemMenuLine(struct Text* text, int item, s8 isUsable, u16* mapOut) {
    Text_SetParams(text, 0, (isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY));
    Text_DrawString(text, GetItemName(item));

    PutText(text, mapOut + 2);

    PutNumberOrBlank(mapOut + 11, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));

    PutIcon(mapOut, GetItemIconId(item), 0x4000);
}

void DrawItemMenuLineLong(struct Text* text, int item, s8 isUsable, u16* mapOut) {
    Text_SetParams(text, 0, (isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY));
    Text_DrawString(text, GetItemName(item));

    PutText(text, mapOut + 2);

    PutNumberOrBlank(mapOut + 10, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));
    PutNumberOrBlank(mapOut + 13, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemMaxUses(item));
    PutSpecialChar(mapOut + 11, isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY, 0x16); // draw special character?

    PutIcon(mapOut, GetItemIconId(item), 0x4000);
}

void DrawItemMenuLineNoColor(struct Text* text, int item, u16* mapOut) {
    Text_SetCursor(text, 0);
    Text_DrawString(text, GetItemName(item));

    PutText(text, mapOut + 2);

    PutNumberOrBlank(mapOut + 11, Text_GetColor(text), GetItemUses(item));

    PutIcon(mapOut, GetItemIconId(item), 0x4000);
}

void DrawItemStatScreenLine(struct Text* text, int item, int nameColor, u16* mapOut) {
    int color;

    ClearText(text);

    color = nameColor;
    Text_SetColor(text, color);

    Text_DrawString(text, GetItemName(item));

    color = (nameColor == TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE;
    PutSpecialChar(mapOut + 12, color, 0x16);

    color = (nameColor != TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY;
    PutNumberOrBlank(mapOut + 11, color, GetItemUses(item));
    PutNumberOrBlank(mapOut + 14, color, GetItemMaxUses(item));

    PutText(text, mapOut + 2);

    PutIcon(mapOut, GetItemIconId(item), 0x4000);
}

u16 GetItemAfterUse(int item) {
    if (GetItemAttributes(item) & IA_UNBREAKABLE)
        return item; // unbreakable items don't loose uses!

    item -= (1 << 8); // lose one use

    if (item < (1 << 8))
        return 0; // return no item if uses < 0

    return item; // return used item
}

u16 GetUnitEquippedWeapon(struct Unit* unit) {
    int i;

    for (i = 0; i < UNIT_ITEM_COUNT; ++i)
        if (CanUnitUseWeapon(unit, unit->items[i]) == TRUE)
            return unit->items[i];

    return 0;
}

int GetUnitEquippedWeaponSlot(struct Unit* unit) {
    int i;

    for (i = 0; i < UNIT_ITEM_COUNT; ++i)
        if (CanUnitUseWeaponNow(unit, unit->items[i]) == TRUE)
            return i;

    return -1;
}

s8 IsItemCoveringRange(int item, int range) {
    int min = GetItemMinRange(item);
    int max = GetItemMaxRange(item);

    if ((min <= range) && (range <= max))
        return TRUE;

    return FALSE;
}

void EquipUnitItemSlot(struct Unit* unit, int itemSlot) {
    int item, i;

    item = unit->items[itemSlot];

    for (i = itemSlot; i != 0; --i)
        unit->items[i] = unit->items[i - 1];

    unit->items[0] = item;
}

s8 IsItemEffectiveAgainst(u16 item, struct Unit* unit) {
    if (unit->jinfo) {
        int jid = unit->jinfo->id;
        const u8* effList = GetItemEffectiveness(item);

        if (!effList)
            return FALSE;

        for (; *effList; ++effList)
            if (*effList == jid)
                // NOTE: maybe there's a better way to make this work (using an inline maybe?)
                goto check_flying_effectiveness_negation;

        return FALSE;

        check_flying_effectiveness_negation: { 
            u32 attributes;
            int i;

            if (GetItemEffectiveness(item) != gUnknown_088ADF2A)
                if (GetItemEffectiveness(item) != gUnknown_088ADEF1)
                    return TRUE;

            attributes = 0;

            for (i = 0; i < UNIT_ITEM_COUNT; ++i)
                attributes = attributes | GetItemAttributes(unit->items[i]);

            if (!(attributes & IA_NEGATE_FLYING))
                return TRUE;
            else
                return FALSE;
        }
    }

    return FALSE;
}

s8 IsUnitEffectiveAgainst(struct Unit* actor, struct Unit* target) {
    int actorClass = actor->jinfo->id;
    int targetClass = target->jinfo->id;

    const u8* effList = NULL;

    switch (actorClass) {

    case 0x2B:
    case 0x2C:
        effList = gUnknown_088ADF39;
        break;

    } // switch (actorClass)

    if (!effList)
        return FALSE;

    for (; *effList; ++effList)
        if (*effList == targetClass)
            // NOTE: see note in IsItemEffectiveAgainst
            goto is_effective;

    return FALSE;

    is_effective:
        return TRUE;
}

char* GetItemDisplayRangeString(int item) {
    int rangeTextIdLookup[10] = {
        // TODO: TEXT ID CONSTANTS
        0x522, 0x523, 0x524, 0x525, 0x526, // 0-Mag/2, 1, 1-2, 1-3, 2
        0x527, 0x528, 0x529, 0x52A, 0x52B, // 2-3, 3-10, 3-15, Total, --
    };

    switch (GetItemEncodedRange(item)) {

    case 0x10: // 1-mag/2
        return GetMsg(rangeTextIdLookup[0]);

    case 0x11: // 1-1
        return GetMsg(rangeTextIdLookup[1]);

    case 0x12: // 1-2
        return GetMsg(rangeTextIdLookup[2]);

    case 0x13: // 1-3
        return GetMsg(rangeTextIdLookup[3]);

    case 0x22: // 2-2
        return GetMsg(rangeTextIdLookup[4]);

    case 0x23: // 2-3
        return GetMsg(rangeTextIdLookup[5]);

    case 0x3A: // 3-10
        return GetMsg(rangeTextIdLookup[6]);

    case 0x3F: // 3-15
        return GetMsg(rangeTextIdLookup[7]);

    case 0xFF: // total
        return GetMsg(rangeTextIdLookup[8]);

    default: // bad
        return GetMsg(rangeTextIdLookup[9]);

    } // switch (GetItemEncodedRange(item))
}

int GetWeaponLevelFromExp(int wexp) {
    if (wexp < WPN_EXP_E)
        return WPN_LEVEL_0;

    if (wexp < WPN_EXP_D)
        return WPN_LEVEL_E;

    if (wexp < WPN_EXP_C)
        return WPN_LEVEL_D;

    if (wexp < WPN_EXP_B)
        return WPN_LEVEL_C;

    if (wexp < WPN_EXP_A)
        return WPN_LEVEL_B;

    if (wexp < WPN_EXP_S)
        return WPN_LEVEL_A;

    return WPN_LEVEL_S;
}

char* GetItemDisplayRankString(int item) {
    int rankTextIdLookup[] = {
        // TODO: TEXT ID CONSTANTS
        0x52C, 0x52D, 0x52E, 0x52F, // --, E, D, C
        0x530, 0x531, 0x532, 0x533, // B, A, S, Prf
    };

    // reuse of the same variable for different purposes :/
    int var = GetItemRequiredExp(item);

    if ((GetItemAttributes(item) & IA_LOCK_ANY) && GetWeaponLevelFromExp(var) == WPN_LEVEL_0)
        var = 7;
    else
        var = GetWeaponLevelFromExp(var);

    return GetMsg(rankTextIdLookup[var]);
}

int GetDisplayRankSpecialCharFromExp(int wexp) {
    u8 rankTextIdLookup[] = {
        // TODO: TEXT ID CONSTANTS
        0x14, 0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18
    };

    return rankTextIdLookup[GetWeaponLevelFromExp(wexp)];
}

char* GetWeaponTypeDisplayString(int wpnType) {
    int wtypeTextIdLookup[] = {
        // TODO: TEXT ID CONSTANTS
        0x505, 0x506, 0x507, 0x508, // Sword, Lance, Axe, Box
        0x509, 0x50A, 0x50B, 0x50C, // Staff, Anima, Light, Dark
        0x50D, 0x50E, 0x50F,        // Item, Bllsta, Dragon
    };

    return GetMsg(wtypeTextIdLookup[wpnType]);
}

void GetWeaponExpProgressState(int wexp, int* outValue, int* outMax) {
    switch (GetWeaponLevelFromExp(wexp)) {

    case WPN_LEVEL_0:
        *outValue = 0;
        *outMax = 0;
        return;

    case WPN_LEVEL_E:
        *outValue = wexp      - WPN_EXP_E;
        *outMax   = WPN_EXP_D - WPN_EXP_E;
        return;

    case WPN_LEVEL_D:
        *outValue = wexp      - WPN_EXP_D;
        *outMax   = WPN_EXP_C - WPN_EXP_D;
        return;

    case WPN_LEVEL_C:
        *outValue = wexp      - WPN_EXP_C;
        *outMax   = WPN_EXP_B - WPN_EXP_C;
        return;

    case WPN_LEVEL_B:
        *outValue = wexp      - WPN_EXP_B;
        *outMax   = WPN_EXP_A - WPN_EXP_B;
        return;

    case WPN_LEVEL_A:
        *outValue = wexp      - WPN_EXP_A;
        *outMax   = WPN_EXP_S - WPN_EXP_A;
        return;

    case WPN_LEVEL_S:
        *outValue = 0;
        *outMax = 0;
        return;

    } // switch (GetWeaponLevelFromExp(wexp))
}

s8 IsItemDanceRing(int item) {
    switch (GetItemIndex(item)) {

    case IID_HOPLON_SHIELD: // ????????
    case IID_FILLAS_MIGHT:
    case IID_NINISS_GRACE:
    case IID_THORS_IRE:
    case IID_SETS_LITANY:
        return TRUE;

    default:
        return FALSE;

    } // switch (GetItemIndex(item))
}

s8 IsItemDisplayUsable(struct Unit* unit, int item) {
    if (GetItemAttributes(item) & IA_WEAPON)
        return CanUnitUseWeapon(unit, item);

    if (GetItemAttributes(item) & IA_STAFF)
        return CanUnitUseStaff(unit, item);

    if (GetItemUseEffect(item)) {
        if (unit->statusId == UNIT_STATUS_SLEEP)
            return FALSE;

        if (unit->statusId == UNIT_STATUS_BERSERK)
            return FALSE;

        if (!(UNIT_ATTRIBUTES(unit) & UNIT_ATTR_THIEF) && GetItemIndex(item) == IID_LOCKPICK)
            return FALSE;

        if (!(UNIT_ATTRIBUTES(unit) & UNIT_ATTRS_REFRESH) && IsItemDanceRing(item))
            return FALSE;
    }

    return TRUE;
}

s8 CanUnitUse_unused(struct Unit* unit, int item) {
    if (GetItemAttributes(item) & IA_WEAPON)
        return CanUnitUseWeapon(unit, item);
    else
        return CanUnitUseItem(unit, item);
}

int GetUnitItemHealAmount(struct Unit* unit, int item) {
    int result = 0;

    switch (GetItemIndex(item)) {

    case IID_HEAL_STAFF:
    case IID_PHYSIC_STAFF:
    case IID_FORTIFY_STAFF:
    case IID_VULNERARY:
    case IID_VULNERARY_2:
        result = 10;
        break;

    case IID_MEND_STAFF:
        result = 20;
        break;

    case IID_RECOVER_STAFF:
    case IID_ELIXIR:
        result = 80;
        break;

    } // switch (GetItemIndex(item))

    if (GetItemAttributes(item) & IA_STAFF) {
        result += GetUnitPower(unit);

        if (result > 80)
            result = 80;
    }

    return result;
}

int GetUnitItemSlot(struct Unit* unit, int itemIndex) {
    int i;

    for (i = 0; i < UNIT_ITEM_COUNT; ++i)
        if (GetItemIndex(unit->items[i]) == itemIndex)
            return i;

    return (-1);
}

s8 IsItemStealable(int item) {
    return (GetItemType(item) == ITYPE_ITEM);
}

s8 IsItemHammernable(int item) {
    if (!item)
        return FALSE;

    if (!(GetItemAttributes(item) & (IA_WEAPON | IA_STAFF)))
        return FALSE;

    if (GetItemAttributes(item) & (IA_UNBREAKABLE | IA_HAMMERNE | IA_LOCK_3))
        return FALSE;

    if (GetItemUses(item) == GetItemMaxUses(item))
        return FALSE;

    return TRUE;
}

int GetItemReachBits(int item) {
    switch (GetItemEncodedRange(item)) {

    case 0x11:
        return REACH_RANGE1;

    case 0x12:
        return REACH_RANGE1 | REACH_RANGE2;

    case 0x13:
        return REACH_RANGE1 | REACH_RANGE2 | REACH_RANGE3;

    case 0x14:
        return REACH_RANGE1 | REACH_TO10;

    case 0x22:
        return REACH_RANGE2;

    case 0x23:
        return REACH_RANGE2 | REACH_RANGE3;

    case 0x33:
        return REACH_RANGE3;

    case 0x3A:
        return REACH_RANGE3 | REACH_TO10;

    case 0x3F:
        return REACH_RANGE3 | REACH_TO15;

    default:
        return REACH_NONE;

    } // switch (GetItemEncodedRange(item))
}

int GetUnitWeaponReachBits(struct Unit* unit, int itemSlot) {
    int i, item, result = 0;

    if (itemSlot >= 0)
        return GetItemReachBits(unit->items[itemSlot]);

    for (i = 0; (i < UNIT_ITEM_COUNT) && (item = unit->items[i]); ++i)
        if (CanUnitUseWeapon(unit, item))
            result |= GetItemReachBits(item);

    return result;
}

int GetUnitItemUseReachBits(struct Unit* unit, int itemSlot) {
    int i, tmp, range = 0;

    if (itemSlot >= 0) {
        tmp = unit->items[itemSlot];

        if (!CanUnitUseItem(unit, tmp))
            return REACH_NONE;

        range = GetItemMaxRange(tmp);

        if (range == 0)
            range = 99;
    } else {
        for (i = 0; (i < UNIT_ITEM_COUNT) && (tmp = unit->items[i]); ++i) {
            if (CanUnitUseItem(unit, tmp)) {
                tmp = GetItemMaxRange(tmp);

                if (tmp == 0)
                    tmp = 99;

                if (range < tmp)
                    range = tmp;
            }
        }
    }

    switch (range) {

    case 1:
        return REACH_RANGE1;

    case 2:
        return REACH_RANGE1 | REACH_RANGE2;

    case 99:
        return REACH_MAGBY2;

    default:
        return REACH_NONE;

    } // switch (range)
}

int GetUnitStaffReachBits(struct Unit* unit) {
    int i, tmp, range = 0;

    for (i = 0; (i < UNIT_ITEM_COUNT) && (tmp = unit->items[i]); ++i) {
        if (CanUnitUseStaff(unit, tmp)) {
            if (GetItemIndex(tmp) == IID_NIGHTMARE) {
                tmp = 99;
            } else {
                tmp = GetItemMaxRange(tmp);

                if (tmp == 0)
                    tmp = 99;
            }

            if (range < tmp)
                range = tmp;
        }
    }

    switch (range) {

    case 1:
        return REACH_RANGE1;

    case 2:
        return REACH_RANGE1 | REACH_RANGE2;

    case 99:
        return REACH_MAGBY2;

    default:
        return REACH_NONE;

    } // switch (range)
}

int GetConvoyItemCostSum(void) {
    int i, result = 0;

    const u16* convoy = GetConvoyItemArray();

    for (i = 0; (i < CONVOY_ITEM_COUNT) && (*convoy); ++i) {
        result += GetItemCost(*convoy);
        convoy++;
    }

    return result;
}

int GetUnitItemCostSum(void) {
    int i, j, item, result = 0;

    for (i = 1; i < 0x40; ++i) {
        struct Unit* unit = GetUnit(i);

        if (!unit)
            continue;

        if (!unit->pinfo)
            continue;

        if (unit->flags & (UNIT_FLAG_DEAD | UNIT_FLAG_AWAY))
            continue;

        for (j = 0; (j < UNIT_ITEM_COUNT) && (item = unit->items[j]); ++j)
            result += GetItemCost(item);
    }

    return result;
}

int GetPartyTotalGoldValue(void) {
    int result = 0;

    result += GetConvoyItemCostSum();
    result += GetUnitItemCostSum();
    result += GetGold();

    if (result > 9999999) // TODO: use a constant?
        result = 9999999;

    return result;
}

void SetItemUnsealedForCharacter(int item, u8 unk) {
    SetChapterUnk1C(GetItemType(item), unk);
}

s8 IsItemUnsealedForUnit(struct Unit* unit, int item) {
    return (GetChapterUnk1C(GetItemType(item)) == unit->pinfo->id) ? TRUE : FALSE;
}
