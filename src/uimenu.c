#include "global.h"

#include "hardware.h"
#include "m4a.h"
#include "sound.h"
#include "text.h"
#include "proc.h"
#include "bmio.h"
#include "uiutils.h"
#include "statscreen.h"

#include "uimenu.h"

// data

static CONST_DATA
struct ProcScr sProc_MenuMain[] =
{
    PROC_REPEAT(Menu_Idle),

    PROC_CALL(EndGreenText),
    PROC_END
};

static CONST_DATA
struct ProcScr sProc_Menu[] =
{
    PROC_NAME("E_Menu"),
    PROC_SLEEP(0),

    PROC_WHILE_EXISTS(gUnknown_0859A548),

    PROC_CALL(StartGreenText),
    PROC_CALL(Menu_Draw),

    PROC_CALL(Menu_CallDefinedConstructors),

    PROC_GOTO_SCR(sProc_MenuMain),
    PROC_END
};

static CONST_DATA
struct ProcScr sProc_MenuItem[] =
{
    PROC_BLOCK
};

static void Menu_AutoHelpBox_OnInit(struct MenuProc* proc);
static void Menu_AutoHelpBox_OnLoop(struct MenuProc* proc);

static CONST_DATA
struct ProcScr sProc_MenuAutoHelpBox[] =
{
    PROC_CALL(Menu_AutoHelpBox_OnInit),
    PROC_REPEAT(Menu_AutoHelpBox_OnLoop),
    PROC_END
};

static void Menu_FrozenHelpBox_OnLoop(struct MenuProc* proc);

static CONST_DATA
struct ProcScr sProc_MenuFrozenHelpBox[] =
{
    PROC_REPEAT(Menu_FrozenHelpBox_OnLoop),
    PROC_END
};

static void Menu_Frozen_OnLoop(struct MenuProc* proc);

static CONST_DATA
struct ProcScr sProc_MenuFrozen[] =
{
    PROC_REPEAT(Menu_Frozen_OnLoop),
    PROC_END
};

// functions

struct MenuProc* StartMenuAdjusted(
    const struct MenuDef* def,
    int xSubject, int xTileLeft, int xTileRight)
{
    struct MenuRect rect = def->rect;

    if (xSubject < 120)
        rect.x = xTileRight;
    else
        rect.x = xTileLeft;

    return StartMenuAt(def, rect, NULL);
}

struct MenuProc* StartMenuChild(const struct MenuDef* def, struct Proc* parent)
{
    return StartMenuAt(def, def->rect, parent);
}

struct MenuProc* StartOrphanMenuAt(const struct MenuDef* def, struct MenuRect rect)
{
    return StartMenuAt(def, rect, NULL);
}

struct MenuProc* StartMenu(const struct MenuDef* def)
{
    return StartMenuAt(def, def->rect, NULL);
}

struct MenuProc* StartOrphanMenuAdjustedExt(
    const struct MenuDef* def,
    int xSubject, int xTileLeft, int xTileRight,
    int backBg,
    int tile,
    int frontBg,
    int unk)
{
    struct MenuRect rect = def->rect;

    if (xSubject < 120)
        rect.x = xTileRight;
    else
        rect.x = xTileLeft;

    return StartMenuExt(def, rect, backBg, tile, frontBg, unk, NULL);
}

struct MenuProc* StartMenuExt2(
    const struct MenuDef* def,
    int backBg,
    int tile,
    int frontBg,
    int unk,
    struct Proc* parent)
{
    return StartMenuExt(def, def->rect, backBg, tile, frontBg, unk, parent);
}

struct MenuProc* StartOrphanMenuAtExt(
    const struct MenuDef* def,
    struct MenuRect rect,
    int backBg,
    int tile,
    int frontBg,
    int unk)
{
    return StartMenuExt(def, rect, backBg, tile, frontBg, unk, NULL);
}

struct MenuProc* StartOrphanMenuExt(
    const struct MenuDef* def,
    int backBg,
    int tile,
    int frontBg,
    int unk)
{
    return StartMenuExt(def, def->rect, backBg, tile, frontBg, unk, NULL);
}

struct MenuProc* StartMenuAt(
    const struct MenuDef* def,
    struct MenuRect rect,
    struct Proc* parent)
{
    return StartMenuExt(def, rect, 1, TILE(0, 0), 0, 0, parent);
}

