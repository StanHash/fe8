
#include "text.h"

#include <stdarg.h>
#include <stdio.h>

#include "global.h"
#include "armfunc.h"
#include "proc.h"
#include "hardware.h"
#include "dma.h"
#include "sprite.h"

#define CHAR_NEWLINE 0x01

struct SpecialCharSt
{
    /* 00 */ s8 color;
    /* 01 */ s8 id;
    /* 02 */ s16 chrPosition;
};

struct TextPrintProc
{
    /* 00 */ PROC_HEADER;

    /* 2C */ struct Text* text;
    /* 30 */ char const* str;
    /* 34 */ s8 interval;
    /* 35 */ s8 clock;
    /* 36 */ s8 charPerTick;
};

struct Font EWRAM_DATA gDefaultFont = {};
struct Font* EWRAM_DATA gActiveFont = 0;

static u8 EWRAM_DATA sLang = 0;
static struct SpecialCharSt EWRAM_DATA sSpecialCharStList[64] = {};

int GetLang(void)
{
    return sLang;
}

void SetLang(int lang)
{
    sLang = lang;
}

void ResetText(void)
{
    InitTextFont(&gDefaultFont, (void *)(VRAM + 0x1000), 0x80, 0);
    sSpecialCharStList[0].color = -1;
}

void InitTextFont(struct Font* font, void* drawDest, int chr, int pal)
{
    if (font == NULL)
        font = &gDefaultFont;

    font->drawDest = drawDest;
    font->getDrawDest = GetTextDrawDest;
    font->pal = pal;
    font->tile = TILE(chr, pal);
    font->chrCounter = 0;
    font->lang = GetLang();

    SetTextFont(font);
    InitSystemTextFont();
}

void SetTextFontGlyphs(int glyphSet)
{
    if (glyphSet == TEXT_GLYPHS_SYSTEM)
    {
        gActiveFont->glyphs = TextGlyphs_System;
    }
    else
    {
        gActiveFont->glyphs = TextGlyphs_Talk;
    }
}

void ResetTextFont(void)
{
    gActiveFont->chrCounter = 0;
    sSpecialCharStList[0].color = -1;
}

void SetTextFont(struct Font* font)
{
    if (font == NULL)
        gActiveFont = &gDefaultFont;
    else
        gActiveFont = font;
}

void InitText(struct Text *text, int tileWidth)
{
    text->chrPosition = gActiveFont->chrCounter;
    text->tileWidth = tileWidth;
    text->dbId = 0;
    text->dbEnabled = FALSE;
    text->isPrinting = FALSE;

    gActiveFont->chrCounter += tileWidth;

    ClearText(text);
}

void InitTextDb(struct Text* text, int tileWidth)
{
    text->chrPosition = gActiveFont->chrCounter;
    text->tileWidth = tileWidth;
    text->dbId = 0;
    text->dbEnabled = TRUE;
    text->isPrinting = FALSE;

    gActiveFont->chrCounter += tileWidth * 2;
}

void InitTextList(struct TextInitInfo const* info)
{
    while (info->text != NULL)
    {
        InitText(info->text, info->tileWidth);
        info++;
    }
}

void ClearText(struct Text* text)
{
    text->x = 0;
    text->color = 0;

    CpuFastFill16(0, gActiveFont->getDrawDest(text), text->tileWidth * 2 * CHR_SIZE);
}

void ClearTextPart(struct Text* text, int tileOff, int tileWidth)
{
    void* dst = gActiveFont->drawDest + (text->chrPosition + text->dbId * text->tileWidth + tileOff) * 2 * CHR_SIZE;

    CpuFastFill16(0, dst, tileWidth * 2 * CHR_SIZE);
}

int Text_GetChrOffset(struct Text* text)
{
    return (text->chrPosition + text->dbId * text->tileWidth) * 2;
}

int Text_GetCursor(struct Text* text)
{
    return text->x;
}

void Text_SetCursor(struct Text* text, int x)
{
    text->x = x;
}

