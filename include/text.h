
#pragma once

#include "global.h"

#include "proc.h"

enum
{
    LANG_JAPANESE,
    LANG_ENGLISH,
};

enum
{
    TEXT_GLYPHS_SYSTEM,
    TEXT_GLYPHS_TALK,
};

enum
{
    TEXT_COLOR_0123 = 0,
    TEXT_COLOR_0456 = 1,
    TEXT_COLOR_0789 = 2,
    TEXT_COLOR_0ABC = 3,
    TEXT_COLOR_0DEF = 4,
    TEXT_COLOR_0030 = 5,
    TEXT_COLOR_4DEF = 6,
    TEXT_COLOR_456F = 7,
    TEXT_COLOR_47CF = 8,
    TEXT_COLOR_MASK = 9,
    TEXT_COLOR_4567 = 10,
    TEXT_COLOR_4DEF_B = 11,
    TEXT_COLOR_4567_B = 12,

    TEXT_COLOR_COUNT,

    TEXT_COLOR_SYSTEM_WHITE = TEXT_COLOR_0123,
    TEXT_COLOR_SYSTEM_GRAY  = TEXT_COLOR_0456,
    TEXT_COLOR_SYSTEM_BLUE  = TEXT_COLOR_0789,
    TEXT_COLOR_SYSTEM_GOLD  = TEXT_COLOR_0ABC,
    TEXT_COLOR_SYSTEM_GREEN = TEXT_COLOR_0DEF,

    // TEXT_COLOR_TALK_...
};

enum
{
    TEXT_SPECIAL_BIGNUM_0,
    TEXT_SPECIAL_BIGNUM_1,
    TEXT_SPECIAL_BIGNUM_2,
    TEXT_SPECIAL_BIGNUM_3,
    TEXT_SPECIAL_BIGNUM_4,
    TEXT_SPECIAL_BIGNUM_5,
    TEXT_SPECIAL_BIGNUM_6,
    TEXT_SPECIAL_BIGNUM_7,
    TEXT_SPECIAL_BIGNUM_8,
    TEXT_SPECIAL_BIGNUM_9,
    TEXT_SPECIAL_SMALLNUM_0,
    TEXT_SPECIAL_SMALLNUM_1,
    TEXT_SPECIAL_SMALLNUM_2,
    TEXT_SPECIAL_SMALLNUM_3,
    TEXT_SPECIAL_SMALLNUM_4,
    TEXT_SPECIAL_SMALLNUM_5,
    TEXT_SPECIAL_SMALLNUM_6,
    TEXT_SPECIAL_SMALLNUM_7,
    TEXT_SPECIAL_SMALLNUM_8,
    TEXT_SPECIAL_SMALLNUM_9,
    TEXT_SPECIAL_DASH,
    TEXT_SPECIAL_PLUS,
    TEXT_SPECIAL_SLASH,
    TEXT_SPECIAL_TILDE,
    TEXT_SPECIAL_S,
    TEXT_SPECIAL_A,
    TEXT_SPECIAL_B,
    TEXT_SPECIAL_C,
    TEXT_SPECIAL_D,
    TEXT_SPECIAL_E,
    TEXT_SPECIAL_G,
    TEXT_SPECIAL_K,
    TEXT_SPECIAL_COLON,
    TEXT_SPECIAL_DOT,
    TEXT_SPECIAL_HP_A,
    TEXT_SPECIAL_HP_B,
    TEXT_SPECIAL_LV_A,
    TEXT_SPECIAL_LV_B,
    TEXT_SPECIAL_ARROW,
    TEXT_SPECIAL_HEART,
    TEXT_SPECIAL_100_A,
    TEXT_SPECIAL_100_B,
    TEXT_SPECIAL_PERCENT,
    // TODO: rest

    TEXT_SPECIAL_NOTHING = 0xFF,
};

struct Glyph
{
    struct Glyph const* next; // (only used in Shift-JIS fonts) next element in linked list
    u8 sjisByte1;             // (only used in Shift-JIS fonts) second byte of character
    u8 width;                 // width of the glyph in pixels
    u32 bitmap[16];           // image data of the glyph (16x16 pixels, 2 bits per pixel)
};

struct Text
{
    /* 00 */ u16 chrPosition;
    /* 02 */ u8 x;
    /* 03 */ u8 color;
    /* 04 */ u8 tileWidth;
    /* 05 */ bool dbEnabled;
    /* 06 */ u8 dbId;
    /* 07 */ bool isPrinting;
};

struct Font
{
    /* 00 */ u8* drawDest;

    // pointer to table of glyph structs
    // In ASCII fonts, there is just one byte per character, so the glyph
    // for a given character is obtained by indexing this array.
    // In Shift-JIS fonts, each character is 2 bytes. Each element in
    // this array is a linked list. byte2 - 0x40 is the index of the head
    // of the list, and the list is traversed until a matching byte1 is found.
    /* 04 */ struct Glyph const* const* glyphs;

