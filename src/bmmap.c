#include "global.h"

#include "constants/terrains.h"

#include "hardware.h"
#include "chapterdata.h"
#include "proc.h"
#include "event.h"
#include "unit.h"
#include "bmmap.h"
#include "bmidoten.h"
#include "bmtrick.h"

static void BmMapInit(void* buffer, u8*** outHandle, int width, int height);

static void RenderBmMapColumn(u16 xOffset);
static void RenderBmMapLine(u16 yOffset);

static void RefreshUnitsOnBmMap(void);
static void RefreshTorchlightsOnBmMap(void);
static void RefreshMinesOnBmMap(void);

enum { MAP_POOL_SIZE = 0x7B8 };

// TODO: figure out what's up with this (overlaps with a lot of other objects?)
extern u16 gBmMapBuffer[];

EWRAM_DATA struct Vec2 gMapSize = {};

EWRAM_DATA u8** gMapUnit     = NULL;
EWRAM_DATA u8** gMapTerrain  = NULL;
EWRAM_DATA u8** gMapMovement = NULL;
EWRAM_DATA u8** gMapRange    = NULL;
EWRAM_DATA u8** gMapFog      = NULL;
EWRAM_DATA u8** gMapHidden   = NULL;
EWRAM_DATA u8** gMapMovement2      = NULL;

EWRAM_DATA static u8 sBmMapUnitPool[MAP_POOL_SIZE] = {};
EWRAM_DATA static u8 sBmMapTerrainPool[MAP_POOL_SIZE] = {};
EWRAM_DATA static u8 sBmMapFogPool[MAP_POOL_SIZE] = {};
EWRAM_DATA static u8 sBmMapHiddenPool[MAP_POOL_SIZE] = {};
EWRAM_DATA static u8 sBmMapUnkPool[MAP_POOL_SIZE] = {};

EWRAM_DATA static u16 sTilesetConfig[0x1000 + 0x200] = {};

EWRAM_DATA static u16 sBmBaseTilesPool[MAP_POOL_SIZE] = {};

static u8** sInitializingMap;

static u8 sBmMapMovementPool[MAP_POOL_SIZE];
static u8 sBmMapRangePool[MAP_POOL_SIZE];

u8*   gTilesetTerrainLookup = (u8*)(sTilesetConfig + 0x1000);
u16** gBmMapBaseTiles       = (u16**)(sBmBaseTilesPool);

void InitChapterMap(int chapterId) {
    UnpackChapterMap(gBmMapBuffer, chapterId);
    UnpackChapterMapGraphics(chapterId);

    BmMapInit(sBmMapUnitPool,     &gMapUnit,     gMapSize.x, gMapSize.y);
    BmMapInit(sBmMapTerrainPool,  &gMapTerrain,  gMapSize.x, gMapSize.y);
    BmMapInit(sBmMapMovementPool, &gMapMovement, gMapSize.x, gMapSize.y);
    BmMapInit(sBmMapRangePool,    &gMapRange,    gMapSize.x, gMapSize.y);
    BmMapInit(sBmMapFogPool,      &gMapFog,      gMapSize.x, gMapSize.y);
    BmMapInit(sBmMapHiddenPool,   &gMapHidden,   gMapSize.x, gMapSize.y);
    BmMapInit(sBmMapUnkPool, &gMapMovement2, gMapSize.x, gMapSize.y);

    BmMapFill(gMapUnit, 0);
    BmMapFill(gMapTerrain, 0);

    InitBaseTilesBmMap();
    ApplyTrapMapChanges();
    RefreshTerrainBmMap();

    // TODO: chapter id definitions
    if (gPlaySt.chapter == 0x75)
        sub_8019624();
}

void InitChapterPreviewMap(int chapterId) {
    UnpackChapterMap(gBmMapBuffer, chapterId);

    BmMapInit(sBmMapUnitPool,    &gMapUnit,    gMapSize.x, gMapSize.y);
    BmMapInit(sBmMapTerrainPool, &gMapTerrain, gMapSize.x, gMapSize.y);

    BmMapFill(gMapUnit, 0);
    BmMapFill(gMapTerrain, 0);

    InitBaseTilesBmMap();
    RefreshTerrainBmMap();
}