void Text_Skip(struct Text* text, int x)
{
    text->x += x;
}

void Text_SetColor(struct Text* text, int color)
{
    text->color = color;
}

int Text_GetColor(struct Text* text)
{
    return text->color;
}

void Text_SetParams(struct Text* text, int cursor, int color)
{
    text->x = cursor;
    text->color = color;
}

void PutText(struct Text* text, u16* tm)
{
    int tile = gActiveFont->tile + (text->chrPosition + text->dbId * text->tileWidth) * 2;
    int i;

    for (i = 0; i < text->tileWidth; i++)
    {
        tm[TM_OFFSET(0, 0)] = tile++;
        tm[TM_OFFSET(0, 1)] = tile++;

        tm++;
    }

    if (text->dbEnabled)
        text->dbId = text->dbId ^ 1;
}

void PutBlankText(struct Text* text, u16* tm)
{
    int i;

    for (i = 0; i < text->tileWidth; i++)
    {
        tm[TM_OFFSET(0, 0)] = 0;
        tm[TM_OFFSET(0, 1)] = 0;

        tm++;
    }
}

int GetStringTextLen(char const* str)
{
    struct Glyph const* glyph;
    char byte2, byte1;

    int result = 0;

    if (gActiveFont->lang != LANG_JAPANESE)
        return GetStringTextLenAscii(str);

    while (*str > 1)
    {
        byte1 = *str++;

        if (byte1 < 0x20)
            continue;

        byte2 = *str++;

        glyph = gActiveFont->glyphs[byte2 - 0x40];

        while (glyph)
        {
            if (glyph->sjisByte1 == byte1)
            {
                result += glyph->width;
                break;
            }

            glyph = glyph->next;
        }
    }

    return result;
}

char const* GetCharTextLen(char const* str, int* outWidth)
{
    struct Glyph const* glyph;
    char byte2, byte1;

    if (gActiveFont->lang != LANG_JAPANESE)
        return GetCharTextLenAscii(str, outWidth);

    byte1 = *str++;
    byte2 = *str++;

    glyph = gActiveFont->glyphs[byte2 - 0x40];

    while (glyph)
    {
        if (glyph->sjisByte1 == byte1)
        {
            *outWidth = glyph->width;
            break;
        }

        glyph = glyph->next;
    }

    return str;
}

int GetStringTextCenteredPos(int areaLength, char const* str)
{
    return (areaLength - GetStringTextLen(str)) / 2;
}

void GetStringTextBox(char const* str, int* outWidth, int* outHeight)
{
    *outWidth = 0;
    *outHeight = 0;

    str = sub_800A2A4();

    while (*str > 1)
    {
        int width = GetStringTextLen(str);

        if (*outWidth < width)
            *outWidth = width;

        *outHeight += 16;

        str = GetStringLineEnd(str);

        if (*str == 0)
            break;

        str++;
    }
}

char const* GetStringLineEnd(char const* str)
{
    char chr = *str;

    while (chr > 1)
    {
        str++;
        chr = *str;
    }

    return str;
}

void Text_DrawString(struct Text* text, char const* str)
{
    struct Glyph const* glyph;
    char byte2, byte1;

    if (gActiveFont->lang != LANG_JAPANESE)
    {
        Text_DrawStringAscii(text, str);
        return;
    }

    while (*str > 1)
    {
        byte1 = *str++;

        if (byte1 < 0x20)
            continue;

        byte2 = *str++;

    retry_draw:
        glyph = gActiveFont->glyphs[byte2 - 0x40];

        while (glyph)
        {
            if (glyph->sjisByte1 == byte1)
            {
                gActiveFont->drawGlyph(text, glyph);
                break;
            }

            glyph = glyph->next;

            if (glyph == NULL)
            {
                byte1 = 0x81;
                byte2 = 0xA7;

                goto retry_draw;
            }
        }
    }
}

