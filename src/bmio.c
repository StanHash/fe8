#include "global.h"

#include <string.h>

#include "constants/jids.h"

#include "armfunc.h"
#include "proc.h"
#include "hardware.h"
#include "oam.h"
#include "text.h"
#include "uiutils.h"
#include "chapterdata.h"
#include "random.h"
#include "sprite.h"
#include "unit.h"
#include "bmmap.h"
#include "bmbattle.h"
#include "bmtrick.h"
#include "mu.h"
#include "uimenu.h"

#include "bmio.h"

// General Battle Map System Stuff, mostly low level hardware stuff but also more

// TODO: move to where appropriate
extern const struct ProcScr gProc_BMapMain[]; // gProc_BMapMain

struct WeatherParticle {
    /* 00 */ short xPosition;
    /* 02 */ short yPosition;

    /* 04 */ short xSpeed;
    /* 06 */ short ySpeed;

    /* 08 */ u8  gfxIndex;
    /* 09 */ u8  typeId;
};

union WeatherEffectData {
    /**
     * Array of weather particles
     */
    struct WeatherParticle particles[0x40];

    /**
     * Buffer for cloud graphics
     */
    u32 gfxData[0xC0];
};

union GradientEffectData {
    /**
     * Buffer holding colors for vertical gradient.
     */
    u16 lines[320];

    /**
     * Buffer holding 8 different variations of the tileset palette
     * Variations have increasing amounts of red; used for flames weather effect
     */
    u16 fireGradient[8][0x40];
};

struct BMVSyncProc {
    PROC_HEADER;

    /* 2C */ const struct TileGfxAnim* tileGfxAnimStart;
    /* 30 */ const struct TileGfxAnim* tileGfxAnimCurrent;

    /* 34 */ short tileGfxAnimClock;
    /* 36 */ short tilePalAnimClock;

    /* 38 */ const struct TilePalAnim* tilePalAnimStart;
    /* 3C */ const struct TilePalAnim* tilePalAnimCurrent;
};

static void BMapVSync_UpdateMapImgAnimations(struct BMVSyncProc* proc);
static void BMapVSync_UpdateMapPalAnimations(struct BMVSyncProc* proc);
static void BMapVSync_InitMapAnimations(struct BMVSyncProc* proc);
static void BMapVSync_OnEnd(struct BMVSyncProc* proc);
static void BMapVSync_OnLoop(struct BMVSyncProc* proc);

static void WfxNone_Init(void);
static void WfxSnow_Init(void);
static void WfxSnow_VSync(void);
static void WfxRain_Init(void);
static void WfxRain_VSync(void);
static void WfxSandStorm_Init(void);
static void WfxSandStorm_VSync(void);
static void WfxSnowStorm_Init(void);
static void WfxSnowStorm_VSync(void);
static void WfxBlueHSync(void);
static void WfxBlue_Init(void);
static void WfxBlue_VSync(void);
static void WfxFlamesHSync(void);
static void WfxFlamesInitGradient(void);
static void WfxFlamesInitParticles(void);
static void WfxFlames_Init(void);
static void WfxFlamesUpdateGradient(void);
static void WfxFlamesUpdateParticles(void);
static void WfxFlames_VSync(void);
static void WfxCloudsOffsetGraphicsEffect(u32* lines);
static void WfxClouds_Init(void);
static void WfxClouds_VSync(void);
static void WfxClouds_Update(void);
static void WfxInit(void);
static void WfxVSync(void);
static void WfxUpdate(void);

static void ClearBattleMapState(void);
static void InitMoreBMapGraphics(void);

// TODO: figure out if those variables should really belong to EWRAM_DATA
static EWRAM_DATA union WeatherEffectData sWeatherEffect = {};
static EWRAM_DATA union GradientEffectData sGradientEffect = {};

static CONST_DATA struct ProcScr sProc_BMVSync[] = { // gProc_VBlankHandler
    PROC_MARK(PROC_MARK_1),
    PROC_ONEND(BMapVSync_OnEnd),

    PROC_SLEEP(0),

PROC_LABEL(0),
    PROC_CALL(BMapVSync_UpdateMapImgAnimations),
    PROC_CALL(BMapVSync_UpdateMapPalAnimations),

    PROC_CALL(SyncUnitSpriteSheet),
    PROC_CALL(WfxVSync),

    PROC_REPEAT(BMapVSync_OnLoop),

    PROC_END
};

CONST_DATA struct ProcScr gProc_MapTask[] = { // gProc_MapTask
    PROC_NAME("MAPTASK"),
    PROC_END_DUPS,
    PROC_MARK(PROC_MARK_1),

    PROC_SLEEP(0),

PROC_LABEL(0),
    PROC_CALL(PutUnitSpritesOam),
    PROC_CALL(WfxUpdate),
    PROC_CALL(UpdateBmMapDisplay),

    PROC_SLEEP(0),
    PROC_GOTO(0)
};

// TODO: better repr?
static CONST_DATA u16 sObj_RainParticle1[] = {
    1, 0x0000, 0x0000, 0x102A
};

// TODO: better repr?
static CONST_DATA u16 sObj_RainParticle2[] = {
    1, 0x8000, 0x0000, 0x100A
};