void sub_8019624(void) {
    int ix, iy;

    // Automatic water shadows?

    for (iy = 0; iy < gMapSize.y; ++iy) {
        for (ix = 0; ix < gMapSize.x; ++ix) {
            int connexion;

            if (gMapTerrain[iy][ix] != TERRAIN_WATER)
                continue;

            connexion = 0;

            if (ix > 0) {
                if (gMapTerrain[iy][ix - 1] == TERRAIN_FLOOR_17)
                    connexion = 1;

                if (gMapTerrain[iy][ix - 1] == TERRAIN_STAIRS)
                    connexion = 1;

                if (gMapTerrain[iy][ix - 1] == TERRAIN_CHEST_20)
                    connexion = 1;

                if (gMapTerrain[iy][ix - 1] == TERRAIN_CHEST_21)
                    connexion = 1;
            }

            if (iy > 0) {
                if (gMapTerrain[iy - 1][ix] == TERRAIN_FLOOR_17)
                    connexion += 2;

                if (gMapTerrain[iy - 1][ix] == TERRAIN_STAIRS)
                    connexion += 2;

                if (gMapTerrain[iy - 1][ix] == TERRAIN_CHEST_20)
                    connexion += 2;

                if (gMapTerrain[iy - 1][ix] == TERRAIN_CHEST_21)
                    connexion += 2;
            }

            if (ix > 0 && iy > 0)
                if ((gMapTerrain[iy]    [ix - 1] == TERRAIN_FLOOR_17) &&
                    (gMapTerrain[iy + 1][ix - 1] == TERRAIN_WATER) &&
                    (gMapTerrain[iy - 1][ix]     != TERRAIN_FLOOR_17))
                    connexion = 4;

            switch (connexion) {

            case 1: // straight shadow on the left
                gBmMapBaseTiles[iy][ix] = 0x2DC;
                break;

            case 2: // straight shadow on the top
                gBmMapBaseTiles[iy][ix] = 0x2D8;
                break;

            case 3: // shadow on both the left and the top
                gBmMapBaseTiles[iy][ix] = 0x358;
                break;

            case 4: // shadow on the left, but stronger at the top-left than bottom-left
                gBmMapBaseTiles[iy][ix] = 0x35C;
                break;

            } // switch (connexion)
        }
    }
}

void sub_8019778(void) {
    UnpackChapterMap(gBmMapBuffer, gPlaySt.chapter);

    InitBaseTilesBmMap();
    ApplyTrapMapChanges();
    RefreshTerrainBmMap();
    sub_8019624();
}

void BmMapInit(void* buffer, u8*** outHandle, int x, int y) {
    int i;
    u8* itBuffer;

    sInitializingMap = buffer;

    x  += 2; // two tiles on each edge (shared)
    y += 4; // two tiles on each edge

    // itBuffer = start of tile area (the first y * sizeof(u8*) bytes are reserved for row pointers)
    itBuffer = buffer + y * sizeof(u8*);

    // Setting up the row pointers
    for (i = 0; i < y; ++i) {
        sInitializingMap[i] = itBuffer;
        itBuffer += x;
    }

    // first row is actually the third, ensuring the top two map rows as safety
    *outHandle = sInitializingMap + 2;
}

void BmMapFill(u8** map, int value) {
    int size = (gMapSize.y + 4) * (gMapSize.x + 2);

    if (size % 2)
        size = size - 1;

    value = (0xFF & value);
    value += value << 8;

    CpuFill16(value, map[-2], size);

    SetSubjectMap(map);
}

void BmMapFillEdges(u8** map, u8 value) {
    int ix, iy;

    u8** theMap = map;

    // Set tile values for horizontal edges
    for (iy = 0; iy < gMapSize.y; ++iy) {
        theMap[iy][0]              = value;
        theMap[iy][gMapSize.x-1] = value;
    }

    // Set tile values for vertical edges
    for (ix = 0; ix < gMapSize.x; ++ix) {
        theMap[0]             [ix] = value;
        theMap[gMapSize.y-1][ix] = value;
    }
}

