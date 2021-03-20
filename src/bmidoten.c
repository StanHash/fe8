#include "global.h"

#include "constants/terrains.h"

#include "random.h"
#include "armfunc.h"
#include "bmitem.h"
#include "unit.h"
#include "bmmap.h"
#include "proc.h"
#include "mu.h"

#include "bmidoten.h"

static void RevertMovementScript(u8* begin, u8* end);

inline void SetSubjectMap(u8** map)
{
    gWorkingMap = map;
}

void FillMovementMapForUnit(struct Unit* unit)
{
    SetMovCostTable(GetUnitMovementCost(unit));
    SetSubjectMap(gMapMovement);

    MapFillMovement(unit->x, unit->y, UNIT_MOV(unit), unit->id);
}

void FillMovementMapForUnitAndMovement(struct Unit* unit, s8 movement)
{
    SetMovCostTable(GetUnitMovementCost(unit));
    SetSubjectMap(gMapMovement);

    MapFillMovement(unit->x, unit->y, movement, unit->id);
}

void MapMovementFillMovementFromUnit(struct Unit* unit)
{
    SetMovCostTable(GetUnitMovementCost(unit));
    SetSubjectMap(gMapMovement);

    MapFillMovement(unit->x, unit->y, MAP_MOVEMENT_EXTENDED, 0);
}

void MapRangeFillMovementFromPosition(int x, int y, const s8 mct[TERRAIN_COUNT])
{
    SetMovCostTable(mct);
    SetSubjectMap(gMapRange);

    MapFillMovement(x, y, MAP_MOVEMENT_EXTENDED, 0);
}

void MapMovementFillMovementFromPosition(int x, int y, const s8 mct[TERRAIN_COUNT])
{
    SetMovCostTable(mct);
    SetSubjectMap(gMapMovement);

    MapFillMovement(x, y, MAP_MOVEMENT_EXTENDED, 0);
}

void MapFillMovementFromUnitAt(struct Unit* unit, int x, int y, int movement)
{
    SetMovCostTable(GetUnitMovementCost(unit));

    MapFillMovement(x, y, movement, unit->id);
}

void SetMovCostTable(const s8 mct[TERRAIN_COUNT])
{
    int i;

    for (i = 0; i < TERRAIN_COUNT; ++i)
        gWorkingMovTable[i] = mct[i];
}

void MapFillMovement(int x, int y, int movement, int unitId)
{
    gMovMapFillState.pUnk04 = gUnknown_030049B0;
    gMovMapFillState.pUnk00 = gUnknown_03004C50;

    gMovMapFillState.movement = movement;

    if (unitId == 0)
    {
        gMovMapFillState.hasUnit = FALSE;
    }
    else
    {
        gMovMapFillState.hasUnit = TRUE;
        gMovMapFillState.unitId = unitId;
    }

    gMovMapFillState.maxMovementValue = MAP_MOVEMENT_MAX;

    BmMapFill(gWorkingMap, -1);

    gMovMapFillState.pUnk04->x = x;
    gMovMapFillState.pUnk04->y = y;
    gMovMapFillState.pUnk04->connexion = 5;
    gMovMapFillState.pUnk04->leastMoveCost = 0;

    gWorkingMap[y][x] = 0;

    gMovMapFillState.pUnk04++;
    gMovMapFillState.pUnk04->connexion = 4;

    MapFloodCore();
}

void sub_801A570(int connexion, int x, int y)
{
    // This is a C implementation of the ARM (asm) function MapFloodCoreStep
    // Probably used during testing before switching to the a more "optimal" version.

    short tileMovementCost;

    x += gMovMapFillState.pUnk00->x;
    y += gMovMapFillState.pUnk00->y;

    tileMovementCost = gWorkingMovTable[gMapTerrain[y][x]]
        + (s8) gWorkingMap[(u8) gMovMapFillState.pUnk00->y][(u8) gMovMapFillState.pUnk00->x];

    if (tileMovementCost >= gWorkingMap[y][x])
        return;

    if (gMovMapFillState.hasUnit && gMapUnit[y][x])
        if ((gMapUnit[y][x] ^ gMovMapFillState.unitId) & 0x80)
            return;

    if (tileMovementCost > gMovMapFillState.movement)
        return;

    gMovMapFillState.pUnk04->x = x;
    gMovMapFillState.pUnk04->y = y;
    gMovMapFillState.pUnk04->connexion = connexion;
    gMovMapFillState.pUnk04->leastMoveCost = tileMovementCost;

    gMovMapFillState.pUnk04++;

    gWorkingMap[y][x] = tileMovementCost;
}