static CONST_DATA u16* sRainParticleObjLookup[3] = { // Weather particle Obj Data Lookup
    sObj_RainParticle1, sObj_RainParticle2, sObj_RainParticle2
};

// TODO: better repr?
static CONST_DATA u16 sObj_BackgroundClouds[] = { // Obj Data
    18,

    0x4000, 0xC000, 0,
    0x4000, 0xC030, 6,
    0x4000, 0xC070, 0,
    0x4000, 0xC0A0, 6,
    0x8000, 0x80E0, 0,
    0x0020, 0x8000, 10,
    0x4020, 0xC020, 0,
    0x4020, 0xC050, 6,
    0x4020, 0xC090, 0,
    0x4020, 0xC0C0, 6,
    0x4040, 0xC000, 0,
    0x4040, 0xC0B0, 0,
    0x4060, 0xC000, 4,
    0x4060, 0xC0B0, 4,
    0x4080, 0xC000, 0,
    0x4080, 0xC0B0, 0,
    0x40A0, 0xC000, 0,
    0x40A0, 0xC0B0, 0,
};

static CONST_DATA struct ProcScr sProc_DelayedBMapDispResume[] = { // gProc_GameGfxUnblocker
    PROC_SLEEP(0),

    PROC_CALL(UnlockGameGraphicsLogic),
    PROC_END
};

/**
 * Each 3 array entries represent one config template
 * First two values are initial speed, third is type id
 * Used for the "slower" weathers (regular snow, rain & flames)
 */
static const u16 sInitialParticleConfigTemplates[] = {
    0xB0,  0xC0,  0,
    0xB0,  0xD0,  0,
    0xB0,  0xE0,  0,
    0xB0,  0xF0,  0,
    0xB0,  0x100, 0,
    0xB0,  0x110, 0,

    0xF0,  0x140, 1,
    0xF0,  0x150, 1,
    0xF0,  0x160, 1,
    0xF0,  0x170, 1,
    0xF0,  0x180, 1,
    0xF0,  0x190, 1,
    0xF0,  0x1A0, 1,

    0x100, 0x200, 2,
    0xF0,  0x220, 2,
    0xE0,  0x240, 2,
};

void BMapVSync_UpdateMapImgAnimations(struct BMVSyncProc* proc) {
    if (!proc->tileGfxAnimStart)
        return;

    if (proc->tileGfxAnimClock) {
        proc->tileGfxAnimClock--;
        return;
    }

    proc->tileGfxAnimClock = proc->tileGfxAnimCurrent->time;

    CpuFastCopy(
        proc->tileGfxAnimCurrent->data,
        BG_TILE_ADDR(0x140),
        proc->tileGfxAnimCurrent->size
    );

    if ((++proc->tileGfxAnimCurrent)->time == 0)
        proc->tileGfxAnimCurrent = proc->tileGfxAnimStart;
}

void BMapVSync_UpdateMapPalAnimations(struct BMVSyncProc* proc) {
    if (!proc->tilePalAnimStart)
        return;

    if (proc->tilePalAnimClock) {
        proc->tilePalAnimClock--;
        return;
    }

    proc->tilePalAnimClock = proc->tilePalAnimCurrent->time;

    CpuCopy16(
        proc->tilePalAnimCurrent->data,
        proc->tilePalAnimCurrent->colorStart + (0x10 * BM_BGPAL_6) + gPal,
        proc->tilePalAnimCurrent->colorCount*2
    );

    EnablePalSync();

    if ((++proc->tilePalAnimCurrent)->time == 0)
        proc->tilePalAnimCurrent = proc->tilePalAnimStart;
}

void BMapVSync_InitMapAnimations(struct BMVSyncProc* proc) {
    proc->tileGfxAnimClock = 0;
    proc->tilePalAnimClock = 0;

    proc->tileGfxAnimStart = proc->tileGfxAnimCurrent =
        gChapterDataAssetTable[GetChapterInfo(gPlaySt.chapter)->mapTileAnim1Id];

    proc->tilePalAnimStart = proc->tilePalAnimCurrent =
        gChapterDataAssetTable[GetChapterInfo(gPlaySt.chapter)->mapTileAnim2Id];
}

void BMapVSync_OnEnd(struct BMVSyncProc* proc) {
    SetOnHBlankB(NULL);
}

void BMapVSync_OnLoop(struct BMVSyncProc* proc) {
    Proc_Goto(proc, 0);
}

void StartGameVBlankProc(void) {
    BMapVSync_InitMapAnimations(
        SpawnProc(sProc_BMVSync, PROC_TREE_VSYNC));

    WfxInit();
    gBmSt.gameGfxSemaphore = 0;
}

void EndGameVBlankProc(void) {
    EndEachProc(sProc_BMVSync);
}

void LockGameGraphicsLogic(void) {
    if (++gBmSt.gameGfxSemaphore > 1)
        return; // gfx was already blocked, nothing needs to be done.

    SetOnHBlankB(NULL);
    SetBackdropColor(0);
    LockEachMarkedProc(1);
}

void UnlockGameGraphicsLogic(void) {
    struct Proc* proc;

    if (!gBmSt.gameGfxSemaphore)
        return; // wasn't blocked

    if (--gBmSt.gameGfxSemaphore)
        return; // still blocked

    ReleaseEachMarkedProc(1);

    proc = FindProc(sProc_BMVSync);

    if (proc) {
        // restart vblank proc
        Proc_End(proc);
        StartGameVBlankProc();
    }
}

