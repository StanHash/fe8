#include "global.h"
#include "m4a.h"
#include "proc.h"
#include "sound.h"

struct Struct02024E5C
{
    u8 filler0[2];
    u16 unk2;
    u16 songId;
    s8 unk6;
    s8 unk7;
    s8 maxChannels;
};

EWRAM_DATA static struct Struct02024E5C sSoundSt = {0};

static struct Proc *sMusicProc1;
static struct Proc *sMusicProc2;

struct MusicProc {
    PROC_HEADER;
    /*0x2A*/ s16 filler2A[16];
    /*0x4A*/ s16 songId;
    /*0x4C*/ s16 delayCounter; // 16
    /*0x4E*/ s16 unk4E; // 17
    /*0x50*/ s16 filler50[2];
    /*0x54*/ struct MusicPlayerInfo *player;
    /*0x58*/ s32 unk58; // 23
    /*0x5C*/ s32 unk5C; // 25
    /*0x60*/ s16 filler60[2];
    /*0x64*/ s16 unk64;
    /*0x66*/ s16 unk66;
    /*0x68*/ s16 unk68;
    /*0x6A*/ s16 unk6A;
};

static void sub_8002788(struct Proc *proc);

int GetCurrentBgmSong(void)
{
    return sSoundSt.songId;
}

bool IsBgmPlaying(void)
{
    return sSoundSt.unk6;
}

void Sound_SetVolume8002274(int volume)
{
    m4aMPlayVolumeControl(&gUnknown_03006690, 0xFFFF, volume);
    m4aMPlayVolumeControl(&gUnknown_03006760, 0xFFFF, volume);
    m4aMPlayVolumeControl(&gUnknown_030066D0, 0xFFFF, volume);
    m4aMPlayVolumeControl(&gUnknown_030063C0, 0xFFFF, volume);
    m4aMPlayVolumeControl(&gUnknown_03006400, 0xFFFF, volume);
    m4aMPlayVolumeControl(&gUnknown_03006610, 0xFFFF, volume);
    m4aMPlayVolumeControl(&gUnknown_03006720, 0xFFFF, volume);
}

void SetBgmVolume(int volume)
{
    m4aMPlayVolumeControl(&gMpi_FightBgm, 0xFFFF, volume);
    m4aMPlayVolumeControl(&gMpi_MapBgm, 0xFFFF, volume);
}

void FadeBgmOut(int speed)
{
    if (speed < 0)
        speed = 6;
    if (sMusicProc1 != NULL)
    {
        Proc_Break(sMusicProc1);
        sMusicProc1 = NULL;
    }
    if (sMusicProc2 != NULL)
    {
        Proc_Break(sMusicProc2);
        sMusicProc2 = NULL;
    }
    m4aMPlayFadeOut(&gMpi_FightBgm, speed);
    m4aMPlayFadeOut(&gMpi_MapBgm, speed);
    sSoundSt.unk6 = FALSE;
}

void Sound_FadeOut800237C(int speed)
{
    if (speed < 0)
        speed = 6;
    if (sMusicProc1 != NULL)
    {
        Proc_Break(sMusicProc1);
        sMusicProc1 = NULL;
    }
    if (sMusicProc2 != NULL)
    {
        Proc_Break(sMusicProc2);
        sMusicProc2 = NULL;
    }
    m4aMPlayFadeOut(&gMpi_FightBgm, speed);
    m4aMPlayFadeOutTemporarily(&gMpi_MapBgm, speed);
    sSoundSt.unk6 = FALSE;
    sSoundSt.unk7 = 1;
}

void SoundStuff_80023E0(int speed)
{
    if (speed == 0)
        speed = 6;
    m4aMPlayFadeOut(&gUnknown_03006690, speed);
    m4aMPlayFadeOut(&gUnknown_03006760, speed);
    m4aMPlayFadeOut(&gUnknown_030066D0, speed);
    m4aMPlayFadeOut(&gUnknown_030063C0, speed);
    m4aMPlayFadeOut(&gUnknown_03006400, speed);
    m4aMPlayFadeOut(&gUnknown_03006610, speed);
    m4aMPlayFadeOut(&gUnknown_03006720, speed);
}

void StartBgmCore(int songId, struct MusicPlayerInfo *player)
{
    sSoundSt.unk6 = TRUE;
    sSoundSt.unk7 = 0;
    sSoundSt.songId = songId;
    PlaySongCore(songId, player);
    m4aMPlayImmInit(&gMpi_FightBgm);
    m4aMPlayImmInit(&gMpi_MapBgm);
}

void StartOrChangeBgm(int songId, int speed, struct MusicPlayerInfo *player)
{
    if (sSoundSt.unk6 && GetCurrentBgmSong() == songId)
        return;
    if (gPlaySt.configBgmDisable == 0)
    {
        CancelDelaySong();
        if (sSoundSt.unk6)
        {
            FadeBgmOut(speed);
            PlaySongDelayed(songId, speed * 16, player);
        }
        else
        {
            StartBgmCore(songId, player);
        }
    }
}