void GenerateMovementInstructionsToPoint(int x, int y, u8 output[])
{
    u8* outputStart = output;

    short bestCost;
    short bestDirectionCount;

    u8 neighbourCosts[4];
    u8 bestDirections[4];

    short nextDirection = 0;

    int i;

    // The basic algorithm here is:
    // 1. start from the end point
    // 2. get on the point that's closest to the origin (lowest cost)
    // 3. write direction to output
    // 4. repeat 2-4 until we reached origin (cost = 0)
    // 5. reverse and terminate output

    // As we build the list *in reverse*, the directions are also "reversed" as we traverse the path.

    while (((s8**) gWorkingMap)[y][x] != 0)
    {
        // Build neighbor cost list

        if (x == (gMapSize.x - 1))
            neighbourCosts[MU_COMMAND_MOVE_LEFT] |= 0xFF;
        else
            neighbourCosts[MU_COMMAND_MOVE_LEFT] = gWorkingMap[y][x+1];

        if (x == 0)
            neighbourCosts[MU_COMMAND_MOVE_RIGHT] |= 0xFF;
        else
            neighbourCosts[MU_COMMAND_MOVE_RIGHT] = gWorkingMap[y][x-1];

        if (y == (gMapSize.y - 1))
            neighbourCosts[MU_COMMAND_MOVE_UP] |= 0xFF;
        else
            neighbourCosts[MU_COMMAND_MOVE_UP] = gWorkingMap[y+1][x];

        if (y == 0)
            neighbourCosts[MU_COMMAND_MOVE_DOWN] |= 0xFF;
        else
            neighbourCosts[MU_COMMAND_MOVE_DOWN] = gWorkingMap[y-1][x];

        // find best cost

        bestCost = 0x100;
        bestDirectionCount = 0;

        for (i = 0; i < 4; ++i)
            if (bestCost > neighbourCosts[i])
                bestCost = neighbourCosts[i];

        // list all directions that have best cost

        for (i = 0; i < 4; ++i)
            if (bestCost == neighbourCosts[i])
                bestDirections[bestDirectionCount++] = i;

        // get next direction (choose randomly if necessary)

        switch (bestDirectionCount)
        {

        case 1:
            nextDirection = bestDirections[0];
            break;

        case 2:
            nextDirection = bestDirections[RandNext(2)];
            break;

        case 3:
            nextDirection = bestDirections[RandNext(3)];
            break;

        case 4:
            nextDirection = bestDirections[RandNext(4)];
            break;

        } // switch (bestDirectionCount)

        // write next direction

        *output++ = nextDirection;

        // update position given direction
        // reminder: directions are reversed

        switch (nextDirection)
        {

        case MU_COMMAND_MOVE_LEFT:
            x++;
            break;

        case MU_COMMAND_MOVE_RIGHT:
            x--;
            break;

        case MU_COMMAND_MOVE_UP:
            y++;
            break;

        case MU_COMMAND_MOVE_DOWN:
            y--;
            break;

        } // switch (nextDirection)
    }

    // reverse and terminate output
    RevertMovementScript(outputStart, output);
}

void RevertMovementScript(u8* begin, u8* end)
{
    u8 buffer[MU_COMMAND_MAX_COUNT];

    u8* it = buffer;

    while (end > begin)
        *it++ = *--end;

    *it = MU_COMMAND_HALT;

    for (it = buffer; *it != MU_COMMAND_HALT;)
        *begin++ = *it++;

    *begin = MU_COMMAND_HALT;
}

void ProcessUnitMovement(struct Unit* unit, int x, int y)
{
    u8* it = gUnitMoveBuffer;

    for (;;) {
        gAction.xMove = x;
        gAction.yMove = y;

        switch (*it)
        {

        case MU_COMMAND_MOVE_UP: // up
            y--;
            break;

        case MU_COMMAND_MOVE_DOWN: // down
            y++;
            break;

        case MU_COMMAND_MOVE_LEFT: // left
            x--;
            break;

        case MU_COMMAND_MOVE_RIGHT: // right
            x++;
            break;

        } // switch (*it)

        if (!(UNIT_ATTRIBUTES(unit) & (UNIT_ATTR_THIEF | UNIT_ATTRS_FLYING | UNIT_ATTR_ASSASSIN)))
        {
            if (gMapHidden[y][x] & HIDDEN_BIT_TRAP)
            {
                *++it = MU_COMMAND_HALT;

                gAction.unitActionType = UNIT_ACTION_TRAPPED;
                gAction.xMove = x;
                gAction.yMove = y;

                return;
            }
        }

        if (gMapHidden[y][x] & HIDDEN_BIT_UNIT)
        {
            *it++ = MU_COMMAND_BUMP;
            *it++ = MU_COMMAND_HALT;

            gAction.unitActionType = UNIT_ACTION_TRAPPED;

            return;
        }

        if (*it == MU_COMMAND_HALT)
            break;

        it++;
    }
}

