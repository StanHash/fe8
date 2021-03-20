
#pragma once

#include "global.h"

struct SupportInfo;

// Config
enum { UNIT_LEVEL_MAX = 20 };
enum { UNIT_ITEM_COUNT = 5 };
enum { UNIT_DEFINITION_ITEM_COUNT = 4 };

// Meaningful constants
enum { UNIT_EXP_DISABLED = 0xFF };

struct PInfo
{
    /* 00 */ u16 msgName;
    /* 02 */ u16 msgDesc;
    /* 04 */ u8 id;
    /* 05 */ u8 jidDefault;
    /* 06 */ u16 fid;
    /* 08 */ u8 fidMini;
    /* 09 */ u8 affinity;
    /* 0A */ u8 _u0A;

    /* 0B */ s8 baseLevel;
    /* 0C */ s8 baseHP;
    /* 0D */ s8 basePow;
    /* 0E */ s8 baseSkl;
    /* 0F */ s8 baseSpd;
    /* 10 */ s8 baseDef;
    /* 11 */ s8 baseRes;
    /* 12 */ s8 baseLck;
    /* 13 */ s8 baseCon;

    /* 14 */ u8 baseWexp[8];

    /* 1C */ u8 growthHP;
    /* 1D */ u8 growthPow;
    /* 1E */ u8 growthSkl;
    /* 1F */ u8 growthSpd;
    /* 20 */ u8 growthDef;
    /* 21 */ u8 growthRes;
    /* 22 */ u8 growthLck;

    /* 23 */ u8 _u23;
    /* 24 */ u8 _u24;
    /* 25 */ u8 _u25;
    /* 26 */ u8 _u26;
    /* 27 */ u8 _u27;

    /* 28 */ u32 attributes;

    /* 2C */ struct SupportInfo const* supportInfo;
    /* 30 */ void* _pU30;
};

struct JInfo
{
    /* 00 */ u16 msgName;
    /* 02 */ u16 msgDesc;
    /* 04 */ u8 id;
    /* 05 */ u8 jidPromotion;
    /* 06 */ u8 mapSprite;
    /* 07 */ u8 slowWalking;
    /* 08 */ u16 fidDefault;
    /* 0A */ u8 _u0A;

    /* 0B */ s8 baseHP;
    /* 0C */ s8 basePow;
    /* 0D */ s8 baseSkl;
    /* 0E */ s8 baseSpd;
    /* 0F */ s8 baseDef;
    /* 10 */ s8 baseRes;
    /* 11 */ s8 baseCon;
    /* 12 */ s8 baseMov;

    /* 13 */ s8 maxHP;
    /* 14 */ s8 maxPow;
    /* 15 */ s8 maxSkl;
    /* 16 */ s8 maxSpd;
    /* 17 */ s8 maxDef;
    /* 18 */ s8 maxRes;
    /* 19 */ s8 maxCon;

    /* 1A */ s8 classRelativePower;

    /* 1B */ s8 growthHP;
    /* 1C */ s8 growthPow;
    /* 1D */ s8 growthSkl;
    /* 1E */ s8 growthSpd;
    /* 1F */ s8 growthDef;
    /* 20 */ s8 growthRes;
    /* 21 */ s8 growthLck;

    /* 22 */ u8 promotionHp;
    /* 23 */ u8 promotionPow;
    /* 24 */ u8 promotionSkl;
    /* 25 */ u8 promotionSpd;
    /* 26 */ u8 promotionDef;
    /* 27 */ u8 promotionRes;

    /* 28 */ u32 attributes;

    /* 2C */ u8 baseWexp[8];

    /* 34 */ void const* pBattleAnimDef;
    /* 38 */ s8 const* moveTable[3]; // standard, rain, snow

    /* 44 */ s8 const* pTerrainAvoidLookup;
    /* 48 */ s8 const* pTerrainDefenseLookup;
    /* 4C */ s8 const* pTerrainResistanceLookup;

    /* 50 */ void const* _pU50;
};

struct Unit
{
    /* 00 */ struct PInfo const* pinfo;
    /* 04 */ struct JInfo const* jinfo;

