
#include "global.h"

#include "proc.h"
#include "bmio.h"
#include "unit.h"
#include "event.h"

void GameControl_8009A58(ProcPtr);
void GameControl_8009A60_Null(ProcPtr);
void GameControl_HandleSelectRightL(ProcPtr);
void GameControl_EnableSoundEffects(ProcPtr);
void sub_8009AEC(ProcPtr);
void sub_80099E4(ProcPtr);
void sub_8009B64(ProcPtr);
void sub_8009A84(ProcPtr);
void sub_80C6424(ProcPtr);
void sub_80C645C(ProcPtr);
void sub_80C6444(ProcPtr);
void Make6C_savemenu(ProcPtr);
void GameControl_MasterSwitch(ProcPtr);
void GameControl_RememberChapterId(ProcPtr);
void sub_8009D1C(ProcPtr);
void sub_8009D44(ProcPtr);
void sub_8009E00(ProcPtr);
void GM_End(ProcPtr);
void sub_8009E28(ProcPtr);
void GameControl_PostChapterSwitch(ProcPtr);
void GameControl_RestoreChapterId(ProcPtr);
void GameControl_ChapterSwitch(ProcPtr);
void GameControl_CallPostChapterSaveMenu(ProcPtr);
void sub_8009D6C(ProcPtr);
void sub_80481E0(ProcPtr);
void sub_8009ABC(ProcPtr);
void Start_savemenu2(ProcPtr);
void CallGameEndingEvent(ProcPtr);
void sub_80B7598(ProcPtr);
void sub_8009EFC(ProcPtr);
void sub_8009A24(ProcPtr);
void StartFadeOutBlackMedium(ProcPtr);
void WaitForFade(ProcPtr);
void sub_8048850(ProcPtr);
void StartFadeInBlackMedium(ProcPtr);
void EndBG3Slider(ProcPtr);

extern struct ProcScr CONST_DATA gUnknown_08AA7680[]; // pre-intro cutscene
extern struct ProcScr CONST_DATA gUnknown_08AA71C8[]; // intro cutscene
extern struct ProcScr CONST_DATA gUnknown_08A3DD50[]; // world map wrapper

struct ProcScr CONST_DATA gUnused_085916BC[] =
{
    PROC_CALL(GameControl_8009A58),
    PROC_REPEAT(GameControl_8009A60_Null),

    PROC_END,
};

struct ProcScr CONST_DATA gUnknown_085916D4[] =
{
    PROC_NAME("GAMECTRL"),
    PROC_MARK(PROC_MARK_B),

    PROC_15,

    PROC_CALL(GameControl_HandleSelectRightL),

    PROC_CALL(GameControl_8009A58),
    PROC_REPEAT(GameControl_8009A60_Null),

    // fallthrough

PROC_LABEL(0),
    PROC_START_CHILD_LOCKING(gUnknown_08AA7680),

    // fallthrough

PROC_LABEL(1),
    PROC_CALL(GameControl_EnableSoundEffects),
    PROC_START_CHILD_LOCKING(gUnknown_08AA71C8),
    PROC_CALL(sub_8009AEC),
    PROC_SLEEP(0),

    PROC_GOTO(4),

PROC_LABEL(2),
    PROC_CALL_2(sub_80099E4),
    PROC_CALL(sub_8009B64),
    PROC_SLEEP(0),

    PROC_GOTO(1),

PROC_LABEL(3),
    PROC_SLEEP(0),

    PROC_CALL(sub_8009A84),
    PROC_CALL(sub_8009B64),
    PROC_SLEEP(0),

    PROC_GOTO(1),

PROC_LABEL(4),
    PROC_CALL(GameControl_EnableSoundEffects),
    PROC_CALL(sub_80C6424),

    PROC_GOTO(26),

PROC_LABEL(24),
    PROC_CALL(GameControl_EnableSoundEffects),
    PROC_CALL(sub_80C645C),

