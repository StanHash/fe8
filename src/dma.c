
#include "global.h"

#include "hardware.h"

// TODO: find a better name for this file

struct MoveStats
{
    /* 00 */ int count;
    /* 04 */ int totalLength;
};

struct MoveEntry
{
    /* 00 */ void const* src;
    /* 04 */ void* dest;
    /* 08 */ u16 length;
    /* 0A */ u16 mode;
};

enum { MOVE_MAX = 0x20 };

enum
{
    MOVE_MODE_COPY,
    MOVE_MODE_COPY_FAST,
    MOVE_MODE_FILL_FAST,
};

struct MoveStats EWRAM_DATA gMoveStats = {};
struct MoveEntry EWRAM_DATA gMoveList[MOVE_MAX] = {};

static void ClearMoveList(void)
{
    int i;

    gMoveStats.count = 0;
    gMoveStats.totalLength = 0;

    for (i = 0; i < 32; i++)
    {
        gMoveList[i].src = NULL;
        gMoveList[i].dest = NULL;
        gMoveList[i].length = 0;
        gMoveList[i].mode = 0;
    }

    gMoveList[0].src = NULL;
}

void RegisterDataMove(void const* src, void* dst, int length)
{
    struct MoveEntry* entry = gMoveList + gMoveStats.count;

    entry->src = src;
    entry->dest = dst;
    entry->length = length;
    entry->mode = (length & 0x1F)
        ? MOVE_MODE_COPY
        : MOVE_MODE_COPY_FAST;

    gMoveStats.totalLength += length;
    gMoveStats.count++;
}

void RegisterDataFill(u32 value, void* dst, int length)
{
    struct MoveEntry* entry = gMoveList + gMoveStats.count;

    entry->src = (void const*) value;
    entry->dest = dst;
    entry->length = length;
    entry->mode = MOVE_MODE_FILL_FAST;

    gMoveStats.totalLength += length;
    gMoveStats.count++;
}

void ApplyDataMoves(void)
{
    struct MoveEntry* it = gMoveList;
    int i;

    for (i = 0; i < gMoveStats.count; ++i)
    {
        switch (it->mode)
        {

        case MOVE_MODE_COPY:
            CpuCopy16(it->src, it->dest, it->length);
            break;

        case MOVE_MODE_COPY_FAST:
            CpuFastCopy(it->src, it->dest, it->length);
            break;

        case MOVE_MODE_FILL_FAST:
            CpuFastFill((u32) it->src, it->dest, it->length);
            break;

        }

        it++;
    }

    ClearMoveList();
}