void Text_DrawNumber(struct Text *text, int number)
{
    if (number == 0)
    {
        Text_DrawCharacter(text, "0");
        return;
    }

    while (number != 0)
    {
        u16 chr = '0' + number % 10;
        number /= 10;

        Text_DrawCharacter(text, (char const*) &chr);

        text->x -= 15;
    }
}

void Text_DrawNumberLeftAlign(struct Text* text, int number)
{
    int length;
    int r0;
    int i;

    if (number == 0)
    {
        Text_DrawCharacter(text, "0");
        text->x += 8;
        return;
    }

    length = 1;
    r0 = number / 10;
    while (r0 != 0)
    {
        length++;
        r0 /= 10;
    }

    text->x += (length - 1) * 8;

    for (i = 0; i < length; i++)
    {
        u16 c = '0' + number % 10;

        number /= 10;
        Text_DrawCharacter(text, (char *)&c);
        text->x -= 15;
    }

    text->x += length * 8 + 8;
}

void Text_DrawNumberOrBlank(struct Text* text, int number)
{
    if (number == 255 || number == -1)
    {
        Text_Skip(text, -8);
        Text_DrawString(text, GetMsg(0x535));
    }
    else
    {
        Text_DrawNumber(text, number);
    }
}

#if NONMATCHING
const char *Text_DrawCharacter(struct Text *text, const char *b)
{
    struct Glyph *r1 = NULL;
    char r3;
    char r2;

    if (gActiveFont->lang)
        return Text_DrawCharacterAscii(text, b);

    r3 = *b++;
    r2 = *b++;

    while (1)
    {
        r1 = gActiveFont->sjisByte1[r2 - 0x40];
        goto _080041BE;
      _080041BC:
        r1 = r1->next;
      _080041BE:
        if (r1 == NULL)
        {
            r3 = 0x81;
            r2 = 0xA7;
        }
        else
        {
            if (r1->sjisByte1 == r3)
            {
                gActiveFont->drawGlyph(text, r1);
                break;
            }
            goto _080041BC;
        }
    }
    return b;
}
#else
__attribute__((naked))
const char *Text_DrawCharacter(struct Text *text, const char *b)
{
    asm(".syntax unified\n\
	push {r4, r5, r6, lr}\n\
	adds r5, r0, #0\n\
	adds r4, r1, #0\n\
	ldr r0, _0800419C  @ gActiveFont\n\
	ldr r1, [r0]\n\
	ldrb r1, [r1, #0x16]\n\
	adds r6, r0, #0\n\
	cmp r1, #0\n\
	beq _080041A0\n\
	adds r0, r5, #0\n\
	adds r1, r4, #0\n\
	bl Text_DrawCharacterAscii\n\
	b _080041E2\n\
	.align 2, 0\n\
_0800419C: .4byte gActiveFont\n\
_080041A0:\n\
	ldrb r3, [r4]\n\
	adds r4, #1\n\
	ldrb r2, [r4]\n\
	adds r4, #1\n\
_080041A8:\n\
	ldr r0, [r6]\n\
	ldr r1, [r0, #4]\n\
	lsls r0, r2, #2\n\
	adds r0, r0, r1\n\
	ldr r1, _080041B8  @ 0xFFFFFF00\n\
	adds r0, r0, r1\n\
	ldr r1, [r0]\n\
	b _080041BE\n\
	.align 2, 0\n\
_080041B8: .4byte 0xFFFFFF00\n\
_080041BC:\n\
	ldr r1, [r1]\n\
_080041BE:\n\
	cmp r1, #0\n\
	bne _080041D0\n\
	movs r3, #0x81\n\
	movs r2, #0xa7\n\
	ldr r6, _080041CC  @ gActiveFont\n\
	b _080041A8\n\
	.align 2, 0\n\
_080041CC: .4byte gActiveFont\n\
_080041D0:\n\
	ldrb r0, [r1, #4]\n\
	cmp r0, r3\n\
	bne _080041BC\n\
	ldr r0, [r6]\n\
	ldr r2, [r0, #8]\n\
	adds r0, r5, #0\n\
	bl _call_via_r2\n\
	adds r0, r4, #0\n\
_080041E2:\n\
	pop {r4, r5, r6}\n\
	pop {r1}\n\
	bx r1\n\
    .syntax divided");
}
#endif