void UnpackChapterMap(void* into, int chapterId) {
    // Decompress map data
    Decompress(
        GetChapterMapData(chapterId), into);

    // Setting map size
    gMapSize.x = ((u8*)(into))[0];
    gMapSize.y = ((u8*)(into))[1];

    // Decompress tileset config
    Decompress(
        gChapterDataAssetTable[GetChapterInfo(chapterId)->mapTileConfigId], sTilesetConfig);

    // Setting max camera offsets (?) TODO: figure out
    gBmSt.unk28.x = gMapSize.x*16 - 240;
    gBmSt.unk28.y = gMapSize.y*16 - 160;
}

void UnpackChapterMapGraphics(int chapterId) {
    // Decompress tileset graphics (part 1)
    Decompress(
        gChapterDataAssetTable[GetChapterInfo(chapterId)->mapObj1Id],
        (void*)(BG_VRAM + 0x20 * 0x400)); // TODO: tile id constant?

    // Decompress tileset graphics (part 2, if it exists)
    if (gChapterDataAssetTable[GetChapterInfo(chapterId)->mapObj2Id])
        Decompress(
            gChapterDataAssetTable[GetChapterInfo(chapterId)->mapObj2Id],
            (void*)(BG_VRAM + 0x20 * 0x600)); // TODO: tile id constant?

    // Apply tileset palette
    ApplyPaletteExt(
        gChapterDataAssetTable[GetChapterInfo(chapterId)->mapPaletteId],
        0x20 * 6, 0x20 * 10); // TODO: palette id constant?
}

void UnpackChapterMapPalette(void) {
    ApplyPaletteExt(
        gChapterDataAssetTable[GetChapterInfo(gPlaySt.chapter)->mapPaletteId],
        0x20 * 6, 0x20 * 10); // TODO: palette id constant?
}

void InitBaseTilesBmMap(void) {
    int ix, iy;

    u16** rows;
    u16*  tiles;
    u16*  itBuffer;

    rows  = gBmMapBaseTiles;
    tiles = gBmMapBuffer;

    gMapSize.y++; // ?

    // Ignore first short (x, y byte pair)
    tiles++;

    // Tile buffer starts after the rows
    itBuffer = (u16*)(gBmMapBaseTiles + gMapSize.y);

    for (iy = 0; iy < gMapSize.y; ++iy) {
        // Set row buffer
        rows[iy] = itBuffer;
        itBuffer += gMapSize.x;

        // Set tiles
        for (ix = 0; ix < gMapSize.x; ++ix)
            gBmMapBaseTiles[iy][ix] = *tiles++;
    }

    // Fill "bottom" row with empty tiles?
    // "bottom" as the y had been increased too this is just weird

    tiles = gBmMapBaseTiles[iy - 1];

    for (ix = 0; ix < gMapSize.x; ++ix)
        *tiles++ = 0;

    gMapSize.y--; // ?
}

void RefreshTerrainBmMap(void) {
    int ix, iy;

    for (iy = 0; iy < gMapSize.y; ++iy)
        for (ix = 0; ix < gMapSize.x; ++ix)
            gMapTerrain[iy][ix] = gTilesetTerrainLookup[gBmMapBaseTiles[iy][ix] >> 2];

    UpdateAllLightRunes();
}

int GetTrueTerrainAt(int x, int y) {
    return gTilesetTerrainLookup[gBmMapBaseTiles[y][x] >> 2];
}

void DisplayBmTile(u16* bg, int xTileMap, int yTileMap, int xBmMap, int yBmMap) {
    u16* out = bg + yTileMap * 0x40 + xTileMap * 2; // TODO: BG_LOCATED_TILE?
    u16* tile = sTilesetConfig + gBmMapBaseTiles[yBmMap][xBmMap];

    // TODO: palette id constants
    u16 base = gMapFog[yBmMap][xBmMap] ? (6 << 12) : (11 << 12);

    out[0x00 + 0] = base + *tile++;
    out[0x00 + 1] = base + *tile++;
    out[0x20 + 0] = base + *tile++;
    out[0x20 + 1] = base + *tile++;
}

void nullsub_8(void) {}

