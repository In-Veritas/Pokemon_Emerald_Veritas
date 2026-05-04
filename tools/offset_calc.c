#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

// GBA types - must come before everything
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef u8 bool8;
typedef u32 bool32;
typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile s16 vs16;
typedef volatile s32 vs32;

// Guard GBA headers - we already defined the types above
#define GUARD_GBA_GBA_H
#define GUARD_GBA_TYPES_H
#define GUARD_GBA_DEFINES_H
#define GUARD_GBA_IO_REG_H
#define GUARD_GBA_MULTIBOOT_H
#define GUARD_GBA_SYSCALL_H
#define GUARD_GBA_ISAGBPRINT_H
#define GUARD_GBA_FLASH_INTERNAL_H
#define GUARD_GBA_MACRO_H
#define GUARD_GBA_DMA_H
#define GUARD_GBA_M4A_INTERNAL_H

// Stub OamData (needed by sprite.h)
struct OamData { u32 data[2]; };

// Stub macros
#define EWRAM_DATA
#define ALIGNED(x) __attribute__((aligned(x)))
#define NAKED
#define INCBIN(...) {0}
#define INCBIN_U8 INCBIN
#define INCBIN_U16 INCBIN
#define INCBIN_U32 INCBIN
#define INCBIN_S8 INCBIN
#define INCBIN_S16 INCBIN
#define INCBIN_S32 INCBIN
#define INCBIN_COMP INCBIN
#define _(x) {x}
#define __(x) {x}
#define asm_unified(x)
#define BLOCK_CROSS_JUMP

// Stub function pointer types
typedef void (*IntrFunc)(void);
typedef void (*MainCallback)(void);

// GBA hardware register stubs
#define REG_BASE 0x04000000
#define REG_OFFSET_BLDALPHA 0x52
#define REG_ADDR_BLDALPHA ((vu16*)(REG_BASE + REG_OFFSET_BLDALPHA))
#define REG_BLDALPHA (*(vu16*)0)

#include "config.h"
#include "constants/global.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/species.h"
#include "constants/pokedex.h"
#include "constants/berry.h"
#include "constants/pokemon.h"
#include "constants/easy_chat.h"
#include "constants/maps.h"
#include "constants/trainer_hill.h"

#include "global.h"

int main(void) {
    printf("=== SaveBlock2 ===\n");
    printf("sizeof(SaveBlock2) = 0x%04lX (%lu)\n", (unsigned long)sizeof(struct SaveBlock2), (unsigned long)sizeof(struct SaveBlock2));
    printf("encryptionKey: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock2, encryptionKey));
    printf("_saveSentinel: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock2, _saveSentinel));
    printf("saveVersion: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock2, saveVersion));
    printf("playerLookStyle: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock2, playerLookStyle));

    printf("\n=== SaveBlock1 ===\n");
    printf("sizeof(SaveBlock1) = 0x%04lX (%lu)\n", (unsigned long)sizeof(struct SaveBlock1), (unsigned long)sizeof(struct SaveBlock1));
    printf("money: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, money));
    printf("coins: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, coins));
    printf("pcItems: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, pcItems));
    printf("bagPocket_Items: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, bagPocket_Items));
    printf("pokeblocks: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, pokeblocks));
    printf("seen1: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, seen1));
    printf("objectEvents: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, objectEvents));
    printf("flags: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, flags));
    printf("vars: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, vars));
    printf("gameStats: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, gameStats));
    printf("berryTrees: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, berryTrees));
    printf("secretBases: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, secretBases));
    printf("tvShows: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, tvShows));
    printf("pokeNews: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, pokeNews));
    printf("oldMan: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, oldMan));
    printf("dewfordTrends: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, dewfordTrends));
    printf("contestWinners: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, contestWinners));
    printf("daycare: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, daycare));
    printf("linkBattleRecords: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, linkBattleRecords));
    printf("giftRibbons: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, giftRibbons));
    printf("roamer: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, roamer));
    printf("mysteryGift: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, mysteryGift));
    printf("ramScript: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, ramScript));
    printf("recordMixingGift: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, recordMixingGift));
    printf("seen2: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, seen2));
    printf("lilycoveLady: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, lilycoveLady));
    printf("trainerNameRecords: 0x%04lX\n", (unsigned long)offsetof(struct SaveBlock1, trainerNameRecords));

    printf("\n=== Key struct sizes ===\n");
    printf("sizeof(Pokemon): %lu\n", (unsigned long)sizeof(struct Pokemon));
    printf("sizeof(ItemSlot): %lu\n", (unsigned long)sizeof(struct ItemSlot));
    printf("sizeof(SecretBase): %lu\n", (unsigned long)sizeof(struct SecretBase));
    printf("sizeof(TVShow): %lu\n", (unsigned long)sizeof(TVShow));
    printf("sizeof(PokeNews): %lu\n", (unsigned long)sizeof(PokeNews));
    printf("sizeof(OldMan): %lu\n", (unsigned long)sizeof(OldMan));
    printf("sizeof(ContestWinner): %lu\n", (unsigned long)sizeof(struct ContestWinner));
    printf("sizeof(LinkBattleRecords): %lu\n", (unsigned long)sizeof(struct LinkBattleRecords));
    printf("sizeof(LilycoveLady): %lu\n", (unsigned long)sizeof(LilycoveLady));
    printf("sizeof(DayCare): %lu\n", (unsigned long)sizeof(struct DayCare));
    printf("sizeof(ObjectEvent): %lu\n", (unsigned long)sizeof(struct ObjectEvent));
    printf("sizeof(BerryTree): %lu\n", (unsigned long)sizeof(struct BerryTree));
    printf("sizeof(DewfordTrend): %lu\n", (unsigned long)sizeof(struct DewfordTrend));
    printf("sizeof(LinkBattleRecord): %lu\n", (unsigned long)sizeof(struct LinkBattleRecord));

    return 0;
}
