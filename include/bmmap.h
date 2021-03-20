#ifndef GUARD_BMMAP_H
#define GUARD_BMMAP_H

void InitChapterMap(int chapterId);
void InitChapterPreviewMap(int chapterId);
void sub_8019624(void);
void sub_8019778(void);
void BmMapFill(u8** map, int value);
void BmMapFillEdges(u8** map, u8 value);
void UnpackChapterMap(void* into, int chapterId);
void UnpackChapterMapGraphics(int chapterId);
void UnpackChapterMapPalette(void);
void InitBaseTilesBmMap(void);
void RefreshTerrainBmMap(void);
int GetTrueTerrainAt(int x, int y);
void DisplayBmTile(u16* bg, int xOut, int yOut, int xMap, int yMap);
void nullsub_8(void);
void DisplayMovementViewTile(u16* out, int xMap, int yMap, int xOut, int yOut);
void RenderBmMap(void);
void RenderBmMapOnBg2(void);
void UpdateBmMapDisplay(void);
void RefreshEntityBmMaps(void);
char* GetTerrainName(int terrainId);
int GetTerrainHealAmount(int terrainId);
int GetTerrainHealsStatus(int terrainId);
void sub_801A278(void);
void RevertMapChange(int id);

extern struct Vec2 gMapSize;
extern u8** gMapUnit;
extern u8** gMapTerrain;
extern u8** gMapMovement;
extern u8** gMapRange;
extern u8** gMapFog;
extern u8** gMapHidden;
extern u8** gMapMovement2;

extern u8*   gTilesetTerrainLookup;
extern u16** gBmMapBaseTiles;

enum
{
    // Magic values for gMapMovement

    MAP_MOVEMENT_MAX = 120,
    MAP_MOVEMENT_EXTENDED = 124,
};

enum
{
    // For use with gMapHidden

    HIDDEN_BIT_UNIT = (1 << 0),
    HIDDEN_BIT_TRAP = (1 << 1),
};

#endif // GUARD_BMMAP_H
