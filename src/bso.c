
#include "bso.h"

#include "hardware.h"
#include "oam.h"

enum
{
    // For use with BaSpriteObject::flags

    BSO_BIT_ENABLED = (1 << 0),
    BSO_BIT_HIDDEN  = (1 << 1),
    BSO_BIT_2       = (1 << 2),
    BSO_BIT_FROZEN  = (1 << 3),
};

enum
{
    BSO_MAX_COUNT = 50,
};

struct BaSpriteData
{
    /* 00 */ u32 header;

    union
    {

    struct
    {
        /* 04 */ u16 pa;
        /* 06 */ u16 pb;
        /* 08 */ u16 pc;
        /* 0A */ u16 pd;
    } affine;

    struct
    {
        /* 04 */ u16 oam2;
        /* 06 */ short x;
        /* 08 */ short y;
    } object;

    } as;
};

static int  AnimInterpret(struct BaSpriteObject* anim);
static void AnimInsert(struct BaSpriteObject* anim);
static void AnimDisplayPrivate(struct BaSpriteObject* anim);
static void Anim_8005334(struct BaSpriteObject* anim, u32 instruction);

#define ANINS_IS_NOT_FORCESPRITE(instruction) ((instruction) & 0x80000000)
#define ANINS_IS_PTRINS(instruction) ((instruction) & 0x40000000)

#define ANINS_FORCESPRITE_GET_ADDRESS(instruction) ((void*) ((instruction) &~ 0xF0000003))
#define ANINS_FORCESPRITE_GET_DELAY(instruction) ((((instruction) >> 26) & 0x1C) + ((instruction) & 3))

#define ANINS_PTRINS_GET_TYPE(instruction) (0x3 & ((instruction) >> 28))
#define ANINS_PTRINS_GET_ADDRESS(instruction) ((void*) ((instruction) &~ 0xF0000000))

#define ANINS_GET_TYPE(instruction) (0x3F & ((instruction) >> 24))

#define ANINS_WAIT_GET_DELAY(instruction) ((instruction) & 0xFFFF)

#define ANINS_MOVE_GET_XOFF(instruction) (((int) ((instruction) << 24)) >> 24)
#define ANINS_MOVE_GET_YOFF(instruction) (((int) ((instruction) << 16)) >> 24)
#define ANINS_MOVE_GET_DELAY(instruction) (((instruction) >> 16) & 0xFF)

#define ANINS_COMMAND_GET_ID(instruction) (0xFF & (instruction))

#define ANINS_FRAME_GET_DELAY(instruction) ((instruction) & 0xFFFF)
#define ANINS_FRAME_GET_UNK(instruction) ((instruction) >> 16) & 0xFF

enum
{
    BSO_INS_KIND_STOP    = 0,
    BSO_INS_KIND_END     = 1,
    BSO_INS_KIND_LOOP    = 2,
    BSO_INS_KIND_MOVE    = 3,
    BSO_INS_KIND_WAIT    = 4,
    BSO_INS_KIND_COMMAND = 5,
    BSO_INS_KIND_FRAME   = 6,
};

typedef void (*AnimCallback_t) (struct BaSpriteObject* anim);

EWRAM_DATA static struct BaSpriteObject sAnimPool[BSO_MAX_COUNT] = {};
EWRAM_DATA static struct BaSpriteObject* sFirstAnim = NULL;

void BsoUpdateAll(void)
{
    struct BaSpriteObject* anim;
    int boolNeedsSort = FALSE;

    if (!sFirstAnim)
        return;

    for (anim = sFirstAnim;; anim = anim->next)
    {
        if (BSO_IS_DISABLED(anim))
            continue;

        if (!(anim->flags & BSO_BIT_FROZEN))
        {
            if (anim->timer == 0 || --anim->timer == 0)
            {
                do
                {
                    if (AnimInterpret(anim) == TRUE)
                        boolNeedsSort = TRUE;
                }
                while (anim->timer == 0);
            }

            if (BSO_IS_DISABLED(anim))
                continue;
        }

        if (!(anim->flags & BSO_BIT_HIDDEN))
            AnimDisplayPrivate(anim);

        if (!anim->next)
            break;
    }

    if (boolNeedsSort == TRUE)
        BsoSort();
}

void BsoInit(void)
{
    struct BaSpriteObject* it;

    for (it = sAnimPool; it < sAnimPool + BSO_MAX_COUNT; ++it)
    {
        it->flags = 0;
        it->prev = NULL;
        it->next = NULL;
    }

    sFirstAnim = NULL;
}

struct BaSpriteObject* BsoCreate2(const void* frameData)
{
    struct BaSpriteObject* anim;

    // Find anim slot for new anim
    for (anim = sAnimPool; (anim < sAnimPool + BSO_MAX_COUNT) && !BSO_IS_DISABLED(anim); ++anim) {}

