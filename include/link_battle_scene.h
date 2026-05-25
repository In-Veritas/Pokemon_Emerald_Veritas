#ifndef GUARD_LINK_BATTLE_SCENE_H
#define GUARD_LINK_BATTLE_SCENE_H

// Returned by LinkBattleScene_ResolveAndGetTerrain when no terrain was
// negotiated (terrain picker was removed; this is now always returned).
// The mugshot BG palette picker (battle_transition.c) treats this as
// "use the default PLAIN-row pair."
#define LINK_SCENE_NONE   0xFF

void LinkBattleScene_Reset(void);
void LinkBattleScene_FillSendBuffer(void);
void LinkBattleScene_StashPeerData(void);
u8 LinkBattleScene_ResolveAndGetTerrain(void);
bool8 LinkBattleScene_IsPlayerRsStyle(u8 playerIdx);
u16 LinkBattleScene_GetTrainerPicId(u8 playerIdx, u8 playerGender);
u16 LinkBattleScene_GetTrainerBackPicId(u8 playerIdx, u8 playerGender);
u32 LinkBattleScene_GetSyncSeed(void);

#endif // GUARD_LINK_BATTLE_SCENE_H
