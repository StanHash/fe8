
#pragma once

#include "global.h"

struct BaSpriteObject
{
    /* 00 */ u16 flags;
    /* 02 */ short x;
    /* 04 */ short y;
    /* 06 */ short timer;
    /* 08 */ u16 oam2;
    /* 0A */ u16 priority;
    /* 0C */ u16 flags2;
    /* 0E */ u16 nextRoundId;
    /* 10 */ u16 flags3;
    /* 12 */ u8 currentRoundType;
    /* 13 */ u8 unk_13;

    /* 14 */ u8 cqSize;
    /* 15 */ u8 cq[7];

    /* 1C */ u32 oam01;

    /* 20 */ u32 const* scrCur;
    /* 24 */ u32 const* script;
    /* 28 */ void const* imgSheet;
    /* 2C */ void const* unk_2C;
    /* 30 */ void const* sprDataPool;

    /* 34 */ struct BaSpriteObject* prev;
    /* 38 */ struct BaSpriteObject* next;

    /* 3C */ const void* sprData;
    /* 40 */ const void* unk_40;
    /* 44 */ const void* unk_44;
};

enum
{
    // Animation Command Identifiers

    // TODO: complete during battle animation decomp

    BSO_CMD_NOP     = 0x00,
    BSO_CMD_WAIT_01 = 0x01, // wait for hp depletion
    BSO_CMD_WAIT_02 = 0x02, // wait for dodge start, then dodge
    BSO_CMD_WAIT_03 = 0x03, // wait for attack start
    BSO_CMD_WAIT_04 = 0x04,
    BSO_CMD_WAIT_05 = 0x05, // wait for spell animation?
    // TODO: more
    BSO_CMD_WAIT_13 = 0x13, // unk
    BSO_CMD_WAIT_18 = 0x18, // wait for dodge start, then forward dodge
    BSO_CMD_WAIT_2D = 0x2D, // unk
    BSO_CMD_WAIT_39 = 0x39, // unk
    BSO_CMD_WAIT_52 = 0x52, // unk
};

// TODO: add macro helpers for writing animation scripts.

#define BSO_IS_DISABLED(anim) ((anim)->flags == 0)

void BsoUpdateAll(void);
void BsoInit(void);
struct BaSpriteObject* BsoCreate2(void const* script);
struct BaSpriteObject* BsoCreate(void const* script, u16 displayPriority);
void BsoSort(void);
void BsoRemove(struct BaSpriteObject* anim);
void BsoPutOam(struct BaSpriteObject* anim);
