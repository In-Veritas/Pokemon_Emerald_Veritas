#include "global.h"
#include "save.h"
#include "item.h"
#include "event_data.h"
#include "constants/heal_locations.h"
#include "constants/flags.h"

// Save version 4 structure - extended options still in vanilla section (causing offset issues)
// This version has sentinel at end but extended options before Pokedex
struct SaveBlock2_v4
{
    u8 playerName[7 + 1];
    u8 playerGender;
    u8 specialSaveWarpFlags;
    u8 playerTrainerId[4];
    u16 playTimeHours;
    u8 playTimeMinutes;
    u8 playTimeSeconds;
    u8 playTimeVBlanks;
    u8 optionsButtonMode;
    // These extended options overflow the u16, shifting all subsequent offsets by 2 bytes
    u16 optionsTextSpeed:3;
    u16 optionsWindowFrameType:5;
    u16 optionsSound:1;
    u16 optionsBattleStyle:1;
    u16 optionsBattleSceneOff:1;
    u16 regionMapZoom:1;
    u16 optionsBattleItemAnimation:2;
    u16 optionsDiveSpeed:2;
    u16 optionsHpBarSpeed:4;
    u16 optionsExpBarSpeed:4;
    struct Pokedex pokedex;
    u8 filler_90[0x8];
    struct Time localTimeOffset;
    struct Time lastBerryTreeUpdate;
    u32 gcnLinkFlags;
    u32 encryptionKey;
    struct PlayersApprentice playerApprentice;
    struct Apprentice apprentices[4];
    struct BerryCrush berryCrush;
    struct PokemonJumpRecords pokeJump;
    struct BerryPickingResults berryPick;
    struct RankingHall1P hallRecords1P[9][2][3];
    struct RankingHall2P hallRecords2P[2][3];
    u16 contestLinkResults[5][4];
    struct BattleFrontier frontier;
    // Veritas extensions at end
    u8 pokemonVeritasSentinel; // 0xAA
    u16 pokemonVeritasSaveVersion;
    u8 playerLookStyle;
};