    /* 08 */ void (*drawGlyph)(struct Text* text, struct Glyph const* glyph);
    /* 0C */ void*(*getDrawDest)(struct Text* text);
    /* 10 */ u16 tile;
    /* 12 */ u16 chrCounter;
    /* 14 */ u16 pal;
    /* 16 */ u8 lang;
};

struct TextInitInfo
{
    struct Text* text;
    u8 tileWidth;
};

int GetLang(void);
void SetLang(int lang);
void ResetText(void);
void InitTextFont(struct Font* font, void* drawDest, int chr, int pal);
void SetTextFontGlyphs(int glyphSet);
void ResetTextFont(void);
void SetTextFont(struct Font* font);
void InitText(struct Text *text, int tileWidth);
void InitTextDb(struct Text* text, int tileWidth);
void InitTextList(struct TextInitInfo const* info);
void ClearText(struct Text* text);
void ClearTextPart(struct Text* text, int tileOff, int tileWidth);
int Text_GetChrOffset(struct Text* text);
int Text_GetCursor(struct Text* text);
void Text_SetCursor(struct Text *th, int x);
void Text_Skip(struct Text* text, int x);
void Text_SetColor(struct Text* text, int color);
int Text_GetColor(struct Text* text);
void Text_SetParams(struct Text* text, int cursor, int color);
void PutText(struct Text* text, u16* tm);
void PutBlankText(struct Text* text, u16* tm);
int GetStringTextLen(char const* str);
char const* GetCharTextLen(char const* str, int* outWidth);
int GetStringTextCenteredPos(int areaLength, char const* str);
void GetStringTextBox(const char* str, int* outWidth, int* outHeight);
char const* GetStringLineEnd(char const* str);
void Text_DrawString(struct Text* text, char const* str);
void Text_DrawNumber(struct Text *text, int number);
void Text_DrawNumberLeftAlign(struct Text* text, int number);
void Text_DrawNumberOrBlank(struct Text* text, int number);
char const* Text_DrawCharacter(struct Text* text, char const* str);
void* GetTextDrawDest(struct Text* text);
u16 const* GetColorLut(int color);
void DrawTextGlyph(struct Text* text, struct Glyph const* glyph);
void DrawTextGlyphNoClear(struct Text* text, struct Glyph const* glyph);
void InitSystemTextFont(void);
void InitTalkTextFont(void);
void SetTextDrawNoClear(void);
void PutDrawText(struct Text* text, u16* tm, int color, int x, int tileWidth, const char* str);
void Text_InsertDrawString(struct Text* text, int cursor, int color, const char* str);
void Text_InsertDrawNumberOrBlank(struct Text* text, int cursor, int color, int number);
void Text_DrawStringAscii(struct Text* text, char const* str);
char const* Text_DrawCharacterAscii(struct Text* text, char const* str);
char const* GetCharTextLenAscii(char const* str, int* outWidth);
int GetStringTextLenAscii(char const* str);
void sub_8004598(void);
void InitSpriteTextFont(struct Font* font, void* drawDest, int pal);
void InitSpriteText(struct Text* text);
void DrawSpriteTextBackground(struct Text* text);
void DrawSpriteTextBackgroundColor0(struct Text* text);
void DrawSpriteTextBackgroundExt(struct Text* text, u32 line);
void* GetSpriteTextDrawDest(struct Text* text);
void DrawSpriteTextGlyph(struct Text* text, struct Glyph const* glyph);
char const* StartTextPrint(struct Text* text, char const* str, int interval, int charPerTick);
bool IsTextPrinting(struct Text* text);
void EndTextPrinting(void);
void StartGreenText(ProcPtr parent);
void EndGreenText(void);
void PutTextPart(struct Text* text, u16* tm, int length);
void DrawSpecialCharGlyph(int chrPosition, int color, struct Glyph const* glyph);
int GetSpecialCharChr(int color, int id);
void PutSpecialChar(u16* tm, int color, int id);
void PutNumberExt(u16* tm, int color, int number, int idZero);
void PutNumber(u16* tm, int color, int number);
void PutNumberOrBlank(u16* tm, int color, int number);
void PutNumberTwoChr(u16* tm, int color, int number);
void PutNumberSmall(u16* tm, int color, int number);
void PutNumberBonus(int number, u16* tm);
void PutNumber2Digit(u16* tm, int color, int number);
void PutNumber2DigitSmall(u16* tm, int color, int number);
void PutNumber2DigitExt(u16* tm, int color, int number, int idZero);
void SpecialCharTest(void);
void PutTime(u16* tm, int color, int time, bool alwaysDisplayPunctuation);
void PutTwoSpecialChar(u16* tm, int color, int idA, int idB);
