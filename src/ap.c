#include "global.h"
#include "ap.h"
#include "sprite.h"
#include "proc.h"
#include "hardware.h"
#include "oam.h"

#define AP_MAX_COUNT 0x14 // 20

struct APProc
{
    PROC_HEADER;

    u8 _pad[0x50-0x29];
    
    struct APHandle* pHandle;

    u32 xPosition;
    u32 yPosition;
};

static void APProc_OnUpdate(struct APProc* proc);
static void APProc_OnEnd(struct APProc* proc);

static struct ProcScr CONST_DATA sProcScr_ApProc[] =
{
    PROC_ONEND(APProc_OnEnd),
    PROC_REPEAT(APProc_OnUpdate),

    PROC_END,
};

static u8 CONST_DATA sOamTileSizeLut[] =
{
    1, 1,
    2, 2,
    4, 4,
    8, 8,

    2, 1,
    4, 1,
    4, 2,
    8, 4,

    1, 2,
    1, 4,
    2, 4,
    4, 8,
};

static struct APHandle sAPArray[AP_MAX_COUNT];

void InitAnims(void) {
    struct APHandle* it;

    const struct APHandle* base = sAPArray;
    const void*  nullDefinition = NULL;

    it = sAPArray + AP_MAX_COUNT - 1;

    do {
        (it--)->pDefinition = nullDefinition;
    } while ((int)it >= (int)base); // casts are needed for match
}

struct APHandle* StartAnim(const void* apDefinition, u16 aObjNode) {
    struct APHandle* result;

    if (!(result = FindAnim(NULL)))
        return NULL;

    InitAnim(result, apDefinition, aObjNode);
    return result;
}

void Anim_End(struct APHandle* handle) {
    if (handle && handle->pDefinition)
        handle->pDefinition = NULL;
}

s8 Anim_Display(struct APHandle* handle, int x, int y) {
    if (!handle || !handle->pDefinition)
        return FALSE;
    
    PutAnim(handle, x, y);
    return ExecAnim(handle);
}

void PutAnim(struct APHandle* handle, int x, int y) {
    int rotScaleMask = 0;

    if (!handle || !handle->pDefinition)
        return;

    if (handle->pCurrentRotScale) {
        PutAnimAffine(handle);
        rotScaleMask = (handle->rotScaleIndex << 9);
    }

    PutSpriteExt(
        handle->objLayer,
        x | rotScaleMask, y,
        handle->pCurrentObjData,
        handle->tileBase
    );

    if (handle->pGraphics && handle->gfxNeedsUpdate)
        SyncAnimImg(handle);
}

s8 ExecAnim(struct APHandle* handle) {
    int tmp; // needed to match

    if (!handle || !handle->pDefinition)
        return FALSE;
    
    if (!handle->frameInterval)
        return TRUE;
    
    // timer going down
    if (handle->frameTimer != 0) {
        if ((handle->frameTimer = handle->frameTimer-1) > 0)
            return TRUE;
        
        handle->frameTimer = 0;
        handle->pAnimDataCurrent += 2;
    }

    // handle animation end
    if (!handle->pAnimDataCurrent[0]) {
        switch (handle->pAnimDataCurrent[1]) {
        case (u16)(-1): // loop back to start
            handle->pAnimDataCurrent = handle->pAnimDataStart;
            return ExecAnim(handle);
        
        case 1: // delete handle
            Anim_End(handle);
        case 0: // end animation
            return FALSE;
        }
    }

    // Increasing the subframe clock
    tmp = (handle->pAnimDataCurrent[0] * handle->frameInterval);
    handle->subframeTimer += tmp;

    // Check if next frame wasn't reached yet
    if (handle->subframeTimer < 0x100) {
        handle->frameTimer = 1;
        return ExecAnim(handle);
    }

    // Setting clock values depending on subframe clock
    handle->frameTimer    = (handle->subframeTimer >> 8);
    handle->subframeTimer = (handle->subframeTimer & 0xFF);

    // Setting new frame
    handle->pCurrentObjData = handle->pFrameData + handle->pFrameData[handle->pAnimDataCurrent[1]]/2;

    // Handling RotScale data (if any)
    if (handle->pCurrentObjData[0] & 0x8000) {
        handle->pCurrentRotScale  = handle->pCurrentObjData;
        handle->pCurrentObjData += (handle->pCurrentObjData[0] & 0x7FFF)*3 + 1;
    } else {
        handle->pCurrentRotScale = NULL;
    }

    // Gfx needs update
    handle->gfxNeedsUpdate = TRUE;
    return TRUE;
}

