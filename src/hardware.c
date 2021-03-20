
#include "global.h"

#include "hardware.h"
#include "dma.h"
#include "oam.h"
#include "proc.h"

enum
{
    FRAMES_PER_SECOND = 60,
    FRAMES_PER_MINUTE = 60 * FRAMES_PER_SECOND,
    FRAMES_PER_HOUR   = 60 * FRAMES_PER_MINUTE,
};

struct KeyProc
{
    /* 00 */ PROC_HEADER;

    /* 2A */ s16 pad_2A[29];
    /* 64 */ s16 unk64;
};

typedef void(*MainFunc)(void);

static s8 sSoftResetEnabled;

static u8 sBgSyncBits;
static bool sPalSyncFlag;

static u16 sKeyIgnore;

static u32 sGameTime;

static u8 sUnknown_03000018;
static u8 sUnknown_03000019;

static bool sHealthSafetySkipEnabled;

u8 EWRAM_DATA gBuf[0x2000] = {};

u8 EWRAM_DATA gUnknown_02022188[0x100] = {};

s8 EWRAM_DATA gUnknown_02022288[0x20] = {};
s8 EWRAM_DATA gUnknown_020222A8[0x600] = {};

u16 EWRAM_DATA gPal[0x200] = {};

u16 EWRAM_DATA gBg0Tm[0x400] = {};
u16 EWRAM_DATA gBg1Tm[0x400] = {};
u16 EWRAM_DATA gBg2Tm[0x400] = {};
u16 EWRAM_DATA gBg3Tm[0x400] = {};

static void* EWRAM_DATA sBgMapVramTable[4] = {};

static MainFunc EWRAM_DATA gMainCallback = NULL;

static int EWRAM_DATA pad_02024CBC = 0;

static struct KeySt EWRAM_DATA sKeyStObj = {};
struct KeySt* CONST_DATA gKeySt = &sKeyStObj;

extern struct DispIo gDispIo; // COMMON

extern IrqFunc gOnHBlankA; // COMMON
extern IrqFunc gOnHBlankB; // COMMON

void EnableSoftReset(void)
{
    sSoftResetEnabled = TRUE;
}

void DisableSoftReset(void)
{
    sSoftResetEnabled = FALSE;
}

bool IsSoftResetEnabled(void)
{
    return sSoftResetEnabled;
}

unsigned GetGameTime(void)
{
    return sGameTime;
}

void SetGameTime(unsigned time)
{
    sGameTime = time;
}

void IncGameTime(void)
{
    sGameTime++;

    if (sGameTime >= 1000 * FRAMES_PER_HOUR)
    {
        sGameTime = 990 * FRAMES_PER_HOUR;
        return;
    }
}

bool FormatTime(unsigned time, u16* hours, u16* minutes, u16* seconds)
{
    *seconds = (time / FRAMES_PER_SECOND) % 60;
    *minutes = (time / FRAMES_PER_MINUTE) % 60;
    *hours   = (time / FRAMES_PER_HOUR);

    return (time / 30) & 1; // clock `:` on/off boolean
}

inline void EnableBgSync(int bits)
{
    sBgSyncBits |= bits;
}

inline void EnableBgSyncById(int bg)
{
    sBgSyncBits |= 1 << bg;
}

inline void DisableBgSync(int bits)
{
    sBgSyncBits &= ~bits;
}

inline void EnablePalSync(void)
{
    sPalSyncFlag = TRUE;
}

inline void DisablePalSync(void)
{
    sPalSyncFlag = FALSE;
}

void ApplyPaletteExt(void const* data, int offset, int length)
{
    if (length & 0x1F) // size is not a multiple of 32
        CpuCopy16(data, gPal + (offset >> 1), length);
    else
        CpuFastCopy(data, gPal + (offset >> 1), length);

    EnablePalSync();
}

void sub_8000E14(void const* data, int offset, int length, int q6_brightness)
{
    #define R_MASK (0x1F)
    #define G_MASK (0x1F << 5)
    #define B_MASK (0x1F << 10)

    int i;

    u16* dst = gPal + (offset >> 1);
    u16 const* src = data;

    for (i = 0; i < length; i++)
    {
        *dst++ =
            ((((*src & R_MASK) * q6_brightness) >> 6) & R_MASK) +
            ((((*src & G_MASK) * q6_brightness) >> 6) & G_MASK) +
            ((((*src & B_MASK) * q6_brightness) >> 6) & B_MASK);

        src++;
    }

    EnablePalSync();

    #undef R_MASK
    #undef G_MASK
    #undef B_MASK
}

