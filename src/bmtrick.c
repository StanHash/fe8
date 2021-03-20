#include "global.h"

#include "constants/terrains.h"

#include "unit.h"
#include "bmmap.h"
#include "chapterdata.h"
#include "proc.h"
#include "event.h"
#include "bmtrick.h"

static void GenerateFireTileTrapTargets(int x, int y, int damage);
static void GenerateArrowTrapTargets(int x, int y, int damage);
static void GenerateGasTrapTargets(int x, int y, int damage, int facing);
static s8 ShouldSkipGasTrapDisplay(int x, int y, int facing);

EWRAM_DATA static struct Trap sTrapPool[TRAP_MAX_COUNT] = {};
EWRAM_DATA static struct Trap sTrapLast = {};

inline struct Trap* GetTrap(int id)
{
    return sTrapPool + id;
}

void ResetTraps(void)
{
    int i;

    for (i = 0; i < TRAP_MAX_COUNT; ++i)
        sTrapPool[i].type = TRAP_NONE;

    sTrapLast.type = TRAP_NONE;
}

struct Trap* GetTrapAt(int x, int y)
{
    struct Trap* it;

lop:
    for (it = GetTrap(0); it->type != TRAP_NONE; ++it)
    {
        // Check trap position
        if ((x == it->x) && (y == it->y))
            return it;

        // Check if we on a wall, and there is a wall above
        // In which case the trap would be on the topmost wall tile
        if (gMapTerrain[y][x] == TERRAIN_WALL_1B)
        {
            if ((y > 0) && gMapTerrain[y-1][x] == TERRAIN_WALL_1B)
            {
                y = y-1;
                goto lop;
            }
        }
    }

    return NULL;
}

struct Trap* GetSpecificTrapAt(int x, int y, int trapType)
{
    struct Trap* it;

    for (it = GetTrap(0); it->type != TRAP_NONE; ++it)
    {
        // Check trap position
        if ((it->x == x) && (it->y == y) && (it->type == trapType))
            return it;

        // Check if we want a wall
        if (trapType == TERRAIN_WALL_1B)
        {
            // Check if we on a wall, and there is a wall above
            // In which case the trap would be on the topmost wall tile
            if (gMapTerrain[y][x] == TERRAIN_WALL_1B)
            {
                if ((y > 0) && gMapTerrain[y-1][x] == TERRAIN_WALL_1B)
                {
                    return GetTrapAt(x, y-1);
                }
            }
        }
    }

    return NULL;
}

struct Trap* AddTrap(int x, int y, int trapType, int meta)
{
    struct Trap* trap;

    // Find first free trap
    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap) {}

    trap->x = x;
    trap->y = y;
    trap->type = trapType;
    trap->extra = meta;

    return trap;
}

struct Trap* AddTrapExt(int x, int y, int trapType, int meta, int turnCountdown, int turnInterval, int damage)
{
    struct Trap* trap = AddTrap(x, y, trapType, meta);

    trap->data[TRAP_EXTDATA_TRAP_TURNFIRST] = turnCountdown;
    trap->data[TRAP_EXTDATA_TRAP_TURNNEXT]  = turnInterval;
    trap->data[TRAP_EXTDATA_TRAP_COUNTER]   = turnCountdown;
    trap->data[TRAP_EXTDATA_TRAP_DAMAGE]    = damage;

    return trap;
}

struct Trap* RemoveTrap(struct Trap* trap)
{
    while (trap->type != TRAP_NONE)
    {
        *trap++ = *(trap + 1);
    }

    // return trap; // BUG
}

void AddFireTrap(int x, int y, int turnCountdown, int turnInterval)
{
    AddTrapExt(x, y, TRAP_FIRETILE, 0, turnCountdown, turnInterval, 10);
}

void AddGasTrap(int x, int y, int facing, int turnCountdown, int turnInterval)
{
    AddTrapExt(x, y, TRAP_GAS, facing, turnCountdown, turnInterval, 3);
}

void AddArrowTrap(int x, int turnCountdown, int turnInterval)
{
    AddTrapExt(x, 0, TRAP_LIGHTARROW, 0, turnCountdown, turnInterval, 10);
}

void sub_802E36C(int x, int y, int turnCountdown, int turnInterval)
{
    AddTrapExt(x, y, TRAP_MAPCHANGE2, 0, turnCountdown, turnInterval, 0);
}

void AddTrap8(int x, int y)
{
    AddTrap(x, y, TRAP_8, 0);
}