void* GetTextDrawDest(struct Text* text)
{
    int chrNumber = (text->chrPosition + text->dbId * text->tileWidth + text->x / 8);

    return gActiveFont->drawDest + chrNumber * 2 * CHR_SIZE;
}

static u16 const* CONST_DATA s2bppTo4bppLutTable[] =
{
    [TEXT_COLOR_0123] = TextColorLut_0123,
    [TEXT_COLOR_0456] = TextColorLut_0456,
    [TEXT_COLOR_0789] = TextColorLut_0789,
    [TEXT_COLOR_0ABC] = TextColorLut_0ABC,
    [TEXT_COLOR_0DEF] = TextColorLut_0DEF,
    [TEXT_COLOR_0030] = TextColorLut_0030,
    [TEXT_COLOR_4DEF] = TextColorLut_4DEF,
    [TEXT_COLOR_456F] = TextColorLut_456F,
    [TEXT_COLOR_47CF] = TextColorLut_47CF,
    [TEXT_COLOR_MASK] = TextColorLut_Mask,
    [TEXT_COLOR_4567] = TextColorLut_4567,
    [TEXT_COLOR_4DEF_B] = TextColorLut_4DEF_B,
    [TEXT_COLOR_4567_B] = TextColorLut_4567_B,
};

u16 const* GetColorLut(int color)
{
    return s2bppTo4bppLutTable[color];
}

void DrawTextGlyph(struct Text* text, struct Glyph const* glyph)
{
    u8* drawDest = gActiveFont->getDrawDest(text);
    int subx = text->x & 7;
    u32 const* bitmap = glyph->bitmap;

    DrawGlyph(GetColorLut(text->color), drawDest, bitmap, subx);
    text->x += glyph->width;
}

void DrawTextGlyphNoClear(struct Text* text, struct Glyph const* glyph)
{
    int i;

    u32* dst = (u32*) gActiveFont->getDrawDest(text);
    int subx = text->x & 7;
    u32 const* bitmap = glyph->bitmap;

    u64 bitmapRow;

    u16 const* maskLut = GetColorLut(TEXT_COLOR_MASK);
    u16 const* colorLut = GetColorLut(text->color);

    int unused;

    for (i = 0; i < 16; ++i)
    {
        // read one row of 32 bits from the bitmap
        bitmapRow = (u64) *bitmap << subx * 2;

        dst[0x00] &= maskLut[bitmapRow & 0xFF] | (maskLut[(bitmapRow >> 8) & 0xFF] << 16);
        dst[0x00] |= colorLut[bitmapRow & 0xFF] | (colorLut[(bitmapRow >> 8) & 0xFF] << 16);

        dst[0x10] &= maskLut[(bitmapRow >> 16) & 0xFF] | (maskLut[(bitmapRow >> 24) & 0xFF] << 16);
        dst[0x10] |= colorLut[(bitmapRow >> 16) & 0xFF] | (colorLut[(bitmapRow >> 24) & 0xFF] << 16);

        dst[0x20] &= maskLut[(bitmapRow >> 32) & 0xFF] | (maskLut[(bitmapRow >> 40) & 0xFF] << 16);
        dst[0x20] |= colorLut[(bitmapRow >> 32) & 0xFF] | (colorLut[(bitmapRow >> 40) & 0xFF] << 16);

        dst++;
        bitmap++;
    }

    text->x += glyph->width;
}

void InitSystemTextFont(void)
{
    ApplyPalette(Pal_SystemText, gActiveFont->pal);
    PAL_COLOR(gActiveFont->pal, 0) = 0;

    gActiveFont->drawGlyph = DrawTextGlyph;
    SetTextFontGlyphs(TEXT_GLYPHS_SYSTEM);
}