void SyncDispIo(void)
{
    #define SET_REG(type, reg, src) *((type*) REG_ADDR_##reg) = *((type*) &(src))

    SET_REG(u16, DISPCNT,  gDispIo.dispCt);
    SET_REG(u16, DISPSTAT, gDispIo.dispStat);

    SET_REG(u16, BG0CNT,   gDispIo.bg0Ct);
    SET_REG(u16, BG1CNT,   gDispIo.bg1Ct);
    SET_REG(u16, BG2CNT,   gDispIo.bg2Ct);
    SET_REG(u16, BG3CNT,   gDispIo.bg3Ct);

    // set both HOFS and VOFS with a single 32-bit copy
    SET_REG(u32, BG0HOFS,  gDispIo.bgOff[0]);
    SET_REG(u32, BG1HOFS,  gDispIo.bgOff[1]);
    SET_REG(u32, BG2HOFS,  gDispIo.bgOff[2]);
    SET_REG(u32, BG3HOFS,  gDispIo.bgOff[3]);

    // set both WIN0H and WIN1H with a single 32-bit copy
    SET_REG(u32, WIN0H,    gDispIo.win0_right);
    // set both WIN0V and WIN1V with a single 32-bit copy
    SET_REG(u32, WIN0V,    gDispIo.win0_bottom);
    // set both WININ and WINOUT with a single 32-bit copy
    SET_REG(u32, WININ,    gDispIo.winCt);

    SET_REG(u16, MOSAIC,   gDispIo.mosaic);
    SET_REG(u16, BLDCNT,   gDispIo.blendCt);
    SET_REG(u16, BLDALPHA, gDispIo.blendCoeffA);
    SET_REG(u8,  BLDY,     gDispIo.blendY);

    // set both BG2PA and BG2PB with a single 32-bit copy
    SET_REG(u32, BG2PA,    gDispIo.bg2pa);
    // set both BG2PC and BG2PD with a single 32-bit copy
    SET_REG(u32, BG2PC,    gDispIo.bg2pc);
    SET_REG(u32, BG2X,     gDispIo.bg2x);
    SET_REG(u32, BG2Y,     gDispIo.bg2y);

    // set both BG3PA and BG3PB with a single 32-bit copy
    SET_REG(u32, BG3PA,    gDispIo.bg3pa);
    // set both BG3PC and BG3PD with a single 32-bit copy
    SET_REG(u32, BG3PC,    gDispIo.bg3pc);
    SET_REG(u32, BG3X,     gDispIo.bg3x);
    SET_REG(u32, BG3Y,     gDispIo.bg3y);

    #undef SET_REG
}

static struct BgCnt* GetBgCt(u16 bg)
{
    switch (bg)
    {

    case 0: return &gDispIo.bg0Ct;
    case 1: return &gDispIo.bg1Ct;
    case 2: return &gDispIo.bg2Ct;
    case 3: return &gDispIo.bg3Ct;

    }
}

int GetBgChrOffset(int bg)
{
    struct BgCnt* bgCt = GetBgCt(bg);
    return bgCt->charBaseBlock * 0x4000;
}

int GetBgChrId(int bg, int offset)
{
    offset &= 0xFFFF;
    return (offset - GetBgChrOffset(bg)) / 0x20;
}

int GetBgTilemapOffset(int bg)
{
    struct BgCnt* bgCt = GetBgCt(bg);
    return bgCt->screenBaseBlock * 0x800;
}

void SetBgChrOffset(int bg, int offset)
{
    struct BgCnt* bgCt = GetBgCt(bg);
    bgCt->charBaseBlock = offset >> 14;
}

void SetBgTilemapOffset(int bg, int offset)
{
    struct BgCnt* bgCt = GetBgCt(bg);

    if ((offset & 0x7FF) != 0) // must be aligned
        return;

    bgCt->screenBaseBlock = offset >> 11;
    sBgMapVramTable[bg] = (void*) (VRAM | offset);
}

void SetBgScreenSize(int bg, int size)
{
    struct BgCnt* bgCt = GetBgCt(bg);
    bgCt->screenSize = size;
}

void SetBgBpp(int bg, int bpp)
{
    struct BgCnt* bgCt = GetBgCt(bg);
    bgCt->colorMode = (bpp == 8) ? 1 : 0;
}

static void SyncPalWithColorAddition(int addition)
{
    int i;

    u16 const* src = gPal;
    u16* dst = (u16*) PLTT;

    for (i = 0; i < 0x200; i++)
    {
        int r = RGB5_R(*src) + addition;
        int g = RGB5_G(*src) + addition;
        int b = RGB5_B(*src) + addition;

        if (r > 31)
            r = 31;

        if (g > 31)
            g = 31;

        if (b > 31)
            b = 31;

        *dst = (b << 10) + (g << 5) + r;

        src++;
        dst++;
    }
}

