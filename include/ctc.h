#ifndef GUARD_CTC_H
#define GUARD_CTC_H

extern u16 CONST_DATA gObject_8x8[];
extern u16 CONST_DATA gObject_16x16[];
extern u16 CONST_DATA gObject_32x32[];
extern u16 CONST_DATA gObject_64x64[]; // Unused
extern u16 CONST_DATA gObject_8x16[];
extern u16 CONST_DATA gObject_16x32[];
extern u16 CONST_DATA gObject_32x64[]; // Unused
extern u16 CONST_DATA gObject_16x8[];
extern u16 CONST_DATA gObject_16x8_VFlipped[]; // Unused
extern u16 CONST_DATA gObject_32x16[];
extern u16 CONST_DATA gObject_64x32[];
extern u16 CONST_DATA gObject_32x8[];
extern u16 CONST_DATA gObject_8x32[];
extern u16 CONST_DATA gObject_32x8_VFlipped[]; // Unused
extern u16 CONST_DATA gObject_8x16_HFlipped[];
extern u16 CONST_DATA gObject_8x8_HFlipped[];
extern u16 CONST_DATA gObject_8x8_VFlipped[]; // Unused
extern u16 CONST_DATA gObject_8x8_HFlipped_VFlipped[]; // Unused
extern u16 CONST_DATA gObject_16x16_VFlipped[];

void PutObjectAffine(int id, int pa, int pb, int pc, int pd);
void ClearSprites(void);
void PutSprite(int layer, int x, int y, const u16* object, int oam2);
void PutSpriteExt(int layer, int xOam1, int yOam0, const u16* object, int oam2);
void PushSpriteLayerObjects(int layer);

// ??? StartSpriteRefresher(???);
// ??? MoveSpriteRefresher(???);

#endif  // GUARD_CTC_H
