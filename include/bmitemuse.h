#ifndef GUARD_ITEMUSE_H
#define GUARD_ITEMUSE_H

s8 CanUnitUseItem(struct Unit* unit, int item);
int GetItemCantUseMsgid(struct Unit* unit, int item);
void ItemEffect_Call(struct Unit* unit, int item);
s8 CanUseHealingItem(struct Unit* unit);
s8 sub_802909C(struct Unit* unit);
s8 CanUsePureWater(struct Unit* unit);
s8 CanUseTorch(struct Unit* unit);
s8 CanUseAntidote(struct Unit* unit);
s8 CanUseChestKey(struct Unit* unit);
s8 CanUseDoorKey(struct Unit* unit);
s8 CanOpenBridge(struct Unit* unit);
s8 CanUseLockpick(struct Unit* unit);
s8 CanUsePromotionItem(struct Unit* unit, int item);
s8 CanUseStatBooster(struct Unit* unit, int item);
s8 CanUseJunaFruit(struct Unit* unit);
s8 CanUnitUseItemPrepScreen(struct Unit* unit, int item);
s8 sub_802A108(struct Unit* unit);

u8 RepairMenuItemIsAvailable(const struct MenuItemDef*, int number);
int RepairMenuItemDraw(struct MenuProc*, struct            MenuItemProc*);
u8 RepairMenuItemSelect(struct MenuProc*, struct          MenuItemProc*);
int RepairMenuItemOnChange(struct MenuProc*, struct        MenuItemProc*);
int nullsub_24(struct MenuProc*, struct MenuItemProc*);

#endif // GUARD_ITEMUSE_H