void StartBgm(int songId, struct MusicPlayerInfo *player)
{
    StartOrChangeBgm(songId, 3, player);
}

void StartBgmExt(int songId, int speed, struct MusicPlayerInfo *player)
{
    StartOrChangeBgm(songId, speed, player);
}

static void sub_80024F0(struct Proc *proc)
{
    struct MusicProc *mproc = (struct MusicProc *)proc;
    int volume = Interpolate(0, 0, 0x100, mproc->delayCounter, mproc->unk4E);

    m4aMPlayVolumeControl(&gMpi_FightBgm, 0xFFFF, volume);
    m4aMPlayVolumeControl(&gMpi_MapBgm, 0xFFFF, volume);
    mproc->delayCounter++;
    if (mproc->delayCounter >= mproc->unk4E)
    {
        Proc_Break((struct Proc *)mproc);
        sMusicProc1 = NULL;
    }
}

static struct ProcScr sMusicProc1Script[] =
{
    PROC_END_DUPS,
    PROC_REPEAT(sub_80024F0),
    PROC_END,
};

void StartBgmFadeIn(int songId, int b, struct MusicPlayerInfo *player)
{
    struct MusicProc *proc;

    if (gPlaySt.configBgmDisable == 0)
    {
        sSoundSt.unk6 = TRUE;
        sSoundSt.unk7 = 0;
        sSoundSt.songId = songId;
        proc = SpawnProc(sMusicProc1Script, PROC_TREE_3);
        m4aMPlayStop(&gMpi_FightBgm);
        m4aMPlayStop(&gMpi_MapBgm);
        PlaySongCore(songId, player);
        m4aMPlayImmInit(&gMpi_FightBgm);
        m4aMPlayImmInit(&gMpi_MapBgm);
        m4aMPlayVolumeControl(&gMpi_FightBgm, 0xFFFF, 0);
        m4aMPlayVolumeControl(&gMpi_MapBgm, 0xFFFF, 0);
        proc->delayCounter = 0;
        proc->unk4E = b * 16;
        sMusicProc1 = (struct Proc *)proc;
    }
}

void OverrideBgm(int songId)
{
    if (gPlaySt.configBgmDisable == 0)
    {
        sSoundSt.unk2 = sSoundSt.songId;
        if (sSoundSt.unk7 == 0)
            m4aMPlayFadeOutTemporarily(&gMpi_MapBgm, 3);
        sSoundSt.unk6 = FALSE;
        sSoundSt.unk7 = 0;
        if (songId != 0)
            PlaySongDelayed(songId, 32, &gMpi_FightBgm);
    }
}

void RestoreBgm(void)
{
    if (gPlaySt.configBgmDisable == 0 && sSoundSt.unk2 != 0)
    {
        m4aMPlayFadeOut(&gMpi_FightBgm, 3);
        m4aMPlayFadeIn(&gMpi_MapBgm, 6);
        sSoundSt.unk6 = TRUE;
        sSoundSt.unk7 = 0;
        sSoundSt.songId = sSoundSt.unk2;
        sSoundSt.unk2 = 0;
    }
}

void sub_80026BC(u16 speed)
{
    if (gPlaySt.configBgmDisable == 0 && sSoundSt.unk2 != 0)
    {
        m4aMPlayFadeOut(&gMpi_FightBgm, 3);
        m4aMPlayFadeIn(&gMpi_MapBgm, speed);
        sSoundSt.unk6 = TRUE;
        sSoundSt.unk7 = 0;
        sSoundSt.songId = sSoundSt.unk2;
        sSoundSt.unk2 = 0;
    }
}

void MakeBgmOverridePersist(void)
{
    if (gPlaySt.configBgmDisable == 0)
    {
        sSoundSt.songId = sSoundSt.unk2;
        sSoundSt.unk2 = 0;
    }
}

struct ProcScr sMusicProc2Script[] =
{
    PROC_SLEEP(0),
    PROC_REPEAT(sub_8002788),
    PROC_END,
};

void StartBgmVolumeChange(int volumeInit, int volumeEnd, int duration, ProcPtr parent)
{
    struct MusicProc *proc;

    if (parent)
        proc = SpawnProcLocking(sMusicProc2Script, parent);
    else
        proc = SpawnProc(sMusicProc2Script, PROC_TREE_3);
    proc->unk64 = volumeInit;
    proc->unk66 = volumeEnd;
    proc->unk68 = 0;
    proc->unk6A = duration;
    if (volumeInit == 0)
        volumeInit = 1;
    SetBgmVolume(volumeInit);
    sMusicProc2 = (struct Proc *)proc;
}

static void sub_8002788(struct Proc *proc)
{
    struct MusicProc *mproc = (struct MusicProc *)proc;
    SetBgmVolume(Interpolate(4, mproc->unk64, mproc->unk66, mproc->unk68++, mproc->unk6A));
    if (mproc->unk68 >= mproc->unk6A)
    {
        if (mproc->unk66 == 0)
        {
            m4aSongNumStop(GetCurrentBgmSong());
            sSoundSt.unk6 = FALSE;
            sSoundSt.unk2 = 0;
            sSoundSt.songId = 0;
        }
        else
        {
            sSoundSt.unk6 = TRUE;
        }
        Proc_Break(proc);
        sMusicProc2 = NULL;
    }
}