    PROC_GOTO(26),

PROC_LABEL(25),
    PROC_CALL(GameControl_EnableSoundEffects),
    PROC_CALL(sub_80C6444),

    PROC_GOTO(26),

PROC_LABEL(26),
    PROC_SLEEP(0),

    PROC_CALL(sub_8009AEC),
    PROC_SLEEP(0),

    PROC_GOTO(1),

PROC_LABEL(5),
    PROC_CALL(GameControl_EnableSoundEffects),
    PROC_CALL(Make6C_savemenu),
    PROC_SLEEP(0),

    PROC_CALL(GameControl_MasterSwitch),
    PROC_SLEEP(0),

    PROC_GOTO(7),

PROC_LABEL(6),
    PROC_CALL(GameControl_RememberChapterId),
    PROC_SLEEP(0),

    PROC_CALL(StartChapter),
    PROC_SLEEP(0),

    PROC_GOTO(9),

PROC_LABEL(7),
    PROC_CALL(sub_8009D1C),
    PROC_CALL(sub_8009D44),
    PROC_CALL(GameControl_RememberChapterId),
    PROC_CALL(sub_8009E00),
    PROC_SLEEP(0),

    PROC_START_CHILD_LOCKING(gUnknown_08A3DD50),
    PROC_CALL(GM_End),

    PROC_CALL(sub_8009E28),
    PROC_SLEEP(0),

    PROC_CALL(StartChapter),
    PROC_SLEEP(0),

    // fallthrough

PROC_LABEL(9),
    PROC_SLEEP(0),

    PROC_CALL(GameControl_PostChapterSwitch),
    PROC_SLEEP(0),

    PROC_CALL(GameControl_RestoreChapterId),

    // fallthrough

PROC_LABEL(19),
    PROC_CALL(GameControl_ChapterSwitch),

    PROC_CALL(GameControl_CallPostChapterSaveMenu),
    PROC_SLEEP(0),

    PROC_GOTO(7),

PROC_LABEL(8),
    PROC_CALL(sub_8009E28),

    PROC_CALL(GameControl_StartResumedGame),
    PROC_SLEEP(0),

    PROC_CALL(GameControl_RememberChapterId),
    PROC_CALL(sub_8009D6C),

    PROC_GOTO(9),

PROC_LABEL(14),
    PROC_SLEEP(0),

    // fallthrough

PROC_LABEL(10),
    PROC_SLEEP(0),

    PROC_GOTO(4),

PROC_LABEL(15),
    PROC_GOTO(7),

PROC_LABEL(12),
    PROC_CALL(sub_80481E0),
    PROC_SLEEP(0),

    PROC_CALL(sub_8009ABC),

    PROC_GOTO(5),

PROC_LABEL(16),
    PROC_CALL(ClearTemporaryUnits),
    PROC_CALL(GameCtrl_DeclareCompletedChapter),

    PROC_CALL(Start_savemenu2),
    PROC_SLEEP(0),

    // fallthrough

PROC_LABEL(17),
    PROC_CALL(GameCtrl_DeclareCompletedPlaythrough),

    PROC_CALL(CallGameEndingEvent),
    PROC_SLEEP(0),

    PROC_WHILE(EventEngineExists),

    PROC_CALL(sub_80B7598),
    PROC_CALL(sub_8009EFC),

    PROC_SLEEP(30),

    PROC_GOTO(4),

PROC_LABEL(18),
    PROC_CALL(sub_8009A24),

    PROC_CALL(StartFadeOutBlackMedium),
    PROC_REPEAT(WaitForFade),

    PROC_CALL(sub_8048850),

    PROC_WHILE(EventEngineExists),
    PROC_SLEEP(0),

    PROC_CALL(StartFadeInBlackMedium),
    PROC_REPEAT(WaitForFade),

    PROC_CALL(EndBG3Slider),

    PROC_GOTO(0),

    PROC_END,
};
