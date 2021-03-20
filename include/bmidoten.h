#ifndef GUARD_BMIDOTEN_H
#define GUARD_BMIDOTEN_H

struct UnkMovMapFill
{
    /* 00 */ s8 x;
    /* 01 */ s8 y;
    /* 02 */ u8 connexion;
    /* 03 */ u8 leastMoveCost;
};

struct MovMapFillState
{
    /* 00 */ struct UnkMovMapFill* pUnk00;
    /* 04 */ struct UnkMovMapFill* pUnk04;
    /* 08 */ s8 hasUnit;
    /* 09 */ u8 movement;
    /* 0A */ u8 unitId;
    /* 0B */ u8 maxMovementValue;
};

void FillMovementMapForUnit(struct Unit* unit);
void FillMovementMapForUnitAndMovement(struct Unit* unit, s8 movement);
void MapMovementFillMovementFromUnit(struct Unit* unit);
void MapRangeFillMovementFromPosition(int x, int y, const s8 mct[]);
void MapMovementFillMovementFromPosition(int x, int y, const s8 mct[]);
void MapFillMovementFromUnitAt(struct Unit* unit, int x, int y, int movement);
void SetMovCostTable(const s8 mct[]);
void MapFillMovement(int x, int y, int movement, int unitId);
void sub_801A570(int connexion, int x, int y);
void GenerateMovementInstructionsToPoint(int x, int y, u8 output[]);
void ProcessUnitMovement(struct Unit* unit, int x, int y);
void MapMovementMarkMovementEdges(void);
void MapMarkMovementEdges(void);
void MapAddInRange(int x, int y, int range, int value);
void MapSetInRange(int x, int y, int range, int value);
void FillMapAttackRangeForUnit(struct Unit* unit);
void FillRangeMapByRangeMask(struct Unit* unit, int reach);
void FillMapStaffRangeForUnit(struct Unit* unit);
void FillRangeMapForDangerZone(s8 boolDisplayStaffRange);
void MapSetInMagicSealedRange(int value);
void SetSubjectMap(u8** map);
void MapIncInBoundedRange(short x, short y, short minRange, short maxRange);
u8* GetCurrentMovCostTable(void);

extern u8** gWorkingMap;
extern u8 gWorkingMovTable[];

extern struct UnkMovMapFill gUnknown_030049B0[];
extern struct UnkMovMapFill gUnknown_03004C50[];

extern struct MovMapFillState gMovMapFillState;

#endif // GUARD_BMIDOTEN_H