void DelaySong_OnLoop(struct Proc *proc)
{
    struct MusicProc *mproc = (struct MusicProc *)proc;
    mproc->delayCounter--;
    if (mproc->delayCounter < 0)
    {
        sSoundSt.unk6 = TRUE;
        sSoundSt.songId = mproc->songId;
        PlaySongCore(mproc->songId, mproc->player);
        Proc_End((struct Proc *)proc);
    }
}

struct ProcScr CONST_DATA ProcScr_DelaySong[] =
{
    PROC_REPEAT(DelaySong_OnLoop),
    PROC_END,
};

void PlaySongDelayed(int songId, int delay, struct MusicPlayerInfo *player)
{
    if (gPlaySt.configBgmDisable == 0)
    {
        struct MusicProc *mproc = SpawnProc(ProcScr_DelaySong, PROC_TREE_3);

        mproc->delayCounter = delay;
        mproc->songId = songId;
        mproc->player = player;
    }
}

void PlaySongCore(int songId, struct MusicPlayerInfo *player)
{
    if (songId < 128)
    {
        sub_80028FC(songId);
        sub_80A3F08(0, songId);
    }

    if (player != NULL)
        MPlayStart(player, gSongTable[songId].header);
    else
        m4aSongNumStart(songId);
}

void Sound_SetDefaultMaxNumChannels(void)
{
    Sound_SetMaxNumChannels(7);
    sSoundSt.maxChannels = -1;
}

void Sound_SetMaxNumChannels(int maxchn)
{
    sSoundSt.maxChannels = maxchn;
    m4aSoundMode(maxchn << SOUND_MODE_MAXCHN_SHIFT);
}

void sub_80028FC(int songId)
{
    switch (songId)
    {
    case 1:
    case 2:
    case 0x42:
    case 0x43:
    case 0x40:
    case 0x56:
    case 0x74:
        if (sSoundSt.maxChannels != 8)
            Sound_SetMaxNumChannels(8);
        break;
    default:
        if (sSoundSt.maxChannels != -1)
            Sound_SetDefaultMaxNumChannels();
        break;
    }
}

int IsMusicProc2Running(void)
{
    if (FindProc(sMusicProc2Script) != NULL)
        return TRUE;
    else
        return FALSE;
}

void sub_800296C(struct Proc *proc)
{
    struct MusicProc *mproc = (struct MusicProc *)proc;
    if (IsBgmPlaying() != 0 && mproc->unk64 != 0)
    {
        if (mproc->unk5C == -1)
            StartBgmVolumeChange(mproc->unk64, mproc->unk66, mproc->unk58, proc);
        else
            StartBgmVolumeChange(mproc->unk64, 0, mproc->unk58, proc);
    }
}

void sub_80029BC(struct Proc *proc)
{
    struct MusicProc *mproc = (struct MusicProc *)proc;
    if (mproc->unk5C > 0)
    {
        StartBgm(mproc->unk5C, 0);
        SetBgmVolume(mproc->unk66);
    }
    else
    {
        Proc_Goto(proc, 0);
    }
}

static struct ProcScr sMusicProc4Script[] =
{
    PROC_SLEEP(1),
    PROC_CALL(sub_800296C),
    PROC_SLEEP(1),
    PROC_CALL(sub_80029BC),
    PROC_SLEEP(8),
    PROC_LABEL(0),
    PROC_SLEEP(0),
    PROC_END,
};

void sub_80029E8(int songId, int b, int c, int d, struct Proc *parent)
{
    struct MusicProc *mproc;

    if (IsBgmPlaying() != 0 && songId == sSoundSt.songId && b == c)
        return;

    if (parent != NULL)
        mproc = SpawnProcLocking(sMusicProc4Script, parent);
    else
        mproc = SpawnProc(sMusicProc4Script, PROC_TREE_3);
    mproc->unk58 = d;
    if (IsBgmPlaying() != 0 && songId == sSoundSt.songId)
        mproc->unk5C = -1;
    else
        mproc->unk5C = songId;
    mproc->unk64 = b;
    mproc->unk66 = c;
}

int sub_8002A6C(void)
{
    if (FindProc(sMusicProc4Script) != NULL)
        return TRUE;
    else
        return FALSE;
}

void sub_8002A88(int songId)
{
    if (songId != sSoundSt.songId)
    {
        if (IsBgmPlaying() != 0)
            SetBgmVolume(0);
        StartBgmCore(songId, 0);
    }
}

void CancelDelaySong(void)
{
    EndEachProc(ProcScr_DelaySong);
}

void sub_8002AC8(void)
{
    CancelDelaySong();
    m4aMPlayFadeOut(&gMpi_FightBgm, 1);
    m4aMPlayFadeOut(&gMpi_MapBgm, 1);
    sSoundSt.unk2 = 0;
    sSoundSt.songId = 0;
}