void AllocWeatherParticles(unsigned weatherId) {
    switch (weatherId) {

    case WEATHER_SNOW:
    case WEATHER_SNOWSTORM:
    case WEATHER_RAIN:
    case WEATHER_SANDSTORM:
        InitOam(0x20);
        break;

    case WEATHER_FLAMES:
        InitOam(0x10);
        break;

    default:
        InitOam(0);
        break;

    } // switch (weatherId)
}

void WfxNone_Init(void) {
    AllocWeatherParticles(gPlaySt.chapterWeatherId);
    SetOnHBlankB(NULL);
}

void WfxSnow_Init(void) {
    int i;

    int gfxTileIndices[] = {
        0x29,
        0x09,
        0x08
    };

    AllocWeatherParticles(gPlaySt.chapterWeatherId);

    for (i = 0; i < 0x40; ++i) {
        unsigned templateIndex = (i & 0xF) * 3;

        sWeatherEffect.particles[i].xPosition = RandNextB();
        sWeatherEffect.particles[i].yPosition = RandNextB();

        sWeatherEffect.particles[i].xSpeed = sInitialParticleConfigTemplates[templateIndex + 0] * 2;
        sWeatherEffect.particles[i].ySpeed = sInitialParticleConfigTemplates[templateIndex + 1] * 2;
        sWeatherEffect.particles[i].typeId = sInitialParticleConfigTemplates[templateIndex + 2];

        sWeatherEffect.particles[i].gfxIndex = gfxTileIndices[sInitialParticleConfigTemplates[templateIndex + 2]];
    }
}

void WfxSnow_VSync(void) {
    if (GetOamSplice()) {
        struct { short x, y; } origins[3];
        int i;

        struct WeatherParticle* it = sWeatherEffect.particles + ((GetGameTime() % 2) * 0x20);

        origins[0].x = (gBmSt.camera.x * 12) / 16;
        origins[0].y = gBmSt.camera.y;

        origins[1].x = gBmSt.camera.x;
        origins[1].y = gBmSt.camera.y;

        origins[2].x = (gBmSt.camera.x * 20) / 16;
        origins[2].y = gBmSt.camera.y;

        for (i = 0; i < 0x20; ++i) {
            it->xPosition += it->xSpeed;
            it->yPosition += it->ySpeed;

            PutOamLo(
                ((it->xPosition >> 8) - origins[it->typeId].x) & 0xFF,
                ((it->yPosition >> 8) - origins[it->typeId].y) & 0xFF,
                Sprite_8x8,
                (BM_OBJPAL_1 << 12) + it->gfxIndex
            );

            ++it;
        }
    }
}

void WfxRain_Init(void) {
    int i;

    AllocWeatherParticles(gPlaySt.chapterWeatherId);

    for (i = 0; i < 0x40; ++i) {
        unsigned templateIndex = (i & 0xF) * 3;

        sWeatherEffect.particles[i].xPosition = RandNextB();
        sWeatherEffect.particles[i].yPosition = RandNextB();

        sWeatherEffect.particles[i].xSpeed   = sInitialParticleConfigTemplates[templateIndex + 0] * 6;
        sWeatherEffect.particles[i].ySpeed   = sInitialParticleConfigTemplates[templateIndex + 1] * 16;
        sWeatherEffect.particles[i].gfxIndex = sInitialParticleConfigTemplates[templateIndex + 2];
    }
}

void WfxRain_VSync(void) {
    if (GetOamSplice()) {
        int i;

        struct WeatherParticle* it = sWeatherEffect.particles + ((GetGameTime() % 2) * 0x20);

        for (i = 0; i < 0x20; ++i) {
            it->xPosition += it->xSpeed;
            it->yPosition += it->ySpeed;

            PutOamLo(
                ((it->xPosition >> 8) - gBmSt.camera.x) & 0xFF,
                ((it->yPosition >> 8) - gBmSt.camera.y) & 0xFF,
                sRainParticleObjLookup[it->gfxIndex],
                0
            );

            ++it;
        }
    }
}

void WfxSandStorm_Init(void) {
    int i;

    AllocWeatherParticles(gPlaySt.chapterWeatherId);

    Decompress(gUnknown_085A3964, gBuf);
    CopyTileGfxForObj(gBuf, OBJ_VRAM0 + 0x1C * 0x20, 4, 4);

    for (i = 0; i < 0x40; ++i) {
        sWeatherEffect.particles[i].xPosition = RandNextB();
        sWeatherEffect.particles[i].yPosition = (RandNextB() % 160 + 240) & 0xFF;

        sWeatherEffect.particles[i].xSpeed = (RandNextB() & 0x7) - 32;
        sWeatherEffect.particles[i].ySpeed = 0;
    }
}

void WfxSandStorm_VSync(void) {
    if (GetOamSplice()) {
        int i;

        struct WeatherParticle* it = sWeatherEffect.particles + ((GetGameTime() % 2) * 0x20);

        for (i = 0; i < 0x20; ++i) {
            it->xPosition += it->xSpeed;

            PutOamLo(
                ((it->xPosition & 0xFF) - 0x10) & 0x1FF,
                it->yPosition,
                Sprite_32x32,
                (BM_OBJPAL_1 << 12) + 0x1C
            );

            ++it;
        }
    }
}

