
#include "global.h"

#include "armfunc.h"
#include "ap.h"
#include "fontgrp.h"
#include "hardware.h"
#include "proc.h"
#include "random.h"
#include "mu.h"
#include "sound.h"

u16 const gUninitializedMemory[] = { 0x4641, 0x464A, 0x4653, 0x465C };

void AgbMain(void)
{
    int waitCnt;

    // clear RAM
    DmaFill32(3, 0, (void *)IWRAM_START, 0x7F80); // reset the area for the IWRAM ARM section.
    CpuFastFill(0, (void *)EWRAM_START, 0x40000);    

    waitCnt = (REG_WAITCNT != 0);

    SetHealthSafetySkipEnable(waitCnt);

    if (waitCnt == TRUE)
        RegisterRamReset(~2);

    REG_WAITCNT = 0x45B4;

    IrqInit();
    SetOnVBlank(NULL);

    REG_DISPSTAT = 8;
    REG_IME = 1;

    InitKeySt(gKeySt);
    RefreshKeySt(gKeySt);

    InitRamFuncs();
    sub_80A2C3C();
    InitProcs();
    InitAnims();
    InitMus();

    RandInitB(0x42D690E9);
    RandInit(RandNextB());

    DisableSoftReset();
    sub_80A7374();
    sub_80A40A8();

    // initialize sound
    m4aSoundInit();
    Sound_SetDefaultMaxNumChannels();

    SetOnVBlank(GeneralVBlankHandler);
    sub_80BC81C();
    SetSomeByte(1);
    Font_InitForUIDefault();

    NewGameControl();

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