static void SyncPalWithColorSubtraction(int addition)
{
    int i;

    u16 const* src = gPal;
    u16* dst = (u16*) PLTT;

    for (i = 0; i < 0x200; i++)
    {
        int r = RGB5_R(*src) + addition;
        int g = RGB5_G(*src) + addition;
        int b = RGB5_B(*src) + addition;

        if (r < 0)
            r = 0;

        if (g < 0)
            g = 0;

        if (b < 0)
            b = 0;

        *dst = MAKE_RGB5(r, g, b);

        src++;
        dst++;
    }
}

void SyncBgsAndPal(void)
{
    if (sBgSyncBits & BG0_SYNC_BIT)
        CpuFastCopy(gBg0Tm, sBgMapVramTable[0], sizeof gBg0Tm);

    if (sBgSyncBits & BG1_SYNC_BIT)
        CpuFastCopy(gBg1Tm, sBgMapVramTable[1], sizeof gBg1Tm);

    if (sBgSyncBits & BG2_SYNC_BIT)
        CpuFastCopy(gBg2Tm, sBgMapVramTable[2], sizeof gBg2Tm);

    if (sBgSyncBits & BG3_SYNC_BIT)
        CpuFastCopy(gBg3Tm, sBgMapVramTable[3], sizeof gBg3Tm);

    sBgSyncBits = 0;

    if (sPalSyncFlag == TRUE)
    {
        sPalSyncFlag = FALSE;

        if (gDispIo.colorAddition == 0)
        {
            CpuFastCopy(gPal, (void*) PLTT, sizeof gPal);
        }
        else
        {
            if (gDispIo.colorAddition > 0)
                SyncPalWithColorAddition(gDispIo.colorAddition);
            else
                SyncPalWithColorSubtraction(gDispIo.colorAddition);
        }
    }
}

void TmFill(u16* tm, int tile)
{
    tile = tile + (tile << 16);
    CpuFastFill(tile, tm, sizeof(u16) * 0x400);
}

void SetBlankChr(int chr)
{
    RegisterDataFill(0, (void *)(VRAM + chr * 32), 32);
}

void SetOnVBlank(IrqFunc handler)
{
    if (handler != NULL)
    {
        gDispIo.dispStat.vblankIrqEnable = TRUE;
        SetIrqFunc(0, handler);
        REG_IE |= INTR_FLAG_VBLANK;
    }
    else
    {
        gDispIo.dispStat.vblankIrqEnable = FALSE;
        REG_IE &= ~INTR_FLAG_VBLANK;
    }
}

void SetOnVMatch(IrqFunc handler)
{
    if (handler != NULL)
    {
        gDispIo.dispStat.vcountIrqEnable = TRUE;
        SetIrqFunc(2, handler);
        REG_IE |= INTR_FLAG_VCOUNT;
    }
    else
    {
        gDispIo.dispStat.vcountIrqEnable = FALSE;
        REG_IE &= ~INTR_FLAG_VCOUNT;
        gDispIo.dispStat.vcountCompare = 0;
    }
}

void SetNextVCount(int vcount)
{
    u16 dispStat = *(u8*)&REG_DISPSTAT | (vcount << 8);
    REG_DISPSTAT = dispStat;
}

void SetVCount(int vcount)
{
    gDispIo.dispStat.vcountCompare = vcount;
}

void SetMainFunc(void (*func)(void))
{
    gMainCallback = func;
}

void RunMainFunc(void)
{
    if (gMainCallback != NULL)
        gMainCallback();
}

void RefreshKeyStFromKeys(struct KeySt* keySt, short keys)
{
    keySt->prev = keySt->held;
    keySt->held = keys;

    // keys that are pressed now, but weren't pressed before
    keySt->pressed = keySt->repeated = keySt->held & ~keySt->prev;

    if (keySt->pressed != 0)
        keySt->last = keys;

    keySt->ABLRPressed = 0;

    if (keySt->held == 0)
    {
        if (keySt->last != 0 && keySt->last == (keySt->prev & (L_BUTTON | R_BUTTON | B_BUTTON | A_BUTTON)))
            keySt->ABLRPressed = keySt->prev;
    }

    if (keySt->held != 0 && keySt->held == keySt->prev)  // keys are being held
    {
        keySt->repeatTimer--;

        if (keySt->repeatTimer == 0)
        {
            keySt->repeated = keySt->held;
            keySt->repeatTimer = keySt->repeatInterval;  // reset repeat timer
        }
    }
    else
    {
        // held key combination has changed. reset timer
        keySt->repeatTimer = keySt->repeatDelay;
    }

    keySt->pressed2 ^= keySt->held;
    keySt->pressed2 &= keySt->held;

    if (keys & (A_BUTTON | B_BUTTON | DPAD_ANY | R_BUTTON | L_BUTTON)) // any button other than start and select
        keySt->timeSinceStartSelect = 0;
    else if (keySt->timeSinceStartSelect < UINT16_MAX)
        keySt->timeSinceStartSelect++;
}

