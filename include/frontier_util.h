#ifndef GUARD_FRONTIER_UTIL_H
#define GUARD_FRONTIER_UTIL_H

void CallFrontierUtilFunc(void);
u8 GetFrontierBrainStatus(void);
void CopyFrontierTrainerText(u8 whichText, u16 trainerId);
void ResetWinStreaks(void);
u32 GetCurrentFacilityWinStreak(void);
void ResetFrontierTrainerIds(void);
u8 GetPlayerSymbolCountForFacility(u8 facility);
void ShowRankingHallRecordsWindow(void);
void ScrollRankingHallRecordsWindow(void);
void ClearRankingHallRecords(void);
void SaveGameFrontier(void);
u8 GetFrontierBrainTrainerPicIndex(void);
u8 GetFrontierBrainTrainerClass(void);
void CopyFrontierBrainTrainerName(u8 *dst);
bool8 IsFrontierBrainFemale(void);
void SetFrontierBrainObjEventGfx_2(void);
void CreateFrontierBrainPokemon(void);
u16 GetFrontierBrainMonSpecies(u8 monId);
void SetFrontierBrainObjEventGfx(u8 facility);
u16 GetFrontierBrainMonMove(u8 monId, u8 moveSlotId);
u8 GetFrontierBrainMonNature(u8 monId);
u8 GetFrontierBrainMonEvs(u8 monId, u8 evStatId);
s32 GetFronterBrainSymbol(void);

// Veritas: Lv50 cap helpers, used by load_save.c's cap-aware SavePlayerParty
// and ResetSketchedMoves overrides (frontier_util.c + battle_dome.c).
bool32 IsFrontierLv50CapActive(void);
bool32 IsFrontierPartyReduced(void);
void SetFrontierPartyReducedFlag(bool32 reduced);
void FrontierLv50_SaveReducedPartyToSaveBlock1(void);
void FrontierLv50_RestoreSavedSlotExp(struct Pokemon *mon, u8 slot);

// Veritas elite-trainer dialog override. Returns TRUE and writes
// gStringVar4 with a raw custom string when trainerId is one of the
// elite-cohort ids (FRONTIER_TRAINER_GABRIEL..YANN); returns FALSE for
// every other trainer so the caller can fall through to the EasyChat
// pipeline. whichText is FRONTIER_BEFORE_TEXT / FRONTIER_PLAYER_LOST_TEXT
// / FRONTIER_PLAYER_WON_TEXT from constants/frontier_util.h.
bool32 TryCopyEliteTrainerText(u8 whichText, u16 trainerId);

extern const u16 gFrontierBannedSpecies[];

#endif // GUARD_FRONTIER_UTIL_H