struct MenuProc* StartMenuExt(
    const struct MenuDef* def,
    struct MenuRect rect,
    int backBg,
    int tile,
    int frontBg,
    int unk,
    struct Proc* parent)
{
    struct MenuProc* proc;
    int i, itemCount;

    int xTileInner = rect.x + 1;
    int yTileInner = rect.y + 1;

    SetBgOffset(frontBg, 0, 0);
    SetBgOffset(backBg, 0, 0);

    PlaySe(0x68); /* TODO: song ids! */

    if (parent)
    {
        proc = SpawnProcLocking(sProc_Menu, parent);
        proc->state = 0;
    }
    else
    {
        LockGameLogic();

        proc = SpawnProc(sProc_Menu, PROC_TREE_3);
        proc->state = MENU_STATE_GAMELOCKING;
    }

    if (rect.h < 0)
        proc->state |= MENU_STATE_NOTSHOWN;

    for (i = 0, itemCount = 0; def->menuItems[i].isAvailable; ++i)
    {
        int availability = GetOverriddenMenuCommandUsability(&def->menuItems[i], i);

        if (!availability)
            availability = def->menuItems[i].isAvailable(&def->menuItems[i], i);

        if (availability != MENU_NOTSHOWN)
        {
            struct MenuItemProc* item = SpawnProc(sProc_MenuItem, proc);
            proc->menuItems[itemCount++] = item;

            item->def = &def->menuItems[i];
            item->itemNumber = i;
            item->availability = availability;

            item->xTile = xTileInner;
            item->yTile = yTileInner;

            if (!(proc->state & MENU_STATE_NOTSHOWN))
                InitText(&item->text, rect.w - 1);

            yTileInner += 2;
        }
    }

    proc->def = def;
    proc->rect = rect;
    proc->itemCount = itemCount;
    proc->itemCurrent = 0;
    proc->itemPrevious = -1;

    if (rect.y + rect.h < yTileInner)
        proc->rect.h = yTileInner + 1 - rect.y;

    proc->backBg = backBg & 3;
    proc->tile = tile;
    proc->frontBg = frontBg & 3;
    proc->unk68 = unk;

    gKeySt->pressed = 0;

    return proc;
}

struct Proc* EndMenu(struct MenuProc* proc)
{
    struct MenuItemProc* item = proc->menuItems[proc->itemCurrent];

    proc->state |= MENU_STATE_ENDING;

    if (item->def->onSwitchOut)
        item->def->onSwitchOut(proc, item);

    if (proc->def->onEnd)
        proc->def->onEnd(proc);

    if (proc->state & MENU_STATE_GAMELOCKING)
        UnlockGameLogic();

    Proc_End(proc);

    SetBgOffset(proc->frontBg, 0, 0);
    SetBgOffset(proc->backBg, 0, 0);

    return proc->proc_parent;
}

void EndAllMenus(void)
{
    ForEachProc(sProc_Menu, (ProcFunc) EndMenu);
}

inline
void SyncMenuBgs(struct MenuProc* proc)
{
    EnableBgSync(BG_SYNC_BIT(proc->backBg) + BG_SYNC_BIT(proc->frontBg));
}

inline
void ClearMenuBgs(struct MenuProc* proc)
{
    TmFill(GetBgTilemap(proc->frontBg), 0);
    TmFill(GetBgTilemap(proc->backBg), 0);

    SyncMenuBgs(proc);
}

inline
s8 HasMenuChangedItem(struct MenuProc* proc)
{
    return proc->itemCurrent != proc->itemPrevious;
}

void Menu_CallDefinedConstructors(struct MenuProc* proc)
{
    if (proc->def->onInit)
        proc->def->onInit(proc);

    if (proc->menuItems[proc->itemCurrent]->def->onSwitchIn)
        proc->menuItems[proc->itemCurrent]->def->onSwitchIn(proc, proc->menuItems[proc->itemCurrent]);
}