void AddTrap9(int x, int y, int meta)
{
    AddTrap(x, y, TRAP_9, meta);
}

void AddSnagsAndWalls(void)
{
    int ix, iy;

    for (iy = gMapSize.y - 1; iy >= 0; --iy)
    {
        for (ix = gMapSize.x - 1; ix >= 0; --ix)
        {
            switch (gMapTerrain[iy][ix])
            {

            case TERRAIN_WALL_1B:
                if (gMapTerrain[iy-1][ix] == TERRAIN_WALL_1B)
                    continue; // walls are stacked, only the topmost tile gets a trap

                AddTrap(
                    ix, iy, TRAP_OBSTACLE,
                    GetChapterInfo(gPlaySt.chapter)->mapCrackedWallHeath);

                break;

            case TERRAIN_SNAG:
                AddTrap(ix, iy, TRAP_OBSTACLE, 20);
                break;

            } // switch (gMapTerrain[iy][ix])
        }
    }
}

void ApplyTrapMapChanges(void)
{
    struct Trap* trap;

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        switch (trap->type)
        {

        case TRAP_MAPCHANGE:
            ApplyMapChangesById(trap->extra);
            break;

        case TRAP_MAPCHANGE2:
            // this is a mystery
            ApplyMapChangesById(trap->extra ? trap->y : trap->x);
            break;

        } // switch (trap->type)
    }
}

void UpdateAllLightRunes(void)
{
    struct Trap* trap;

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        switch (trap->type)
        {

        case TRAP_LIGHT_RUNE:
            gMapTerrain[trap->y][trap->x] = TERRAIN_TILE_00;
            break;

        }
    }
}

int GetTrapExt1At(int x, int y)
{
    struct Trap* trap;

    if ((trap = GetTrapAt(x, y)) != NULL)
    {
        return trap->extra;
    }

    if ((gMapTerrain[y][x] == TERRAIN_WALL_1B) && (gMapTerrain[y-1][x] == TERRAIN_WALL_1B))
    {
        if ((trap = GetTrapAt(x, y-1)) != NULL)
        {
            return trap->extra;
        }
    }

    return 0;
}

const struct MapChange* GetMapChangesPointerById(int id)
{
    const struct MapChange* mapChange = GetChapterMapChanges(gPlaySt.chapter);

    if (!mapChange)
        return NULL;

    while (mapChange->id >= 0)
    {
        if (id == mapChange->id)
            return mapChange;

        ++mapChange;
    }

    return NULL;
}

int GetMapChangesIdAt(int x, int y)
{
    int result = -1;

    const struct MapChange* mapChange = GetChapterMapChanges(gPlaySt.chapter);

    if (!mapChange)
        return result;

    while (mapChange->id >= 0)
    {
        if (x >= mapChange->xOrigin)
            if (y >= mapChange->yOrigin)
                if (mapChange->xOrigin + mapChange->xSize - 1 >= x)
                    if (mapChange->yOrigin + mapChange->ySize - 1 >= y)
                        result = mapChange->id;

        ++mapChange;
    }

    return result;
}

void ApplyMapChangesById(int id)
{
    int ix = 0, iy = 0;

    const struct MapChange* mapChange = GetMapChangesPointerById(id);
    const u16* tileDataIt = mapChange->data;

    for (iy = 0; iy < mapChange->ySize; ++iy)
    {
        for (ix = 0; ix < mapChange->xSize; ++ix)
        {
            if (*tileDataIt != 0)
            {
                gBmMapBaseTiles[mapChange->yOrigin + iy][mapChange->xOrigin + ix] = *tileDataIt++;
            }
            else
            {
                ++tileDataIt;
            }
        }
    }
}

void AddMapChange(int id)
{
    AddTrap(0, 0, TRAP_MAPCHANGE, id);
}

void RemoveMapChange(int id)
{
    struct Trap* trap;

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        if (trap->type == TRAP_MAPCHANGE && trap->extra == id)
            RemoveTrap(trap);
    }
}

s8 AreMapChangeTriggered(int id)
{
    struct Trap* trap;

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        if (trap->type == TRAP_MAPCHANGE && trap->extra == id)
            return TRUE;
    }

    return FALSE;
}

void HideIfUnderRoof(struct Unit* unit)
{
    if (gMapTerrain[unit->y][unit->x] == TERRAIN_ROOF)
    {
        unit->flags |= (UNIT_FLAG_HIDDEN | UNIT_FLAG_UNDER_ROOF);
    }
}

