// Save version 4 → 5 migration
// SaveBlock2 is unchanged. SaveBlock1 is unchanged structurally,
// but we selectively skip record mixing fields to wipe corrupted data.

bool8 UpdateSave_v4_v5(const struct SaveSectorLocation *locations)
{
    const struct SaveBlock2* sOldSaveBlock2Ptr = (struct SaveBlock2*)(locations[SECTOR_ID_SAVEBLOCK2].data);
    const struct SaveBlock1* sOldSaveBlock1Ptr = (struct SaveBlock1*)(locations[SECTOR_ID_SAVEBLOCK1_START].data);
    const struct PokemonStorage* sOldPokemonStoragePtr = (struct PokemonStorage*)(locations[SECTOR_ID_PKMN_STORAGE_START].data);

    u32 i;

    // SaveBlock2 — direct copy, struct is identical
    CpuCopy16(sOldSaveBlock2Ptr, gSaveBlock2Ptr, sizeof(struct SaveBlock2));
    gSaveBlock2Ptr->_saveSentinel = 0xFF;
    gSaveBlock2Ptr->saveVersion = SAVE_VERSION;

    // SaveBlock1 — field-by-field, skipping record mixing fields
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
    // Record mixing fields intentionally NOT copied — wiped to fix corruption
    // COPY_BLOCK(tvShows);
    // COPY_BLOCK(pokeNews);
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
    // COPY_FIELD(oldMan);
    // COPY_BLOCK(dewfordTrends);
    // COPY_BLOCK(contestWinners);
    COPY_FIELD(daycare);
    // COPY_FIELD(linkBattleRecords);
    COPY_BLOCK(giftRibbons);
    COPY_FIELD(externalEventData);
    COPY_FIELD(externalEventFlags);
    COPY_FIELD(roamer);
    COPY_FIELD(enigmaBerry);
    COPY_FIELD(mysteryGift);
    COPY_BLOCK(trainerHillTimes);
    COPY_FIELD(ramScript);
    // COPY_FIELD(recordMixingGift);
    COPY_BLOCK(seen2);
    // COPY_FIELD(lilycoveLady);
    // COPY_BLOCK(trainerNameRecords);
    COPY_BLOCK(registeredTexts);
    COPY_FIELD(trainerHill);
    COPY_FIELD(waldaPhrase);
    COPY_FIELD(registeredItemLastSelected);
    COPY_FIELD(registeredItemListCount);
    COPY_BLOCK(registeredItems);

    #undef COPY_FIELD
    #undef COPY_BLOCK
    #undef COPY_ARRAY

    // Pokemon Storage — direct copy, struct is identical
    CpuCopy16(sOldPokemonStoragePtr, gPokemonStoragePtr, sizeof(struct PokemonStorage));

    return TRUE;
}
