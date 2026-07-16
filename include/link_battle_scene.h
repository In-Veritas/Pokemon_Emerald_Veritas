#ifndef GUARD_LINK_BATTLE_SCENE_H
#define GUARD_LINK_BATTLE_SCENE_H

// Per-player pick stored locally; broadcast over the existing trainer card
// block exchange (see LinkBattleScene_FillSendBuffer / Resolve).
// LINK_SCENE_RANDOM means "roll any terrain at resolve time."
// LINK_SCENE_NONE is returned by ResolveAndGetTerrain when no terrain was
// negotiated (non-cable-club battle, or nobody picked); battle code then
// keeps the map-derived gBattleTerrain / forced Building backdrop.
#define LINK_SCENE_RANDOM 0xFE
#define LINK_SCENE_NONE   0xFF

void LinkBattleScene_Reset(void);
void LinkBattleScene_StoreLocalPick(u8 pick);
void LinkBattleScene_FillSendBuffer(void);
void LinkBattleScene_StashPeerData(void);
u8 LinkBattleScene_ResolveAndGetTerrain(void);
bool8 LinkBattleScene_IsPlayerRsStyle(u8 playerIdx);
u16 LinkBattleScene_GetTrainerPicId(u8 playerIdx, u8 playerGender);
u16 LinkBattleScene_GetTrainerBackPicId(u8 playerIdx, u8 playerGender);
u32 LinkBattleScene_GetSyncSeed(void);

#endif // GUARD_LINK_BATTLE_SCENE_H