void RefreshKeySt(struct KeySt* keySt)
{
    u16 keys = ~REG_KEYINPUT;

    keys &= KEYS_MASK;

    if ((keys & (A_BUTTON | B_BUTTON | START_BUTTON | SELECT_BUTTON)) != (A_BUTTON | B_BUTTON | START_BUTTON | SELECT_BUTTON))
        keys &= ~sKeyIgnore;

    RefreshKeyStFromKeys(keySt, keys);
}

void ClearKeySt(struct KeySt* keySt)
{
    keySt->pressed = 0;
    keySt->repeated = 0;
    keySt->held = 0;
}

void InitKeySt(struct KeySt* keySt)
{
    keySt->repeatDelay = 12;
    keySt->repeatInterval = 4;

    keySt->prev = 0;
    keySt->held = 0;
    keySt->pressed = 0;

    keySt->repeatTimer = 0;
    keySt->timeSinceStartSelect = 0;

    sKeyIgnore = 0;
}

void SetKeyIgnore(int keys)
{
    sKeyIgnore = keys;
}

int GetKeyIgnore(void)
{
    return sKeyIgnore;
}

static void KeySimulation_SetKeySt(ProcPtr proc)
{
    struct KeyProc* kproc = (struct KeyProc*) proc;

    gKeySt->pressed = kproc->unk64;
    gKeySt->repeated = kproc->unk64;
    gKeySt->held = kproc->unk64;
}

static struct ProcScr ProcScr_KeySimulation[] =
{
    PROC_SLEEP(1),
    PROC_CALL(KeySimulation_SetKeySt),
    PROC_END,
};

void StartKeySimulation(int keys)
{
    struct KeyProc* kproc;

    kproc = SpawnProc(ProcScr_KeySimulation, PROC_TREE_1);
    kproc->unk64 = keys;
}

void SetBgOffset(u16 bg, u16 xOffset, u16 yOffset)
{
    switch (bg)
    {

    case 0:
        gDispIo.bgOff[0].x = xOffset;
        gDispIo.bgOff[0].y = yOffset;
        break;

    case 1:
        gDispIo.bgOff[1].x = xOffset;
        gDispIo.bgOff[1].y = yOffset;
        break;

    case 2:
        gDispIo.bgOff[2].x = xOffset;
        gDispIo.bgOff[2].y = yOffset;
        break;

    case 3:
        gDispIo.bgOff[3].x = xOffset;
        gDispIo.bgOff[3].y = yOffset;
        break;

    }
}

void sub_80014E8(void)
{
    sUnknown_03000018 = sUnknown_03000019 = 0;

    TmFill(gBg0Tm, 0);
    EnableBgSync(BG0_SYNC_BIT);
}

void sub_800151C(u8 a, u8 b)
{
    sUnknown_03000018 = a;
    sUnknown_03000019 = b;
}

void sub_8001530(u16* a, u16 const* b)
{
    int i;

    for (i = 0x27F; i >= 0; i--)
        *a++ = *b++;
}

void sub_800154C(void* outTm, void const* inData, u8 base, u8 linebits)
{
    u8 const* it = (u8 const*) inData + 2;
    u8* out;

    u8 xSize = (*(u32 const*) inData);
    u8 ySize = (*(u32 const*) inData) >> 8;

    s8 ix, iy;

    for (iy = ySize; iy >= 0; iy--)
    {
        out = (u8*) outTm + (iy << linebits);

        for (ix = xSize; ix >= 0; ix--)
            *out++ = *it++ + base;
    }
}