void WfxSnowStorm_Init(void) {
    int i;

    u8 typeLookup[] = { 0, 0, 0, 0, 0, 0, 1, 1 };

    AllocWeatherParticles(gPlaySt.chapterWeatherId);

    Decompress(gUnknown_085A39EC, gBuf);
    CopyTileGfxForObj(gBuf, OBJ_VRAM0 + 0x18 * 0x20, 8, 4);

    for (i = 0; i < 0x40; ++i) {
        unsigned type = typeLookup[i & 7];

        sWeatherEffect.particles[i].xPosition = RandNextB();
        sWeatherEffect.particles[i].yPosition = RandNextB();

        sWeatherEffect.particles[i].ySpeed    = (RandNextB() & 0x3FF) - 0x100;
        sWeatherEffect.particles[i].gfxIndex  = type;

        switch (type) {

        case 0:
            sWeatherEffect.particles[i].xSpeed = 0x700 + (RandNextB() & 0x1FF);
            break;

        case 1:
            sWeatherEffect.particles[i].xSpeed = 0xA00 + (RandNextB() & 0x1FF);
            break;

        } // switch(type)
    }
}

void WfxSnowStorm_VSync(void) {
    if (GetOamSplice()) {
        int i;

        struct WeatherParticle* it = sWeatherEffect.particles + ((GetGameTime() % 2) * 0x20);

        for (i = 0; i < 0x20; ++i) {
            it->xPosition += it->xSpeed;
            it->yPosition += it->ySpeed;

            PutOamLo(
                ((it->xPosition >> 8) - gBmSt.camera.x) & 0xFF,
                ((it->yPosition >> 8) - gBmSt.camera.y) & 0xFF,
                Sprite_32x32,
                (BM_OBJPAL_1 << 12) + 0x18 + (it->gfxIndex * 4)
            );

            ++it;
        }

    }
}

void WfxBlueHSync(void) {
    u16 nextLine = (REG_VCOUNT + 1);

    if (nextLine > 160)
        nextLine = 0;

    nextLine += gBmSt.camera.y / 2;

    if (nextLine >= 320)
        ((u16*)(PLTT))[0] = 0;
    else
        ((u16*)(PLTT))[0] = nextLine[sGradientEffect.lines];
}

void WfxBlue_Init(void) {
    u16* palIt = sGradientEffect.lines;
    int i = 0;

    void(*handler)(void) = WfxBlueHSync;

    for (; i < 320; ++i)
        *palIt++ = MAKE_RGB5(0, 0, (31 - i / 10));

    SetOnHBlankB(handler);
}

void WfxBlue_VSync(void) {}

void WfxFlamesHSync(void) {
    const u16* src;
    u16* dst;

    u16 nextLine = (REG_VCOUNT + 1);

    if (nextLine < 96)
        return;

    if (nextLine >= 160)
        return;

    nextLine -= 96;

    src  = sGradientEffect.fireGradient[0];
    src += nextLine * 8;

    dst = ((u16*)(PLTT)) + 0x70 + (nextLine % 8) * 8;

    CpuFastCopy(src, dst, 8);
}

void WfxFlamesInitGradientPublic(void) {
    int k, j, i;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 0x10; ++j) {
            const int color = gPal[0x10 * (i + BM_BGPAL_TILESET_BASE) + j];

            int r = RGB5_R(color);
            int g = RGB5_G(color);
            int b = RGB5_B(color);

            for (k = 0; k < 8; ++k) {
                r = r + 2;

                if (r > 31)
                    r = 31;

                sGradientEffect.fireGradient[k][0x10 * i + j] = 
                    (b << 10) + (g << 5) + r;
            }
        }
    }
}

void WfxFlamesInitGradient(void) {
    int i, j, k;

    UnpackChapterMapPalette();

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 0x10; ++j) {
            const int color = gPal[0x10 * (i + BM_BGPAL_TILESET_BASE) + j];

            int r = RGB5_R(color);
            int g = RGB5_G(color);
            int b = RGB5_B(color);

            for (k = 0; k < 8; ++k) {
                r = r + 2;

                if (r > 31)
                    r = 31;

                sGradientEffect.fireGradient[k][0x10 * i + j] = 
                    (b << 10) + (g << 5) + r;
            }
        }
    }

    SetOnHBlankB(WfxFlamesHSync);
}

void WfxFlamesInitParticles(void) {
    int i;

    AllocWeatherParticles(gPlaySt.chapterWeatherId);
    Decompress(gUnknown_085A3A84, OBJ_VRAM0 + 0x18 * 0x20);
    ApplyPaletteExt(gUnknown_085A3AC0, 0x340, 0x20);

    for (i = 0; i < 0x10; ++i) {
        sWeatherEffect.particles[i].xPosition = RandNextB();
        sWeatherEffect.particles[i].yPosition = RandNextB();

        sWeatherEffect.particles[i].xSpeed = -sInitialParticleConfigTemplates[i*3 + 0];
        sWeatherEffect.particles[i].ySpeed = -sInitialParticleConfigTemplates[i*3 + 1];
    }
}