void Menu_Draw(struct MenuProc* proc)
{
    int i;

    if (proc->state & MENU_STATE_NOTSHOWN)
        return;

    DrawUiFrame(
        GetBgTilemap(proc->backBg),
        proc->rect.x, proc->rect.y, proc->rect.w, proc->rect.h,
        proc->tile, proc->def->style);

    ClearUiFrame(
        GetBgTilemap(proc->frontBg),
        proc->rect.x, proc->rect.y, proc->rect.w, proc->rect.h);

    for (i = 0; i < proc->itemCount; ++i)
    {
        struct MenuItemProc* item = proc->menuItems[i];

        if (item->def->onDraw)
        {
            item->def->onDraw(proc, item);
            continue;
        }

        if (item->def->color)
            Text_SetColor(&item->text, item->def->color);

        if (item->availability == MENU_DISABLED)
            Text_SetColor(&item->text, TEXT_COLOR_SYSTEM_GRAY);

        if (!item->def->nameMsgId)
            Text_DrawString(&item->text, item->def->name);
        else
            Text_DrawString(&item->text, GetMsg(item->def->nameMsgId));

        PutText(
            &item->text,
            GetBgTilemap(proc->frontBg) + TM_OFFSET(item->xTile, item->yTile));
    }

    Menu_DrawHoverThing(proc, proc->itemCurrent, TRUE);
    SyncMenuBgs(proc);
}

void Menu_DrawHoverThing(struct MenuProc* proc, int item, s8 boolHover)
{
    int x, y, w;

    if (proc->state & MENU_STATE_FLAT)
        return;

    x = proc->rect.x + 1;
    y = proc->menuItems[item]->yTile;
    w = proc->rect.w - 2;

    switch (boolHover)
    {

    case TRUE:
        DrawUiItemHoverExt(proc->backBg, proc->tile, x, y, w);
        break;

    case FALSE:
        ClearUiItemHoverExt(proc->backBg, proc->tile, x, y, w);
        break;

    }
}

void Menu_Idle(struct MenuProc* proc)
{
    int x, y, actions;

    if (proc->state & MENU_STATE_FROZEN)
    {
        Menu_GetCursorGfxCurrentPosition(proc, &x, &y);
        DisplayFrozenUiHand(x, y);

        return;
    }

    if (proc->state & MENU_STATE_DOOMED)
    {
        EndMenu(proc);
        return;
    }

    Menu_HandleDirectionInputs(proc);
    actions = Menu_HandleSelectInputs(proc);

    if (actions & MENU_ACT_END)
        EndMenu(proc);

    if (actions & MENU_ACT_SND6A)
        PlaySe(0x6A); // TODO: song ids!

    if (actions & MENU_ACT_SND6B)
        PlaySe(0x6B); // TODO: song ids!

    if (actions & MENU_ACT_CLEAR)
        ClearMenuBgs(proc);

    if (actions & MENU_ACT_ENDFACE)
        EndFaceById(0);

    if (actions & MENU_ACT_DOOM)
        proc->state |= MENU_STATE_DOOMED;

    if (actions & MENU_ACT_SKIPCURSOR)
        return;

    if (proc->state & MENU_STATE_NOCURSOR)
        return;

    Menu_GetCursorGfxCurrentPosition(proc, &x, &y);
    Menu_UpdateMovingCursorGfxPosition(proc, &x, &y);

    DisplayUiHand(x, y);
}

void Menu_HandleDirectionInputs(struct MenuProc* proc)
{
    proc->itemPrevious = proc->itemCurrent;

    // Handle Up keyin

    if (gKeySt->repeated & DPAD_UP)
    {
        if (proc->itemCurrent == 0)
        {
            if (gKeySt->repeated != gKeySt->pressed)
                return;

            proc->itemCurrent = proc->itemCount;
        }

        proc->itemCurrent--;
    }

    // Handle down keyin

    if (gKeySt->repeated & DPAD_DOWN)
    {
        if (proc->itemCurrent == (proc->itemCount - 1))
        {
            if (gKeySt->repeated != gKeySt->pressed)
                return;

            proc->itemCurrent = -1;
        }

        proc->itemCurrent++;
    }

    // Update hover display

    if (proc->itemPrevious != proc->itemCurrent)
    {
        Menu_DrawHoverThing(proc, proc->itemPrevious, FALSE);
        Menu_DrawHoverThing(proc, proc->itemCurrent, TRUE);

        PlaySe(0x66); // TODO: song ids!
    }

    // Call def's switch in/out funcs

    if (HasMenuChangedItem(proc))
    {
        if (proc->menuItems[proc->itemPrevious]->def->onSwitchOut)
            proc->menuItems[proc->itemPrevious]->def->onSwitchOut(proc, proc->menuItems[proc->itemPrevious]);

        if (proc->menuItems[proc->itemCurrent]->def->onSwitchIn)
            proc->menuItems[proc->itemCurrent]->def->onSwitchIn(proc, proc->menuItems[proc->itemCurrent]);
    }
}