bool8 UpdateSave_v4_v5(const struct SaveSectorLocation *locations)
{
    const struct SaveBlock2_v4* sOldSaveBlock2Ptr = (struct SaveBlock2_v4*)(locations[SECTOR_ID_SAVEBLOCK2].data);
    const struct SaveBlock1* sOldSaveBlock1Ptr = (struct SaveBlock1*)(locations[SECTOR_ID_SAVEBLOCK1_START].data);
    const struct PokemonStorage* sOldPokemonStoragePtr = (struct PokemonStorage*)(locations[SECTOR_ID_PKMN_STORAGE_START].data);

    u32 arg, i, j, k;

    #define COPY_FIELD(field) gSaveBlock2Ptr->field = sOldSaveBlock2Ptr->field
    #define COPY_BLOCK(field) CpuCopy16(&sOldSaveBlock2Ptr->field, &gSaveBlock2Ptr->field, sizeof(gSaveBlock2Ptr->field))
    #define COPY_ARRAY(field) for(i = 0; i < min(ARRAY_COUNT(gSaveBlock2Ptr->field), ARRAY_COUNT(sOldSaveBlock2Ptr->field)); i++) gSaveBlock2Ptr->field[i] = sOldSaveBlock2Ptr->field[i];

    // Set new format version
    gSaveBlock2Ptr->pokemonVeritasSentinel = 0xAA;
    gSaveBlock2Ptr->pokemonVeritasSaveVersion = SAVE_VERSION;

    // Copy vanilla fields
    COPY_ARRAY(playerName);
    COPY_FIELD(playerGender);
    COPY_FIELD(specialSaveWarpFlags);
    COPY_ARRAY(playerTrainerId);
    COPY_FIELD(playTimeHours);
    COPY_FIELD(playTimeMinutes);
    COPY_FIELD(playTimeSeconds);
    COPY_FIELD(playTimeVBlanks);
    COPY_FIELD(optionsButtonMode);

    // Copy vanilla options (these stay in the same place)
    COPY_FIELD(optionsTextSpeed);
    COPY_FIELD(optionsWindowFrameType);
    COPY_FIELD(optionsSound);
    COPY_FIELD(optionsBattleStyle);
    COPY_FIELD(optionsBattleSceneOff);
    COPY_FIELD(regionMapZoom);

    // Copy extended options (now moved to end of struct in v5)
    COPY_FIELD(optionsBattleItemAnimation);
    COPY_FIELD(optionsDiveSpeed);
    COPY_FIELD(optionsHpBarSpeed);
    COPY_FIELD(optionsExpBarSpeed);

    // Initialize new padding field
    gSaveBlock2Ptr->optionsExtendedPadding = 0;

    // Copy remaining vanilla fields
    COPY_FIELD(pokedex);
    COPY_FIELD(localTimeOffset);
    COPY_FIELD(lastBerryTreeUpdate);
    COPY_FIELD(gcnLinkFlags);
    COPY_FIELD(encryptionKey);
    COPY_FIELD(playerApprentice);
    COPY_BLOCK(apprentices);
    COPY_FIELD(berryCrush);
    COPY_FIELD(pokeJump);
    COPY_FIELD(berryPick);
    COPY_BLOCK(hallRecords1P);
    COPY_BLOCK(hallRecords2P);
    COPY_BLOCK(contestLinkResults);
    COPY_FIELD(frontier);

    // Copy Veritas extension
    COPY_FIELD(playerLookStyle);

    #undef COPY_FIELD
    #undef COPY_BLOCK
    #undef COPY_ARRAY

    // SaveBlock1 - direct copy since unchanged
    #define COPY_FIELD(field) gSaveBlock1Ptr->field = sOldSaveBlock1Ptr->field
    #define COPY_BLOCK(field) CpuCopy16(&sOldSaveBlock1Ptr->field, &gSaveBlock1Ptr->field, sizeof(gSaveBlock1Ptr->field))
    #define COPY_ARRAY(field) for(i = 0; i < min(ARRAY_COUNT(gSaveBlock1Ptr->field), ARRAY_COUNT(sOldSaveBlock1Ptr->field)); i++) gSaveBlock1Ptr->field[i] = sOldSaveBlock1Ptr->field[i];

    COPY_FIELD(pos);
    COPY_FIELD(location);
    COPY_FIELD(continueGameWarp);
    COPY_FIELD(dynamicWarp);
    COPY_FIELD(lastHealLocation);
    COPY_FIELD(escapeWarp);
    COPY_FIELD(secretBaseWarp);
    COPY_FIELD(savedMusic);
    COPY_FIELD(weather);
    COPY_FIELD(weatherCycleStage);
    COPY_FIELD(flashLevel);
    COPY_FIELD(mapLayoutId);
    COPY_BLOCK(mapView);
    COPY_FIELD(playerPartyCount);
    COPY_ARRAY(playerParty);
    COPY_FIELD(money);
    COPY_FIELD(coins);
    COPY_FIELD(registeredItemSelect);
    COPY_BLOCK(pcItems);
    COPY_BLOCK(bagPocket_Items);
    COPY_BLOCK(bagPocket_KeyItems);
    COPY_BLOCK(bagPocket_PokeBalls);
    COPY_BLOCK(bagPocket_TMHM);
    COPY_BLOCK(bagPocket_Berries);
    COPY_BLOCK(pokeblocks);
    COPY_BLOCK(seen1);
    COPY_BLOCK(berryBlenderRecords);
    COPY_FIELD(trainerRematchStepCounter);
    COPY_BLOCK(trainerRematches);
    COPY_BLOCK(objectEvents);
    COPY_BLOCK(objectEventTemplates);
    COPY_BLOCK(flags);
    COPY_BLOCK(vars);
    COPY_BLOCK(gameStats);
    COPY_BLOCK(berryTrees);
    COPY_BLOCK(secretBases);
    COPY_BLOCK(playerRoomDecorations);
    COPY_BLOCK(playerRoomDecorationPositions);
    COPY_BLOCK(decorationDesks);
    COPY_BLOCK(decorationChairs);
    COPY_BLOCK(decorationPlants);
    COPY_BLOCK(decorationOrnaments);
    COPY_BLOCK(decorationMats);
    COPY_BLOCK(decorationPosters);
    COPY_BLOCK(decorationDolls);
    COPY_BLOCK(decorationCushions);
    COPY_BLOCK(tvShows);
    COPY_BLOCK(pokeNews);
    COPY_FIELD(outbreakPokemonSpecies);
    COPY_FIELD(outbreakLocationMapNum);
    COPY_FIELD(outbreakLocationMapGroup);
    COPY_FIELD(outbreakPokemonLevel);
    COPY_FIELD(outbreakUnused1);
    COPY_FIELD(outbreakUnused2);
    COPY_BLOCK(outbreakPokemonMoves);
    COPY_FIELD(outbreakUnused3);
    COPY_FIELD(outbreakPokemonProbability);
    COPY_FIELD(outbreakDaysLeft);
    COPY_FIELD(gabbyAndTyData);
    COPY_BLOCK(easyChatProfile);
    COPY_BLOCK(easyChatBattleStart);
    COPY_BLOCK(easyChatBattleWon);
    COPY_BLOCK(easyChatBattleLost);
    COPY_BLOCK(mail);
    COPY_BLOCK(unlockedTrendySayings);
    COPY_FIELD(oldMan);
    COPY_BLOCK(dewfordTrends);
    COPY_BLOCK(contestWinners);
    COPY_FIELD(daycare);
    COPY_FIELD(linkBattleRecords);
    COPY_BLOCK(giftRibbons);
    COPY_FIELD(externalEventData);
    COPY_FIELD(externalEventFlags);
    COPY_FIELD(roamer);
    COPY_FIELD(enigmaBerry);
    COPY_FIELD(mysteryGift);
    COPY_BLOCK(trainerHillTimes);
    COPY_FIELD(ramScript);
    COPY_FIELD(recordMixingGift);
    COPY_BLOCK(seen2);
    COPY_FIELD(lilycoveLady);
    COPY_BLOCK(trainerNameRecords);
    COPY_BLOCK(registeredTexts);
    COPY_FIELD(trainerHill);
    COPY_FIELD(waldaPhrase);
    COPY_FIELD(registeredItemLastSelected);
    COPY_FIELD(registeredItemListCount);
    COPY_BLOCK(registeredItems);

    #undef COPY_FIELD
    #undef COPY_BLOCK
    #undef COPY_ARRAY

    // Pokemon Storage - direct copy
    CpuCopy16(sOldPokemonStoragePtr, gPokemonStoragePtr, sizeof(struct PokemonStorage));

    return TRUE;
}
