
#pragma once

#include "global.h"

enum
{
    IRQ_VBLANK  = 0,
    IRQ_HBLANK  = 1,
    IRQ_VMATCH  = 2,
    IRQ_TIMER0  = 3,
    IRQ_TIMER1  = 4,
    IRQ_TIMER2  = 5,
    IRQ_TIMER3  = 6,
    IRQ_SERIAL  = 7,
    IRQ_DMA0    = 8,
    IRQ_DMA1    = 9,
    IRQ_DMA2    = 10,
    IRQ_DMA3    = 11,
    IRQ_KEYPAD  = 12,
    IRQ_GAMEPAK = 13,

    IRQ_COUNT,
};

typedef void (*IrqFunc)(void);

void IrqMain(void);

void IrqInit(void);
void SetIrqFunc(int num, IrqFunc func);