    // return null if no anim slot was found
    if (anim == sAnimPool + BSO_MAX_COUNT)
        return NULL;

    anim->flags = BSO_BIT_ENABLED;

    anim->scrCur = frameData;
    anim->script = frameData;

    anim->timer = 0;
    anim->oam2 = 0;
    anim->priority = 0;
    anim->flags2 = 0;
    anim->flags3 = 0;
    anim->oam01 = 0;

    anim->cqSize = 0;

    anim->unk_2C = NULL;
    anim->sprDataPool = NULL;
    anim->unk_40 = NULL;
    anim->unk_44 = NULL;

    AnimInsert(anim);

    return anim;
}

struct BaSpriteObject* BsoCreate(const void* frameData, u16 displayPriority)
{
    struct BaSpriteObject* anim;

    // Find anim slot for new anim
    for (anim = sAnimPool; (anim < sAnimPool + BSO_MAX_COUNT) && !BSO_IS_DISABLED(anim); ++anim) {}

    // return null if no anim slot was found
    if (anim == sAnimPool + BSO_MAX_COUNT)
        return NULL;

    anim->flags = BSO_BIT_ENABLED;

    anim->scrCur = frameData;
    anim->script = frameData;

    anim->timer = 0;
    anim->oam2 = 0;
    anim->priority = displayPriority;
    anim->flags2 = 0;
    anim->flags3 = 0;
    anim->oam01 = 0;

    anim->cqSize = 0;

    anim->unk_2C = NULL;
    anim->sprDataPool = NULL;
    anim->unk_40 = NULL;
    anim->unk_44 = NULL;

    AnimInsert(anim);

    return anim;
}

void BsoSort(void)
{
    struct BaSpriteObject* anim;

    for (anim = sAnimPool; anim < sAnimPool + BSO_MAX_COUNT; ++anim)
    {
        if (BSO_IS_DISABLED(anim))
            continue;

        anim->prev = NULL;
        anim->next = NULL;
    }

    sFirstAnim = NULL;

    for (anim = sAnimPool; anim < sAnimPool + BSO_MAX_COUNT; ++anim)
    {
        if (BSO_IS_DISABLED(anim))
            continue;

        AnimInsert(anim);
    }
}

void BsoRemove(struct BaSpriteObject* anim)
{
    if (anim->prev == NULL)
    {
        sFirstAnim = anim->next;
        anim->next->prev = NULL;
    }
    else
    {
        anim->prev->next = anim->next;
        anim->next->prev = anim->prev;
    }

    anim->flags = 0;
    anim->prev = NULL;
    anim->next = NULL;
}

void BsoPutOam(struct BaSpriteObject* anim)
{
    AnimDisplayPrivate(anim);
}

int AnimInterpret(struct BaSpriteObject* anim)
{
    int boolNeedsResort = FALSE;

    u32 instruction = *anim->scrCur++;

    if (ANINS_IS_NOT_FORCESPRITE(instruction))
    {
        if (ANINS_IS_PTRINS(instruction))
        {
            switch (ANINS_PTRINS_GET_TYPE(instruction))
            {

            case 0: // function call
                ((AnimCallback_t) (ANINS_PTRINS_GET_ADDRESS(instruction)))(anim);
                break;

            case 1: // set new frame data
                anim->script   = ANINS_PTRINS_GET_ADDRESS(instruction);
                anim->scrCur = ANINS_PTRINS_GET_ADDRESS(instruction);

                anim->timer = 1;

                break;

            } // switch (ANINS_PTRINS_GET_TYPE(instruction))
        }
        else
        {
            switch (ANINS_GET_TYPE(instruction))
            {

            case BSO_INS_KIND_STOP:
                anim->scrCur--;
                anim->timer = 1;

                anim->flags2 = (anim->flags2 & 0xFFF) | 0x4000;

                break;

            case BSO_INS_KIND_END:
                anim->flags = 0;
                anim->timer = 1;

                boolNeedsResort = TRUE;

                break;

            case BSO_INS_KIND_LOOP:
                anim->scrCur = anim->script;
                anim->timer = 1;

                break;

            case BSO_INS_KIND_WAIT:
                anim->timer = ANINS_WAIT_GET_DELAY(instruction);
                break;

            case BSO_INS_KIND_MOVE:
                anim->x += ANINS_MOVE_GET_XOFF(instruction);
                anim->y += ANINS_MOVE_GET_YOFF(instruction);

                anim->timer = ANINS_MOVE_GET_DELAY(instruction);

                break;

            case BSO_INS_KIND_COMMAND:
                anim->flags2 = (anim->flags2 & 0xFFF) | 0x1000;

                anim->cq[anim->cqSize] = ANINS_COMMAND_GET_ID(instruction);
                anim->cqSize++;

                anim->timer = 1;

                switch (ANINS_COMMAND_GET_ID(instruction))
                {

                case BSO_CMD_WAIT_01:
                case BSO_CMD_WAIT_02:
                case BSO_CMD_WAIT_03:
                case BSO_CMD_WAIT_04:
                case BSO_CMD_WAIT_05:
                case BSO_CMD_WAIT_13:
                case BSO_CMD_WAIT_18:
                case BSO_CMD_WAIT_2D:
                case BSO_CMD_WAIT_39:
                case BSO_CMD_WAIT_52:
                    anim->scrCur--;
                    break;

                } // switch (ANINS_COMMAND_GET_ID(instruction))

                break;

            case BSO_INS_KIND_FRAME:
                anim->timer = ANINS_FRAME_GET_DELAY(instruction);
                anim->unk_13 = ANINS_FRAME_GET_UNK(instruction);

                anim->imgSheet = (const void*) (*anim->scrCur++);

                // ???????
                anim->sprData = (const void*) (*anim->scrCur++);
                anim->sprData += (unsigned) anim->sprDataPool;

                anim->flags2 = (anim->flags2 & 0xFFF) | 0x2000;

                break;

            } // switch (0x3F & (instruction >> 24))
        }
    }
    else
    {
        anim->sprData = ANINS_FORCESPRITE_GET_ADDRESS(instruction);
        anim->timer       = ANINS_FORCESPRITE_GET_DELAY(instruction);
    }

    return boolNeedsResort;
}