void MapMovementMarkMovementEdges(void) {
    int ix, iy;

    for (iy = gMapSize.y - 1; iy >= 0; --iy)
    {
        for (ix = gMapSize.x - 1; ix >= 0; --ix)
        {
            if (gMapMovement[iy][ix] > MAP_MOVEMENT_MAX)
                continue;

            if ((s8) gMapMovement[iy][ix] == gMovMapFillState.maxMovementValue)
                continue;

            if ((s8) gMapMovement[iy][ix - 1] < 0 && (ix != 0))
                gMapMovement[iy][ix - 1] = gMovMapFillState.maxMovementValue;

            if ((s8) gMapMovement[iy][ix + 1] < 0 && (ix != (gMapSize.x - 1)))
                gMapMovement[iy][ix + 1] = gMovMapFillState.maxMovementValue;

            if ((s8) gMapMovement[iy - 1][ix] < 0 && (iy != 0))
                gMapMovement[iy - 1][ix] = gMovMapFillState.maxMovementValue;

            if ((s8) gMapMovement[iy + 1][ix] < 0 && (iy != (gMapSize.y - 1)))
                gMapMovement[iy + 1][ix] = gMovMapFillState.maxMovementValue;
        }
    }

    gMovMapFillState.maxMovementValue++;
}

void MapMarkMovementEdges(void)
{
    int ix, iy;

    for (iy = gMapSize.y - 1; iy >= 0; --iy)
    {
        for (ix = gMapSize.x - 1; ix >= 0; --ix)
        {
            if (gWorkingMap[iy][ix] > MAP_MOVEMENT_MAX)
                continue;

            if ((s8) gWorkingMap[iy][ix] == gMovMapFillState.maxMovementValue)
                continue;

            if ((s8) gWorkingMap[iy][ix - 1] < 0 && (ix != 0))
                gWorkingMap[iy][ix - 1] = gMovMapFillState.maxMovementValue;

            if ((s8) gWorkingMap[iy][ix + 1] < 0 && (ix != (gMapSize.x - 1)))
                gWorkingMap[iy][ix + 1] = gMovMapFillState.maxMovementValue;

            if ((s8) gWorkingMap[iy - 1][ix] < 0 && (iy != 0))
                gWorkingMap[iy - 1][ix] = gMovMapFillState.maxMovementValue;

            if ((s8) gWorkingMap[iy + 1][ix] < 0 && (iy != (gMapSize.y - 1)))
                gWorkingMap[iy + 1][ix] = gMovMapFillState.maxMovementValue;
        }
    }

    gMovMapFillState.maxMovementValue++;
}

void MapAddInRange(int x, int y, int range, int value)
{
    int ix, iy, iRange;

    // Handles rows [y, y+range]
    // For each row, decrement range
    for (iRange = range, iy = y; (iy <= y + range) && (iy < gMapSize.y); --iRange, ++iy)
    {
        int xMin, xMax, xRange;

        xMin = x - iRange;
        xRange = 2 * iRange + 1;

        if (xMin < 0)
        {
            xRange += xMin;
            xMin = 0;
        }

        xMax = xMin + xRange;

        if (xMax > gMapSize.x)
        {
            xMax -= (xMax - gMapSize.x);
            xMax = gMapSize.x;
        }

        for (ix = xMin; ix < xMax; ++ix)
        {
            gWorkingMap[iy][ix] += value;
        }
    }

    // Handle rows [y-range, y-1], starting from the bottom most row
    // For each row, decrement range
    for (iRange = (range - 1), iy = (y - 1); (iy >= y - range) && (iy >= 0); --iRange, --iy)
    {
        int xMin, xMax, xRange;

        xMin = x - iRange;
        xRange = 2 * iRange + 1;

        if (xMin < 0)
        {
            xRange += xMin;
            xMin = 0;
        }

        xMax = xMin + xRange;

        if (xMax > gMapSize.x)
        {
            xMax -= (xMax - gMapSize.x);
            xMax = gMapSize.x;
        }

        for (ix = xMin; ix < xMax; ++ix)
        {
            gWorkingMap[iy][ix] += value;
        }
    }
}