void DisplayMovementViewTile(u16* bg, int xBmMap, int yBmMap, int xTileMap, int yTileMap) {
    bg = bg + 2*(yTileMap * 0x20 + xTileMap);

    if (!bg)
        nullsub_8();

    // TODO: tile macros?
    // TODO: are the movement and range maps s8[][]?

    if (((s8**)(gMapMovement))[yBmMap][xBmMap] >= 0) {
        bg[0x00 + 0] = 0x4280;
        bg[0x00 + 1] = 0x4281;
        bg[0x20 + 0] = 0x4282;
        bg[0x20 + 1] = 0x4283;

        return;
    }

    if (((s8**)(gMapRange))[yBmMap][xBmMap]) {
        if (bg[0]) {
            bg[0x00 + 0] = 0x5284;
            bg[0x00 + 1] = 0x5285;
            bg[0x20 + 0] = 0x5286;
            bg[0x20 + 1] = 0x5287;

            return;
        } else {
            bg[0x00 + 0] = 0x5280;
            bg[0x00 + 1] = 0x5281;
            bg[0x20 + 0] = 0x5282;
            bg[0x20 + 1] = 0x5283;

            return;
        }
    }

    bg[0x00 + 0] = 0;
    bg[0x00 + 1] = 0;
    bg[0x20 + 0] = 0;
    bg[0x20 + 1] = 0;
}

void RenderBmMap(void) {
    int ix, iy;

    gBmSt.mapRenderOrigin.x = gBmSt.camera.x >> 4;
    gBmSt.mapRenderOrigin.y = gBmSt.camera.y >> 4;

    for (iy = (10 - 1); iy >= 0; --iy)
        for (ix = (15 - 1); ix >= 0; --ix)
            DisplayBmTile(gBg3Tm, ix, iy,
                (short) gBmSt.mapRenderOrigin.x + ix, (short) gBmSt.mapRenderOrigin.y + iy);

    EnableBgSync(BG3_SYNC_BIT);
    SetBgOffset(3, 0, 0);

    SetDispEnable(1, 1, 1, 1, 1);
}

void RenderBmMapOnBg2(void) {
    int ix, iy;

    SetBgChrOffset(2, 0x8000);

    gBmSt.mapRenderOrigin.x = gBmSt.camera.x >> 4;
    gBmSt.mapRenderOrigin.y = gBmSt.camera.y >> 4;

    for (iy = (10 - 1); iy >= 0; --iy)
        for (ix = (15 - 1); ix >= 0; --ix)
            DisplayBmTile(gBg2Tm, ix, iy,
                (short) gBmSt.mapRenderOrigin.x + ix, (short) gBmSt.mapRenderOrigin.y + iy);

    EnableBgSync(BG2_SYNC_BIT);
    SetBgOffset(2, 0, 0);
}

void UpdateBmMapDisplay(void) {
    // TODO: figure out

    if (gBmSt.camera.x != gBmSt.cameraPrevious.x) {
        if (gBmSt.camera.x > gBmSt.cameraPrevious.x) {
            if (((gBmSt.camera.x - 1) ^ (gBmSt.cameraPrevious.x - 1)) & 0x10)
                RenderBmMapColumn(15);
        } else {
            if ((gBmSt.camera.x ^ gBmSt.cameraPrevious.x) & 0x10)
                RenderBmMapColumn(0);
        }
    }

    if (gBmSt.camera.y != gBmSt.cameraPrevious.y) {
        if (gBmSt.camera.y > gBmSt.cameraPrevious.y) {
            if (((gBmSt.camera.y - 1) ^ (gBmSt.cameraPrevious.y - 1)) & 0x10)
                RenderBmMapLine(10);
        } else {
            if ((gBmSt.camera.y ^ gBmSt.cameraPrevious.y) & 0x10)
                RenderBmMapLine(0);
        }
    }

    gBmSt.cameraPrevious = gBmSt.camera;

    SetBgOffset(3,
        gBmSt.camera.x - gBmSt.mapRenderOrigin.x * 16,
        gBmSt.camera.y - gBmSt.mapRenderOrigin.y * 16
    );

    // TODO: GAME STATE BITS CONSTANTS
    if (gBmSt.gameStateBits & 1) {
        SetBgOffset(2,
            gBmSt.camera.x - gBmSt.mapRenderOrigin.x * 16,
            gBmSt.camera.y - gBmSt.mapRenderOrigin.y * 16
        );
    }
}