void InitTalkTextFont(void)
{
    ApplyPalette(Pal_TalkText, gActiveFont->pal);
    PAL_COLOR(gActiveFont->pal, 0) = 0;

    gActiveFont->drawGlyph = DrawTextGlyph;
    SetTextFontGlyphs(TEXT_GLYPHS_TALK);
}

void SetTextDrawNoClear(void)
{
    gActiveFont->drawGlyph = DrawTextGlyphNoClear;
}

void PutDrawText(struct Text* text, u16* tm, int color, int x, int tileWidth, const char* str)
{
    struct Text tmpText;

    if (text == NULL)
    {
        text = &tmpText;
        InitText(text, tileWidth);
    }

    Text_SetCursor(text, x);
    Text_SetColor(text, color);
    Text_DrawString(text, str);

    PutText(text, tm);
}

void Text_InsertDrawString(struct Text* text, int cursor, int color, const char* str)
{
    Text_SetCursor(text, cursor);
    Text_SetColor(text, color);
    Text_DrawString(text, str);
}

void Text_InsertDrawNumberOrBlank(struct Text* text, int cursor, int color, int number)
{
    Text_SetCursor(text, cursor);
    Text_SetColor(text, color);
    Text_DrawNumberOrBlank(text, number);
}

void Text_DrawStringAscii(struct Text *text, char const* str)
{
    while (*str != 0 && *str != CHAR_NEWLINE)
    {
        struct Glyph const* glyph = gActiveFont->glyphs[*str++];

        if (glyph == NULL)
            glyph = gActiveFont->glyphs['?'];

        gActiveFont->drawGlyph(text, glyph);
    }
}

char const* Text_DrawCharacterAscii(struct Text *text, char const* str)
{
    struct Glyph const* glyph = gActiveFont->glyphs[*str++];

    if (glyph == NULL)
        glyph = gActiveFont->glyphs['?'];

    gActiveFont->drawGlyph(text, glyph);

    return str;
}

char const* GetCharTextLenAscii(char const* str, int* outWidth)
{
    struct Glyph const* glyph = gActiveFont->glyphs[*str++];

    if (glyph == NULL)
        glyph = gActiveFont->glyphs['?'];

    *outWidth = glyph->width;

    return str;
}

int GetStringTextLenAscii(char const* str)
{
    int width = 0;

    while (*str != 0 && *str != CHAR_NEWLINE)
    {
        struct Glyph const* glyph = gActiveFont->glyphs[*str++];

        width += glyph->width;
    }

    return width;
}

void sub_8004598(void)
{
}

void InitSpriteTextFont(struct Font* font, void* drawDest, int pal)
{
    font->drawDest = drawDest;
    font->getDrawDest = GetSpriteTextDrawDest;
    font->pal = 0x10 + (pal & 0xF);
    font->tile = ((u32) drawDest & 0x1FFFF) >> 5;
    font->chrCounter = 0;
    font->lang = GetLang();

    SetTextFont(font);

    font->drawGlyph = DrawSpriteTextGlyph;
}

void InitSpriteText(struct Text* text)
{
    text->chrPosition = gActiveFont->chrCounter;
    text->tileWidth = 0x20;
    text->dbId = 0;
    text->dbEnabled = FALSE;
    text->isPrinting = FALSE;

    gActiveFont->chrCounter += 2 * 0x20;

    text->x = 0;
    text->color = 0;
}

void DrawSpriteTextBackground(struct Text* text)
{
    if (text->tileWidth == 0)
        return;

    text->x = 0;

    CpuFastFill(0x44444444, gActiveFont->getDrawDest(text),                   0x1B * CHR_SIZE);
    CpuFastFill(0x44444444, gActiveFont->getDrawDest(text) + 0x20 * CHR_SIZE, 0x1B * CHR_SIZE);
}

