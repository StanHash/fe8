
#pragma once

#include "global.h"

#include "proc.h"

#include "m4a.h"

int GetCurrentBgmSong(void);
bool IsBgmPlaying(void);
void Sound_SetVolume8002274(int volume);
void SetBgmVolume(int volume);
void FadeBgmOut(int speed);
// ??? Sound_FadeOut800237C(???);
// ??? SoundStuff_80023E0(???);
void StartBgmCore(int songId, struct MusicPlayerInfo* mpi);
void StartOrChangeBgm(int songId, int speed, struct MusicPlayerInfo* mpi);
void StartBgm(int songId, struct MusicPlayerInfo* mpi);
void StartBgmExt(int songId, int speed, struct MusicPlayerInfo* mpi);
// ??? sub_80024F0(???);
void StartBgmFadeIn(int songId, int b, struct MusicPlayerInfo* mpi);
void OverrideBgm(int songId);
void RestoreBgm(void);
// ??? sub_80026BC(???);
void MakeBgmOverridePersist(void);
void StartBgmVolumeChange(int volumeInit, int volumeEnd, int duration, ProcPtr parent);
// ??? sub_8002788(???);
// ??? DelaySong_OnLoop(???);
void PlaySongDelayed(int songId, int delay, struct MusicPlayerInfo *player);
void PlaySongCore(int songId, struct MusicPlayerInfo *player);
void Sound_SetDefaultMaxNumChannels(void);
void Sound_SetMaxNumChannels(int maxchn);
void sub_80028FC(int songId);
// ??? IsMusicProc2Running(???);
// ??? sub_800296C(???);
// ??? sub_80029BC(???);
// ??? sub_80029E8(???);
// ??? sub_8002A6C(???);
// ??? sub_8002A88(???);
void CancelDelaySong(void);
// ??? sub_8002AC8(???);

#define PlaySe(id) \
    if (!gRAMChapterData.configSeDisable) \
        m4aSongNumStart((id))