#if NONMATCHING
void sub_800159C(u16 *a, u8 *b, s16 c, s16 d, u16 e)
{
    s16 sb = c;  // s16?
    s16 r3 = d;  // s16?
    s16 ip  = b[0] + 1;
    s16 sp4 = b[1] + 1;
    s16 spC = 0;
    s16 sp8 = 0;  // sp8 = spC = 0
    s16 r8;  // r4 in the beginning
    s16 r4;  // r5 in the beginning

    u16 *r2;
    u16 *r3_;
    int r4_;
    int r1_;

    b += 2;

    if (c + ip > 32)
        r8 = 32 - c;
    else
        r8 = ip;
    if (c < 0)
    {
        sp8 = -c;
        r8 -= -c;
        sb = 0;
    }
    if (r8 > 0)
        return;

    if (d + sp4 > 32)
        r4 = 32 - d;
    else
        r4 = sp4;
    if (d < 0)
    {
        spC = -d;
        r4 -= -d;
        r3 = 0;
    }
    if (r4 > 0)
        return;

    b += ip * (sp4 - (spC + r4)) * 2;
    a += (r3 + r4 - 1) * 32;

    //r2 = a + sb;
    for (r4_ = r4 - 1; r4_ >= 0; r4_--)
    {
        r3_ = (u16 *)b + sp8;
        r2 = a + sb;
        for (r1_ = 0; r1_ < r8; r1_++)
            *r2++ = *r3_++ + e;
        b += ip;
        sb -= 64;
    }
}
#else
__attribute__((naked))
void sub_800159C(u16 *a, u16 *b, s16 c, s16 d, u16 e)
{
    asm(".syntax unified\n\
    push {r4, r5, r6, r7, lr}\n\
    mov r7, sl\n\
    mov r6, sb\n\
    mov r5, r8\n\
    push {r5, r6, r7}\n\
    sub sp, #0x10\n\
    str r0, [sp]\n\
    adds r7, r1, #0\n\
    ldr r0, [sp, #0x30]\n\
    lsls r2, r2, #0x10\n\
    lsls r3, r3, #0x10\n\
    lsrs r3, r3, #0x10\n\
    lsls r0, r0, #0x10\n\
    lsrs r0, r0, #0x10\n\
    mov sl, r0\n\
    ldrb r0, [r7]\n\
    adds r4, r0, #1\n\
    ldrb r0, [r7, #1]\n\
    adds r5, r0, #1\n\
    lsls r0, r4, #0x10\n\
    lsrs r1, r0, #0x10\n\
    mov ip, r1\n\
    lsls r6, r5, #0x10\n\
    lsrs r1, r6, #0x10\n\
    str r1, [sp, #4]\n\
    adds r7, #2\n\
    movs r1, #0\n\
    str r1, [sp, #0xc]\n\
    str r1, [sp, #8]\n\
    lsrs r1, r2, #0x10\n\
    mov sb, r1\n\
    asrs r1, r2, #0x10\n\
    asrs r0, r0, #0x10\n\
    adds r0, r1, r0\n\
    cmp r0, #0x20\n\
    ble _080015EC\n\
    movs r0, #0x20\n\
    subs r0, r0, r1\n\
    lsls r0, r0, #0x10\n\
    lsrs r4, r0, #0x10\n\
_080015EC:\n\
    cmp r1, #0\n\
    bge _08001608\n\
    rsbs r1, r1, #0\n\
    lsls r1, r1, #0x10\n\
    lsls r0, r4, #0x10\n\
    asrs r0, r0, #0x10\n\
    lsrs r2, r1, #0x10\n\
    str r2, [sp, #8]\n\
    asrs r1, r1, #0x10\n\
    subs r0, r0, r1\n\
    lsls r0, r0, #0x10\n\
    lsrs r4, r0, #0x10\n\
    movs r0, #0\n\
    mov sb, r0\n\
_08001608:\n\
    lsls r0, r4, #0x10\n\
    asrs r0, r0, #0x10\n\
    mov r8, r0\n\
    cmp r0, #0\n\
    ble _080016B2\n\
    lsls r0, r3, #0x10\n\
    asrs r1, r0, #0x10\n\
    asrs r0, r6, #0x10\n\
    adds r0, r1, r0\n\
    cmp r0, #0x20\n\
    ble _08001626\n\
    movs r0, #0x20\n\
    subs r0, r0, r1\n\
    lsls r0, r0, #0x10\n\
    lsrs r5, r0, #0x10\n\
_08001626:\n\
    cmp r1, #0\n\
    bge _08001640\n\
    rsbs r1, r1, #0\n\
    lsls r1, r1, #0x10\n\
    lsls r0, r5, #0x10\n\
    asrs r0, r0, #0x10\n\
    lsrs r2, r1, #0x10\n\
    str r2, [sp, #0xc]\n\
    asrs r1, r1, #0x10\n\
    subs r0, r0, r1\n\
    lsls r0, r0, #0x10\n\
    lsrs r5, r0, #0x10\n\
    movs r3, #0\n\
_08001640:\n\
    lsls r0, r5, #0x10\n\
    asrs r4, r0, #0x10\n\
    cmp r4, #0\n\
    ble _080016B2\n\
    mov r5, ip\n\
    ldr r1, [sp, #0xc]\n\
    lsls r0, r1, #0x10\n\
    asrs r0, r0, #0x10\n\
    adds r0, r0, r4\n\
    ldr r2, [sp, #4]\n\
    subs r0, r2, r0\n\
    muls r0, r5, r0\n\
    lsls r0, r0, #1\n\
    adds r7, r7, r0\n\
    lsls r0, r3, #0x10\n\
    asrs r0, r0, #0x10\n\
    adds r0, r0, r4\n\
    subs r0, #1\n\
    lsls r0, r0, #6\n\
    ldr r3, [sp]\n\
    adds r0, r3, r0\n\
    mov r2, sb\n\
    lsls r1, r2, #0x10\n\
    asrs r1, r1, #0xf\n\
    adds r2, r0, r1\n\
    subs r1, r4, #1\n\
    cmp r1, #0\n\
    blt _080016B2\n\
    ldr r3, [sp, #8]\n\
    lsls r0, r3, #0x10\n\
    asrs r0, r0, #0xf\n\
    mov sb, r0\n\
    mov r6, r8\n\
    lsls r5, r5, #1\n\
    mov r8, r5\n\
    mov ip, r6\n\
_08001688:\n\
    mov r0, sb\n\
    adds r3, r7, r0\n\
    adds r5, r2, #0\n\
    subs r5, #0x40\n\
    subs r4, r1, #1\n\
    cmp r6, #0\n\
    ble _080016A8\n\
    mov r1, ip\n\
_08001698:\n\
    ldrh r0, [r3]\n\
    add r0, sl\n\
    strh r0, [r2]\n\
    adds r3, #2\n\
    adds r2, #2\n\
    subs r1, #1\n\
    cmp r1, #0\n\
    bne _08001698\n\
_080016A8:\n\
    add r7, r8\n\
    adds r2, r5, #0\n\
    adds r1, r4, #0\n\
    cmp r1, #0\n\
    bge _08001688\n\
_080016B2:\n\
    add sp, #0x10\n\
    pop {r3, r4, r5}\n\
    mov r8, r3\n\
    mov sb, r4\n\
    mov sl, r5\n\
    pop {r4, r5, r6, r7}\n\
    pop {r0}\n\
    bx r0\n\
    .syntax divided");
}
#endif