void PutAnimAffine(struct APHandle* handle) {
    int i, count;
    const u16* it;

    if (!handle || !handle->pDefinition)
        return;
    
    if (!handle->pCurrentRotScale)
        return;
    
    count = handle->pCurrentRotScale[0] & 0x7FFF; // rotscale data count
    it    = handle->pCurrentRotScale + 1;         // rotscale data start

    for (i = 0; i < count; it += 3, i++) {
        SetObjAffine(
            handle->rotScaleIndex + i,  // oam rotscale index

            Div(+COS_Q12(it[0])*16, it[1]), // pa
            Div(-SIN_Q12(it[0])*16, it[2]), // pb
            Div(+SIN_Q12(it[0])*16, it[1]), // pc
            Div(+COS_Q12(it[0])*16, it[2])  // pd
        );
    }
}

void Anim_SetAnimId(struct APHandle* handle, int index) {
    const u16* animDataList;

    if (!handle || !handle->pDefinition)
        return;
    
    // anim data offset array is defined by there
    // its entries are offsets relative to the table itself
    animDataList = handle->pDefinition + handle->pDefinition[1]/2;

    handle->pAnimDataStart   = animDataList + animDataList[index]/2;
    handle->pAnimDataCurrent = handle->pAnimDataStart;

    AnimRunFirstFrame(handle);
}

void Anim_SetInfo(struct APHandle* handle, const u16* definition) {
    if (!handle || !handle->pDefinition)
        return;
    
    SetAnimInfo(handle, definition);
    AnimRunFirstFrame(handle);
}

void SyncAnimImg(struct APHandle* handle) {
    const u16* itGfxData;
    const u16* itObjData;
    u32 tileOffset;
    int i;

    if (!handle || !handle->pDefinition)
        return;
    
    i = handle->pCurrentObjData[0];

    itObjData = handle->pCurrentObjData + 1;
    itGfxData = handle->pCurrentObjData + 1 + i*3;
    
    tileOffset = 0;

    // This expression is used a lot here, so using helper macro for readability
    #define OBJ_SIZE_TABLE_INDEX(aIt) ((((aIt[0] & 0xC000)>>12)+((aIt[1] & 0xC000)>>14))*2)

    while ((i--) > 0) {
        RegisterObjectTileGraphics(
            handle->pGraphics + (*itGfxData & 0x3FF) * 0x20,              // source location
            OBJ_VRAM0 + ((handle->tileBase & 0x3FF) * 0x20) + tileOffset, // target location
            sOamTileSizeLut[OBJ_SIZE_TABLE_INDEX(itObjData)+0],        // x size (tiles)
            sOamTileSizeLut[OBJ_SIZE_TABLE_INDEX(itObjData)+1]         // y size (tiles)
        );

        if (!gDispIo.dispCt.obj1dMap)
            // Adding (width * sizeof(Tile4bpp))
            tileOffset += (sOamTileSizeLut[OBJ_SIZE_TABLE_INDEX(itObjData)]) * 0x20;
        else
            // Using the square of the width here?
            // Maybe it's bugged, since I don't think the obj1dMap flag is ever set
            tileOffset += ((
                (sOamTileSizeLut[OBJ_SIZE_TABLE_INDEX(itObjData)]) *
                (sOamTileSizeLut[OBJ_SIZE_TABLE_INDEX(itObjData)])
            )& 0x3FF) * 0x20;

        itObjData += 3;
        itGfxData += 1;
    }

    #undef OBJ_SIZE_TABLE_INDEX

    handle->gfxNeedsUpdate = FALSE;
}