    /* 08 */ s8 level;
    /* 09 */ u8 exp;

    /* 0A */ u8 aiFlags;

    /* 0B */ s8 id;

    /* 0C */ u32 flags;

    /* 10 */ s8 x;
    /* 11 */ s8 y;

    /* 12 */ s8 maxHP;
    /* 13 */ s8 curHP;
    /* 14 */ s8 pow;
    /* 15 */ s8 skl;
    /* 16 */ s8 spd;
    /* 17 */ s8 def;
    /* 18 */ s8 res;
    /* 19 */ s8 lck;

    /* 1A */ s8 conBonus;
    /* 1B */ u8 rescue;
    /* 1C */ u8 ballistaId;
    /* 1D */ s8 movBonus;

    /* 1E */ u16 items[UNIT_ITEM_COUNT];
    /* 28 */ u8 wexp[8];

    /* 30 */ u8 statusId : 4;
    /* 30 */ u8 statusDuration : 4;

    /* 31 */ u8 torchDuration : 4;
    /* 31 */ u8 barrierDuration : 4;

    /* 32 */ u8 supports[UNIT_SUPPORT_MAX_COUNT];
    /* 39 */ s8 supportBits;

    /* 3A */ u8 pad_3A[2];

    /* 3C */ struct MapSprite* mapSprite;

    /* 40 */ u16 aiConfig;
    /* 42 */ u8 aiA;
    /* 43 */ u8 aiApc;
    /* 44 */ u8 aiB;
    /* 45 */ u8 aiBpc;
    /* 46 */ u8 unk_46;
    /* 47 */ u8 unk_47;
};

struct UnitInfo
{
    /* 00 */ u8 pid;
    /* 01 */ u8 jid;
    /* 02 */ u8 pidLead;

    /* 03 */ u8 autolevel  : 1;
    /* 03 */ u8 allegiance : 2;
    /* 03 */ u8 level      : 5;

    /* 04 */ u16 xPosition  : 6; /* 04:0 to 04:5 */
    /* 04 */ u16 yPosition  : 6; /* 04:6 to 05:3 */
    /* 05 */ u16 genMonster : 1; /* 05:4 */
    /* 05 */ u16 itemDrop   : 1; /* 05:5 */
    /* 05 */ u16 sumFlag    : 1; /* 05:6 */
    /* 05 */ u16 extraData  : 9; /* 05:7 to 06:7 */
    /* 07 */ u16 redaCount  : 8;

    /* 08 */ void const* redas;

    /* 0C */ u8 items[UNIT_DEFINITION_ITEM_COUNT];
    /* 10 */ u8 ai[4];
};

enum
{
    // Unit::flags constants

    UNIT_FLAG_NONE         = 0,

    UNIT_FLAG_HIDDEN         = (1 << 0),
    UNIT_FLAG_TURN_ENDED     = (1 << 1),
    UNIT_FLAG_DEAD           = (1 << 2),
    UNIT_FLAG_UNDEPLOYED     = (1 << 3),
    UNIT_FLAG_RESCUING       = (1 << 4),
    UNIT_FLAG_RESCUED        = (1 << 5),
    UNIT_FLAG_HAD_ACTION     = (1 << 6),
    UNIT_FLAG_UNDER_ROOF     = (1 << 7),
    UNIT_FLAG_SEEN           = (1 << 8),
    UNIT_FLAG_UNSEEN         = (1 << 9), // hidden by fog
    UNIT_FLAG_AI_PROCESSED   = (1 << 10),
    UNIT_FLAG_IN_BALLISTA    = (1 << 11),
    UNIT_FLAG_DROPS_ITEM     = (1 << 12),
    UNIT_FLAG_GROWTH_BOOST   = (1 << 13),
    UNIT_FLAG_SOLOANIM_A     = (1 << 14),
    UNIT_FLAG_SOLOANIM_B     = (1 << 15),
    UNIT_FLAG_AWAY           = (1 << 16),
    UNIT_FLAG_ARENA_A        = (1 << 17),
    UNIT_FLAG_ARENA_B        = (1 << 18),
    UNIT_FLAG_ARENA_C        = (1 << 19),
    UNIT_FLAG_20             = (1 << 20),
    UNIT_FLAG_WAS_UNDEPLOYED = (1 << 21),
    UNIT_FLAG_CUTSCENE       = (1 << 22),
    UNIT_FLAG_ALTERNATE_FACE = (1 << 23),
    UNIT_FLAG_24             = (1 << 24), // unused?
    UNIT_FLAG_25             = (1 << 25),
    UNIT_FLAG_WAS_AWAY       = (1 << 26),