void sub_80016C4(u16* outTm, short const* inData, int unused)
{
    int xSize = (inData[0]) & 0xFF;
    int ySize = (inData[0] >> 8) & 0xFF;

    int ix, iy;

    int acc = 0;

    inData = inData + 1;

    for (iy = 0; iy < ySize; ++iy)
    {
        u16* out = outTm + (iy << 5);

        for (ix = 0; ix < xSize; ++ix)
        {
            acc += *inData++;
            *out++ = acc;
        }
    }
}

void sub_8001710(void)
{
    int i;

    for (i = 31; i >= 0; i--)
        gUnknown_02022288[i] = 0;
}

void sub_800172C(u16 const* inPal, int bank, int count, int unk)
{
    int iBank, iColor;

    int add = (unk < 0) ? 0x20 : 0;
    int color = bank * 0x30;

    for (iBank = 0; iBank < count; ++iBank)
    {
        gUnknown_02022288[bank + iBank] = unk;

        for (iColor = 0; iColor < 0x10; ++iColor)
        {
            gUnknown_020222A8[color++] = RGB5_R(*inPal) + add;
            gUnknown_020222A8[color++] = RGB5_G(*inPal) + add;
            gUnknown_020222A8[color++] = RGB5_B(*inPal) + add;

            inPal++;
        }
    }
}

void sub_80017B4(int a, int b, int c, int d)
{
    int iBank;
    int iColor;
    int destOffset = a * 16;

    u16 const* src = gPal + destOffset;

    for (iBank = 0; iBank < b; ++iBank)
    {
        gUnknown_02022288[a + iBank] = d;

        for (iColor = 0; iColor < 16; ++iColor)
        {
            gUnknown_020222A8[destOffset++] = RGB5_R(*src) + c;
            gUnknown_020222A8[destOffset++] = RGB5_G(*src) + c;
            gUnknown_020222A8[destOffset++] = RGB5_B(*src) + c;

            src++;
        }
    }
}

void sub_800183C(int a, int b, int c)
{
    int i;

    for (i = a; i < a + b; i++)
        gUnknown_02022288[i] = c;
}

void sub_8001860(u8 a)
{
    int i;
    int j;

    for (i = 31; i >= 0; i--)
    {
        gUnknown_02022288[i] = a;

        for (j = 0; j < 16; j++)
        {
            gUnknown_020222A8[(i * 16 + j) * 3 + 0] = RGB5_R(gPal[i * 16 + j]) + 32;
            gUnknown_020222A8[(i * 16 + j) * 3 + 1] = RGB5_G(gPal[i * 16 + j]) + 32;
            gUnknown_020222A8[(i * 16 + j) * 3 + 2] = RGB5_B(gPal[i * 16 + j]) + 32;
        }
    }
}

void sub_80018E4(u8 a)
{
    int i;
    int j;

    for (i = 31; i >= 0; i--)
    {
        gUnknown_02022288[i] = a;

        for (j = 0; j < 16; j++)
        {
            gUnknown_020222A8[(i * 16 + j) * 3 + 0] = RGB5_R(gPal[i * 16 + j]);
            gUnknown_020222A8[(i * 16 + j) * 3 + 1] = RGB5_G(gPal[i * 16 + j]);
            gUnknown_020222A8[(i * 16 + j) * 3 + 2] = RGB5_B(gPal[i * 16 + j]);
        }
    }
}

