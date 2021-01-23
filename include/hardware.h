
#pragma once

struct KeySt
{
    u8 repeatDelay;     // initial delay before generating auto-repeat presses
    u8 repeatInterval;  // time between auto-repeat presses
    u8 repeatTimer;     // (decreased by one each frame, reset to repeatDelay when Presses change and repeatInterval when reaches 0)
    u16 held;       // keys that are currently held down
    u16 repeated;   // auto-repeated keys
    u16 pressed;        // keys that went down this frame
    u16 prev;       // keys that were held down last frame
    u16 last;
    u16 ABLRPressed; // 1 for Release (A B L R Only), 0 Otherwise
    u16 pressed2;
    u16 timeSinceStartSelect; // Time since last Non-Start Non-Select Button was pressed
};

struct BgOff { u16 x, y; };

struct DispIo
{
    /* 00 */ struct DispCnt dispCt;
    /* 04 */ struct DispStat dispStat;
    /* 08 */ u8 pad_08[4];
    /* 0C */ struct BgCnt bg0Ct;
    /* 10 */ struct BgCnt bg1Ct;
    /* 14 */ struct BgCnt bg2Ct;
    /* 18 */ struct BgCnt bg3Ct;
    /* 1C */ struct BgOff bgOff[4];
    /* 2C */ u8 win0_right, win0_left, win1_right, win1_left;
    /* 30 */ u8 win0_bottom, win0_top, win1_bottom, win1_top;
    /* 34 */ struct WinCnt winCt;
    /* 38 */ u16 mosaic;
    /* 3A */ u8 pad_3A[2];
    /* 3C */ struct BlendCnt blendCt;
    /* 40 */ u8 pad_40[4];
    /* 44 */ u8 blendCoeffA;
    /* 45 */ u8 blendCoeffB;
    /* 46 */ u8 blendY;
    /* 48 */ u16 bg2pa;
    /* 4A */ u16 bg2pb;
    /* 4C */ u16 bg2pc;
    /* 4E */ u16 bg2pd;
    /* 50 */ u32 bg2x;
    /* 54 */ u32 bg2y;
    /* 58 */ u16 bg3pa;
    /* 5A */ u16 bg3pb;
    /* 5C */ u16 bg3pc;
    /* 5E */ u16 bg3pd;
    /* 60 */ u32 bg3x;
    /* 64 */ u32 bg3y;
    /* 68 */ s8 colorAddition;
};

void EnableSoftReset(void);
void DisableSoftReset(void);
bool IsSoftResetEnabled(void);
unsigned GetGameTime(void);
void SetGameTime(unsigned time);
void IncGameTime(void);
bool FormatTime(unsigned time, u16* hours, u16* minutes, u16* seconds);
void EnablePalSync(void);
void DisablePalSync(void);
void EnableBgSync(int bits);
void EnableBgSyncById(int bg);
void DisableBgSync(int bits);
void ApplyPaletteExt(void const* data, int offset, int length);
void sub_8000E14(void const* data, int offset, int length, int q6_brightness);
void SyncDispIo(void);
int GetBgChrOffset(int bg);
int GetBgChrId(int bg, int offset);
int GetBgTilemapOffset(int bg);
void SetBgChrOffset(int bg, int offset);
void SetBgTilemapOffset(int bg, int offset);
void SetBgScreenSize(int bg, int size);
void SetBgBpp(int bg, int bpp);
void SyncBgsAndPal(void);
void TmFill(u16* tm, int tile);
void SetBlankChr(int chr);
void SetOnVBlank(InterruptHandler handler);
void SetOnVMatch(InterruptHandler handler);
void SetNextVCount(int vcount);
void SetVCount(int vcount);
void SetMainFunc(void(*)(void));
void RunMainFunc();
void RefreshKeyStFromKeys(struct KeySt* keySt, short keys);
void RefreshKeySt(struct KeySt* keySt);
void ClearKeySt(struct KeySt* keySt);
void InitKeySt(struct KeySt* keySt);
void SetKeyIgnore(int keys);
int GetKeyIgnore(void);
void StartKeySimulation(int keys);
void SetBgOffset(u16 bg, u16 xOffset, u16 yOffset);
void sub_80014E8(void);
void sub_800151C(u8 a, u8 b);
void sub_8001530(u16* a, u16 const* b);
void sub_800154C(void* outTm, void const* inData, u8 base, u8 linebits);
// void sub_800159C(???);
void sub_80016C4(u16* outTm, short const* inData, int unused);
void sub_8001710(void);
void sub_800172C(u16 const* inPal, int bank, int count, int unk);
void sub_80017B4(int a, int b, int c, int d);
void sub_800183C(int a, int b, int c);
void sub_8001860(u8 a);
void sub_80018E4(u8 a);
void sub_8001964(u8 a);
void sub_80019E8(u8 a);
void sub_8001A6C(void);
void SetupBackgrounds(u16 const* config);
u16* GetBgTilemap(int bg);
void SetHealthSafetySkipEnable(bool val);
bool IsHealthSafetySkipEnabled(void);
void SoftResetIfKeyCombo(void);
void sub_8001CB0(int unk);
void SetOnHBlankA(InterruptHandler func);
void SetOnHBlankB(InterruptHandler func);
int GetBgFromTm(u16* tm);
void SetBgPriority(int bg, int priority);
int GetBgPriority(int bg);
void SetBlendConfig(u16 eff, u8 ca, u8 cb, u8 cy);
void SetBlendTargetA(int bg0, int bg1, int bg2, int bg3, int obj);
void SetBlendTargetB(int bg0, int bg1, int bg2, int bg3, int obj);
void SetBlendBackdropA(int enable);
void SetBlendBackdropB(int enable);
void SetBlendNone(void);