void WfxFlames_Init(void) {
    WfxFlamesInitGradient();
    WfxFlamesInitParticles();
}

void WfxFlamesUpdateGradient(void) {
    int i, j;

    CpuFastCopy(
        gPal + BM_BGPAL_TILESET_BASE * 0x10,
        ((u16*)(PLTT)) + BM_BGPAL_TILESET_BASE * 0x10,

        0x20 * 4
    );

    for (i = 12; i < 16; ++i) {
        const int color = gPal[(BM_BGPAL_TILESET_BASE + 2) * 0x10 + i];

        int r = RGB5_R(color);
        int g = RGB5_G(color);
        int b = RGB5_B(color);

        for (j = 0; j < 8; ++j) {
            r = r + 2;

            if (r > 31)
                r = 31;

            sGradientEffect.fireGradient[j][0x10 * 2 + i] = 
                (b << 10) + (g << 5) + r;
        }

    }
}

void WfxFlamesUpdateParticles(void) {
    struct WeatherParticle* it = sWeatherEffect.particles;

    if (GetOamSplice()) {
        int i;

        for (i = 0; i < 0x10; ++i, ++it) {
            int yDisplay;
            int objTile;

            it->xPosition += it->xSpeed;
            it->yPosition += it->ySpeed;

            yDisplay = ((it->yPosition >> 8) - gBmSt.camera.y) & 0xFF;

            if (yDisplay < 0x40)
                continue;

            if (yDisplay > 0xA0)
                continue;

            objTile = 31 - ((yDisplay - 0x40) / 8);

            if (objTile < 24)
                objTile = 24;

            PutOamLo(
                ((it->xPosition >> 8) - gBmSt.camera.x) & 0xFF,
                yDisplay,
                Sprite_8x8,
                (BM_OBJPAL_10 << 12) + objTile
            );
        }
    }
}

void WfxFlames_VSync(void) {
    WfxFlamesUpdateGradient();
    WfxFlamesUpdateParticles();
}

void WfxCloudsOffsetGraphicsEffect(u32* lines) {
    u32 lineBuf[8];
    int iy, ix;

    // What this function is doing is "shifting" a 14
    // tile wide 4bpp image one pixel to the right(?)

    // Remember: lowest nibble of any gfx data is the leftmost pixel

    // Saving the rightmost tile column for later
    for (iy = 0; iy < 8; ++iy)
        lineBuf[iy] = lines[iy + 0x68];

    // Shift all tiles right one pixel
    for (ix = (14 - 1); ix >= 0; --ix) {
        for (iy = 0; iy < 8; ++iy) {
            lines[(8*(ix - 1)) + iy + 8] =
                (lines[(8*(ix - 1)) + iy + 8] << 4) | (lines[(8*(ix - 1)) + iy] >> 28);
        }
    }

    // the leftmost pixel column now contains garbage
    // but that's only, we're fixing it now
    // this is why we needed the rightmost column to be saved
    for (iy = 0; iy < 8; ++iy) {
        lines[iy] &= ~0xF;
        lines[iy] = (lines[iy]) | (lineBuf[iy] >> 28);
    }
}

void WfxClouds_Init(void) {
    AllocWeatherParticles(WEATHER_NONE);

    Decompress(
        gUnknown_085A3B00,
        sWeatherEffect.gfxData
    );

    ApplyPaletteExt(
        gUnknown_085A401C,
        ((0x10 + BM_OBJPAL_10) * 0x10 * sizeof(u16)),
        0x10 * sizeof(u16)
    );
}

void WfxClouds_VSync(void) {
    u32* gfx = sWeatherEffect.gfxData;

    switch (GetGameTime() % 8) {

    case 0:
        WfxCloudsOffsetGraphicsEffect(gfx + 0 * (14 * 8));
        break;

    case 2:
        WfxCloudsOffsetGraphicsEffect(gfx + 1 * (14 * 8));
        break;

    case 4:
        WfxCloudsOffsetGraphicsEffect(gfx + 2 * (14 * 8));
        break;

    case 6:
        WfxCloudsOffsetGraphicsEffect(gfx + 3 * (14 * 8));
        break;

    case 7:
        CopyTileGfxForObj(gfx, OBJ_VRAM0 + (0x20 * 18), 14, 4);
        break;

    } // switch (GetGameTime() % 8)
}

void WfxClouds_Update(void) {
    int y = gBmSt.camera.y;

    PutSprite(
        14,
        0, -(y / 5),
        sObj_BackgroundClouds,
        0xAC12
    );
}

void WfxInit(void) {
    switch (gPlaySt.chapterWeatherId) {

    case WEATHER_NONE:
        WfxNone_Init();
        break;

    case WEATHER_SNOW:
        WfxSnow_Init();
        break;

    case WEATHER_SANDSTORM:
        WfxSandStorm_Init();
        break;

    case WEATHER_SNOWSTORM:
        WfxSnowStorm_Init();
        break;

    case WEATHER_RAIN:
        WfxRain_Init();
        break;

    case WEATHER_3:
        WfxBlue_Init();
        break;

    case WEATHER_FLAMES:
        WfxFlames_Init();
        break;

    case WEATHER_CLOUDS:
        WfxClouds_Init();
        break;

    } // switch (gPlaySt.chapterWeatherId)
}

