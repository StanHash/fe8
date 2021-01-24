
#include "armfunc.h"

#undef DrawGlyph
#undef DecodeString
#undef PutOamHi
#undef PutOamLo
#undef MapFloodCoreStep
#undef MapFloodCore

extern u8 RamFuncArea[];

extern void(*DrawGlyphRamFunc)(u16 const* cvtLut, void* chr, u32 const* glyph, int offset);
extern void(*DecodeStringRamFunc)(char const* src, char* dst);
extern void(*PutOamHiRamFunc)(int x, int y, u16 const* sprite, int oam2);
extern void(*PutOamLoRamFunc)(int x, int y, u16 const* sprite, int oam2);
extern void(*MapFloodCoreStepRamFunc)(int connect, int x, int y);
extern void(*MapFloodCoreRamFunc)(void);

void InitRamFuncs(void)
{
    int size = ArmCodeEnd - ArmCodeStart;

    CpuCopy16(ArmCodeStart, RamFuncArea, size);

    DrawGlyphRamFunc        = (void*) RamFuncArea + (((u8*) DrawGlyph)        - ArmCodeStart);
    DecodeStringRamFunc     = (void*) RamFuncArea + (((u8*) DecodeString)     - ArmCodeStart);
    PutOamHiRamFunc         = (void*) RamFuncArea + (((u8*) PutOamHi)         - ArmCodeStart);
    PutOamLoRamFunc         = (void*) RamFuncArea + (((u8*) PutOamLo)         - ArmCodeStart);
    MapFloodCoreStepRamFunc = (void*) RamFuncArea + (((u8*) MapFloodCoreStep) - ArmCodeStart);
    MapFloodCoreRamFunc     = (void*) RamFuncArea + (((u8*) MapFloodCore)     - ArmCodeStart);
}

void DrawGlyphRam(u16 const* cvt, void* chr, u32 const* glyph, int offset)
{
    DrawGlyphRamFunc(cvt, chr, glyph, offset);
}

void DecodeStringRam(char const* src, char* dst)
{
    DecodeStringRamFunc(src, dst);
}

void PutOamHiRam(int x, int y, u16 const* sprite, int oam2)
{
    PutOamHiRamFunc(x, y, sprite, oam2);
}

void PutOamLoRam(int x, int y, u16 const* sprite, int oam2)
{
    PutOamLoRamFunc(x, y, sprite, oam2);
}

void MapFloodCoreStepRam(int connect, int x, int y)
{
    MapFloodCoreStepRamFunc(connect, x, y);
}

void MapFloodCoreRam(void)
{
    MapFloodCoreRamFunc();
}
