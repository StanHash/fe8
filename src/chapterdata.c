#include "global.h"
#include "chapterdata.h"
#include "hardware.h"
#include "agb_sram.h"

const struct ChapterInfo* GetChapterInfo(unsigned chapter) {
    if (chapter == 0x7F)
        return (const struct ChapterInfo*) gUnknown_08A1FB34[0];

    return gChapterDataTable + chapter;
}

const void* GetChapterMapData(unsigned chapter) {
    if (chapter != 0x7F)
        return gChapterDataAssetTable[GetChapterInfo(chapter)->mapMainLayerId];

    ReadSramFast(GetTrialMapSavedMapAddress(), gBuf, GetTrialMapSavedMapSize());
    return gBuf;
}

const void* GetChapterMapChanges(unsigned chapter) {
    if (chapter != 0x7F)
        return gChapterDataAssetTable[GetChapterInfo(chapter)->mapChangeLayerId];

    return gUnknown_08A1FB34[1];
}

const void* GetChapterEventInfo(unsigned chapter) {
    if (chapter != 0x7F)
        return gChapterDataAssetTable[GetChapterInfo(chapter)->mapEventDataId];

    return gUnknown_08A1FB34[2];
}

const char* sub_80346E0(unsigned chapter) {
    if (chapter != 0x7F)
        // ???????????????????
        return GetMsg((int)(&GetChapterInfo(chapter)->unk70));

    return gUnknown_08A1FB34[3];
}

int GetRankingRefId(void) {
    u8 difficultState = gPlaySt.chapterStateBits & CHAPTER_FLAG_DIFFICULT;
    return difficultState ? 1 : 0;
}