void WfxVSync(void) {
    switch (gPlaySt.chapterWeatherId) {

    case WEATHER_SNOW:
        WfxSnow_VSync();
        break;

    case WEATHER_SANDSTORM:
        WfxSandStorm_VSync();
        break;

    case WEATHER_SNOWSTORM:
        WfxSnowStorm_VSync();
        break;

    case WEATHER_RAIN:
        WfxRain_VSync();
        break;

    case WEATHER_3:
        WfxBlue_VSync();
        break;

    case WEATHER_FLAMES:
        WfxFlames_VSync();
        break;

    case WEATHER_CLOUDS:
        WfxClouds_VSync();
        break;

    } // switch (gPlaySt.chapterWeatherId)
}

void WfxUpdate(void) {
    if (gPlaySt.chapterWeatherId == WEATHER_CLOUDS)
        WfxClouds_Update();
}

void DisableMapPaletteAnimations(void) {
    struct BMVSyncProc* proc = FindProc(sProc_BMVSync);

    if (proc)
        proc->tilePalAnimStart = NULL;
}

void ResetMapPaletteAnimations(void) {
    struct BMVSyncProc* proc = FindProc(sProc_BMVSync);

    if (proc)
        proc->tilePalAnimStart = proc->tilePalAnimCurrent =
            gChapterDataAssetTable[GetChapterInfo(gPlaySt.chapter)->mapTileAnim2Id];
}

void SetupWeather(unsigned weatherId) {
    gPlaySt.chapterWeatherId = weatherId;

    AllocWeatherParticles(weatherId);
    WfxInit();
}

u8 GetTextSpeed(void) {
    u8 speedLookup[4] = { 8, 4, 1, 0 };
    return speedLookup[gPlaySt.configTextSpeed];
}

int IsFirstPlaythrough(void) {
    if (!sub_80A3870())
        return TRUE;

    if (gPlaySt.chapterStateBits & CHAPTER_FLAG_7)
        return FALSE;

    return gPlaySt.unk41_5;
}

void InitClearChapterState(int isDifficult, s8 unk) {
    CpuFill16(0, &gPlaySt, sizeof(gPlaySt));

    gPlaySt.chapter = 0;

    if (isDifficult)
        gPlaySt.chapterStateBits |= CHAPTER_FLAG_DIFFICULT;

    // TODO: WHAT ARE THOSE
    gPlaySt.unk42_6 = unk;
    gPlaySt.configBattleAnim = 0;
    gPlaySt.unk40_2 = 0;
    gPlaySt.unk40_3 = 0;
    gPlaySt.configNoAutoCursor = 0;
    gPlaySt.configTextSpeed = 1; // TODO: (DEFAULT?) TEXT SPEED DEFINITIONS
    gPlaySt.configWalkSpeed = 0;
    gPlaySt.configBgmDisable = 0;
    gPlaySt.configSeDisable = 0;
    gPlaySt.configWindowColor = 0;
    gPlaySt.unk41_7 = 0;
    gPlaySt.unk41_8 = 0;
    gPlaySt.configBattlePreviewKind = 0;
    gPlaySt.unk42_8 = 0;
    gPlaySt.unk43_2 = 0;
    gPlaySt.unk40_1 = 0;
    gPlaySt.unk41_5 = 0;
}

void ClearBattleMapState(void) {
    int logicLock = gBmSt.gameLogicSemaphore;

    CpuFill16(0, &gBmSt, sizeof(gBmSt));
    gBmSt.gameLogicSemaphore = logicLock;
}

void StartChapter(struct GameCtrlProc* gameCtrl) {
    int i;

    SetupBackgrounds(NULL);

    SetMainFunc(OnGameLoopMain);
    SetOnVBlank(OnVBlank);

    ClearBattleMapState();
    sub_80156D4();
    ApplyUnitSpritePalettes();
    ClearTemporaryFlags();
    ResetUnitSprites();
    ClearMenuCommandOverride();
    ResetTraps();

    gPlaySt.chapterPhaseIndex = FACTION_GREEN; // TODO: PHASE/ALLEGIANCE DEFINITIONS
    gPlaySt.chapterTurnNumber = 0;

    // TODO: BATTLE MAP/CHAPTER/OBJECTIVE TYPE DEFINITION (STORY/TOWER/SKIRMISH)
    if (GetBattleMapKind() == 2) {
        if (!(gPlaySt.chapterStateBits & CHAPTER_FLAG_PREPSCREEN))
            gPlaySt.chapterVisionRange = 3 * (RandNext_100() & 1);
    } else {
        gPlaySt.chapterVisionRange =
            GetChapterInfo(gPlaySt.chapter)->initialFogLevel;
    }

    gPlaySt.chapterWeatherId =
        GetChapterInfo(gPlaySt.chapter)->initialWeather;

    ResetHLayers();
    InitChapterMap(gPlaySt.chapter);
    AddSnagsAndWalls();

    gPlaySt.unk4 = GetGameTime();
    gPlaySt.chapterTotalSupportGain = 0;

    gPlaySt.unk48 = 0;
    gPlaySt.unk4A_1 = 0;
    gPlaySt.unk4B = 0;
    gPlaySt.unk4A_5 = 0;

    for (i = 1; i < 0x40; ++i) {
        struct Unit* unit = GetUnit(i);

        if (unit && unit->pinfo) {
            if (unit->flags & UNIT_FLAG_WAS_UNDEPLOYED)
                unit->flags = unit->flags | UNIT_FLAG_UNDEPLOYED;
            else
                unit->flags = unit->flags &~ UNIT_FLAG_UNDEPLOYED;
        }
    }

    ClearTemporaryUnits();
    LoadChapterBallistae();

    if (gameCtrl)
        StartMapMain(gameCtrl);

    SetBackdropColor(0);

    SetBlendTargetA(TRUE, TRUE, TRUE, TRUE, TRUE);
    SetBlendBackdropA(TRUE);

    SetBlendConfig(3, 0, 0, 0x10);
}