void UpdateUnitsUnderRoof(void)
{
    int i;

    for (i = 1; i < 0xC0; ++i)
    {
        struct Unit* unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit))
            continue;

        if (!(unit->flags & UNIT_FLAG_UNDER_ROOF))
            continue;

        if (gMapTerrain[unit->y][unit->x] != TERRAIN_ROOF)
        {
            unit->flags = (unit->flags &~ (UNIT_FLAG_UNDER_ROOF | UNIT_FLAG_HIDDEN)) | UNIT_FLAG_SEEN;
        }
    }

    RefreshEntityBmMaps();
    RefreshUnitSprites();
}

void GenerateFireTileTrapTargets(int x, int y, int damage)
{
    AddTarget(x, y, gMapUnit[y][x], damage);
}

void GenerateArrowTrapTargets(int x, int y, int damage)
{
    int iy;

    for (iy = 0; iy < gMapSize.y; ++iy)
    {
        if (gMapUnit[iy][x])
            AddTarget(x, iy, gMapUnit[iy][x], damage);
    }
}

void GenerateGasTrapTargets(int x, int y, int damage, int facing)
{
    int i;

    int xInc = 0;
    int yInc = 0;

    switch (facing)
    {

    case FACING_UP:
        xInc = 0;
        yInc = -1;

        break;

    case FACING_DOWN:
        xInc = 0;
        yInc = +1;

        break;

    case FACING_LEFT:
        xInc = -1;
        yInc = 0;

        break;

    case FACING_RIGHT:
        xInc = +1;
        yInc = 0;

        break;

    } // switch (facing)

    for (i = 2; i >= 0; --i)
    {
        x += xInc;
        y += yInc;

        if (gMapUnit[y][x])
            AddTarget(x, y, gMapUnit[y][x], damage);
    }
}

s8 ShouldSkipGasTrapDisplay(int x, int y, int facing)
{
    int i;

    int xInc = 0;
    int yInc = 0;

    s8 boolHasNoEffect = TRUE;

    switch (facing)
    {

    case FACING_UP:
        xInc = 0;
        yInc = -1;

        break;

    case FACING_DOWN:
        xInc = 0;
        yInc = +1;

        break;

    case FACING_LEFT:
        xInc = -1;
        yInc = 0;

        break;

    case FACING_RIGHT:
        xInc = +1;
        yInc = 0;

        break;

    } // switch (facing)

    for (i = 0; i < 3; ++i)
    {
        x += xInc;
        y += yInc;

        if (gMapUnit[y][x])
            boolHasNoEffect = FALSE;
    }

    return boolHasNoEffect;
}

void MakeTargetListForTurnTrapDamage(void)
{
    struct Trap* trap;

    InitTargets(0, 0);

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        if ((s8) trap->data[TRAP_EXTDATA_TRAP_COUNTER] == 0)
        {
            switch (trap->type)
            {

            case TRAP_FIRETILE:
                GenerateFireTileTrapTargets(trap->x, trap->y, (s8) trap->data[TRAP_EXTDATA_TRAP_DAMAGE]);
                break;

            case TRAP_LIGHTARROW:
                GenerateArrowTrapTargets(trap->x, trap->y, (s8) trap->data[TRAP_EXTDATA_TRAP_DAMAGE]);
                break;

            case TRAP_GAS:
                GenerateGasTrapTargets(trap->x, trap->y, (s8) trap->data[TRAP_EXTDATA_TRAP_DAMAGE], trap->extra);
                break;

            }
        }
    }
}

