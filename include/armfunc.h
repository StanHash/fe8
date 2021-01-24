
#pragma once

#include "global.h"

extern u8 const ArmCodeStart[];
extern u8 const ArmCodeEnd[];

void sub_8000234(void);
void ClearOam(void* oam, int count);
int GetChecksum32(void const* data, int length);
void TmFillRect(u16* tm, int width, int height, u16 tile);
void TmCopyRect(u16 const* src, u16* dst, int width, int height);
void TmApplyTsa(u16* tm, u8 const* tsa, u16 tile);
void PutOamHi(int x, int y, u16 const* sprite, int oam2);
void PutOamLo(int x, int y, u16 const* sprite, int oam2);
void DrawGlyph(u16 const* cvt, void* chr, u32 const* glyph, int offset);
// ??? sub_8000620(???);
void DecodeString(char const* src, char* dst);
void MapFloodCoreStep(int connect, int x, int y);
void MapFloodCore(void);

void InitRamFuncs(void);

void DrawGlyphRam(u16 const* cvt, void* chr, u32 const* glyph, int offset);
void DecodeStringRam(char const* src, char* dst);
void PutOamHiRam(int x, int y, u16 const* sprite, int oam2);
void PutOamLoRam(int x, int y, u16 const* sprite, int oam2);
void MapFloodCoreStepRam(int connect, int x, int y);
void MapFloodCoreRam(void);

#define DrawGlyph DrawGlyphRam
#define DecodeString DecodeStringRam
#define PutOamHi PutOamHiRam
#define PutOamLo PutOamLoRam
#define MapFloodCoreStep MapFloodCoreStepRam
#define MapFloodCore MapFloodCoreRam

// TODO: let the linker handle those

void sub_8000234_t(void);
void ClearOam_t(void* oam, int count);
int GetChecksum32_t(void const* data, int length);
void TmFillRect_t(u16* tm, int width, int height, u16 tile);
void TmCopyRect_t(u16 const* src, u16* dst, int width, int height);
void TmApplyTsa_t(u16* tm, u8 const* tsa, u16 tile);

#define sub_8000234 sub_8000234_t
#define ClearOam ClearOam_t
#define GetChecksum32 GetChecksum32_t
#define TmFillRect TmFillRect_t
#define TmCopyRect TmCopyRect_t
#define TmApplyTsa TmApplyTsa_t
