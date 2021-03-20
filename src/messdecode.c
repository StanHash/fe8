#include "global.h"

#include "armfunc.h"
#include "proc.h"
#include "bmio.h"
#include "bmitem.h"
#include "unit.h"

const char *gUnknown_08591AA4[][2] =
{
    {"a ", "A "},
    {"an ", "An "},
};

const char *StrGetGenericArticle(s8 *a, s8 b)
{
    switch (a[0])
    {
    case 'A':
    case 'E':
    case 'I':
    case 'O':
    case 'U':
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
        return gUnknown_08591AA4[1][b];
    default:
        return gUnknown_08591AA4[0][b];
    }
}

void PrependArticleToString(char *a, const char *b, s8 c)
{
    const char *r6;
    u8 r5;
    s16 i;

    if (b == NULL)
        r6 = StrGetGenericArticle(a, c);
    else
        r6 = b;
    r5 = strlen(r6);
    for (i = strlen(a); i >= 0; i--)
        a[i + r5] = a[i];
    for (i = 0; i < r5; i++)
        a[i] = r6[i];
}

void StrStripSomeEndCharacters(s8 *a)
{
    s16 r3 = 0;
    u8 r1;

    while (a[r3] != 0)
    {
        r1 = a[r3];
        if (r1 == 0x10)
            r3 += 2;
        if (r1 == 0x80)
            r3 += 1;
        r3++;
    }

    r3--;
    while (r3 >= 0)
    {
        r1 = a[r3];
        if (r1 != 0x1F)
            return;
        r1 = a[r3 - 1];
        if (r1 != 0x80)
            a[r3] = 0;
        r3--;
    }
}

char *GetMsg(int index)
{
    if (index == gCurrentTextIndex)
        return gCurrentTextString.buffer0202A6AC;
    DecodeString(gUnknown_0815D48C[index], gCurrentTextString.buffer0202A6AC);
    StrStripSomeEndCharacters(gCurrentTextString.buffer0202A6AC);
    gCurrentTextIndex = index;
    return gCurrentTextString.buffer0202A6AC;
}

char *GetMsgTo(int index, char *buffer)
{
    DecodeString(gUnknown_0815D48C[index], buffer);
    StrStripSomeEndCharacters(buffer);
    return buffer;
}

char *StrExpandNames(void)
{
    u8 *r5 = gCurrentTextString.buffer0202AC01;
    u8 *r4 = gCurrentTextString.buffer0202B156;

    StringCopy(r5, gCurrentTextString.buffer0202A6AC);
    while (*r5 != 0)
    {
        if (*r5 < 32)
        {
            *r4++ = *r5++;
        }
        else if (*r5 != 0x80)
        {
            *r4++ = *r5++;
        }
        else
        {
            int r1;

            r5++;
            switch (*r5)
            {
            case 0x12:
                r1 = 0;
                break;
            case 0x13:
                r1 = 1;
                break;
            case 0x14:
                r1 = 2;
                break;
            case 0x15:
                r1 = 3;
                break;
            case 0x20:
                StringCopy(r4, GetTacticianNameString());
                goto label;
            case 0x22:
                StringCopy(r4, GetItemName(gAction.unk6));
                goto label;
            default:
                *r4++ = 0x80;
                *r4++ = *r5++;
                continue;
            }
            StringCopy(r4, GetMsg(GetPInfo(gPlaySt.unk1C[r1])->msgName));
        label:
            while (*r4 != 0)
                r4++;
            r5++;
        }
    }
    *r4 = 0;
    return gCurrentTextString.buffer0202B156;
}

#if NONMATCHING
void *StrExpandTact(void)
{
    u8 *r5 = gCurrentTextString.buffer0202B4AC;
    u8 *r4 = gCurrentTextString.buffer0202B5AC;

    StringCopy(r5, gCurrentTextString.buffer0202A6AC);
    while (*r5 != 0)
    {
        u8 r1 = *r5;

        if (r1 < 32)
        {
            *r4++ = *r5++;
        }
        else if (r1 != 0x80)
        {
            *r4++ = *r5++;
        }
        else
        {
            r5++;
            if (*r5 != 0x20)
            {
                *r4++ = r1;
                *r4++ = *r5++;
            }
            else
            {
                //_0800A3F8
                StringCopy(r4, GetTacticianNameString());
                while (*r4 != 0)
                    r4++;
                r5++;
            }
        }
    }
    *r4 = 0;
    return gCurrentTextString.buffer0202B5AC;
}
#else
__attribute__((naked))
char *StrExpandTact(void)
{
    asm(".syntax unified\n\
    push {r4, r5, lr}\n\
    ldr r5, _0800A3D0  @ gUnknown_0202B4AC\n\
    movs r0, #0x80\n\
    lsls r0, r0, #1\n\
    adds r4, r5, r0\n\
    ldr r0, _0800A3D4  @ 0xFFFFF200\n\
    adds r1, r5, r0\n\
    adds r0, r5, #0\n\
    bl StringCopy\n\
    b _0800A416\n\
    .align 2, 0\n\
_0800A3D0: .4byte gUnknown_0202B4AC\n\
_0800A3D4: .4byte 0xFFFFF200\n\
_0800A3D8:\n\
    adds r1, r0, #0\n\
    cmp r1, #0x1f\n\
    bls _0800A3F0\n\
    cmp r1, #0x80\n\
    bne _0800A3F0\n\
    adds r5, #1\n\
    ldrb r0, [r5]\n\
    cmp r0, #0x20\n\
    beq _0800A3F8\n\
    strb r1, [r4]\n\
    adds r4, #1\n\
    ldrb r0, [r5]\n\
_0800A3F0:\n\
    strb r0, [r4]\n\
    adds r5, #1\n\
    adds r4, #1\n\
    b _0800A416\n\
_0800A3F8:\n\
    bl GetTacticianNameString\n\
    adds r1, r0, #0\n\
    adds r0, r4, #0\n\
    bl StringCopy\n\
    ldrb r0, [r4]\n\
    adds r1, r5, #1\n\
    cmp r0, #0\n\
    beq _0800A414\n\
_0800A40C:\n\
    adds r4, #1\n\
    ldrb r0, [r4]\n\
    cmp r0, #0\n\
    bne _0800A40C\n\
_0800A414:\n\
    adds r5, r1, #0\n\
_0800A416:\n\
    ldrb r0, [r5]\n\
    cmp r0, #0\n\
    bne _0800A3D8\n\
    movs r0, #0\n\
    strb r0, [r4]\n\
    ldr r0, _0800A428  @ gUnknown_0202B5AC\n\
    pop {r4, r5}\n\
    pop {r1}\n\
    bx r1\n\
    .align 2, 0\n\
_0800A428: .4byte gUnknown_0202B5AC\n\
    .syntax divided");
}
#endif
