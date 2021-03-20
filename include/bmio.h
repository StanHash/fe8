#ifndef GUARD_BMIO_H
#define GUARD_BMIO_H

enum {
    BM_BGPAL_6 = 6,
    BM_BGPAL_TILESET_BASE = 7,
};

enum {
    BM_OBJPAL_1  = 1,
    BM_OBJPAL_10 = 10,
};

struct TileGfxAnim {
    /* 00 */ u16         time;
    /* 02 */ u16         size;
    /* 04 */ const void* data;
};

struct TilePalAnim {
    /* 00 */ const void* data;
    /* 04 */ u8          time;
    /* 05 */ u8          colorCount;
    /* 06 */ u8          colorStart;
};

struct GameCtrlProc {
    PROC_HEADER;
};

struct BMapMainProc {
    PROC_HEADER;

    /* 29 */ u8 pad29[0x54 - 0x29];
    /* 54 */ struct GameCtrlProc* gameCtrl;
};

void StartGameVBlankProc(void);
void EndGameVBlankProc(void);
void LockGameGraphicsLogic(void);
void UnlockGameGraphicsLogic(void);
void AllocWeatherParticles(unsigned weatherId);
void WfxFlamesInitGradientPublic(void);
void DisableMapPaletteAnimations(void);
void ResetMapPaletteAnimations(void);
void SetupWeather(unsigned weatherId);

// this may have been two (or more?) source files
// if this is to be split, this the starting point.

u8 GetTextSpeed(void);
int IsFirstPlaythrough(void);
void InitClearChapterState(int isDifficult, s8);
void StartChapter(struct GameCtrlProc* gameCtrl);
void RestartBattleMap(void);
void GameControl_StartResumedGame(struct GameCtrlProc* gameCtrl);
void RefreshBMapDisplay_FromBattle(void);
void BMapDispResume_FromBattleDelayed(void);
void ReloadGameCoreGraphics(void);
struct BMapMainProc* StartMapMain(struct GameCtrlProc* gameCtrl);
void EndMapMain(void);
void ChapterEndUnitCleanup(void);
void MapMain_ResumeFromPhaseIdle(struct BMapMainProc* mapMain);
void MapMain_ResumeFromAction(struct BMapMainProc* mapMain);
void MapMain_ResumeFromBerserkPhase(struct BMapMainProc* mapMain);
void MapMain_ResumeFromArena(struct BMapMainProc* mapMain);
void MapMain_ResumeFromPhaseSwitch(struct BMapMainProc* mapMain);
void GameCtrl_DeclareCompletedChapter(void);
void GameCtrl_DeclareCompletedPlaythrough(void);
char* GetTacticianNameString(void);
void SetTacticianName(const char* newName);

#endif // GUARD_BMIO_H
