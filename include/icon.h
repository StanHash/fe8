
#pragma once

#include "global.h"

void InitIcons(void);
void ClearIcons(void);
void ApplyIconPalettes(int pal);
void ApplyIconPalette(int num, int pal);
int CountActiveIcons(void);
u16 IconSlot2Chr(int num);
int GetNewIconSlot(int icon);
int GetIconChr(int icon);
void PutIcon(u16* tm, int icon, int tile);
void ClearIcon(int icon);
void PutIconObjImg(int icon, int chr);

extern u16 const Pal_Icons[2][16];
extern u8 const Img_Icons[];
