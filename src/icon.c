
#include "icon.h"

#include "hardware.h"
#include "dma.h"

enum { MAX_ICON_COUNT = 224 };
enum { MAX_ICON_DISPLAY_COUNT = 0x20 };

struct IconSt
{
    /* 00 */ u8 refCount;
    /* 01 */ u8 dispId;
};

static EWRAM_DATA struct IconSt sIconStTable[MAX_ICON_COUNT] = {};
static EWRAM_DATA u8 sIconDisplayList[MAX_ICON_DISPLAY_COUNT] = {};

void InitIcons(void)
{
    ClearIcons();
}

void ClearIcons(void)
{
    CpuFill16(0, &sIconStTable, sizeof sIconStTable);
    CpuFill16(0, &sIconDisplayList, sizeof sIconDisplayList);
}

void ApplyIconPalettes(int pal)
{
    ApplyPalettes(Pal_Icons[0], pal, 2);
}

void ApplyIconPalette(int num, int pal)
{
    ApplyPalette(Pal_Icons[num], pal);
}

int CountActiveIcons(void)
{ 
    int i, result = 0;

    for (i = MAX_ICON_DISPLAY_COUNT - 1; i >= 0; i--)
    {
        if (sIconDisplayList[i] != 0)
            result++;
    }

    return result;
}

u16 IconSlot2Chr(int num)
{
    // TODO: constants for VRAM chr allocation
    return 0x300 - num * 4;
}

int GetNewIconSlot(int icon)
{
    int i;

    for (i = 0; i < MAX_ICON_DISPLAY_COUNT; ++i)
    {
        if (sIconDisplayList[i] == 0)
        {
            sIconDisplayList[i] = icon + 1;
            return i;
        }
    }

    return -1;
}

int GetIconChr(int icon)
{
    if (sIconStTable[icon].dispId != 0)
    {
        if (sIconStTable[icon].refCount < UINT8_MAX)
            sIconStTable[icon].refCount++;

        return IconSlot2Chr(sIconStTable[icon].dispId);
    }

    sIconStTable[icon].refCount++;
    sIconStTable[icon].dispId = GetNewIconSlot(icon) + 1;

    RegisterVramMove(
        Img_Icons + (icon * CHR_SIZE * 4),
        VRAM + CHR_SIZE * IconSlot2Chr(sIconStTable[icon].dispId), CHR_SIZE * 4);

    return IconSlot2Chr(sIconStTable[icon].dispId);
}

void PutIcon(u16* tm, int icon, int tile)
{
    if (icon < 0)
    {
        tm[TM_OFFSET(0, 0)] = 0;
        tm[TM_OFFSET(1, 0)] = 0;
        tm[TM_OFFSET(0, 1)] = 0;
        tm[TM_OFFSET(1, 1)] = 0;
    }
    else
    {
        u16 fulltile = GetIconChr(icon) + tile;

        tm[TM_OFFSET(0, 0)] = fulltile++;
        tm[TM_OFFSET(1, 0)] = fulltile++;
        tm[TM_OFFSET(0, 1)] = fulltile++;
        tm[TM_OFFSET(1, 1)] = fulltile;
    }
}

void ClearIcon(int icon)
{
    sIconDisplayList[sIconStTable[icon].dispId - 1] = 0;
    sIconStTable[icon].dispId = 0;
}

void PutIconObjImg(int icon, int chr)
{
    u8 const* src;
    u8* dst;

    dst = OBJ_VRAM0;
    dst += CHR_SIZE * (chr & 0x3FF);

    if (icon < 0)
    {
        RegisterDataFill(0, dst,         CHR_SIZE * 2);
        RegisterDataFill(0, dst + 0x400, CHR_SIZE * 2);
    }
    else
    {
        src = Img_Icons;
        src += CHR_SIZE * 4 * icon;

        RegisterDataMove(src,                dst,         CHR_SIZE * 2);
        RegisterDataMove(src + CHR_SIZE * 2, dst + 0x400, CHR_SIZE * 2);
    }
}