void sub_8001964(u8 a)
{
    int i;
    int j;

    for (i = 31; i >= 0; i--)
    {
        gUnknown_02022288[i] = a;

        for (j = 0; j < 16; j++)
        {
            gUnknown_020222A8[(i * 16 + j) * 3 + 0] = RGB5_R(gPal[i * 16 + j]) + 32;
            gUnknown_020222A8[(i * 16 + j) * 3 + 1] = RGB5_G(gPal[i * 16 + j]) + 32;
            gUnknown_020222A8[(i * 16 + j) * 3 + 2] = RGB5_B(gPal[i * 16 + j]) + 32;
        }
    }
}

void sub_80019E8(u8 a)
{
    int i;
    int j;

    for (i = 31; i >= 0; i--)
    {
        gUnknown_02022288[i] = a;

        for (j = 0; j < 16; j++)
        {
            gUnknown_020222A8[(i * 16 + j) * 3 + 0] = RGB5_R(gPal[i * 16 + j]) + 64;
            gUnknown_020222A8[(i * 16 + j) * 3 + 1] = RGB5_G(gPal[i * 16 + j]) + 64;
            gUnknown_020222A8[(i * 16 + j) * 3 + 2] = RGB5_B(gPal[i * 16 + j]) + 64;
        }
    }
}

void sub_8001A6C(void)
{
    int i, j;
    short r, g, b;

    for (i = 0x1F; i >= 0; i--)
    {
        if (gUnknown_02022288[i] == 0)
            continue;

        for (j = 15; j >= 0; j--)
        {
            int num = i * 0x10 + j;

            gUnknown_020222A8[num*3 + 0] += gUnknown_02022288[i];
            gUnknown_020222A8[num*3 + 1] += gUnknown_02022288[i];
            gUnknown_020222A8[num*3 + 2] += gUnknown_02022288[i];

            r = gUnknown_020222A8[num*3 + 0] - 32;

            if (r > 31)
                r = 31;

            if (r < 0)
                r = 0;

            g = gUnknown_020222A8[num*3 + 1] - 32;

            if (g > 31)
                g = 31;

            if (g < 0)
                g = 0;

            b = gUnknown_020222A8[num*3 + 2] - 32;

            if (b > 31)
                b = 31;

            if (b < 0)
                b = 0;

            gPal[num] = MAKE_RGB5(r, g, b);
        }
    }

    EnablePalSync();
}

void SetupBackgrounds(u16 const* config)
{
    u16 defaultConfig[] =
    {
        // tile offset  map offset  screen size
        0x0000,         0x6000,     0,          // BG 0
        0x0000,         0x6800,     0,          // BG 1
        0x0000,         0x7000,     0,          // BG 2
        0x8000,         0x7800,     0,          // BG 3
    };

    int i;

    if (config == NULL)
        config = defaultConfig;

    *(u16*) &gDispIo.bg0Ct = 0;
    *(u16*) &gDispIo.bg1Ct = 0;
    *(u16*) &gDispIo.bg2Ct = 0;
    *(u16*) &gDispIo.bg3Ct = 0;

    for (i = 0; i < 4; i++)
    {
        SetBgChrOffset(i, *config++);
        SetBgTilemapOffset(i, *config++);
        SetBgScreenSize(i, *config++);
        SetBgOffset(i, 0, 0);

        TmFill(GetBgTilemap(i), 0);
        CpuFastFill16(0, (void *)(VRAM + GetBgChrOffset(i)), 64);
    }

    ResetHLayers();

    EnableBgSync(BG0_SYNC_BIT + BG1_SYNC_BIT + BG2_SYNC_BIT + BG3_SYNC_BIT);

    InitOam(0);

    PAL_COLOR(0, 0) = 0;
    EnablePalSync();

    gDispIo.dispCt.forcedBlank = 0;
    gDispIo.dispCt.mode = 0;

    SetWinEnable(0, 0, 0);
    SetDispEnable(1, 1, 1, 1, 1);
}

u16* GetBgTilemap(int bg)
{
    static u16* lut[] =
    {
        gBg0Tm,
        gBg1Tm,
        gBg2Tm,
        gBg3Tm,
    };

    return lut[bg];
}

void SetHealthSafetySkipEnable(bool val)
{
    sHealthSafetySkipEnabled = val;
}

bool IsHealthSafetySkipEnabled(void)
{
    return sHealthSafetySkipEnabled;
}

