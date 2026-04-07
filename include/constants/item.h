#ifndef GUARD_ITEM_CONSTANTS_H
#define GUARD_ITEM_CONSTANTS_H

// 1-based pocket IDs used in item data (.pocket field in struct Item)
// These identify the REAL storage pockets and must not be renumbered.
#define POCKET_NONE        0
#define POCKET_ITEMS       1
#define POCKET_POKE_BALLS  2
#define POCKET_TM_HM       3
#define POCKET_BERRIES     4
#define POCKET_KEY_ITEMS   5

// Real storage pocket count (used for gBagPockets[] array)
#define POCKETS_COUNT      5

// 0-based virtual pocket IDs (UI display order)
// Items pocket is split into 4 sub-categories at the UI level.
// BATTLE_POCKET is shown to the player as "HOLD ITEMS".
#define ITEMS_POCKET       0
#define MEDICINE_POCKET    1
#define BATTLE_POCKET      2
#define BALLS_POCKET       3
#define BERRIES_POCKET     4
#define TREASURES_POCKET   5
#define TMHM_POCKET        6
#define KEYITEMS_POCKET    7
#define VIRTUAL_POCKETS_COUNT 8

#endif // GUARD_ITEM_CONSTANTS_H