    // Helpers
    UNIT_FLAGS_UNAVAILABLE = UNIT_FLAG_DEAD | UNIT_FLAG_UNDEPLOYED | UNIT_FLAG_AWAY,
};

enum
{
    // Unit status identifiers

    UNIT_STATUS_NONE = 0,

    UNIT_STATUS_POISON = 1,
    UNIT_STATUS_SLEEP = 2,
    UNIT_STATUS_SILENCED = 3,
    UNIT_STATUS_BERSERK = 4,

    UNIT_STATUS_ATTACK = 5,
    UNIT_STATUS_DEFENSE = 6,
    UNIT_STATUS_CRIT = 7,
    UNIT_STATUS_AVOID = 8,

    UNIT_STATUS_SICK = 9,
    UNIT_STATUS_RECOVER = 10,

    UNIT_STATUS_PETRIFY = 11,
    UNIT_STATUS_12 = 12,
    UNIT_STATUS_PETRIFY_2 = 13,
};

enum
{
    FACTION_BLUE   = 0x00, // player units
    FACTION_GREEN  = 0x40, // ally npc units
    FACTION_RED    = 0x80, // enemy units
    FACTION_PURPLE = 0xC0, // link arena 4th team
};

enum
{
    // Character/Class attributes

    UNIT_ATTR_NONE = 0,

    UNIT_ATTR_MOUNTED = (1 << 0),
    UNIT_ATTR_CANTO = (1 << 1),
    UNIT_ATTR_2 = (1 << 2), // steal?
    UNIT_ATTR_THIEF = (1 << 3),
    UNIT_ATTR_DANCE = (1 << 4),
    UNIT_ATTR_PLAY = (1 << 5),
    UNIT_ATTR_CRITBONUS = (1 << 6),
    UNIT_ATTR_BALLISTAE = (1 << 7),
    UNIT_ATTR_PROMOTED = (1 << 8),
    UNIT_ATTR_SUPPLY = (1 << 9),
    UNIT_ATTR_HORSE = (1 << 10),
    UNIT_ATTR_WYVERN = (1 << 11),
    UNIT_ATTR_PEGASUS = (1 << 12),
    UNIT_ATTR_LORD = (1 << 13),
    UNIT_ATTR_FEMALE = (1 << 14),
    UNIT_ATTR_BOSS = (1 << 15),
    UNIT_ATTR_LOCK_1 = (1 << 16),
    UNIT_ATTR_LOCK_2 = (1 << 17),
    UNIT_ATTR_MONSTER = (1 << 18),
    UNIT_ATTR_TRAINEE = (1 << 19),
    UNIT_ATTR_20 = (1 << 20), // "unselectable"?
    UNIT_ATTR_TRIANGLEATTACK_PEGASI = (1 << 21),
    UNIT_ATTR_TRIANGLEATTACK_ARMORS = (1 << 22),
    UNIT_ATTR_23 = (1 << 23),
    UNIT_ATTR_FINAL_BOSS = (1 << 24),
    UNIT_ATTR_ASSASSIN = (1 << 25),
    UNIT_ATTR_MAGICSEAL = (1 << 26),
    UNIT_ATTR_SUMMON = (1 << 27),
    UNIT_ATTR_LOCK_3 = (1 << 28),
    UNIT_ATTR_LOCK_4 = (1 << 29),
    UNIT_ATTR_LOCK_5 = (1 << 30),
    UNIT_ATTR_LOCK_6 = (1 << 31),