extern u8 EWRAM_DATA gBuf[0x2000];

extern u8 EWRAM_DATA gUnknown_02022188[0x100];
extern s8 EWRAM_DATA gUnknown_02022288[0x20];
extern s8 EWRAM_DATA gUnknown_020222A8[0x600];

extern u16 EWRAM_DATA gPal[0x200];

extern u16 EWRAM_DATA gBg0Tm[0x400];
extern u16 EWRAM_DATA gBg1Tm[0x400];
extern u16 EWRAM_DATA gBg2Tm[0x400];
extern u16 EWRAM_DATA gBg3Tm[0x400];

extern struct KeySt* CONST_DATA gKeySt;

extern short gSinLut[]; // needs to be non-const to match, despite being located in rodata

extern struct DispIo gDispIo;

#define BG_SYNC_BIT(bg) (1 << (bg))

enum
{
    BG0_SYNC_BIT = BG_SYNC_BIT(0),
    BG1_SYNC_BIT = BG_SYNC_BIT(1),
    BG2_SYNC_BIT = BG_SYNC_BIT(2),
    BG3_SYNC_BIT = BG_SYNC_BIT(3),
};

#define TM_OFFSET(x, y) (0x20 * (y) + (x))

#define TILE(chr, pal) ((chr) + ((pal) << 12))
#define TILE_HFLIP 0x0400
#define TILE_VFLIP 0x0800

#define OAM2_PRIORITY(aValue) (((aValue) & 0x3) << 10)

#define RGB5_R(color) ((color) & 0x1F)
#define RGB5_G(color) (((color) >> 5) & 0x1F)
#define RGB5_B(color) (((color) >> 10) & 0x1F)

#define MAKE_RGB5(r, g, b) (((b) << 10) + ((g) << 5) + (r))

#define SIN_Q12(angle) (gSinLut[(angle) & 0xFF])
#define COS_Q12(angle) (gSinLut[0x40 + ((angle) & 0xFF)])

#define ApplyPalettes(data, palnum, palcount) \
    ApplyPaletteExt((data), 0x20 * (palnum), 0x20 * (palcount))

#define ApplyPalette(data, palnum) \
    ApplyPalettes((data), (palnum), 1)

#define SetDispEnable(bg0, bg1, bg2, bg3, obj) \
    gDispIo.dispCt.bg0_on = (bg0); \
    gDispIo.dispCt.bg1_on = (bg1); \
    gDispIo.dispCt.bg2_on = (bg2); \
    gDispIo.dispCt.bg3_on = (bg3); \
    gDispIo.dispCt.obj_on = (obj)

#define SetWinEnable(win0, win1, objwin) \
    gDispIo.dispCt.win0_on = (win0); \
    gDispIo.dispCt.win1_on = (win1); \
    gDispIo.dispCt.objWin_on = (objwin)

#define SetWin0Box(left, top, right, bottom) \
    gDispIo.win0_left = (left); \
    gDispIo.win0_top = (top); \
    gDispIo.win0_right = (right); \
    gDispIo.win0_bottom = (bottom)

#define SetWin1Box(left, top, right, bottom) \
    gDispIo.win1_left = (left); \
    gDispIo.win1_top = (top); \
    gDispIo.win1_right = (right); \
    gDispIo.win1_bottom = (bottom)

#define SetWin0Layers(bg0, bg1, bg2, bg3, obj) \
    gDispIo.winCt.win0_enableBg0 = (bg0); \
    gDispIo.winCt.win0_enableBg1 = (bg1); \
    gDispIo.winCt.win0_enableBg2 = (bg2); \
    gDispIo.winCt.win0_enableBg3 = (bg3); \
    gDispIo.winCt.win0_enableObj = (obj)

#define SetWin1Layers(bg0, bg1, bg2, bg3, obj) \
    gDispIo.winCt.win1_enableBg0 = (bg0); \
    gDispIo.winCt.win1_enableBg1 = (bg1); \
    gDispIo.winCt.win1_enableBg2 = (bg2); \
    gDispIo.winCt.win1_enableBg3 = (bg3); \
    gDispIo.winCt.win1_enableObj = (obj)

#define SetWObjLayers(bg0, bg1, bg2, bg3, obj) \
    gDispIo.winCt.wobj_enableBg0 = (bg0); \
    gDispIo.winCt.wobj_enableBg1 = (bg1); \
    gDispIo.winCt.wobj_enableBg2 = (bg2); \
    gDispIo.winCt.wobj_enableBg3 = (bg3); \
    gDispIo.winCt.wobj_enableObj = (obj)

#define SetWOutLayers(bg0, bg1, bg2, bg3, obj) \
    gDispIo.winCt.wout_enableBg0 = (bg0); \
    gDispIo.winCt.wout_enableBg1 = (bg1); \
    gDispIo.winCt.wout_enableBg2 = (bg2); \
    gDispIo.winCt.wout_enableBg3 = (bg3); \
    gDispIo.winCt.wout_enableObj = (obj)

#define SetBlendAlpha(ca, cb) \
    SetBlendConfig(1, (ca), (cb), 0)

#define SetBlendBrighten(cy) \
    SetBlendConfig(2, 0, 0, (cy))

#define SetBlendDarken(cy) \
    SetBlendConfig(3, 0, 0, (cy))