void DrawSpriteTextBackgroundColor0(struct Text* text)
{
    if (text->tileWidth == 0)
        return;

    text->x = 0;

    CpuFastFill(0x00000000, gActiveFont->getDrawDest(text),                   0x1B * CHR_SIZE);
    CpuFastFill(0x00000000, gActiveFont->getDrawDest(text) + 0x20 * CHR_SIZE, 0x1B * CHR_SIZE);
}

void DrawSpriteTextBackgroundExt(struct Text* text, u32 line)
{
    text->x = 0;

    CpuFastFill(line, gActiveFont->getDrawDest(text), 2 * 0x20 * CHR_SIZE);
}

void* GetSpriteTextDrawDest(struct Text* text)
{
    int chr = (text->chrPosition + text->dbId * text->tileWidth + text->x / 8);

    return gActiveFont->drawDest + chr * CHR_SIZE;
}

void DrawSpriteTextGlyph(struct Text* text, struct Glyph const* glyph)
{
    int i;

    u32* dst = (u32*) gActiveFont->getDrawDest(text);
    int subx = text->x & 7;
    u32 const* bitmap = glyph->bitmap;

    u64 bitmapRow;

    u16 const* lut = GetColorLut(text->color);

    for (i = 0; i < 8; ++i)
    {
        bitmapRow = (u64) *bitmap << subx * 2;

        dst[0x00] |= lut[bitmapRow & 0xFF] | (lut[(bitmapRow >> 8) & 0xFF] << 16);
        dst[0x08] |= lut[(bitmapRow >> 16) & 0xFF] | (lut[(bitmapRow >> 24) & 0xFF] << 16);
        dst[0x10] |= lut[(bitmapRow >> 32) & 0xFF] | (lut[(bitmapRow >> 40) & 0xFF] << 16);

        dst++;
        bitmap++;
    }

    dst = (u32*) (gActiveFont->getDrawDest(text) + 0x20 * CHR_SIZE);

    for (i = 0; i < 8; ++i)
    {
        bitmapRow = (u64) *bitmap << subx * 2;

        dst[0x00] |= lut[bitmapRow & 0xFF] | (lut[(bitmapRow >> 8) & 0xFF] << 16);
        dst[0x08] |= lut[(bitmapRow >> 16) & 0xFF] | (lut[(bitmapRow >> 24) & 0xFF] << 16);
        dst[0x10] |= lut[(bitmapRow >> 32) & 0xFF] | (lut[(bitmapRow >> 40) & 0xFF] << 16);

        dst++;
        bitmap++;
    }

    text->x += glyph->width;
}

static void TextPrint_OnLoop(struct TextPrintProc* proc)
{
    int i;

    proc->clock--;

    if (proc->clock > 0)
        return;

    proc->clock = proc->interval;

    for (i = 0; i < proc->charPerTick; ++i)
    {
        switch (*proc->str)
        {

        case 0: // end
            // fallthrough

        case 1: // newline
            proc->text->isPrinting = FALSE;
            Proc_Break(proc);

            return;

        case 4: // space?
            proc->str++;
            Text_Skip(proc->text, 6);

            break;

        default:
            proc->str = Text_DrawCharacter(proc->text, proc->str);

        }
    }
}

struct ProcScr CONST_DATA ProcScr_TextPrint[] =
{
    PROC_REPEAT(TextPrint_OnLoop),
    PROC_END,
};

char const* StartTextPrint(struct Text* text, char const* str, int interval, int charPerTick)
{
    struct TextPrintProc* proc;

    if (interval == 0)
        Text_DrawString(text, str);

    if (charPerTick == 0)
        charPerTick = 1;

    proc = SpawnProc(ProcScr_TextPrint, PROC_TREE_3);

    proc->text = text;
    proc->str = str;

    proc->charPerTick = charPerTick;
    proc->interval = interval;
    proc->clock = 0;

    text->isPrinting = TRUE;

    return GetStringLineEnd(str);
}

bool IsTextPrinting(struct Text* text)
{
    return text->isPrinting;
}

