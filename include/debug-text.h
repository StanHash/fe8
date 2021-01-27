
#pragma once

#include "global.h"

void DebugInitBg(int bg, int vramOffset);
void DebugPutStr(u16* tm, char const* str);
void DebugPutFmt(u16* tm, char const* fmt, ...);
void DebugScreenInit(void);
void DebugPrintFmt(char const* fmt, ...);
void ClearNumberStr(void);
void GenNumberStr(int number);
void GenNumberOrBlankStr(int number);
void DebugPrintNumber(int number, int length);
void GenNumberHexStr(int number);
void DebugPrintNumberHex(int number, int length);
void DebugPrintStr(char const* str);
void DebugPutScreen(void);
bool DebugUpdateScreen(u16 held, u16 pressed);
void DebugInitObj(int offset, int pal);
void DebugPutObjStr(int x, int y, char const* str);
void DebugPutObjNumber(int x, int y, int number, int length);
void DebugPutObjNumberHex(int x, int y, int number, int length);
void DebugPutObjFmt(short x, short y, char const* fmt, ...);

extern char EWRAM_DATA gNumberStr[9];