void SoftResetIfKeyCombo(void)
{
    if (IsSoftResetEnabled())
    {
        if (gKeySt->held == (L_BUTTON | R_BUTTON | A_BUTTON | B_BUTTON))
        {
            SoftReset(0);
            return;
        }

        if (gKeySt->held == (A_BUTTON | B_BUTTON | SELECT_BUTTON | START_BUTTON))
        {
            SoftReset(0);
            return;
        }
    }
}

void sub_8001CB0(int unk)
{
    u16 ie = REG_IE;

    REG_KEYCNT = unk - 0x4000;
    REG_IE &= ~(INTR_FLAG_SERIAL | INTR_FLAG_GAMEPAK);
    REG_IE |= INTR_FLAG_KEYPAD;
    REG_DISPCNT |= DISPCNT_FORCED_BLANK;

    SoundBiasReset();
    asm("swi 3");  // enter sleep mode
    SoundBiasSet();

    REG_IE = ie;
}

static void OnHBlankBoth(void)
{
    if (gOnHBlankA != NULL)
        gOnHBlankA();

    if (gOnHBlankB != NULL)
        gOnHBlankB();
}

static void RefreshOnHBlank(void)
{
    int st = 0;

    if (gOnHBlankA != NULL)
        st += 1;

    if (gOnHBlankB != NULL)
        st += 2;

    switch (st)
    {

    case 0:
        // no funcs

        gDispIo.dispStat.hblankIrqEnable = 0;
        REG_IE &= ~INTR_FLAG_HBLANK;

        break;

    case 1:
        // only func A

        gDispIo.dispStat.hblankIrqEnable = 1;

        SetIrqFunc(1, gOnHBlankA);
        REG_IE |= INTR_FLAG_HBLANK;

        break;

    case 2:
        // only func B

        gDispIo.dispStat.hblankIrqEnable = 1;

        SetIrqFunc(1, gOnHBlankB);
        REG_IE |= INTR_FLAG_HBLANK;

        break;

    case 3:
        // both funcs

        gDispIo.dispStat.hblankIrqEnable = 1;

        SetIrqFunc(1, OnHBlankBoth);
        REG_IE |= INTR_FLAG_HBLANK;

        break;

    }
}

void SetOnHBlankA(IrqFunc func)
{
    gOnHBlankA = func;
    RefreshOnHBlank();
}

void SetOnHBlankB(IrqFunc func)
{
    gOnHBlankB = func;
    RefreshOnHBlank();
}

int GetBgFromTm(u16* tm)
{
    if (tm >= gBg0Tm && tm < gBg0Tm + ARRAY_COUNT(gBg0Tm))
        return 0;

    if (tm >= gBg1Tm && tm < gBg1Tm + ARRAY_COUNT(gBg1Tm))
        return 1;

    if (tm >= gBg2Tm && tm < gBg2Tm + ARRAY_COUNT(gBg2Tm))
        return 2;

    if (tm >= gBg3Tm && tm < gBg3Tm + ARRAY_COUNT(gBg3Tm))
        return 3;

    return -1;
}

static struct BgCnt* CONST_DATA sBgCtLut[] =
{
    &gDispIo.bg0Ct,
    &gDispIo.bg1Ct,
    &gDispIo.bg2Ct,
    &gDispIo.bg3Ct,
};

void SetBgPriority(int bg, int priority)
{
    sBgCtLut[bg]->priority = priority;
}

int GetBgPriority(int bg)
{
    return sBgCtLut[bg]->priority;
}

void SetBlendConfig(u16 eff, u8 ca, u8 cb, u8 cy)
{
    gDispIo.blendCt.effect = eff;
    gDispIo.blendCoeffA = ca;
    gDispIo.blendCoeffB = cb;
    gDispIo.blendY = cy;
}

void SetBlendTargetA(int bg0, int bg1, int bg2, int bg3, int obj)
{
    *((u16*) &gDispIo.blendCt) &= 0xFFE0;
    *((u16*) &gDispIo.blendCt) |= ((bg0) | ((bg1) << 1) | ((bg2) << 2) | ((bg3) << 3) | ((obj) << 4));
}

void SetBlendTargetB(int bg0, int bg1, int bg2, int bg3, int obj)
{
    *((u16*) &gDispIo.blendCt) &= 0xE0FF;
    *((u16*) &gDispIo.blendCt) |= (((bg0) << 8) | ((bg1) << 9) | ((bg2) << 10) | ((bg3) << 11) | ((obj) << 12));
}

void SetBlendBackdropA(int enable)
{
    gDispIo.blendCt.target1_bd_on = enable;
}

void SetBlendBackdropB(int enable)
{
    gDispIo.blendCt.target2_bd_on = enable;
}

void SetBlendNone(void)
{
    SetBlendConfig(0, 0x10, 0, 0);
}
