//
// Created by laqieer on 2020/3/3.
//

#include "bmdebug.h"

#include "debug-text.h"
#include "text.h"

// if current seconds is even, return 2; else return 3
int Return2or3BySecondParity(void)
{
    int retVal;
    unsigned short hours;
    unsigned short minutes;
    unsigned short seconds;

    FormatTime(GetGameTime(),&hours,&minutes,&seconds);
    if ((seconds & 1) == 0) {
        retVal = 2;
    }
    else {
        retVal = 3;
    }
    return retVal;
}

// if current seconds is even, return 3; else return 2
int Return3or2BySecondParity(void)
{
    int retVal;
    unsigned short hours;
    unsigned short minutes;
    unsigned short seconds;

    FormatTime(GetGameTime(),&hours,&minutes,&seconds);
    if ((seconds & 1) != 0) {
        retVal = 2;
    }
    else {
        retVal = 3;
    }
    return retVal;
}

// return 8
int Get8(void)
{
    return 8;
}

// return 23
int Get23(void)
{
    return 23;
}

// do nothing, return directly
void DummyFunction(void)
{

}

void Loop6C_WaitForSelectPress(struct Proc *proc)
{
    if (gKeySt->pressed & SELECT_BUTTON) {
        Proc_Break(proc);
    }
}

void SetNewKeyStatusWith16(void)
{
    StartKeySimulation(16);
}


// do nothing, return directly
void DummyFunction2(void)
{

}

void DebugPrintWithProc(struct DebugPrintProc *proc)
{
    int x;
    int y;
    int width;
    const char *text;
    struct Text textHandler;

    x = proc->x;
    y = proc->y;
    width = proc->width;
    text = proc->text;
    InitText(&textHandler, width);
    Text_DrawString(&textHandler, text);
    DrawUiFrame2(x, y, width + 2, 4, 0);
    PutText(&textHandler, &gBg0Tm[32 * (y + 1) + (x + 1)]);
    EnableBgSync(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

struct ProcScr gProc_DebugPrintWithProc[] = {
    PROC_SLEEP(1), PROC_CALL(DebugPrintWithProc), PROC_END
};

void DebugPrint(int x, int y, int width, const char *text)
{
    struct DebugPrintProc *proc;

    proc = (struct DebugPrintProc *)SpawnProc(gProc_DebugPrintWithProc, (struct Proc *)PROC_MARK_3);
    proc->x = x;
    proc->y = y;
    proc->text = text;
    proc->width = width;
}

int StartDebugMenu(struct MenuProc *menuProc)
{
    EndMenu(menuProc);
    ClearBg0Bg1();
    StartMenu(&gDebugMenuDef);
    DebugInitBg(2, 0);
    return 1;
}

u8 DebugMenu_MapIdle(struct MenuProc *menuProc, struct MenuItemProc *menuItemProc)
{
    DebugMenuMapIdleCore(menuItemProc, 7, 2);
    return 0;
}

u8 DebugMenu_MapEffect(struct MenuProc *menuProc, struct MenuItemProc *menuItemProc)
{
  EndMapMain();
  gPlaySt.chapter = sub_801C650(menuItemProc->itemNumber);
  gPlaySt.chapterModeIndex = gUnknown_03001780;
  ChapterEndUnitCleanup();
  nullsub_9();
  gPlaySt.unk4A_2 = 2;
  return 23;
}
