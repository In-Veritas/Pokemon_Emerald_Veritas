// Save version 3 structure — SECRET_BASES_COUNT was 20, DECOR_MAX_SECRET_BASE was 16
// SecretBase_Old and related types are defined in save.v0.h (included before this file)

struct SaveBlock1_v3
{
    struct Coords16 pos;
    struct WarpData location;
    struct WarpData continueGameWarp;
    struct WarpData dynamicWarp;
    struct WarpData lastHealLocation;
    struct WarpData escapeWarp;
    struct WarpData secretBaseWarp;
    u16 savedMusic;
    u8 weather;
    u8 weatherCycleStage;
    u8 flashLevel;
    u16 mapLayoutId;
    u16 mapView[0x100];
    u8 playerPartyCount;
    struct Pokemon playerParty[PARTY_SIZE];
    u32 money;
    u16 coins;
    u16 registeredItemSelect;
    struct ItemSlot pcItems[PC_ITEMS_COUNT];
    struct ItemSlot bagPocket_Items[BAG_ITEMS_COUNT];
    struct ItemSlot bagPocket_KeyItems[BAG_KEYITEMS_COUNT];
    struct ItemSlot bagPocket_PokeBalls[BAG_POKEBALLS_COUNT];
    struct ItemSlot bagPocket_TMHM[BAG_TMHM_COUNT];
    struct ItemSlot bagPocket_Berries[BAG_BERRIES_COUNT];
    struct Pokeblock pokeblocks[POKEBLOCKS_COUNT];
    u8 seen1[NUM_DEX_FLAG_BYTES];
    u16 berryBlenderRecords[3];
    u16 trainerRematchStepCounter;
    u8 trainerRematches[MAX_REMATCH_ENTRIES];
    struct ObjectEvent objectEvents[OBJECT_EVENTS_COUNT];
    struct ObjectEventTemplate objectEventTemplates[OBJECT_EVENT_TEMPLATES_COUNT];
    u8 flags[NUM_FLAG_BYTES];
    u16 vars[VARS_COUNT];
    u32 gameStats[NUM_GAME_STATS];
    struct BerryTree berryTrees[BERRY_TREES_COUNT];
    struct SecretBase_Old secretBases[SECRET_BASES_COUNT_OLD];
    u8 playerRoomDecorations[DECOR_MAX_PLAYERS_HOUSE];
    u8 playerRoomDecorationPositions[DECOR_MAX_PLAYERS_HOUSE];
    u8 decorationDesks[10];
    u8 decorationChairs[10];
    u8 decorationPlants[10];
    u8 decorationOrnaments[30];
    u8 decorationMats[30];
    u8 decorationPosters[10];
    u8 decorationDolls[40];
    u8 decorationCushions[10];
    TVShow tvShows[TV_SHOWS_COUNT];
    PokeNews pokeNews[POKE_NEWS_COUNT];
    u16 outbreakPokemonSpecies;
    u8 outbreakLocationMapNum;
    u8 outbreakLocationMapGroup;
    u8 outbreakPokemonLevel;
    u8 outbreakUnused1;
    u16 outbreakUnused2;
    u16 outbreakPokemonMoves[MAX_MON_MOVES];
    u8 outbreakUnused3;
    u8 outbreakPokemonProbability;
    u16 outbreakDaysLeft;
    struct GabbyAndTyData gabbyAndTyData;
    u16 easyChatProfile[EASY_CHAT_BATTLE_WORDS_COUNT];
    u16 easyChatBattleStart[EASY_CHAT_BATTLE_WORDS_COUNT];
    u16 easyChatBattleWon[EASY_CHAT_BATTLE_WORDS_COUNT];
    u16 easyChatBattleLost[EASY_CHAT_BATTLE_WORDS_COUNT];
    struct Mail mail[MAIL_COUNT];
    u8 unlockedTrendySayings[NUM_TRENDY_SAYING_BYTES];
    OldMan oldMan;
    struct DewfordTrend dewfordTrends[SAVED_TRENDS_COUNT];
    struct ContestWinner contestWinners[NUM_CONTEST_WINNERS];
    struct DayCare daycare;
    struct LinkBattleRecords linkBattleRecords;
    u8 giftRibbons[GIFT_RIBBONS_COUNT];
    struct ExternalEventData externalEventData;
    struct ExternalEventFlags externalEventFlags;
    struct Roamer roamer;
    struct EnigmaBerry enigmaBerry;
    struct MysteryGiftSave mysteryGift;
    u8 unused_3598[0x18];
    u32 trainerHillTimes[NUM_TRAINER_HILL_MODES];
    struct RamScript ramScript;
    struct RecordMixingGift recordMixingGift;
    u8 seen2[NUM_DEX_FLAG_BYTES];
    LilycoveLady lilycoveLady;
    struct TrainerNameRecord trainerNameRecords[20];
    u8 registeredTexts[UNION_ROOM_KB_ROW_COUNT][21];
    u8 unused_3D5A[10];
    struct TrainerHillSave trainerHill;
    struct WaldaPhrase waldaPhrase;
    u8 registeredItemLastSelected:4;
    u8 registeredItemListCount:4;
    struct RegisteredItemSlot registeredItems[REGISTERED_ITEMS_MAX];
};