void MapSetInRange(int x, int y, int range, int value)
{
    int ix, iy, iRange;

    // Handles rows [y, y+range]
    // For each row, decrement range
    for (iRange = range, iy = y; (iy <= y + range) && (iy < gMapSize.y); --iRange, ++iy)
    {
        int xMin, xMax, xRange;

        xMin = x - iRange;
        xRange = 2 * iRange + 1;

        if (xMin < 0)
        {
            xRange += xMin;
            xMin = 0;
        }

        xMax = xMin + xRange;

        if (xMax > gMapSize.x)
        {
            xMax -= (xMax - gMapSize.x);
            xMax = gMapSize.x;
        }

        for (ix = xMin; ix < xMax; ++ix)
        {
            gWorkingMap[iy][ix] = value;
        }
    }

    // Handle rows [y-range, y-1], starting from the bottom most row
    // For each row, decrement range
    for (iRange = (range - 1), iy = (y - 1); (iy >= y - range) && (iy >= 0); --iRange, --iy)
    {
        int xMin, xMax, xRange;

        xMin = x - iRange;
        xRange = 2 * iRange + 1;

        if (xMin < 0)
        {
            xRange += xMin;
            xMin = 0;
        }

        xMax = xMin + xRange;

        if (xMax > gMapSize.x)
        {
            xMax -= (xMax - gMapSize.x);
            xMax = gMapSize.x;
        }

        for (ix = xMin; ix < xMax; ++ix)
        {
            gWorkingMap[iy][ix] = value;
        }
    }
}

inline void MapIncInBoundedRange(short x, short y, short minRange, short maxRange)
{
    MapAddInRange(x, y, maxRange,     +1);
    MapAddInRange(x, y, minRange - 1, -1);
}

void FillMapAttackRangeForUnit(struct Unit* unit)
{
    int ix, iy;

    #define FOR_EACH_IN_MOVEMENT_RANGE(block) \
        for (iy = gMapSize.y - 1; iy >= 0; --iy) \
        { \
            for (ix = gMapSize.x - 1; ix >= 0; --ix) \
            { \
                if (gMapMovement[iy][ix] > MAP_MOVEMENT_MAX) \
                    continue; \
                if (gMapUnit[iy][ix]) \
                    continue; \
                if (gMapMovement2[iy][ix]) \
                    continue; \
                block \
            } \
        }

    switch (GetUnitWeaponReachBits(unit, -1))
    {

    case REACH_RANGE1:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 1, 1);
        })

        break;

    case REACH_RANGE1 | REACH_RANGE2:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 1, 2);
        })

        break;

    case REACH_RANGE1 | REACH_RANGE2 | REACH_RANGE3:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 1, 3);
        })

        break;

    case REACH_RANGE2:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 2, 2);
        })

        break;

    case REACH_RANGE2 | REACH_RANGE3:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 2, 3);
        })

        break;

    case REACH_RANGE3:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 3, 3);
        })

        break;

    case REACH_RANGE3 | REACH_TO10:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 3, 10);
        })

        break;

    case REACH_RANGE1 | REACH_RANGE3:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 1, 1);
            MapIncInBoundedRange(ix, iy, 3, 3);
        })

        break;

    case REACH_RANGE1 | REACH_RANGE3 | REACH_TO10:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 1, 1);
            MapIncInBoundedRange(ix, iy, 3, 10);
        })

        break;

    case REACH_RANGE1 | REACH_RANGE2 | REACH_RANGE3 | REACH_TO10:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 1, 10);
        })

        break;

    case REACH_RANGE1 | REACH_TO10:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 1, 4);
        })

        break;

    } // switch (GetUnitWeaponReachBits(unit, -1))

    if (UNIT_ATTRIBUTES(unit) & UNIT_ATTR_BALLISTAE)
    {
        FOR_EACH_IN_MOVEMENT_RANGE({
            int item = GetBallistaItemAt(ix, iy);

            if (item)
            {
                MapIncInBoundedRange(ix, iy,
                    GetItemMinRange(item), GetItemMaxRange(item));
            }
        })
    }

    #undef FOR_EACH_IN_MOVEMENT_RANGE

    SetSubjectMap(gMapMovement);
}

