#include "global.h"
#include "random.h"
#include "event_data.h"
#include "battle_setup.h"
#include "gym_leader_rematch.h"

static s32 GetRematchIndex(u32 trainerIdx);
static void TryReleaseBirds(void);

static const u16 GymLeaderRematches_AfterNewMauville[] = {
    REMATCH_ROXANNE,
    REMATCH_BRAWLY,
    REMATCH_WATTSON,
    REMATCH_FLANNERY,
    REMATCH_NORMAN,
    REMATCH_WINONA,
    REMATCH_TATE_AND_LIZA,
    REMATCH_JUAN
};

static const u16 GymLeaderRematchesTrainers[] = {
    TRAINER_ROXANNE_1,
    TRAINER_BRAWLY_1,
    TRAINER_WATTSON_1,
    TRAINER_FLANNERY_1,
    TRAINER_NORMAN_1,
    TRAINER_WINONA_1,
    TRAINER_TATE_AND_LIZA_1,
    TRAINER_JUAN_1
};

void UpdateGymLeaderRematch(void)
{
    u32 i;
    s32 rematchIndex;
    s32 tableId;
    bool32 hasUnlockedAnyRematches = FALSE;

    // Loop through all gym leaders and check conditions for each rematch stage
    for (i = 0; i < ARRAY_COUNT(GymLeaderRematches_AfterNewMauville); i++)
    {
        u16 trainerIdx = GymLeaderRematches_AfterNewMauville[i];
        u16 trainerIdx2 = GymLeaderRematchesTrainers[i];
        rematchIndex = GetRematchIndex(trainerIdx);
        tableId = FirstBattleTrainerIdToRematchTableId(gRematchTable, trainerIdx2);

        if (tableId != -1 && tableId < MAX_REMATCH_ENTRIES)
        {
            if (rematchIndex == 1) // 2nd fight
            {
                // Check specific event flag for each trainer
                if (trainerIdx == REMATCH_ROXANNE && FlagGet(FLAG_REMATCH_READY_ROXANNE) && !HasTrainerBeenFought(TRAINER_ROXANNE_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1; // Set to 1 or any positive value to enable rematch
                else if (trainerIdx == REMATCH_BRAWLY && FlagGet(FLAG_REMATCH_READY_BRAWLY) && !HasTrainerBeenFought(TRAINER_BRAWLY_2) && HasTrainerBeenFought(TRAINER_BRAWLY_1))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_WATTSON && FlagGet(FLAG_WATTSON_REMATCH_AVAILABLE) && !HasTrainerBeenFought(TRAINER_WATTSON_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_FLANNERY && FlagGet(FLAG_REMATCH_READY_FLANNERY) && !HasTrainerBeenFought(TRAINER_FLANNERY_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_NORMAN && FlagGet(FLAG_REMATCH_READY_NORMAN) && !HasTrainerBeenFought(TRAINER_NORMAN_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_WINONA && FlagGet(FLAG_REMATCH_READY_WINONA) && !HasTrainerBeenFought(TRAINER_WINONA_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_TATE_AND_LIZA && FlagGet(FLAG_REMATCH_READY_TATEANDLIZA) && !HasTrainerBeenFought(TRAINER_TATE_AND_LIZA_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                // else if (trainerIdx == REMATCH_JUAN && FlagGet(FLAG_REMATCH_READY_JUAN) && !HasTrainerBeenFought(TRAINER_JUAN_2)) Juan 2 isn't accessible
                //     gSaveBlock1Ptr->trainerRematches[tableId] = 1;
            }
            else if ((rematchIndex == 2) && FlagGet(FLAG_SYS_GAME_CLEAR)) // 3rd fight
            {
                if (trainerIdx == REMATCH_ROXANNE && !HasTrainerBeenFought(TRAINER_ROXANNE_3) && HasTrainerBeenFought(TRAINER_ROXANNE_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_BRAWLY && !HasTrainerBeenFought(TRAINER_BRAWLY_3) && HasTrainerBeenFought(TRAINER_BRAWLY_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_WATTSON && !HasTrainerBeenFought(TRAINER_WATTSON_3) && HasTrainerBeenFought(TRAINER_WATTSON_2) && FlagGet(FLAG_WATTSON_REMATCH_AVAILABLE))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_FLANNERY && !HasTrainerBeenFought(TRAINER_FLANNERY_3) && HasTrainerBeenFought(TRAINER_FLANNERY_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_NORMAN && !HasTrainerBeenFought(TRAINER_NORMAN_3) && HasTrainerBeenFought(TRAINER_NORMAN_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_WINONA && !HasTrainerBeenFought(TRAINER_WINONA_3) && HasTrainerBeenFought(TRAINER_WINONA_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_TATE_AND_LIZA && !HasTrainerBeenFought(TRAINER_TATE_AND_LIZA_3) && HasTrainerBeenFought(TRAINER_TATE_AND_LIZA_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_JUAN && !HasTrainerBeenFought(TRAINER_JUAN_3) && HasTrainerBeenFought(TRAINER_JUAN_2))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
            }
            else if ((rematchIndex == 3) && FlagGet(FLAG_COLLECTED_ALL_SILVER_SYMBOLS)) // 4th fight
            {
                if (trainerIdx == REMATCH_ROXANNE && !HasTrainerBeenFought(TRAINER_ROXANNE_4) && HasTrainerBeenFought(TRAINER_ROXANNE_3))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_BRAWLY && !HasTrainerBeenFought(TRAINER_BRAWLY_4) && HasTrainerBeenFought(TRAINER_BRAWLY_3))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_WATTSON && !HasTrainerBeenFought(TRAINER_WATTSON_4) && HasTrainerBeenFought(TRAINER_WATTSON_3))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_FLANNERY && !HasTrainerBeenFought(TRAINER_FLANNERY_4) && HasTrainerBeenFought(TRAINER_FLANNERY_3))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_NORMAN && !HasTrainerBeenFought(TRAINER_NORMAN_4) && HasTrainerBeenFought(TRAINER_NORMAN_3))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_WINONA && !HasTrainerBeenFought(TRAINER_WINONA_4) && HasTrainerBeenFought(TRAINER_WINONA_3))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_TATE_AND_LIZA && !HasTrainerBeenFought(TRAINER_TATE_AND_LIZA_4) && HasTrainerBeenFought(TRAINER_TATE_AND_LIZA_3))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                else if (trainerIdx == REMATCH_JUAN && !HasTrainerBeenFought(TRAINER_JUAN_4) && HasTrainerBeenFought(TRAINER_JUAN_3))
                    gSaveBlock1Ptr->trainerRematches[tableId] = 1;
                gSaveBlock1Ptr->trainerRematches[tableId] = 1;
            }
            else if ((rematchIndex == 4) && AllFourthFightsCompleted())
            {
                gSaveBlock1Ptr->trainerRematches[tableId] = 1;
            }
        }
    }
    TryReleaseBirds();
};

// Function to check if all 4th fights are completed
bool32 AllFourthFightsCompleted(void)
{
    return (HasTrainerBeenFought(TRAINER_ROXANNE_4) &&
            HasTrainerBeenFought(TRAINER_BRAWLY_4) &&
            HasTrainerBeenFought(TRAINER_FLANNERY_4) &&
            HasTrainerBeenFought(TRAINER_NORMAN_4) &&
            HasTrainerBeenFought(TRAINER_WINONA_4) &&
            HasTrainerBeenFought(TRAINER_TATE_AND_LIZA_4) &&
            HasTrainerBeenFought(TRAINER_JUAN_4) &&
            HasTrainerBeenFought(TRAINER_WATTSON_4));
};

// Release the birds if all 3rd gym leader fights are completed
static void TryReleaseBirds(void)
{
    if (HasTrainerBeenFought(TRAINER_ROXANNE_3) &&
        HasTrainerBeenFought(TRAINER_BRAWLY_3) &&
        HasTrainerBeenFought(TRAINER_WATTSON_3) &&
        HasTrainerBeenFought(TRAINER_FLANNERY_3) &&
        HasTrainerBeenFought(TRAINER_NORMAN_3) &&
        HasTrainerBeenFought(TRAINER_WINONA_3) &&
        HasTrainerBeenFought(TRAINER_TATE_AND_LIZA_3) &&
        HasTrainerBeenFought(TRAINER_JUAN_3))
    {
        FlagSet(FLAG_BEAT_ALL_3RD_REMATCHES);
        if (!(FlagGet(FLAG_DEFEATED_ZAPDOS) || FlagGet(FLAG_CAUGHT_ZAPDOS)))
        {
            FlagClear(FLAG_HIDE_ZAPDOS);
        }
        if (!(FlagGet(FLAG_DEFEATED_ARTICUNO) || FlagGet(FLAG_CAUGHT_ARTICUNO)))
        {
            FlagClear(FLAG_HIDE_ARTICUNO);
        }
        if (!(FlagGet(FLAG_DEFEATED_MOLTRES) || FlagGet(FLAG_CAUGHT_MOLTRES)))
        {
            FlagClear(FLAG_HIDE_MOLTRES);
        }
    }
};

static s32 GetRematchIndex(u32 trainerIdx)
{
    s32 i;
    for (i = 0; i < 5; i++)
    {
        if (!HasTrainerBeenFought(gRematchTable[trainerIdx].trainerIds[i]))
        {
            return i;
        }
    }
    return 5;
};
