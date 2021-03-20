
#include "global.h"

#include "random.h"
#include "hardware.h"
#include "sound.h"
#include "armfunc.h"
#include "ap.h"
#include "debug-text.h"
#include "text.h"
#include "proc.h"
#include "mu.h"

static u16 const _[] = { 0x4641, 0x464A, 0x4653, 0x465C };

void AgbMain(void)
{
    bool hasWaitCnt;

    // clear RAM
    DmaFill32(3, 0, (void *)IWRAM_START, 0x7F80); // reset the area for the IWRAM ARM section.
    CpuFastFill(0, (void *)EWRAM_START, 0x40000);

    hasWaitCnt = (REG_WAITCNT != 0);

    SetHealthSafetySkipEnable(hasWaitCnt);

    if (hasWaitCnt == TRUE)
        RegisterRamReset(~2);

    REG_WAITCNT = 0x45B4;

    IrqInit();
    SetOnVBlank(NULL);

    REG_DISPSTAT = 8;
    REG_IME = 1;

    InitKeySt(gKeySt);
    RefreshKeySt(gKeySt);

    InitRamFuncs();
    SramInit();
    InitProcs();
    InitAnims();
    MU_Init();

    RandInitB(0x42D690E9);
    RandInit(RandNextB());

    DisableSoftReset();
    sub_80A7374();
    sub_80A40A8();

    // initialize sound
    m4aSoundInit();
    Sound_SetDefaultMaxNumChannels();

    SetOnVBlank(OnVBlank);
    GmDataInit();

    SetLang(LANG_ENGLISH);
    ResetText();

    StartGame();

    while (TRUE)
    {
        RunMainFunc();
        SoftResetIfKeyCombo();
    }
}

void PutBuildInfo(u16* tm)
{
    DebugPutStr(tm, "2005/02/04(FRI) 16:55:40");
    DebugPutStr(tm - 0x20, "_2003");
}
