
#pragma once

void RegisterDataMove(void const* src, void* dst, int length);
void RegisterDataFill(u32 value, void* dst, int length);
void ApplyDataMoves(void);

#define RegisterVramMove(src, offset, length) \
    RegisterDataMove( \
        (src), \
        (void*) VRAM + (0x1FFFF & (offset)), \
        (length))
