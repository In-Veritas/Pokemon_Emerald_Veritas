// Save version 4 → 5 migration
// Wipes only record mixing data received from OTHER players
// (TV shows, news, other players' secret bases).
// All player-owned data is preserved.

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

    // SaveBlock1 — full copy first, then selectively wipe other players' data
    CpuCopy16(sOldSaveBlock1Ptr, gSaveBlock1Ptr, sizeof(struct SaveBlock1));

    // Wipe TV shows and news (contain data received from other players)
    CpuFill16(0, &gSaveBlock1Ptr->tvShows, sizeof(gSaveBlock1Ptr->tvShows));
    CpuFill16(0, &gSaveBlock1Ptr->pokeNews, sizeof(gSaveBlock1Ptr->pokeNews));

    // Wipe other players' secret bases (index 0 is the player's own)
    for (i = 1; i < SECRET_BASES_COUNT; i++)
        CpuFill16(0, &gSaveBlock1Ptr->secretBases[i], sizeof(struct SecretBase));

    // Wipe record mixing gift
    CpuFill16(0, &gSaveBlock1Ptr->recordMixingGift, sizeof(gSaveBlock1Ptr->recordMixingGift));

    // Pokemon Storage — direct copy, struct is identical
    CpuCopy16(sOldPokemonStoragePtr, gPokemonStoragePtr, sizeof(struct PokemonStorage));

    return TRUE;
}