void GenerateDisplayedTrapDamageTargets(void)
{
    struct Trap* trap;

    int specialType = 0;

    InitTargets(0, 0);

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        if (trap->data[TRAP_EXTDATA_TRAP_COUNTER] == 0)
        {
            switch (trap->type)
            {

            case TRAP_FIRETILE:
                if (gMapUnit[trap->y][trap->x])
                {
                    AddTarget(trap->x, trap->y, 0, TRAP_FIRETILE);
                    GenerateFireTileTrapTargets(trap->x, trap->y, trap->data[TRAP_EXTDATA_TRAP_DAMAGE]);
                }

                break;

            case TRAP_GAS:
                switch (trap->extra)
                {

                    // TODO: figure out

                case FACING_UP:
                    specialType = 0x64;
                    break;

                case FACING_DOWN:
                    specialType = 0x65;
                    break;

                case FACING_LEFT:
                    specialType = 0x66;
                    break;

                case FACING_RIGHT:
                    specialType = 0x67;
                    break;

                } // switch (trap->data[TRAP_EXTDATA_GAS_FACING])

                if (!ShouldSkipGasTrapDisplay(trap->x, trap->y, trap->extra))
                {
                    AddTarget(trap->x, trap->y, 0, specialType);
                    GenerateGasTrapTargets(trap->x, trap->y, trap->data[TRAP_EXTDATA_TRAP_DAMAGE], trap->extra);
                }

                break;

            case TRAP_LIGHTARROW:
                AddTarget(trap->x, trap->y, 0, TRAP_LIGHTARROW);
                GenerateArrowTrapTargets(trap->x, trap->y, trap->data[TRAP_EXTDATA_TRAP_DAMAGE]);
                break;

            case TRAP_MAPCHANGE2:
                AddTarget(trap->extra ? trap->x : trap->y, TRAP_INDEX(trap), 0, trap->type);
                break;

            } // switch (trap->type)
        }
    }
}

void CountDownTraps(void)
{
    struct Trap* trap;

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        switch (trap->type)
        {

        case TRAP_FIRETILE:
        case TRAP_GAS:
        case TRAP_LIGHTARROW:
        case TRAP_MAPCHANGE2:
            trap->data[TRAP_EXTDATA_TRAP_COUNTER]--;
            break;

        } // switch (trap->type)
    }
}

void ResetCounterForCountedDownTraps(void)
{
    struct Trap* trap;

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        switch (trap->type)
        {

        case TRAP_FIRETILE:
        case TRAP_GAS:
        case TRAP_LIGHTARROW:
        case TRAP_MAPCHANGE2:
            if (trap->data[TRAP_EXTDATA_TRAP_COUNTER] == 0)
                trap->data[TRAP_EXTDATA_TRAP_COUNTER] = trap->data[TRAP_EXTDATA_TRAP_TURNNEXT];

            break;

        } // switch (trap->type)
    }
}

void sub_802EA00(void)
{
    int truePhase = gPlaySt.chapterPhaseIndex;
    gPlaySt.chapterPhaseIndex = FACTION_RED;

    RefreshEntityBmMaps();

    gPlaySt.chapterPhaseIndex = truePhase;
}

void sub_802EA1C(void)
{
    sub_8026414(3);
}

void sub_802EA28(void)
{
    // TODO: EID/FLAG DEFINITIONS

    if (CheckFlag(0x65) || CountAvailableBlueUnits() == 0)
    {
        CallGameOverEvent();
    }

    if (!IsAnyEnemyUnitAlive())
        SetFlag(0x06);
}

struct Trap* AddLightRune(int x, int y)
{
    struct Trap* trap = AddTrap(x, y, TRAP_LIGHT_RUNE, gMapTerrain[y][x]);

    trap->data[TRAP_EXTDATA_RUNE_TURNSLEFT] = 3;
    gMapTerrain[y][x] = TERRAIN_TILE_00;

    // return trap; // BUG
}

struct Trap* RemoveLightRune(struct Trap* trap)
{
    gMapTerrain[trap->y][trap->x] = GetTrueTerrainAt(trap->x, trap->y);
    return RemoveTrap(trap);
}

void HandleTrapDecay(void)
{
    struct Trap* trap;

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        switch (trap->type)
        {

        case TRAP_TORCHLIGHT:
            trap->extra--;

            if (trap->extra == 0)
            {
                RemoveTrap(trap);
                trap--;
            }

            break;

        case TRAP_LIGHT_RUNE:
            trap->data[TRAP_EXTDATA_RUNE_TURNSLEFT]--;

            if (trap->data[TRAP_EXTDATA_RUNE_TURNSLEFT] == 0)
            {
                RemoveLightRune(trap);
                trap--;
            }

            break;

        } // switch (trap->type)
    }
}

void BattleSomethingTrapChangeTerrain(void)
{
    struct Trap* trap;

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        switch (trap->type)
        {

        case TRAP_LIGHT_RUNE:
            gMapTerrain[trap->y][trap->x] = GetTrueTerrainAt(trap->x, trap->y);
            break;

        } // switch (trap->type)
    }
}

void NullAllLightRunesTerrain(void)
{
    struct Trap* trap;

    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
    {
        switch (trap->type)
        {

        case TRAP_LIGHT_RUNE:
            gMapTerrain[trap->y][trap->x] = TERRAIN_TILE_00;
            break;

        } // switch (trap->type)
    }
}