int Menu_HandleSelectInputs(struct MenuProc* proc)
{
    int result = 0;

    struct MenuItemProc* item = proc->menuItems[proc->itemCurrent];
    const struct MenuItemDef* itemDef = item->def;

    if (itemDef->onIdle)
        result = itemDef->onIdle(proc, item);

    if (gKeySt->pressed & A_BUTTON)
    {
        // A Button press

        result = GetOverriddenMenuCommandEffect(proc, item);

        if ((result == 0xFF) && itemDef->onSelected)
            result = itemDef->onSelected(proc, item);
    }
    else if (gKeySt->pressed & B_BUTTON)
    {
        // B Button press

        if (proc->def->onBPress)
            result = proc->def->onBPress(proc, item);
    }
    else if (gKeySt->pressed & R_BUTTON)
    {
        // R Button press

        if (proc->def->onRPress)
            proc->def->onRPress(proc);
    }

    return result;
}

void Menu_GetCursorGfxCurrentPosition(struct MenuProc* proc, int* xResult, int* yResult)
{
    *xResult = proc->menuItems[proc->itemCurrent]->xTile*8;
    *yResult = proc->menuItems[proc->itemCurrent]->yTile*8;

    if (proc->def->style != 0)
        *xResult -= 4;
}

u8 MenuCommandAlwaysUsable(const struct MenuItemDef* def, int number)
{
    return MENU_ENABLED;
}

u8 MenuCommandAlwaysGrayed(const struct MenuItemDef* def, int number)
{
    return MENU_DISABLED;
}

u8 MenuCommandNeverUsable(const struct MenuItemDef* def, int number)
{
    return MENU_NOTSHOWN;
}

u8 MenuCancelSelect(struct MenuProc* menu, struct MenuItemProc* item)
{
    return MENU_ACT_SKIPCURSOR | MENU_ACT_CLEAR | MENU_ACT_END | MENU_ACT_SND6B;
}

u8 MenuStdHelpBox(struct MenuProc* menu, struct MenuItemProc* item)
{
    ShowTextHelpBox(item->xTile*8, item->yTile*8, item->def->helpMsgId);
}

void Menu_AutoHelpBox_OnInit(struct MenuProc* proc)
{
    LoadHelpBoxGfx(NULL, -1); // TODO: NOPAL constant?
    proc->def->onHelpBox(proc, proc->menuItems[proc->itemCurrent]);
}

void Menu_AutoHelpBox_OnLoop(struct MenuProc* proc)
{
    int x, y;

    Menu_HandleDirectionInputs(proc);

    Menu_GetCursorGfxCurrentPosition(proc, &x, &y);
    Menu_UpdateMovingCursorGfxPosition(proc, &x, &y);

    DisplayUiHand(x, y);

    if (gKeySt->pressed & (B_BUTTON | R_BUTTON))
    {
        MoveableHelpBox_OnEnd();
        Proc_GotoScript(proc, sProc_MenuMain);

        return;
    }

    if (HasMenuChangedItem(proc))
    {
        proc->def->onHelpBox(proc, proc->menuItems[proc->itemCurrent]);
    }
}

u8 MenuCallHelp(struct MenuProc* menu)
{
    Proc_GotoScript(menu, sProc_MenuAutoHelpBox);
}

void Menu_FrozenHelpBox_OnLoop(struct MenuProc* proc)
{
    int x, y;

    Menu_GetCursorGfxCurrentPosition(proc, &x, &y);
    Menu_UpdateMovingCursorGfxPosition(proc, &x, &y);

    DisplayFrozenUiHand(x, y);

    if (gKeySt->pressed & (B_BUTTON | R_BUTTON))
    {
        MoveableHelpBox_OnEnd();
        Proc_GotoScript(proc, sProc_MenuMain);
    }
}

u8 MenuCallHelpBox(struct MenuProc* proc, int msgid)
{
    Proc_GotoScript(proc, sProc_MenuFrozenHelpBox);

    LoadHelpBoxGfx(NULL, -1); // TODO: default constants?
    ShowTextHelpBox(GetUiHandPrevDisplayX(), GetUiHandPrevDisplayY(), msgid);
}

void Menu_Frozen_OnLoop(struct MenuProc* proc)
{
    int x, y;

    Menu_GetCursorGfxCurrentPosition(proc, &x, &y);
    Menu_UpdateMovingCursorGfxPosition(proc, &x, &y);

    DisplayFrozenUiHand(x, y);

    if (gKeySt->pressed & (A_BUTTON | B_BUTTON))
        Proc_GotoScript(proc, sProc_MenuMain);
}