void EndTextPrinting(void)
{
    EndEachProc(ProcScr_TextPrint);
}

static void GreenText_OnLoop(void)
{
    int index = (GetGameTime() / 4) & 0xF;

    PAL_COLOR(0, 14) = *(Pal_GreenTextColors + index);
    EnablePalSync();
}

struct ProcScr ProcScr_GreenTextColor[] =
{
    PROC_END_IF_DUP,
    PROC_REPEAT(GreenText_OnLoop),
    PROC_END,
};

void StartGreenText(ProcPtr parent)
{
    if (parent != NULL)
        SpawnProc(ProcScr_GreenTextColor, parent);
    else
        SpawnProc(ProcScr_GreenTextColor, PROC_TREE_3);
}

void EndGreenText(void)
{
    EndEachProc(ProcScr_GreenTextColor);
}

void PutTextPart(struct Text* text, u16* tm, int length)
{
    int tile = gActiveFont->tile + (text->dbId * text->tileWidth + text->chrPosition) * 2;
    int i;

    for (i = 0; i < text->tileWidth && i < length; i++)
    {
        tm[TM_OFFSET(0, 0)] = tile++;
        tm[TM_OFFSET(0, 1)] = tile++;

        tm++;
    }

    if (text->dbEnabled)
        text->dbId = text->dbId ^ 1;
}

#if NONMATCHING

void DrawSpecialCharGlyph(int chrPosition, int color, struct Glyph const* glyph)
{
    int i;

    u32* dst = (u32*) (gActiveFont->drawDest + chrPosition * 2 * CHR_SIZE);
    u32 const* bitmap = glyph->bitmap;

    int lo, hi;

    u16 const* lut = GetColorLut(color);

    for (i = 0; i < 16; ++i)
    {
        lo = lut[(*bitmap) & 0xFF];
        hi = lut[(*bitmap >> 8) & 0xFF];

        *dst = lo + (hi << 16);

        dst++;
        bitmap++;
    }
}

#else

void DrawSpecialCharGlyph(int chrPosition, int color, struct Glyph const* glyph)
{
    int i;
    u32* r8 = (u32 *)(gActiveFont->drawDest + chrPosition * 64);
    u32* r7 = glyph->bitmap;
    u16 const* r2 = GetColorLut(color);

    for (i = 0; i < 16; i++)
    {
        u32 r0 = *r7++;
        register u32 r4 asm("r4") = r2[r0 & 0xFF];
        register u32 r5 asm("r5") = r2[(r0 >> 8) & 0xFF];
        register u32 var asm("r0") = (r5 << 16);
        u32 var2 = var + r4;

        *r8++ = var2;
    }
}

#endif

static int AddSpecialChar(struct SpecialCharSt* st, int color, int id)
{
    st->color = color;
    st->id = id;
    st->chrPosition = gActiveFont->chrCounter++;

    (st + 1)->color = -1;

    DrawSpecialCharGlyph(st->chrPosition, color, gUnknown_08590B44[id]);

    return st->chrPosition;
}

int GetSpecialCharChr(int color, int id)
{
    struct SpecialCharSt* it = sSpecialCharStList;

    while (TRUE)
    {
        if (it->color < 0)
            return AddSpecialChar(it, color, id);

        if (it->color == color && it->id == id)
            return it->chrPosition;

        it++;
    }
}

void PutSpecialChar(u16* tm, int color, int id)
{
    int chr;

    if (id == TEXT_SPECIAL_NOTHING)
    {
        tm[TM_OFFSET(0, 0)] = 0;
        tm[TM_OFFSET(0, 1)] = 0;

        return;
    }

    chr = GetSpecialCharChr(color, id) * 2 + gActiveFont->tile;

    tm[TM_OFFSET(0, 0)] = chr;
    tm[TM_OFFSET(0, 1)] = chr + 1;
}

