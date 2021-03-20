
#include "gold.h"

enum { MAX_GOLD = 999999 };

int GetGold(void)
{
    if (gPlaySt.chapter == 5) // TODO: chapter number constants
        return 0;

    return gPlaySt.gold;
}

void SetGold(int amount)
{
    gPlaySt.gold = amount;

    if (gPlaySt.gold > MAX_GOLD)
        gPlaySt.gold = MAX_GOLD;
}

void AddGold(int amount)
{
    gPlaySt.gold += amount;

    if (gPlaySt.gold > MAX_GOLD)
        gPlaySt.gold = MAX_GOLD;
}
