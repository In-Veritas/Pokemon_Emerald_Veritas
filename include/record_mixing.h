#ifndef GUARD_RECORD_MIXING_H
#define GUARD_RECORD_MIXING_H

struct PlayerHallRecords
{
    struct RankingHall1P onePlayer[HALL_FACILITIES_COUNT][FRONTIER_LVL_MODE_COUNT];
    struct RankingHall2P twoPlayers[FRONTIER_LVL_MODE_COUNT];
};

void RecordMixingPlayerSpotTriggered(void);
void GetPlayerHallRecords(struct PlayerHallRecords *dst);

// Post-battle auto-mix (after link battle, before link close).
extern bool8 gPostBattleMixActive;
void StartRecordMixingPostBattle(void);

#endif //GUARD_RECORD_MIXING_H