void AnimInsert(struct BaSpriteObject* anim)
{
    struct BaSpriteObject* it = sFirstAnim;

    if (!sFirstAnim)
    {
        sFirstAnim = anim;
        return;
    }

    for (;;)
    {
        if (anim->priority > it->priority)
        {
            struct BaSpriteObject* prev = it->prev;

            anim->prev = it->prev;
            anim->next = it;
            it->prev = anim;

            if (!anim->prev)
                sFirstAnim = anim;
            else
                prev->next = anim;

            break;
        }

        if (!it->next)
        {
            anim->next = it->next;
            anim->prev = it;
            it->next = anim;

            return;
        }

        it = it->next;
    }
}

void AnimDisplayPrivate(struct BaSpriteObject* anim)
{
    unsigned baseAffineId = gOamAffinePutId;

    const struct BaSpriteData* oamData = anim->sprData;
    const struct BaSpriteData* it;

    int i;
    int x, y;

    if (!oamData)
        return;

    if ((oamData->header &~ 0xFFFF) == 0xFFFF0000)
    {
        for (i = oamData->header & 0xFFFF; i != 0; gOamAffinePutId++, --i, oamData++)
        {
            gOamAffinePutIt->aff = oamData->as.affine.pa;
            gOamAffinePutIt++;

            gOamAffinePutIt->aff = oamData->as.affine.pb;
            gOamAffinePutIt++;

            gOamAffinePutIt->aff = oamData->as.affine.pc;
            gOamAffinePutIt++;

            gOamAffinePutIt->aff = oamData->as.affine.pd;
            gOamAffinePutIt++;
        }
    }

    for (it = oamData; it->header != 1 && gOamHiPutIt < gOam + 0x200; ++it)
    {
        x = it->as.object.x + anim->x;
        y = it->as.object.y + anim->y;

        if (x > 0x100)
            x = 0x180;
        else if (x < -0x40)
            x = 0x180;

        if (y > 0x0A0)
            x = 0x180;
        else if (y < -0x40)
            x = 0x180;

        x = x & 0x1FF;
        y = y & 0x0FF;

        i = 0;

        if (it->header & 0x100)
            i = (baseAffineId << 25);

        i = i + anim->oam01;

        *(u32*) ((u32*) gOamHiPutIt)++ = (it->header + i) | (x << 16) | (y);
        *(u16*) ((u32*) gOamHiPutIt)++ = (it->as.object.oam2 & 0xF3FF) + anim->oam2;
    }
}

void Anim_8005334(struct BaSpriteObject* anim, u32 instruction)
{
    switch (ANINS_COMMAND_GET_ID(instruction))
    {

    case BSO_CMD_WAIT_01:
    case BSO_CMD_WAIT_02:
    case BSO_CMD_WAIT_03:
    case BSO_CMD_WAIT_04:
    case BSO_CMD_WAIT_05:
    case BSO_CMD_WAIT_13:
    case BSO_CMD_WAIT_18:
    case BSO_CMD_WAIT_2D:
    case BSO_CMD_WAIT_39:
    case BSO_CMD_WAIT_52:
        anim->scrCur--;
        break;

    } // switch (ANINS_COMMAND_GET_ID(instruction))
}