void FillRangeMapByRangeMask(struct Unit* unit, int reach)
{
    int x = unit->x;
    int y = unit->y;

    switch (reach)
    {

    case REACH_RANGE1:
        MapIncInBoundedRange(x, y, 1, 1);
        break;

    case REACH_RANGE1 | REACH_RANGE2:
        MapIncInBoundedRange(x, y, 1, 2);
        break;

    case REACH_RANGE1 | REACH_RANGE2 | REACH_RANGE3:
        MapIncInBoundedRange(x, y, 1, 3);
        break;

    case REACH_RANGE2:
        MapIncInBoundedRange(x, y, 2, 2);
        break;

    case REACH_RANGE2 | REACH_RANGE3:
        MapIncInBoundedRange(x, y, 2, 3);
        break;

    case REACH_RANGE3:
        MapIncInBoundedRange(x, y, 3, 3);
        break;

    case REACH_RANGE3 | REACH_TO10:
        MapIncInBoundedRange(x, y, 3, 10);
        break;

    case REACH_RANGE1 | REACH_RANGE3:
        MapIncInBoundedRange(x, y, 1, 1);
        MapIncInBoundedRange(x, y, 3, 3);
        break;

    case REACH_RANGE1 | REACH_RANGE3 | REACH_TO10:
        MapIncInBoundedRange(x, y, 1, 1);
        MapIncInBoundedRange(x, y, 3, 10);
        break;

    case REACH_RANGE1 | REACH_RANGE2 | REACH_RANGE3 | REACH_TO10:
        MapIncInBoundedRange(x, y, 1, 10);
        break;

    case REACH_RANGE1 | REACH_TO10:
        MapIncInBoundedRange(x, y, 1, 4);
        break;

    case REACH_MAGBY2:
        MapIncInBoundedRange(x, y, 1, GetUnitMagRange(unit));
        break;

    } // switch (reach)
}

void FillMapStaffRangeForUnit(struct Unit* unit)
{
    int ix, iy;

    int reach = GetUnitStaffReachBits(unit);
    int magBy2Range = GetUnitMagRange(unit);

    #define FOR_EACH_IN_MOVEMENT_RANGE(block) \
        for (iy = gMapSize.y - 1; iy >= 0; --iy) \
        { \
            for (ix = gMapSize.x - 1; ix >= 0; --ix) \
            { \
                if (gMapMovement[iy][ix] > MAP_MOVEMENT_MAX) \
                    continue; \
                if (gMapUnit[iy][ix]) \
                    continue; \
                if (gMapMovement2[iy][ix]) \
                    continue; \
                block \
            } \
        }

    switch (reach)
    {

    case REACH_RANGE1:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 1, 1);
        })

        break;

    case REACH_RANGE1 | REACH_RANGE2:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 1, 2);
        })

        break;

    case REACH_MAGBY2:
        FOR_EACH_IN_MOVEMENT_RANGE({
            MapIncInBoundedRange(ix, iy, 1, magBy2Range);
        })

        break;

    default:
        break;

    } // switch (reach)

    #undef FOR_EACH_IN_MOVEMENT_RANGE
}

void FillRangeMapForDangerZone(s8 boolDisplayStaffRange)
{
    int i, enemyFaction;
    int hasMagicRank, prevHasMagicRank;
    u8 savedUnitId;

    prevHasMagicRank = -1;

    BmMapFill(gMapRange, 0);

    enemyFaction = IsNotEnemyPhaseMaybe();

    for (i = enemyFaction + 1; i < enemyFaction + 0x80; ++i)
    {
        struct Unit* unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit))
            continue; // not a unit

        if (boolDisplayStaffRange && !UnitKnowsMagic(unit))
            continue; // no magic in magic range mode

        if (gPlaySt.chapterVisionRange && (gMapFog[unit->y][unit->x] == 0))
            continue; // in the fog

        if (unit->flags & UNIT_FLAG_UNDER_ROOF)
            continue; // under a roof

        // Fill movement map for unit
        FillMovementMapForUnitAndMovement(unit, UNIT_MOV(unit));

        savedUnitId = gMapUnit[unit->y][unit->x];
        gMapUnit[unit->y][unit->x] = 0;

        hasMagicRank = UnitKnowsMagic(unit);

        if (prevHasMagicRank != hasMagicRank)
        {
            BmMapFill(gMapMovement2, 0);

            if (hasMagicRank)
                MapSetInMagicSealedRange(1);

            prevHasMagicRank = hasMagicRank;
        }

        SetSubjectMap(gMapRange);

        // Apply unit's range to range map

        if (boolDisplayStaffRange)
            FillMapStaffRangeForUnit(unit);
        else
            FillMapAttackRangeForUnit(unit);

        gMapUnit[unit->y][unit->x] = savedUnitId;
    }
}

void MapSetInMagicSealedRange(int value)
{
    int i;

    for (i = FACTION_RED + 1; i < FACTION_RED + 0x40; ++i)
    {
        struct Unit* unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit))
            continue;

        if (UNIT_ATTRIBUTES(unit) & UNIT_ATTR_MAGICSEAL)
            MapSetInRange(unit->x, unit->y, 10, value);
    }
}

inline u8* GetCurrentMovCostTable(void)
{
    return gWorkingMovTable;
}