    // Helpers
    UNIT_ATTRS_REFRESH = UNIT_ATTR_DANCE | UNIT_ATTR_PLAY,
    UNIT_ATTRS_FLYING = UNIT_ATTR_WYVERN | UNIT_ATTR_PEGASUS,
    UNIT_ATTRS_TRIANGLEATTACK_ANY = UNIT_ATTR_TRIANGLEATTACK_ARMORS | UNIT_ATTR_TRIANGLEATTACK_PEGASI,
};

enum
{
    // To check result of GetUnit[Item]UseBits

    UNIT_USEBIT_WEAPON = (1 << 0),
    UNIT_USEBIT_STAFF  = (1 << 1),
};

// TODO: MOVE ELSEWHERE
extern struct JInfo const gClassData[];
extern struct PInfo const gCharacterData[];

void InitUnits(void);
void ClearUnit(struct Unit* unit);
void CopyUnit(struct Unit* from, struct Unit* to);
struct Unit* GetFreeUnit(int faction);
struct Unit* GetFreeBlueUnit(struct UnitInfo const* info);
int GetUnitVision(struct Unit* unit);
void SetUnitStatus(struct Unit* unit, int statusId);
void SetUnitStatusExt(struct Unit* unit, int status, int duration);
int GetUnitSpriteId(struct Unit* unit);
bool UnitAddItem(struct Unit* unit, int item);
void ClearUnitInventory(struct Unit* unit);
void UnitRemoveInvalidItems(struct Unit* unit);
int GetUnitItemCount(struct Unit* unit);
bool UnitHasItem(struct Unit* unit, int item);
int BatchCreateUnits(struct UnitInfo const* info);
void sub_8017A54(struct Unit* unit);
bool CanClassWieldWeaponType(u8 jid, u8 wpnType);
struct Unit* CreateUnit(struct UnitInfo const* info);
void UnitInitFromInfo(struct Unit* unit, struct UnitInfo const* info);
void UnitInitItemsFromInfo(struct Unit* unit, struct UnitInfo const* info);
void UnitInitStats(struct Unit* unit, struct PInfo const* pinfo);
void sub_08017EBC(struct Unit* unit);
void UnitInitSupports(struct Unit* unit);
void UnitAutolevelWeaponExp(struct Unit* unit, struct UnitInfo const* info);
void UnitAutolevelCore(struct Unit* unit, u8 jid, int levelCount);
void UnitAutolevelPenalty(struct Unit* unit, u8 jid, int levelCount);
void UnitApplyBonusLevels(struct Unit* unit, int levelCount);
void UnitAutolevel(struct Unit* unit);
void UnitAutolevelRealistic(struct Unit* unit);
void UnitCheckStatOverflow(struct Unit* unit);
struct Unit* GetUnitByPid(int pid);
struct Unit* GetUnitByPidAndFaction(int pid, int faction);
bool CanUnitCarry(struct Unit* actor, struct Unit* target);
void UnitRescue(struct Unit* actor, struct Unit* target);
void UnitDropRescue(struct Unit* actor, int xTarget, int yTarget);
bool UnitGiveRescue(struct Unit* actor, struct Unit* target);
void KillUnit(struct Unit* unit);
void UnitChangeFaction(struct Unit* unit, int faction);
void UnitSyncMovement(struct Unit* unit);
void UnitGetDropPositionOnDeath(struct Unit* unit, int* xOut, int* yOut);
void UnitBeginAction(struct Unit* unit);
void UnitBeginCantoAction(struct Unit* unit);
void MoveActiveUnit(int x, int y);
void ClearActiveFactionGrayedStates(void);
void TickActiveFactionTurn(void);
void ClearUnitsSeenFlag(void);
void UnitUpdateUsedItem(struct Unit* unit, int itemSlot);
int GetUnitAid(struct Unit* unit);
int GetUnitMagRange(struct Unit* unit);
bool UnitKnowsMagic(struct Unit* unit);
void sub_8018A7C(struct Unit* unit, int x, int y);
int GetUnitKeyItemSlotForTerrain(struct Unit* unit, int terrain);
int GetUnitAidIconId(u32 attributes);
int GetUnitWeaponUsabilityBits(struct Unit* unit);
int GetCombinedEnemyWeaponUsabilityBits(void);
bool CanUnitMove(void);
bool IsPositionMagicSealed(int x, int y);
bool IsUnitMagicSealed(struct Unit* unit);
int GetUnitLastItem(struct Unit* unit);
s8 const* GetUnitMovementCost(struct Unit* unit);
int GetUnitSpriteByJid(int jid);
void UpdatePrevDeployStates(void);
void LoadUnitPrepScreenPositions(void);
void ClearTemporaryUnits(void);
bool IsUnitSlotAvailable(int faction);
void sub_8018F80(void);
void sub_8018FC0(void);
u16 CountAvailableBlueUnits(void);
int CountRedUnits(void);
int CountGreenUnits(void);
void ClearCutsceneUnits(void);
void sub_8019108(void);
int GetUnitCurrentHp(struct Unit* unit);
int GetUnitMaxHp(struct Unit* unit);
int GetUnitPower(struct Unit* unit);
int GetUnitSkill(struct Unit* unit);
int GetUnitSpeed(struct Unit* unit);
int GetUnitDefense(struct Unit* unit);
int GetUnitResistance(struct Unit* unit);
int GetUnitLuck(struct Unit* unit);
int GetUnitFid(struct Unit* unit);
int GetUnitChibiFid(struct Unit* unit);
int GetUnitLeaderPid(struct Unit* unit);
void SetUnitLeaderPid(struct Unit* unit, int pid);
void SetUnitHp(struct Unit* unit, int value);
void AddUnitHp(struct Unit* unit, int amount);
char const* GetUnitRescueName(struct Unit* unit);
char const* GetUnitStatusName(struct Unit* unit);
struct Unit* GetUnit(int id);
struct JInfo const* GetJInfo(int jid);
struct PInfo const* GetPInfo(int pid);
void UnitRemoveItem(struct Unit* unit, int slot);
bool CanUnitCrossTerrain(struct Unit* unit, int terrain);