void PutNumberExt(u16* tm, int color, int number, int idZero)
{
    if (number == 0)
    {
        PutSpecialChar(tm, color, idZero);
        return;
    }

    while (number != 0)
    {
        PutSpecialChar(tm, color, number % 10 + idZero);
        number /= 10;

        tm--;
    }
}

void PutNumber(u16* tm, int color, int number)
{
    PutNumberExt(tm, color, number, TEXT_SPECIAL_BIGNUM_0);
}

void PutNumberOrBlank(u16* tm, int color, int number)
{
    if (number < 0 || number == 0xFF)
        PutTwoSpecialChar(tm - 1, color, TEXT_SPECIAL_DASH, TEXT_SPECIAL_DASH);
    else
        PutNumber(tm, color, number);
}

void PutNumberTwoChr(u16* tm, int color, int number)
{
    if (number == 100)
        PutTwoSpecialChar(tm - 1, color, TEXT_SPECIAL_100_A, TEXT_SPECIAL_100_B);
    else if (number < 0 || number == 255)
        PutTwoSpecialChar(tm - 1, color, TEXT_SPECIAL_DASH, TEXT_SPECIAL_DASH);
    else
        PutNumber(tm, color, number);
}

void PutNumberSmall(u16* tm, int color, int number)
{
    PutNumberExt(tm, color, number, TEXT_SPECIAL_SMALLNUM_0);
}

void PutNumberBonus(int number, u16* tm)
{
    if (number == 0)
        return;

    PutSpecialChar(tm, TEXT_COLOR_SYSTEM_GREEN, TEXT_SPECIAL_PLUS);
    PutNumberSmall(tm + ((number >= 10) ? 2 : 1), TEXT_COLOR_SYSTEM_GREEN, number);
}

inline void PutNumber2DigitExt(u16* tm, int color, int number, int idZero)
{
    PutSpecialChar(tm, color, number % 10 + idZero);
    PutSpecialChar(tm - 1, color, (number / 10) % 10 + idZero);
}

inline void PutNumber2Digit(u16* tm, int color, int number)
{
    PutNumber2DigitExt(tm, color, number, TEXT_SPECIAL_BIGNUM_0);
}

inline void PutNumber2DigitSmall(u16* tm, int color, int number)
{
    PutNumber2DigitExt(tm, color, number, TEXT_SPECIAL_SMALLNUM_0);
}

void SpecialCharTest(void)
{
    int ix, iy;

    int cnt = GetGameTime();

    for (iy = 0; iy < 10; ++iy)
        for (ix = 0; ix < 30; ++ix)
            PutSpecialChar(gBg0Tm + TM_OFFSET(ix, iy * 2), TEXT_COLOR_SYSTEM_WHITE, (cnt++) & 1);

    EnableBgSync(BG0_SYNC_BIT);
}

void PutTime(u16* tm, int color, int time, bool alwaysDisplayPunctuation)
{
    u16 hours, minutes, seconds;
    bool hs = FormatTime(time, &hours, &minutes, &seconds);

    PutNumber(tm + 2, color, hours);
    PutNumber2Digit(tm + 5, color, minutes);
    PutNumber2DigitSmall(tm + 8, color, seconds);

    if (!hs || alwaysDisplayPunctuation)
    {
        PutSpecialChar(tm + 3, color, TEXT_SPECIAL_COLON);
        PutSpecialChar(tm + 6, color, TEXT_SPECIAL_COLON);
    }
    else
    {
        PutSpecialChar(tm + 3, color, TEXT_SPECIAL_NOTHING);
        PutSpecialChar(tm + 6, color, TEXT_SPECIAL_NOTHING);
    }
}

void PutTwoSpecialChar(u16* tm, int color, int idA, int idB)
{
    PutSpecialChar(tm++, color, idA);
    PutSpecialChar(tm++, color, idB);
}

#include "data/fonts/color_lookup_tables.h"
#include "data/fonts/glyphs_1.h"
#include "data/fonts/glyphs_2.h"
#include "data/fonts/glyphs_3.h"
