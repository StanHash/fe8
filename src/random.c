
#include "random.h"

static u16 sRandStA[3];
static int sRandStB;

int NextRn(void)
{
    // This generates a pseudorandom string of 16 bits
    // In other words, a pseudorandom integer that can range from 0 to 65535

    u16 rn = (sRandStA[1] << 11) + (sRandStA[0] >> 5);

    // Shift state[2] one bit
    sRandStA[2] = sRandStA[2] << 1;

    // "carry" the top bit of state[1] to state[2]
    if (sRandStA[1] & 0x8000)
        sRandStA[2]++;

    rn ^= sRandStA[2];

    // Shifting the whole state 16 bits
    sRandStA[2] = sRandStA[1];
    sRandStA[1] = sRandStA[0];
    sRandStA[0] = rn;

    return rn;
}

void RandInit(int seed)
{
    // This table is a collection of 8 possible initial rn state
    // 3 entries will be picked based of which "seed" was given

    u16 initTable[8] =
    {
        0xA36E,
        0x924E,
        0xB784,
        0x4F67,
        0x8092,
        0x592D,
        0x8E70,
        0xA794
    };

    int mod = seed % 7;

    sRandStA[0] = initTable[(mod++ & 7)];
    sRandStA[1] = initTable[(mod++ & 7)];
    sRandStA[2] = initTable[(mod & 7)];

    if ((seed % 23) > 0)
        for (mod = seed % 23; mod != 0; mod--)
            NextRn();
}

void RandSetSt(u16 const* st)
{
    sRandStA[0] = *st++;
    sRandStA[1] = *st++;
    sRandStA[2] = *st++;
}

void RandGetSt(u16* st)
{
    *st++ = sRandStA[0];
    *st++ = sRandStA[1];
    *st++ = sRandStA[2];
}

int RandNext_100(void)
{
    // take the next rn (range 0-0xFFFF) and convert it to a range 0-99 value
    return NextRn() * 100 / 0x10000;

    /*

    fun fact! FE6 does (NextRn() / (0x10000 / 100)) instead of the above, resulting
    in a very slight chance of getting a 100 roll because of integer division rounding.

    */
}

int RandNext(int max)
{
    // take the next rn (range 0-0xFFFF) and convert it to a range 0-(max-1) value
    return NextRn() * max / 0x10000;
}

bool RandRoll(int threshold)
{
    return (threshold > RandNext_100());
}

bool RandRoll2Rn(int threshold)
{
    int average = (RandNext_100() + RandNext_100()) / 2;

    return (threshold > average);
}

// the second implementation of RN is an LCG (Linear Congruental Generator),
// where sRandStB is set to a seed and then advanced and retrieved on demand.

void RandInitB(int seed)
{
    sRandStB = seed;
}

// The LCGRN state is advanced and retrieved here.
unsigned RandNextB(void)
{
    u32 rn = (sRandStB * 4 + 2);
    rn *= (sRandStB * 4 + 3);
    sRandStB = rn >> 2;
    return sRandStB;
}