void RenderBmMapColumn(u16 xOffset) {
    u16 xBmMap = (gBmSt.camera.x >> 4) + xOffset;
    u16 yBmMap = (gBmSt.camera.y >> 4);

    u16 xTileMap = ((gBmSt.camera.x >> 4) - gBmSt.mapRenderOrigin.x + xOffset) & 0xF;
    u16 yTileMap = ((gBmSt.camera.y >> 4) - gBmSt.mapRenderOrigin.y);

    int iy;

    if (!(gBmSt.gameStateBits & 1)) {
        for (iy = 10; iy >= 0; --iy) {
            DisplayBmTile(gBg3Tm,
                xTileMap, (yTileMap + iy) & 0xF,
                xBmMap, (yBmMap + iy));
        }

        EnableBgSync(BG3_SYNC_BIT);
    } else {
        for (iy = 10; iy >= 0; --iy) {
            DisplayBmTile(gBg3Tm,
                xTileMap, (yTileMap + iy) & 0xF,
                xBmMap, (yBmMap + iy));

            DisplayMovementViewTile(gBg2Tm,
                xBmMap, (yBmMap + iy),
                xTileMap, (yTileMap + iy) & 0xF);
        }

        EnableBgSync(BG3_SYNC_BIT | BG2_SYNC_BIT);
    }
}

void RenderBmMapLine(u16 yOffset) {
    u16 xBmMap = (gBmSt.camera.x >> 4);
    u16 yBmMap = (gBmSt.camera.y >> 4) + yOffset;

    u16 xTileMap = ((gBmSt.camera.x >> 4) - gBmSt.mapRenderOrigin.x);
    u16 yTileMap = ((gBmSt.camera.y >> 4) - gBmSt.mapRenderOrigin.y + yOffset) & 0xF;

    int ix;

    if (!(gBmSt.gameStateBits & 1)) {
        for (ix = 15; ix >= 0; --ix) {
            DisplayBmTile(gBg3Tm,
                (xTileMap + ix) & 0xF, yTileMap,
                (xBmMap + ix), yBmMap);
        }

        EnableBgSync(BG3_SYNC_BIT);
    } else {
        for (ix = 15; ix >= 0; --ix) {
            DisplayBmTile(gBg3Tm,
                (xTileMap + ix) & 0xF, yTileMap,
                (xBmMap + ix), yBmMap);

            DisplayMovementViewTile(gBg2Tm,
                (xBmMap + ix), yBmMap,
                (xTileMap + ix) & 0xF, yTileMap);
        }

        EnableBgSync(BG3_SYNC_BIT | BG2_SYNC_BIT);
    }
}

void RefreshUnitsOnBmMap(void) {
    struct Unit* unit;
    int i;

    // 1. Blue & Green units

    for (i = 1; i < FACTION_RED; ++i) {
        unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit))
            continue;

        if (unit->flags & UNIT_FLAG_HIDDEN)
            continue;

        // Put unit on unit map
        gMapUnit[unit->y][unit->x] = i;

        // If fog is enabled, apply unit vision to fog map
        if (gPlaySt.chapterVisionRange)
            MapAddInRange(unit->x, unit->y, GetUnitVision(unit), 1);
    }

    // 2. Red (& Purple) units

    if (gPlaySt.chapterPhaseIndex != FACTION_RED) {
        // 2.1. No red phase

        for (i = FACTION_RED + 1; i < FACTION_PURPLE + 6; ++i) {
            unit = GetUnit(i);

            if (!UNIT_IS_VALID(unit))
                continue;

            if (unit->flags & UNIT_FLAG_HIDDEN)
                continue;

            // If unit is magic seal, set fog in range 0-10.
            // Magic seal set the fog map probably solely for the alternate map palette.
            if (UNIT_ATTRIBUTES(unit) & UNIT_ATTR_MAGICSEAL)
                MapAddInRange(unit->x, unit->y, 10, -1);

            if (gPlaySt.chapterVisionRange && !gMapFog[unit->y][unit->x]) {
                // If in fog, set unit bit on the hidden map, and set the "hidden in fog" flag

                gMapHidden[unit->y][unit->x] |= HIDDEN_BIT_UNIT;
                unit->flags = unit->flags | UNIT_FLAG_UNSEEN;
            } else {
                // If not in fog, put unit on the map, and update flags accordingly

                gMapUnit[unit->y][unit->x] = i;

                if (unit->flags & UNIT_FLAG_UNSEEN)
                    unit->flags = (unit->flags &~ UNIT_FLAG_UNSEEN) | UNIT_FLAG_SEEN;
            }
        }
    } else {
        // 2.2. Yes red phase

        // This does mostly the same as the "No red phase" loop, except:
        // - It always puts the units on the unit map
        // - It never sets the "spotted" unit flag (even if unit is seen)

        for (i = FACTION_RED + 1; i < FACTION_PURPLE + 6; ++i) {
            unit = GetUnit(i);

            if (!UNIT_IS_VALID(unit))
                continue;

            if (unit->flags & UNIT_FLAG_HIDDEN)
                continue;

            // See above
            if (UNIT_ATTRIBUTES(unit) & UNIT_ATTR_MAGICSEAL)
                MapAddInRange(unit->x, unit->y, 10, -1);

            if (gPlaySt.chapterVisionRange) {
                // Update unit flags according to fog level

                if (!gMapFog[unit->y][unit->x])
                    unit->flags = unit->flags | UNIT_FLAG_UNSEEN;
                else
                    unit->flags = unit->flags &~ UNIT_FLAG_UNSEEN;
            }

            // Put on unit map
            gMapUnit[unit->y][unit->x] = i;
        }
    }
}

