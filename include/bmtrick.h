#ifndef GUARD_BMTRICK_H
#define GUARD_BMTRICK_H

enum
{
    TRAP_MAX_COUNT = 64,
};

enum
{
    TRAP_NONE       = 0,
    TRAP_BALLISTA   = 1,
    TRAP_OBSTACLE   = 2, // walls & snags
    TRAP_MAPCHANGE  = 3,
    TRAP_FIRETILE   = 4,
    TRAP_GAS        = 5,
    TRAP_MAPCHANGE2 = 6, // TODO: figure out
    TRAP_LIGHTARROW = 7,
    TRAP_8          = 8,
    TRAP_9          = 9,
    TRAP_TORCHLIGHT = 10,
    TRAP_MINE       = 11,
    TRAP_GORGON_EGG = 12, // TODO: figure out
    TRAP_LIGHT_RUNE = 13,
};

enum
{
    // Ballista extdata definitions
    TRAP_EXTDATA_BLST_RIDDEN   = 1, // "is ridden" boolean
    TRAP_EXTDATA_BLST_ITEMUSES = 2, // ballista item uses

    // Trap (Fire/Gas/Arrow) extdata definitions
    TRAP_EXTDATA_TRAP_TURNFIRST = 0, // start turn countdown
    TRAP_EXTDATA_TRAP_TURNNEXT  = 1, // repeat turn countdown
    TRAP_EXTDATA_TRAP_COUNTER   = 2, // turn counter
    TRAP_EXTDATA_TRAP_DAMAGE    = 3, // trap damage (needs confirmation)

    // Light Rune extdata definitions
    TRAP_EXTDATA_RUNE_TURNSLEFT        = 2, // turns left beofre wearing out
};

struct Trap
{
    /* 00 */ u8 x;
    /* 01 */ u8 y;

    /* 02 */ u8 type;

    /* 03 */ u8 extra; // extra data (meaning varies based on trap type)
    /* 04 */ s8 data[4]; // more extra data (see above enum for per trap type entry allocations)
};

#define TRAP_INDEX(aTrap) ((aTrap) - GetTrap(0))

void ResetTraps(void);
struct Trap* GetTrapAt(int x, int y);
struct Trap* GetSpecificTrapAt(int x, int y, int trapType);
struct Trap* AddTrap(int x, int y, int trapType, int meta);
struct Trap* AddTrapExt(int x, int y, int trapType, int meta, int turnCountdown, int turnInterval, int damage);
struct Trap* RemoveTrap(struct Trap* trap);
void AddFireTrap(int x, int y, int turnCountdown, int turnInterval);
void AddGasTrap(int x, int y, int facing, int turnCountdown, int turnInterval);
void AddArrowTrap(int x, int turnCountdown, int turnInterval);
void sub_802E36C(int x, int y, int turnCountdown, int turnInterval);
void AddTrap8(int x, int y);
void AddTrap9(int x, int y, int meta);
void AddSnagsAndWalls(void);
void ApplyTrapMapChanges(void);
void UpdateAllLightRunes(void);
int GetTrapExt1At(int x, int y);
const struct MapChange* GetMapChangesPointerById(int id);
int GetMapChangesIdAt(int x, int y);
void ApplyMapChangesById(int mapChangeId);
void AddMapChange(int mapChangeId);
void RemoveMapChange(int id);
s8 AreMapChangeTriggered(int id);
void HideIfUnderRoof(struct Unit* unit);
void UpdateUnitsUnderRoof(void);
void MakeTargetListForTurnTrapDamage(void);
void GenerateDisplayedTrapDamageTargets(void);
void CountDownTraps(void);
void ResetCounterForCountedDownTraps(void);
void sub_802EA00(void);
void sub_802EA1C(void);
void sub_802EA28(void);
struct Trap* AddLightRune(int x, int y);
struct Trap* RemoveLightRune(struct Trap* trap);
void HandleTrapDecay(void);
void BattleSomethingTrapChangeTerrain(void);
void NullAllLightRunesTerrain(void);
struct Trap* GetTrap(int id);

#endif // GUARD_BMTRICK_H
