#ifndef GUARD_AP_H
#define GUARD_AP_H


struct APHandle {
    /* 00 */ const u16* pDefinition;      // Pointer to Definition Root
    /* 04 */ const u16* pFrameData;       // Pointer to Frame Data Array
    /* 08 */ const u16* pAnimDataStart;   // Pointer to Current Anim Data (Start, where we go back on loop)
    /* 0C */ const u16* pAnimDataCurrent; // Pointer to Current Anim Data (Cursor, where we are now)
    /* 10 */ const u16* pCurrentObjData;  // Pointer to Current Obj Data
    /* 14 */ const u16* pCurrentRotScale; // Pointer to Current Rot/Scale Data
    /* 18 */ s16 frameTimer;              // Cycle Timer
    /* 1A */ u16 frameInterval;           // Cycle Time Step (0x100 is one frame)
    /* 1C */ u16 subframeTimer;           // Sub frame time offset or something
    /* 1E */ u16 objLayer;                // HiObj layer
    /* 20 */ u8  gfxNeedsUpdate;          // bool defining whether gfx needs update
    /* 21 */ u8  rotScaleIndex;           // Rotation/Scale OAM Index
    /* 22 */ u16 tileBase;                // OAM Extra Data (Tile Index Root & OAM2 Stuff)
    /* 24 */ const void* pGraphics;       // Pointer to graphics (if any)
};


void InitAnims();
struct APHandle* StartAnim(const void* apDefinition, u16 aObjNode);
void Anim_End(struct APHandle* handle);
s8   Anim_Display(struct APHandle* handle, int x, int y);
void PutAnim(struct APHandle*, int, int);
s8   ExecAnim(struct APHandle*);
void PutAnimAffine(struct APHandle*);
void Anim_SetAnimId(struct APHandle* handle, int index);
void Anim_SetInfo(struct APHandle* handle, const u16* definition);
void SyncAnimImg(struct APHandle*);
void SetAnimInfo(struct APHandle*, const u16*);
void AnimRunFirstFrame(struct APHandle*);
void InitAnim(struct APHandle*, const u16*, u16);
struct APHandle* FindAnim(const u16* definition);
// ??? StartAnimProc(???);
// ??? APProc_OnUpdate(???);
// ??? APProc_OnEnd(???);
// ??? SetAnimProcParams(???);
// ??? EndAnimProc(???);
// ??? EndEachAnimProc(???);
// ??? AnimProcExists(???);

#endif // GUARD_AP_H