void RefreshTorchlightsOnBmMap(void) {
    struct Trap* trap;
    
    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap) {
        switch (trap->type) {

        case TRAP_TORCHLIGHT:
            MapAddInRange(trap->x, trap->y, trap->extra, 1);
            break;

        } // switch (trap->type)
    }
}

void RefreshMinesOnBmMap(void) {
    struct Trap* trap;
    
    for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap) {
        switch (trap->type) {

        case TRAP_MINE:
            if (!gMapUnit[trap->y][trap->x])
                gMapHidden[trap->y][trap->x] |= HIDDEN_BIT_TRAP;

            break;

        } // switch (trap->type)
    }
}

void RefreshEntityBmMaps(void) {
    // 1. Clear unit & hidden maps

    BmMapFill(gMapUnit, 0);
    BmMapFill(gMapHidden, 0);

    // 2. Clear fog map, with 1 (visible) if no fog, with 0 (hidden) if yes fog

    BmMapFill(gMapFog, !gPlaySt.chapterVisionRange ? 1 : 0);

    // 3. Populate unit, fog & hidden maps

    RefreshTorchlightsOnBmMap();
    RefreshUnitsOnBmMap();
    RefreshMinesOnBmMap();
}

char* GetTerrainName(int terrainId) {
    return GetMsg(gUnknown_0880D374[terrainId]);
}

int GetTerrainHealAmount(int terrainId) {
    return gUnknown_0880C744[terrainId];
}

int GetTerrainHealsStatus(int terrainId) {
    return gUnknown_0880C785[terrainId];
}

void sub_801A278(void) {
    const u16* tile = sTilesetConfig;

    // TODO: game state bits constants
    if (!sub_800D208() || (gBmSt.gameStateBits & 0x10)) {
        // TODO: macros?
        SetBlankChr(0x400 + (*tile++ & 0x3FF));
        SetBlankChr(0x400 + (*tile++ & 0x3FF));
        SetBlankChr(0x400 + (*tile++ & 0x3FF));
        SetBlankChr(0x400 + (*tile++ & 0x3FF));
    }

    SetBackdropColor(0);
}

void RevertMapChange(int id) {
    const struct MapChange* mapChange;
    u8 ix, iy;

    Decompress(GetChapterMapData(gPlaySt.chapter), gBmMapBuffer);

    mapChange = GetMapChangesPointerById(id);

    for (iy = mapChange->yOrigin; iy < (mapChange->yOrigin + mapChange->ySize); ++iy) {
        u16* itSource = (iy * gMapSize.x) + mapChange->xOrigin + (gBmMapBuffer + 1);
        u16* itDest   = gBmMapBaseTiles[iy] + mapChange->xOrigin;

        for (ix = mapChange->xOrigin; ix < (mapChange->xOrigin + mapChange->xSize); ++ix)
            *itDest++ = *itSource++;
    }
}