#define UNIT_IS_VALID(unit) ((unit) && (unit)->pinfo)

#define UNIT_FACTION(unit) ((unit)->id & 0xC0)

#define UNIT_ATTRIBUTES(unit) ((unit)->pinfo->attributes | (unit)->jinfo->attributes)

#define UNIT_NAME_ID(unit) ((unit)->pinfo->msgName)

#define UNIT_MHP_MAX(unit) (UNIT_FACTION(unit) == FACTION_RED ? 120 : 60)
#define UNIT_POW_MAX(unit) ((unit)->jinfo->maxPow)
#define UNIT_SKL_MAX(unit) ((unit)->jinfo->maxSkl)
#define UNIT_SPD_MAX(unit) ((unit)->jinfo->maxSpd)
#define UNIT_DEF_MAX(unit) ((unit)->jinfo->maxDef)
#define UNIT_RES_MAX(unit) ((unit)->jinfo->maxRes)
#define UNIT_LCK_MAX(unit) (30)
#define UNIT_CON_MAX(unit) ((unit)->jinfo->maxCon)
#define UNIT_MOV_MAX(unit) (15)

#define UNIT_CON_BASE(unit) ((unit)->jinfo->baseCon + (unit)->pinfo->baseCon)
#define UNIT_MOV_BASE(unit) ((unit)->jinfo->baseMov)

#define UNIT_CON(unit) (UNIT_CON_BASE(unit) + (unit)->conBonus)
#define UNIT_MOV(unit) ((unit)->movBonus + UNIT_MOV_BASE(unit))

#define UNIT_IS_GORGON_EGG(unit) (((unit)->jinfo->id == JID_GORGONEGG) || ((unit)->jinfo->id == JID_GORGONEGG2))
#define UNIT_IS_PHANTOM(unit) ((unit)->jinfo->id == JID_PHANTOM)

#define UNIT_ARENA_LEVEL(unit) (((unit)->flags >> 17) & 0x7)

#define UNIT_SUPPORT_INFO(unit) ((unit)->pinfo->supportInfo)

// NOTE: if this ends up being only used in [Get|Set]UnitLeaderCharId, having this as a macro may end up being unnecessary
#define UNIT_LEADER_CHARACTER(unit) ((unit)->supports[UNIT_SUPPORT_MAX_COUNT-1])