void RestartBattleMap(void) {
    SetupBackgrounds(NULL);

    SetMainFunc(OnGameLoopMain);
    SetOnVBlank(OnVBlank);

    sub_80156D4();
    ApplyUnitSpritePalettes();
    ResetUnitSprites();

    ResetTraps();

    gPlaySt.chapterWeatherId =
        GetChapterInfo(gPlaySt.chapter)->initialWeather;

    ResetHLayers();

    InitChapterMap(gPlaySt.chapter);

    AddSnagsAndWalls();
    LoadChapterBallistae();
    EndGameVBlankProc();
    StartGameVBlankProc();

    SpawnProc(gProc_MapTask, PROC_TREE_4);

    SetBackdropColor(0);

    SetDispEnable(1, 1, 1, 0, 0);
}

/**
 * This is called after loading a suspended game
 * To get the game state back to where it was left off
 */
void GameControl_StartResumedGame(struct GameCtrlProc* gameCtrl) {
    struct BMapMainProc* mapMain;

    if (gPlaySt.chapter == 0x7F) // TODO: CHAPTER_SPECIAL enum?
        sub_80A6C8C();

    SetupBackgrounds(NULL);

    SetMainFunc(OnGameLoopMain);
    SetOnVBlank(OnVBlank);

    ClearBattleMapState();

    SetCursorMapPosition(
        gPlaySt.xCursor,
        gPlaySt.yCursor
    );

    LoadGameCoreGfx();
    ApplyUnitSpritePalettes();
    ResetUnitSprites();

    InitChapterMap(gPlaySt.chapter);

    gBmSt.unk3C = TRUE;

    mapMain = StartMapMain(gameCtrl);

    gBmSt.camera.x = sub_8015A40(16 * gBmSt.playerCursor.x);
    gBmSt.camera.y = sub_8015A6C(16 * gBmSt.playerCursor.y);

    switch (gAction.suspendPointType) {

    case SUSPEND_POINT_DURINGACTION:
        MapMain_ResumeFromAction(mapMain);
        break;

    case SUSPEND_POINT_PLAYERIDLE:
    case SUSPEND_POINT_CPPHASE:
        MapMain_ResumeFromPhaseIdle(mapMain);
        break;

    case SUSPEND_POINT_BSKPHASE:
        MapMain_ResumeFromBerserkPhase(mapMain);
        break;

    case SUSPEND_POINT_DURINGARENA:
        MapMain_ResumeFromArena(mapMain);
        break;

    case SUSPEND_POINT_PHASECHANGE:
        MapMain_ResumeFromPhaseSwitch(mapMain);
        break;

    } // switch (gAction.suspendPointType)

    SetBlendTargetA(TRUE, TRUE, TRUE, TRUE, TRUE);
    SetBlendBackdropA(TRUE);

    SetBlendConfig(3, 0, 0, 0x10);
}

void RefreshBMapDisplay_FromBattle(void) {
    SetMainFunc(OnGameLoopMain);
    SetOnVBlank(OnVBlank);

    LoadGameCoreGfx();
    ApplyUnitSpritePalettes();

    ClearBg0Bg1();
    SetWinEnable(0, 0, 0);
    SetBlendNone();

    SetBlankChr(0);
    TmFill(gBg2Tm, 0);

    EnableBgSync(BG2_SYNC_BIT);
}

void BMapDispResume_FromBattleDelayed(void) {
    LoadObjUiGfx();

    MU_Create(&gBattleUnitA.unit);
    MU_SetDefaultFacing_Auto();

    SpawnProc(sProc_DelayedBMapDispResume, PROC_TREE_3);
}

void InitMoreBMapGraphics(void) {
    UnpackChapterMapGraphics(gPlaySt.chapter);
    AllocWeatherParticles(gPlaySt.chapterWeatherId);
    RenderBmMap();
    RefreshUnitSprites();
    ApplyUnitSpritePalettes();
    ForceSyncUnitSpriteSheet();
    InitSystemTextFont();
}

void ReloadGameCoreGraphics(void) {
    SetupBackgrounds(NULL);

    LoadGameCoreGfx();
    InitMoreBMapGraphics();
}

struct BMapMainProc* StartMapMain(struct GameCtrlProc* gameCtrl) {
    struct BMapMainProc* mapMain = SpawnProc(gProc_BMapMain, PROC_TREE_2);

    mapMain->gameCtrl = gameCtrl;
    gameCtrl->proc_lockCnt++;