bool8 UpdateSave_v3_v4(const struct SaveSectorLocation *locations)
{
    const struct SaveBlock2* sOldSaveBlock2Ptr = (struct SaveBlock2*)(locations[SECTOR_ID_SAVEBLOCK2].data);
    const struct SaveBlock1_v3* sOldSaveBlock1Ptr = (struct SaveBlock1_v3*)(locations[SECTOR_ID_SAVEBLOCK1_START].data);
    const struct PokemonStorage* sOldPokemonStoragePtr = (struct PokemonStorage*)(locations[SECTOR_ID_PKMN_STORAGE_START].data);

    u32 i;

    // SaveBlock2 is unchanged between v3 and v4 — direct copy
    CpuCopy16(sOldSaveBlock2Ptr, gSaveBlock2Ptr, sizeof(struct SaveBlock2));
    gSaveBlock2Ptr->_saveSentinel = 0xFF;
    gSaveBlock2Ptr->saveVersion = SAVE_VERSION;

    // SaveBlock1 — field-by-field copy due to SecretBase struct change
    #define COPY_FIELD(field) gSaveBlock1Ptr->field = sOldSaveBlock1Ptr->field
    #define COPY_BLOCK(field) CpuCopy16(&sOldSaveBlock1Ptr->field, &gSaveBlock1Ptr->field, sizeof(gSaveBlock1Ptr->field))

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
    for (i = 0; i < PARTY_SIZE; i++)
        gSaveBlock1Ptr->playerParty[i] = sOldSaveBlock1Ptr->playerParty[i];
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

    // Secret bases: copy first 15 from old save's 20 using shared helper
    CopyOldSecretBases(sOldSaveBlock1Ptr->secretBases, SECRET_BASES_COUNT_OLD);

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
    for (i = 0; i < REGISTERED_ITEMS_MAX; i++)
        gSaveBlock1Ptr->registeredItems[i] = sOldSaveBlock1Ptr->registeredItems[i];

    #undef COPY_FIELD
    #undef COPY_BLOCK

    // Pokemon Storage — unchanged between v3 and v4
    CpuCopy16(sOldPokemonStoragePtr, gPokemonStoragePtr, sizeof(struct PokemonStorage));

    // Warp player to heal location
    SetContinueGameWarpStatus();
    if (gSaveBlock2Ptr->playerGender == MALE)
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F);
    else
        SetContinueGameWarpToHealLocation(HEAL_LOCATION_LITTLEROOT_TOWN_MAYS_HOUSE_2F);

    return TRUE;
}