u8 MenuFrozen(struct MenuProc* proc)
{
    Proc_GotoScript(proc, sProc_MenuFrozen);
}

void MarkSomethingInMenu(void)
{
    struct MenuProc* proc = FindProc(sProc_Menu);

    if (proc)
        proc->state |= MENU_STATE_FROZEN;
}

void UnMarkSomethingInMenu(void)
{
    struct MenuProc* proc = FindProc(sProc_Menu);

    if (proc)
        proc->state &= ~MENU_STATE_FROZEN;
}

static CONST_DATA
u8 sItemCountYOffsetLookup[12] = { 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 3 };

struct MenuProc* StartMenu_AndDoSomethingCommands(
    const struct MenuDef* def, int xSubject, int xTileLeft, int xTileRight)
{
    struct MenuProc* result = StartMenuAdjusted(def, xSubject, xTileLeft, xTileRight);
    int i;

    if (result->itemCount <= 6)
        return result;

    result->rect.y -= sItemCountYOffsetLookup[result->itemCount];

    for (i = 0; i < result->itemCount; ++i)
        result->menuItems[i]->yTile -= sItemCountYOffsetLookup[result->itemCount];

    return result;
}

void Menu_UpdateMovingCursorGfxPosition(struct MenuProc* proc, int* xRef, int* yRef)
{
    int off;

    if (proc->itemCount <= 9)
        return;

    off = (proc->itemCount*16 - 9*16) * proc->itemCurrent / 9;

    SetBgOffset(proc->frontBg, 0, off);
    SetBgOffset(proc->backBg, 0, off);

    *yRef -= off;
}

enum
{
    MENU_OVERRIDE_NONE = 0,
    MENU_OVERRIDE_ISAVAILABLE,
    MENU_OVERRIDE_ONSELECT,
};

struct MenuItemOverride
{
    /* 00 */ short cmdid;
    /* 02 */ short kind;
    /* 04 */ void* func;
};

static
struct MenuItemOverride sMenuOverrides[MENU_OVERRIDE_MAX];

void ClearMenuCommandOverride(void)
{
    int i;

    for (i = 0; i < MENU_OVERRIDE_MAX; ++i)
        sMenuOverrides[i].kind = MENU_OVERRIDE_NONE;
}

void GetOverriddenUnusableMenuCommands(u8 list[MENU_OVERRIDE_MAX])
{
    int i;

    for (i = 0; i < MENU_OVERRIDE_MAX; ++i)
    {
        if (sMenuOverrides[i].kind && sMenuOverrides[i].func == MenuCommandNeverUsable)
            list[i] = sMenuOverrides[i].cmdid;
        else
            list[i] = MENU_ITEM_NONE;
    }
}

void SetOverriddenUnusableMenuCommands(u8 list[MENU_OVERRIDE_MAX])
{
    int i;

    for (i = 0; i < MENU_OVERRIDE_MAX; ++i)
        if (list[i])
            SetMenuCommandOverride(list[i], MENU_OVERRIDE_ISAVAILABLE, MenuCommandNeverUsable);
}

void SetMenuCommandOverride(int cmdid, int kind, void* func)
{
    struct MenuItemOverride* it = sMenuOverrides;

    while ((it->kind != 0) && !((it->kind == kind) && (it->cmdid == cmdid)))
        ++it;

    it->cmdid = cmdid;
    it->kind = kind;
    it->func = func;
}

u8 GetOverriddenMenuCommandUsability(const struct MenuItemDef* def, int number)
{
    struct MenuItemOverride* it = sMenuOverrides;

    for (; it->kind != 0; ++it)
    {
        if (it->kind != MENU_OVERRIDE_ISAVAILABLE)
            continue;

        if (it->cmdid != def->overrideId)
            continue;

        return ((MenuAvailabilityFunc)(it->func))(def, number);
    }

    return 0;
}

u8 GetOverriddenMenuCommandEffect(struct MenuProc* proc, struct MenuItemProc* item)
{
    struct MenuItemOverride* it = sMenuOverrides;

    for (; it->kind != 0; ++it)
    {
        if (it->kind != MENU_OVERRIDE_ONSELECT)
            continue;

        if (it->cmdid != item->def->overrideId)
            continue;

        return ((MenuSelectFunc)(it->func))(proc, item);
    }

    return 0xFF;
}