    StartGameVBlankProc();
    SpawnProc(gProc_MapTask, PROC_TREE_4);

    return mapMain;
}

void EndMapMain(void) {
    struct BMapMainProc* mapMain;

    EndEachMarkedProc(PROC_MARK_1);

    mapMain = FindProc(gProc_BMapMain);
    mapMain->gameCtrl->proc_lockCnt--;

    Proc_End(mapMain);
}

void ChapterEndUnitCleanup(void) {
    int i, j;

    // Clear phantoms
    for (i = 1; i < 0x40; ++i) {
        struct Unit* unit = GetUnit(i);

        if (unit && unit->pinfo)
            if (UNIT_IS_PHANTOM(unit))
                ClearUnit(unit);
    }

    // Clear all non player units (green & red units)
    for (i = 0x41; i < 0xC0; ++i) {
        struct Unit* unit = GetUnit(i);

        if (unit && unit->pinfo)
            ClearUnit(unit);
    }

    // Reset player unit "temporary" states (HP, status, some flags, etc)
    for (j = 1; j < 0x40; ++j) {
        struct Unit* unit = GetUnit(j);

        if (unit && unit->pinfo) {
            SetUnitHp(unit, GetUnitMaxHp(unit));
            SetUnitStatus(unit, UNIT_STATUS_NONE);

            unit->torchDuration = 0;
            unit->barrierDuration = 0;

            if (unit->flags & UNIT_FLAG_UNDEPLOYED)
                unit->flags = unit->flags | UNIT_FLAG_WAS_UNDEPLOYED;
            else
                unit->flags = unit->flags &~ UNIT_FLAG_WAS_UNDEPLOYED;

            unit->flags &= (
                UNIT_FLAG_DEAD | UNIT_FLAG_GROWTH_BOOST | UNIT_FLAG_SOLOANIM_A | UNIT_FLAG_SOLOANIM_B |
                UNIT_FLAG_AWAY | UNIT_FLAG_20 | UNIT_FLAG_WAS_UNDEPLOYED | UNIT_FLAG_25 | UNIT_FLAG_WAS_AWAY);

            if (UNIT_ATTRIBUTES(unit) & UNIT_ATTR_SUPPLY)
                unit->flags = unit->flags &~ UNIT_FLAG_DEAD;

            unit->flags |= UNIT_FLAG_HIDDEN | UNIT_FLAG_UNDEPLOYED;

            unit->rescue = 0;
            unit->supportBits = 0;
        }
    }

    gPlaySt.chapterStateBits = gPlaySt.chapterStateBits &~ CHAPTER_FLAG_PREPSCREEN;
}

void MapMain_ResumeFromPhaseIdle(struct BMapMainProc* mapMain) {
    RefreshEntityBmMaps();
    RefreshUnitSprites();

    SetDispEnable(0, 0, 0, 0, 0);

    Proc_Goto(mapMain, 4);
}

void MapMain_ResumeFromAction(struct BMapMainProc* mapMain) {
    RefreshEntityBmMaps();
    RefreshUnitSprites();

    SetDispEnable(0, 0, 0, 0, 0);

    Proc_Goto(mapMain, 6);

    gActiveUnit = GetUnit(gAction.subjectIndex);
    gMapUnit[gActiveUnit->y][gActiveUnit->x] = 0;

    HideUnitSprite(GetUnit(gAction.subjectIndex));

    MU_Create(gActiveUnit);
    MU_SetDefaultFacing_Auto();
}

void MapMain_ResumeFromBerserkPhase(struct BMapMainProc* mapMain) {
    RefreshEntityBmMaps();
    RefreshUnitSprites();

    SetDispEnable(0, 0, 0, 0, 0);

    Proc_Goto(mapMain, 7);
}

void MapMain_ResumeFromArena(struct BMapMainProc* mapMain) {
    gActiveUnit = GetUnit(gAction.subjectIndex);

    PrepareArena2(gActiveUnit);

    BattleGenerateArena(gActiveUnit);
    BeginBattleAnimations();

    SetDispEnable(0, 0, 0, 0, 0);

    RefreshEntityBmMaps();

    gMapUnit[gAction.yMove][gAction.xMove] = 0;

    RefreshUnitSprites();

    Proc_Goto(mapMain, 10);

    sub_80B578C();
}

void MapMain_ResumeFromPhaseSwitch(struct BMapMainProc* mapMain) {
    RefreshEntityBmMaps();
    RefreshUnitSprites();

    SetDispEnable(0, 0, 0, 0, 0);

    Proc_Goto(mapMain, 8);
}

void GameCtrl_DeclareCompletedChapter(void) {
    RegisterChapterTimeAndTurnCount(&gPlaySt);

    ComputeChapterRankings();
    SaveEndgameRankingsMaybe();

    gPlaySt.chapterStateBits |= CHAPTER_FLAG_5;
}

void GameCtrl_DeclareCompletedPlaythrough(void) {
    SetNextGameActionId(GAME_ACTION_3);
    RegisterCompletedPlaythrough();
}

char* GetTacticianNameString(void) {
    return gPlaySt.playerName;
}

void SetTacticianName(const char* newName) {
    strcpy(gPlaySt.playerName, newName);
}
