
#include "sprite.h"

#include "armfunc.h"
#include "proc.h"
#include "oam.h"

struct SpriteEntry
{
    /* 00 */ struct SpriteEntry* next;
    /* 04 */ s16 oam1;
    /* 06 */ s16 oam0;
    /* 08 */ u16 oam2;
    /* 0A */ // pad
    /* 0C */ const u16* sprite;
};

static void SpriteRefresher_OnIdle(struct SpriteProc* proc);

u16 CONST_DATA Sprite_8x8[] =
{
    1, OAM0_SHAPE_8x8, OAM1_SIZE_8x8, 0,
};

u16 CONST_DATA Sprite_16x16[] =
{
    1, OAM0_SHAPE_16x16, OAM1_SIZE_16x16, 0,
};

u16 CONST_DATA Sprite_32x32[] =
{
    1, OAM0_SHAPE_32x32, OAM1_SIZE_32x32, 0,
};

u16 CONST_DATA Sprite_64x64[] =
{
    1, OAM0_SHAPE_64x64, OAM1_SIZE_64x64, 0,
};

u16 CONST_DATA Sprite_8x16[] =
{
    1, OAM0_SHAPE_8x16, OAM1_SIZE_8x16, 0,
};

u16 CONST_DATA Sprite_16x32[] =
{
    1, OAM0_SHAPE_16x32, OAM1_SIZE_16x32, 0,
};

u16 CONST_DATA Sprite_32x64[] =
{
    1, OAM0_SHAPE_32x64, OAM1_SIZE_32x64, 0,
};

u16 CONST_DATA Sprite_16x8[] =
{
    1, OAM0_SHAPE_16x8, OAM1_SIZE_16x8, 0,
};

u16 CONST_DATA Sprite_16x8_VFlipped[] =
{
    1, OAM0_SHAPE_16x8, OAM1_SIZE_16x8 + OAM1_VFLIP, 0,
};

u16 CONST_DATA Sprite_32x16[] =
{
    1, OAM0_SHAPE_32x16, OAM1_SIZE_32x16, 0,
};

u16 CONST_DATA Sprite_64x32[] =
{
    1, OAM0_SHAPE_64x32, OAM1_SIZE_64x32, 0,
};

u16 CONST_DATA Sprite_32x8[] =
{
    1, OAM0_SHAPE_32x8, OAM1_SIZE_32x8, 0,
};

u16 CONST_DATA Sprite_8x32[] =
{
    1, OAM0_SHAPE_8x32, OAM1_SIZE_8x32, 0,
};

u16 CONST_DATA Sprite_32x8_VFlipped[] =
{
    1, OAM0_SHAPE_32x8, OAM1_SIZE_32x8 + OAM1_VFLIP, 0,
};

u16 CONST_DATA Sprite_8x16_HFlipped[] =
{
    1, OAM0_SHAPE_8x16, OAM1_SIZE_8x16 + OAM1_HFLIP, 0,
};

u16 CONST_DATA Sprite_8x8_HFlipped[] =
{
    1, OAM0_SHAPE_8x8, OAM1_SIZE_8x8 + OAM1_HFLIP, 0,
};

u16 CONST_DATA Sprite_8x8_VFlipped[] =
{
    1, OAM0_SHAPE_8x8, OAM1_SIZE_8x8 + OAM1_VFLIP, 0,
};

u16 CONST_DATA Sprite_8x8_HFlipped_VFlipped[] =
{
    1, OAM0_SHAPE_8x8, OAM1_SIZE_8x8 + OAM1_HFLIP + OAM1_VFLIP, 0,
};

u16 CONST_DATA Sprite_16x16_VFlipped[] =
{
    1, OAM0_SHAPE_16x16, OAM1_SIZE_16x16 + OAM1_VFLIP, 0,
};

static struct ProcScr CONST_DATA sProcSrc_SpriteRefresher[] =
{
    PROC_REPEAT(SpriteRefresher_OnIdle),
    PROC_END,
};

extern struct SpriteEntry* gSpriteAllocIt;

static struct SpriteEntry EWRAM_DATA sSpritePool[0x80] = {};
static struct SpriteEntry EWRAM_DATA sSpriteLayers[0x10] = {};

void PutSpriteAffine(int id, int pa, int pb, int pc, int pd)
{
    gOam[id*0x10 + 0x03] = pa;
    gOam[id*0x10 + 0x07] = pb;
    gOam[id*0x10 + 0x0B] = pc;
    gOam[id*0x10 + 0x0F] = pd;
}

void ClearSprites(void)
{
    int i;

    for (i = 15; i >= 0; i--)
    {
        sSpriteLayers[i].next = &sSpriteLayers[i + 1];
        sSpriteLayers[i].sprite = NULL;
    }

    sSpriteLayers[15].next = NULL;
    sSpriteLayers[12].next = NULL;

    gSpriteAllocIt = sSpritePool;
}

void PutSprite(int layer, int x, int y, u16 const* sprite, int oam2)
{
    gSpriteAllocIt->next = sSpriteLayers[layer].next;
    gSpriteAllocIt->oam1 = x & 0x1FF;
    gSpriteAllocIt->oam0 = y & 0xFF;
    gSpriteAllocIt->oam2 = oam2;
    gSpriteAllocIt->sprite = sprite;

    sSpriteLayers[layer].next = gSpriteAllocIt++;
}

void PutSpriteExt(int layer, int xOam1, int yOam0, u16 const* sprite, int oam2)
{
    gSpriteAllocIt->next = sSpriteLayers[layer].next;
    gSpriteAllocIt->oam1 = xOam1;
    gSpriteAllocIt->oam0 = yOam0;
    gSpriteAllocIt->oam2 = oam2;
    gSpriteAllocIt->sprite = sprite;

    sSpriteLayers[layer].next = gSpriteAllocIt++;
}

void PutSpriteLayerOam(int layer)
{
    struct SpriteEntry* it = sSpriteLayers + layer;

    while (it)
    {
        if (it->sprite)
            PutOamHi(it->oam1, it->oam0, it->sprite, it->oam2);

        it = it->next;
    }
}

void SpriteRefresher_OnIdle(struct SpriteProc* proc)
{
    PutSprite(proc->layer, proc->x, proc->y, proc->sprite, proc->tile);
}

struct SpriteProc* StartSpriteRefresher(ProcPtr parent, int layer, int x, int y, u16 const* sprite, int tile)
{
    struct SpriteProc* proc;

    if (parent)
        proc = SpawnProc(sProcSrc_SpriteRefresher, parent);
    else
        proc = SpawnProc(sProcSrc_SpriteRefresher, PROC_TREE_3);

    proc->x = x;
    proc->y = y;
    proc->layer = layer;
    proc->sprite = sprite;
    proc->tile = tile;

    return proc;
}

void MoveSpriteRefresher(struct SpriteProc* proc, int x, int y)
{
    if (proc == NULL)
        proc = FindProc(sProcSrc_SpriteRefresher);

    proc->x = x;
    proc->y = y;
}