void SetAnimInfo(struct APHandle* handle, const u16* definition) {
    handle->pDefinition = definition;

    // frame data starts at offset in short 0
    handle->pFrameData = definition + (definition[0]/2);
    
    // anim offset array starts at offset in short 1
    // first entry is default anim index
    // anim data is located by indexing the anim offset array.
    // offset is relative to the anim offset array itself, not the definition!

    handle->pAnimDataStart = (
        (definition + definition[1]/2) +  // anim offset array
        (definition[definition[1]/2]/2)   // default anim offset array index
    );

    handle->pAnimDataCurrent = handle->pAnimDataStart;
}

void AnimRunFirstFrame(struct APHandle* handle) {
    int tmp;

    // needed to match (and apparently even a thing in the source cf FE6:08013050)
    handle->frameTimer = 0;

    tmp = handle->frameInterval;

    // This sequence may have been a macro in the original source
    handle->frameTimer    = 0;
    handle->frameInterval = 0x100;

    ExecAnim(handle);

    handle->frameInterval = tmp;
}

void InitAnim(struct APHandle* handle, const u16* definition, u16 objLayer) {
    SetAnimInfo(handle, definition);

    handle->pGraphics  = NULL;
    handle->tileBase   = 0;
    handle->frameTimer = 0;

    handle->objLayer = objLayer;

    handle->pCurrentRotScale = NULL;
    handle->rotScaleIndex    = 0;

    // this is the second time this exact pattern shows. (Macro?)
    handle->frameTimer    = 0;
    handle->frameInterval = 0x100;

    handle->subframeTimer = 0;

    ExecAnim(handle);
}

struct APHandle* FindAnim(const u16* definition) {
    int i = 0;
    struct APHandle* result = sAPArray;

    for (i = 0; i < AP_MAX_COUNT; ++i) {
        if (result->pDefinition == definition)
            return result;
        
        result++;
    }

    return NULL;
}

struct APProc* StartAnimProc(const void* apDefinition, int x, int y, int tileBase, int anim, u16 aObjNode) {
    struct APHandle* handle;
    struct APProc* proc;

    // Setting up handle
    handle = StartAnim(apDefinition, aObjNode);
    Anim_SetAnimId(handle, anim);
    handle->tileBase = tileBase;

    // Making Proc
    proc = SpawnProc(sProcScr_ApProc, PROC_TREE_3);

    // Setting up proc
    proc->pHandle = handle;
    proc->xPosition = x;
    proc->yPosition = y;

    return proc;
}

void APProc_OnUpdate(struct APProc* proc) {
    // Update AP, and end proc if the AP was freed (aka the animation ended)
    if (!Anim_Display(proc->pHandle, proc->xPosition, proc->yPosition))
        if (!proc->pHandle || !proc->pHandle->pDefinition)
            Proc_End(proc);
}

void APProc_OnEnd(struct APProc* proc) {
    // Free AP when proc ends
    Anim_End(proc->pHandle);
}

void SetAnimProcParams(struct APProc* proc, int x, int y, int tileBase) {
    // Set position
    proc->xPosition = x;
    proc->yPosition = y;

    // Set tile base if asked to
    if (tileBase != -1)
        proc->pHandle->tileBase = tileBase;
}

void EndAnimProc(struct APProc* proc) {
    // delet
    Proc_End(proc);
}

void EndEachAnimProc(void) {
    // delet all
    EndEachProc(sProcScr_ApProc);
}

int AnimProcExists(void) {
    return FindProc(sProcScr_ApProc) ? TRUE : FALSE;
}
