#include "global.h"
#include "malloc.h"
#include "battle.h"
#include "battle_tower.h"
#include "cable_club.h"
#include "data.h"
#include "decoration.h"
#include "diploma.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "field_camera.h"
#include "field_effect.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "field_specials.h"
#include "field_weather.h"
#include "graphics.h"
#include "international_string_util.h"
#include "item_icon.h"
#include "link.h"
#include "list_menu.h"
#include "main.h"
#include "mystery_gift.h"
#include "match_call.h"
#include "menu.h"
#include "money.h"
#include "naming_screen.h"
#include "item.h"
#include "overworld.h"
#include "party_menu.h"
#include "pokeblock.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "rayquaza_scene.h"
#include "region_map.h"
#include "rtc.h"
#include "script.h"
#include "script_menu.h"
#include "sound.h"
#include "starter_choose.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "tv.h"
#include "wallclock.h"
#include "window.h"
#include "constants/battle_frontier.h"
#include "constants/battle_pyramid.h"
#include "constants/battle_tower.h"
#include "constants/decorations.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/field_effects.h"
#include "constants/field_specials.h"
#include "constants/items.h"
#include "constants/heal_locations.h"
#include "constants/map_types.h"
#include "constants/mystery_gift.h"
#include "constants/slot_machine.h"
#include "constants/songs.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/region_map_sections.h"
#include "constants/battle_frontier.h"
#include "constants/weather.h"
#include "constants/metatile_labels.h"
#include "constants/game_stat.h"
#include "palette.h"

#define TAG_ITEM_ICON 5500

#define GFXTAG_MULTICHOICE_SCROLL_ARROWS 2000
#define PALTAG_MULTICHOICE_SCROLL_ARROWS 100

#define ELEVATOR_WINDOW_WIDTH  3
#define ELEVATOR_WINDOW_HEIGHT 3
#define ELEVATOR_LIGHT_STAGES  3

EWRAM_DATA bool8 gBikeCyclingChallenge = FALSE;
EWRAM_DATA u8 gBikeCollisions = 0;
static EWRAM_DATA u32 sBikeCyclingTimer = 0;
static EWRAM_DATA u8 sSlidingDoorNextFrameCounter = 0;
static EWRAM_DATA u8 sSlidingDoorFrame = 0;
static EWRAM_DATA u8 sTutorMoveAndElevatorWindowId = 0;
static EWRAM_DATA u16 sLilycoveDeptStore_NeverRead = 0;
static EWRAM_DATA u16 sLilycoveDeptStore_DefaultFloorChoice = 0;
static EWRAM_DATA struct ListMenuItem *sScrollableMultichoice_ListMenuItem = NULL;
static EWRAM_DATA u16 sScrollableMultichoice_ScrollOffset = 0;
static EWRAM_DATA u16 sFrontierExchangeCorner_NeverRead = 0;
static EWRAM_DATA u8 sScrollableMultichoice_ItemSpriteId = 0;
static EWRAM_DATA bool8 sTrainerRecordsCleaned = FALSE;
static EWRAM_DATA u8 sInvalidRecordsCleaned = 0;
static EWRAM_DATA u8 sInvalidLinkBattleRecords = 0;
static EWRAM_DATA u8 sInvalidTrainerNameRecords = 0;
static EWRAM_DATA u8 sInvalidSecretBaseRecords = 0;
static EWRAM_DATA u8 sBattlePointsWindowId = 0;
static EWRAM_DATA u8 sFrontierExchangeCorner_ItemIconWindowId = 0;
static EWRAM_DATA u8 sPCBoxToSendMon = 0;
static EWRAM_DATA u32 sBattleTowerMultiBattleTypeFlags = 0;

struct ListMenuTemplate gScrollableMultichoice_ListMenuTemplate;

void TryLoseFansFromPlayTime(void);
void SetPlayerGotFirstFans(void);
u16 GetNumFansOfPlayerInTrainerFanClub(void);

static void RecordCyclingRoadResults(u32, u8);
static void LoadLinkPartnerObjectEventSpritePalette(u16, u8, u8);
static void Task_PetalburgGymSlideOpenRoomDoors(u8);
static void PetalburgGymSetDoorMetatiles(u8, u16);
static void Task_PCTurnOnEffect(u8);
static void PCTurnOnEffect(struct Task *);
static void PCTurnOnEffect_SetMetatile(s16, s8, s8);
static void PCTurnOffEffect(void);
static void Task_LotteryCornerComputerEffect(u8);
static void LotteryCornerComputerEffect(struct Task *);
static void Task_ShakeCamera(u8);
static void StopCameraShake(u8);
static void Task_MoveElevator(u8);
static void MoveElevatorWindowLights(u16, bool8);
static void Task_MoveElevatorWindowLights(u8);
static void Task_ShowScrollableMultichoice(u8);
static void FillFrontierExchangeCornerWindowAndItemIcon(u16, u16);
static void ShowBattleFrontierTutorWindow(u8, u16);
static void InitScrollableMultichoice(void);
static void ScrollableMultichoice_ProcessInput(u8);
static void ScrollableMultichoice_UpdateScrollArrows(u8);
static void ScrollableMultichoice_MoveCursor(s32, bool8, struct ListMenu *);
static void HideFrontierExchangeCornerItemIcon(u16, u16);
static void ShowBattleFrontierTutorMoveDescription(u8, u16);
static void CloseScrollableMultichoice(u8);
static void ScrollableMultichoice_RemoveScrollArrows(u8);
static void Task_ScrollableMultichoice_WaitReturnToList(u8);
static void Task_ScrollableMultichoice_ReturnToList(u8);
static void ShowFrontierExchangeCornerItemIcon(u16);
static void Task_DeoxysRockInteraction(u8);
static void ChangeDeoxysRockLevel(u8);
static void WaitForDeoxysRockMovement(u8);
static void Task_LinkRetireStatusWithBattleTowerPartner(u8);
static void Task_LoopWingFlapSE(u8);
static void Task_CloseBattlePikeCurtain(u8);
static u8 DidPlayerGetFirstFans(void);
static void SetInitialFansOfPlayer(void);
static u16 PlayerGainRandomTrainerFan(void);
static void BufferFanClubTrainerName_(struct LinkBattleRecords *, u8, u8);

void Special_ShowDiploma(void)
{
    SetMainCallback2(CB2_ShowDiploma);
    LockPlayerFieldControls();
}

void Special_ViewWallClock(void)
{
    gMain.savedCallback = CB2_ReturnToField;
    SetMainCallback2(CB2_ViewWallClock);
    LockPlayerFieldControls();
}

void ResetCyclingRoadChallengeData(void)
{
    gBikeCyclingChallenge = FALSE;
    gBikeCollisions = 0;
    sBikeCyclingTimer = 0;
}

void Special_BeginCyclingRoadChallenge(void)
{
    gBikeCyclingChallenge = TRUE;
    gBikeCollisions = 0;
    sBikeCyclingTimer = gMain.vblankCounter1;
}

u16 GetPlayerAvatarBike(void)
{
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_ACRO_BIKE))
        return 1;
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE))
        return 2;
    return 0;
}

static void DetermineCyclingRoadResults(u32 numFrames, u8 numBikeCollisions)
{
    u8 result;

    if (numBikeCollisions < 100)
    {
        ConvertIntToDecimalStringN(gStringVar1, numBikeCollisions, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringAppend(gStringVar1, gText_SpaceTimes);
    }
    else
    {
        StringCopy(gStringVar1, gText_99TimesPlus);
    }

    if (numFrames < 3600)
    {
        ConvertIntToDecimalStringN(gStringVar2, numFrames / 60, STR_CONV_MODE_RIGHT_ALIGN, 2);
        gStringVar2[2] = CHAR_DEC_SEPARATOR;
        ConvertIntToDecimalStringN(&gStringVar2[3], ((numFrames % 60) * 100) / 60, STR_CONV_MODE_LEADING_ZEROS, 2);
        StringAppend(gStringVar2, gText_SpaceSeconds);
    }
    else
    {
        StringCopy(gStringVar2, gText_1MinutePlus);
    }

    result = 0;
    if (numBikeCollisions == 0)
        result = 5;
    else if (numBikeCollisions < 4)
        result = 4;
    else if (numBikeCollisions < 10)
        result = 3;
    else if (numBikeCollisions < 20)
        result = 2;
    else if (numBikeCollisions < 100)
        result = 1;

    if (numFrames / 60 <= 10)
        result += 5;
    else if (numFrames / 60 <= 15)
        result += 4;
    else if (numFrames / 60 <= 20)
        result += 3;
    else if (numFrames / 60 <= 40)
        result += 2;
    else if (numFrames / 60 < 60)
        result += 1;

    gSpecialVar_Result = result;
}

void FinishCyclingRoadChallenge(void)
{
    const u32 numFrames = gMain.vblankCounter1 - sBikeCyclingTimer;

    DetermineCyclingRoadResults(numFrames, gBikeCollisions);
    RecordCyclingRoadResults(numFrames, gBikeCollisions);
}

static void RecordCyclingRoadResults(u32 numFrames, u8 numBikeCollisions)
{
    u16 low = VarGet(VAR_CYCLING_ROAD_RECORD_TIME_L);
    u16 high = VarGet(VAR_CYCLING_ROAD_RECORD_TIME_H);
    u32 framesRecord = low + (high << 16);

    if (framesRecord > numFrames || framesRecord == 0)
    {
        VarSet(VAR_CYCLING_ROAD_RECORD_TIME_L, numFrames);
        VarSet(VAR_CYCLING_ROAD_RECORD_TIME_H, numFrames >> 16);
        VarSet(VAR_CYCLING_ROAD_RECORD_COLLISIONS, numBikeCollisions);
    }
}

u16 GetRecordedCyclingRoadResults(void)
{
    u16 low = VarGet(VAR_CYCLING_ROAD_RECORD_TIME_L);
    u16 high = VarGet(VAR_CYCLING_ROAD_RECORD_TIME_H);
    u32 framesRecord = low + (high << 16);

    if (framesRecord == 0)
        return FALSE;

    DetermineCyclingRoadResults(framesRecord, VarGet(VAR_CYCLING_ROAD_RECORD_COLLISIONS));
    return TRUE;
}

void UpdateCyclingRoadState(void)
{
    if (gLastUsedWarp.mapNum == MAP_NUM(ROUTE110_SEASIDE_CYCLING_ROAD_SOUTH_ENTRANCE) && gLastUsedWarp.mapGroup == MAP_GROUP(ROUTE110_SEASIDE_CYCLING_ROAD_SOUTH_ENTRANCE))
        return;

    if (VarGet(VAR_CYCLING_CHALLENGE_STATE) == 2 || VarGet(VAR_CYCLING_CHALLENGE_STATE) == 3)
    {
        VarSet(VAR_CYCLING_CHALLENGE_STATE, 0);
        Overworld_SetSavedMusic(MUS_DUMMY);
    }
}

void SetSSTidalFlag(void)
{
    FlagSet(FLAG_SYS_CRUISE_MODE);
    *GetVarPointer(VAR_CRUISE_STEP_COUNT) = 0;
}

void ResetSSTidalFlag(void)
{
    FlagClear(FLAG_SYS_CRUISE_MODE);
}

// Returns TRUE if the Cruise is over
bool32 CountSSTidalStep(u16 delta)
{
    if (!FlagGet(FLAG_SYS_CRUISE_MODE) || (*GetVarPointer(VAR_CRUISE_STEP_COUNT) += delta) < SS_TIDAL_MAX_STEPS)
        return FALSE;

    return TRUE;
}

u8 GetSSTidalLocation(s8 *mapGroup, s8 *mapNum, s16 *x, s16 *y)
{
    u16 *varCruiseStepCount = GetVarPointer(VAR_CRUISE_STEP_COUNT);
    switch (*GetVarPointer(VAR_SS_TIDAL_STATE))
    {
    case SS_TIDAL_BOARD_SLATEPORT:
    case SS_TIDAL_LAND_SLATEPORT:
        return SS_TIDAL_LOCATION_SLATEPORT;
    case SS_TIDAL_HALFWAY_LILYCOVE:
    case SS_TIDAL_EXIT_CURRENTS_RIGHT:
        return SS_TIDAL_LOCATION_ROUTE131;
    case SS_TIDAL_LAND_LILYCOVE:
    case SS_TIDAL_BOARD_LILYCOVE:
        return SS_TIDAL_LOCATION_LILYCOVE;
    case SS_TIDAL_DEPART_LILYCOVE:
    case SS_TIDAL_EXIT_CURRENTS_LEFT:
        return SS_TIDAL_LOCATION_ROUTE124;
    case SS_TIDAL_DEPART_SLATEPORT:
        if (*varCruiseStepCount < 60)
        {
            *mapNum = MAP_NUM(ROUTE134);
            *x = *varCruiseStepCount + 19;
        }
        else if (*varCruiseStepCount < 140)
        {
            *mapNum = MAP_NUM(ROUTE133);
            *x = *varCruiseStepCount - 60;
        }
        else
        {
            *mapNum = MAP_NUM(ROUTE132);
            *x = *varCruiseStepCount - 140;
        }
        break;
    case SS_TIDAL_HALFWAY_SLATEPORT:
        if (*varCruiseStepCount < 66)
        {
            *mapNum = MAP_NUM(ROUTE132);
            *x = 65 - *varCruiseStepCount;
        }
        else if (*varCruiseStepCount < 146)
        {
            *mapNum = MAP_NUM(ROUTE133);
            *x = 145 - *varCruiseStepCount;
        }
        else
        {
            *mapNum = MAP_NUM(ROUTE134);
            *x = 224 - *varCruiseStepCount;
        }
        break;
    }
    *mapGroup = MAP_GROUP(ROUTE132);
    *y = 20;
    return SS_TIDAL_LOCATION_CURRENTS;
}

bool32 ShouldDoWallyCall(void)
{
    if (FlagGet(FLAG_ENABLE_FIRST_WALLY_POKENAV_CALL))
    {
        switch (gMapHeader.mapType)
        {
        case MAP_TYPE_TOWN:
        case MAP_TYPE_CITY:
        case MAP_TYPE_ROUTE:
        case MAP_TYPE_OCEAN_ROUTE:
            if (++(*GetVarPointer(VAR_WALLY_CALL_STEP_COUNTER)) < 250)
                return FALSE;
            break;
        default:
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

bool32 ShouldDoScottFortreeCall(void)
{
    if (FlagGet(FLAG_SCOTT_CALL_FORTREE_GYM))
    {
        switch (gMapHeader.mapType)
        {
        case MAP_TYPE_TOWN:
        case MAP_TYPE_CITY:
        case MAP_TYPE_ROUTE:
        case MAP_TYPE_OCEAN_ROUTE:
            if (++(*GetVarPointer(VAR_SCOTT_FORTREE_CALL_STEP_COUNTER)) < 10)
                return FALSE;
            break;
        default:
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

bool32 ShouldDoScottBattleFrontierCall(void)
{
    if (FlagGet(FLAG_SCOTT_CALL_BATTLE_FRONTIER))
    {
        switch (gMapHeader.mapType)
        {
        case MAP_TYPE_TOWN:
        case MAP_TYPE_CITY:
        case MAP_TYPE_ROUTE:
        case MAP_TYPE_OCEAN_ROUTE:
            if (++(*GetVarPointer(VAR_SCOTT_BF_CALL_STEP_COUNTER)) < 10)
                return FALSE;
            break;
        default:
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

bool32 ShouldDoRoxanneCall(void)
{
    if (FlagGet(FLAG_ENABLE_ROXANNE_FIRST_CALL))
    {
        switch (gMapHeader.mapType)
        {
        case MAP_TYPE_TOWN:
        case MAP_TYPE_CITY:
        case MAP_TYPE_ROUTE:
        case MAP_TYPE_OCEAN_ROUTE:
            if (++(*GetVarPointer(VAR_ROXANNE_CALL_STEP_COUNTER)) < 250)
                return FALSE;
            break;
        default:
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

bool32 ShouldDoNormanRematchCall(void)
{
    if (FlagGet(FLAG_ENABLE_NORMAN_REMATCH_CALL))
    {
        switch (gMapHeader.mapType)
        {
        case MAP_TYPE_TOWN:
        case MAP_TYPE_CITY:
        case MAP_TYPE_ROUTE:
        case MAP_TYPE_OCEAN_ROUTE:
            if (++(*GetVarPointer(VAR_NORMAN_REMATCH_CALL_STEP_COUNTER)) < 250)
                return FALSE;
            break;
        default:
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

bool32 ShouldUpdateGymLeaderRematches(void)
{
    switch (gMapHeader.mapType)
    {
        case MAP_TYPE_TOWN:
        case MAP_TYPE_CITY:
        case MAP_TYPE_ROUTE:
        case MAP_TYPE_OCEAN_ROUTE:
            return TRUE;
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

bool32 ShouldDoRivalRayquazaCall(void)
{
    if (FlagGet(FLAG_DEFEATED_MAGMA_SPACE_CENTER))
    {
        switch (gMapHeader.mapType)
        {
        case MAP_TYPE_TOWN:
        case MAP_TYPE_CITY:
        case MAP_TYPE_ROUTE:
        case MAP_TYPE_OCEAN_ROUTE:
            if (++(*GetVarPointer(VAR_RIVAL_RAYQUAZA_CALL_STEP_COUNTER)) < 250)
                return FALSE;
            break;
        default:
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

u8 GetLinkPartnerNames(void)
{
    u8 i;
    u8 j = 0;
    u8 myLinkPlayerNumber = GetMultiplayerId();
    u8 nLinkPlayers = GetLinkPlayerCount();
    for (i = 0; i < nLinkPlayers; i++)
    {
        if (myLinkPlayerNumber != i)
        {
            StringCopy(gTVStringVarPtrs[j], gLinkPlayers[i].name);
            j++;
        }
    }
    return nLinkPlayers;
}

void SpawnLinkPartnerObjectEvent(void)
{
    u8 j = 0;
    s16 x = 0;
    s16 y = 0;
    u8 movementTypes[] = {
        MOVEMENT_TYPE_FACE_UP,
        MOVEMENT_TYPE_FACE_LEFT,
        MOVEMENT_TYPE_FACE_DOWN,
        MOVEMENT_TYPE_FACE_RIGHT
    };
    s8 coordOffsets[][2] = {
        { 0,  1},
        { 1,  0},
        { 0, -1},
        {-1,  0}
    };
    u8 myLinkPlayerNumber;
    u8 playerFacingDirection;
    u16 linkSpriteId;
    u8 i;

    myLinkPlayerNumber = GetMultiplayerId();
    playerFacingDirection = GetPlayerFacingDirection();
    switch (playerFacingDirection)
    {
    case DIR_WEST:
        j = 2;
        x = gSaveBlock1Ptr->pos.x - 1;
        y = gSaveBlock1Ptr->pos.y;
        break;
    case DIR_NORTH:
        j = 1;
        x = gSaveBlock1Ptr->pos.x;
        y = gSaveBlock1Ptr->pos.y - 1;
        break;
    case DIR_EAST:
        x = gSaveBlock1Ptr->pos.x + 1;
        y = gSaveBlock1Ptr->pos.y;
        break;
    case DIR_SOUTH:
        j = 3;
        x = gSaveBlock1Ptr->pos.x;
        y = gSaveBlock1Ptr->pos.y + 1;
    }
    for (i = 0; i < gSpecialVar_0x8004; i++)
    {
        if (myLinkPlayerNumber != i)
        {
            switch ((u8)gLinkPlayers[i].version)
            {
            case VERSION_RUBY:
            case VERSION_SAPPHIRE:
                if (gLinkPlayers[i].gender == 0)
                    linkSpriteId = OBJ_EVENT_GFX_LINK_RS_BRENDAN;
                else
                    linkSpriteId = OBJ_EVENT_GFX_LINK_RS_MAY;
                break;
            case VERSION_EMERALD:
                if (gLinkPlayers[i].gender == 0)
                    linkSpriteId = OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL;
                else
                    linkSpriteId = OBJ_EVENT_GFX_RIVAL_MAY_NORMAL;
                break;
            default:
                if (gLinkPlayers[i].gender == 0)
                    linkSpriteId = OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL;
                else
                    linkSpriteId = OBJ_EVENT_GFX_RIVAL_MAY_NORMAL;
                break;
            }
            SpawnSpecialObjectEventParameterized(linkSpriteId, movementTypes[j], 240 - i, coordOffsets[j][0] + x + MAP_OFFSET, coordOffsets[j][1] + y + MAP_OFFSET, 0);
            LoadLinkPartnerObjectEventSpritePalette(linkSpriteId, 240 - i, i);
            j++;
            if (j == MAX_LINK_PLAYERS)
                j = 0;
        }
    }
}

static void LoadLinkPartnerObjectEventSpritePalette(u16 graphicsId, u8 localEventId, u8 paletteNum)
{
    u8 adjustedPaletteNum;
    // Note: This temp var is necessary; paletteNum += 6 doesn't match.
    adjustedPaletteNum = paletteNum + 6;
    if (graphicsId == OBJ_EVENT_GFX_LINK_RS_BRENDAN ||
        graphicsId == OBJ_EVENT_GFX_LINK_RS_MAY ||
        graphicsId == OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL ||
        graphicsId == OBJ_EVENT_GFX_RIVAL_MAY_NORMAL)
    {
        u8 obj = GetObjectEventIdByLocalIdAndMap(localEventId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
        if (obj != OBJECT_EVENTS_COUNT)
        {
            u8 spriteId = gObjectEvents[obj].spriteId;
            struct Sprite *sprite = &gSprites[spriteId];
            sprite->oam.paletteNum = adjustedPaletteNum;

            switch (graphicsId)
            {
            case OBJ_EVENT_GFX_LINK_RS_BRENDAN:
                LoadPalette(gObjectEventPal_RubySapphireBrendan, OBJ_PLTT_ID(adjustedPaletteNum), PLTT_SIZE_4BPP);
                break;
            case OBJ_EVENT_GFX_LINK_RS_MAY:
                LoadPalette(gObjectEventPal_RubySapphireMay, OBJ_PLTT_ID(adjustedPaletteNum), PLTT_SIZE_4BPP);
                break;
            case OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL:
                LoadPalette(gObjectEventPal_Brendan, OBJ_PLTT_ID(adjustedPaletteNum), PLTT_SIZE_4BPP);
                break;
            case OBJ_EVENT_GFX_RIVAL_MAY_NORMAL:
                LoadPalette(gObjectEventPal_May, OBJ_PLTT_ID(adjustedPaletteNum), PLTT_SIZE_4BPP);
                break;
            }
        }
    }
}

static const struct UCoords8 sMauvilleGymSwitchCoords[] =
{
    { 0 + MAP_OFFSET, 15 + MAP_OFFSET},
    { 4 + MAP_OFFSET, 12 + MAP_OFFSET},
    { 3 + MAP_OFFSET,  9 + MAP_OFFSET},
    { 8 + MAP_OFFSET,  9 + MAP_OFFSET}
};

// Presses the stepped-on switch and raises the rest
void MauvilleGymPressSwitch(void)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sMauvilleGymSwitchCoords); i++)
    {
        if (i == gSpecialVar_0x8004)
            MapGridSetMetatileIdAt(sMauvilleGymSwitchCoords[i].x, sMauvilleGymSwitchCoords[i].y, METATILE_MauvilleGym_PressedSwitch);
        else
            MapGridSetMetatileIdAt(sMauvilleGymSwitchCoords[i].x, sMauvilleGymSwitchCoords[i].y, METATILE_MauvilleGym_RaisedSwitch);
    }
}

// Sets the gym barriers back to the default state; their alt state is handled by MauvilleCity_Gym_EventScript_SetAltBarriers
void MauvilleGymSetDefaultBarriers(void)
{
    int x, y;
    // All switches/barriers are within these coord ranges
    for (y = 5 + MAP_OFFSET; y < 17 + MAP_OFFSET; y++)
    {
        for (x = 0 + MAP_OFFSET; x < 9 + MAP_OFFSET; x++)
        {
            switch (MapGridGetMetatileIdAt(x, y))
            {
            case METATILE_MauvilleGym_GreenBeamH1_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamH1_Off);
                break;
            case METATILE_MauvilleGym_GreenBeamH2_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamH2_Off);
                break;
            case METATILE_MauvilleGym_GreenBeamH3_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamH3_Off);
                break;
            case METATILE_MauvilleGym_GreenBeamH4_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamH4_Off);
                break;
            case METATILE_MauvilleGym_GreenBeamH1_Off:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamH1_On);
                break;
            case METATILE_MauvilleGym_GreenBeamH2_Off:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamH2_On);
                break;
            case METATILE_MauvilleGym_GreenBeamH3_Off:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamH3_On | MAPGRID_COLLISION_MASK);
                break;
            case METATILE_MauvilleGym_GreenBeamH4_Off:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamH4_On | MAPGRID_COLLISION_MASK);
                break;
            case METATILE_MauvilleGym_RedBeamH1_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamH1_Off);
                break;
            case METATILE_MauvilleGym_RedBeamH2_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamH2_Off);
                break;
            case METATILE_MauvilleGym_RedBeamH3_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamH3_Off);
                break;
            case METATILE_MauvilleGym_RedBeamH4_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamH4_Off);
                break;
            case METATILE_MauvilleGym_RedBeamH1_Off:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamH1_On);
                break;
            case METATILE_MauvilleGym_RedBeamH2_Off:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamH2_On);
                break;
            case METATILE_MauvilleGym_RedBeamH3_Off:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamH3_On | MAPGRID_COLLISION_MASK);
                break;
            case METATILE_MauvilleGym_RedBeamH4_Off:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamH4_On | MAPGRID_COLLISION_MASK);
                break;
            case METATILE_MauvilleGym_GreenBeamV1_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_PoleBottom_On | MAPGRID_COLLISION_MASK);
                break;
            case METATILE_MauvilleGym_GreenBeamV2_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_FloorTile);
                break;
            case METATILE_MauvilleGym_RedBeamV1_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_PoleBottom_Off | MAPGRID_COLLISION_MASK);
                break;
            case METATILE_MauvilleGym_RedBeamV2_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_FloorTile);
                break;
            case METATILE_MauvilleGym_PoleBottom_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamV1_On | MAPGRID_COLLISION_MASK);
                break;
            case METATILE_MauvilleGym_FloorTile:
                if (MapGridGetMetatileIdAt(x, y - 1) == METATILE_MauvilleGym_GreenBeamV1_On)
                    MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamV2_On | MAPGRID_COLLISION_MASK);
                else
                    MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamV2_On | MAPGRID_COLLISION_MASK);
                break;
            case METATILE_MauvilleGym_PoleBottom_Off:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamV1_On | MAPGRID_COLLISION_MASK);
                break;
            case METATILE_MauvilleGym_PoleTop_Off:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_PoleTop_On | MAPGRID_COLLISION_MASK);
                break;
            case METATILE_MauvilleGym_PoleTop_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_PoleTop_Off);
                break;
            }
        }
    }
}

// Presses all switches and deactivates all beams.
void MauvilleGymDeactivatePuzzle(void)
{
    int i, x, y;
    const struct UCoords8 *switchCoords = sMauvilleGymSwitchCoords;
    for (i = ARRAY_COUNT(sMauvilleGymSwitchCoords) - 1; i >= 0; i--)
    {
        MapGridSetMetatileIdAt(switchCoords->x, switchCoords->y, METATILE_MauvilleGym_PressedSwitch);
        switchCoords++;
    }
    for (y = 5 + MAP_OFFSET; y < 17 + MAP_OFFSET; y++)
    {
        for (x = 0 + MAP_OFFSET; x < 9 + MAP_OFFSET; x++)
        {
            switch (MapGridGetMetatileIdAt(x, y))
            {
            case METATILE_MauvilleGym_GreenBeamH1_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamH1_Off);
                break;
            case METATILE_MauvilleGym_GreenBeamH2_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamH2_Off);
                break;
            case METATILE_MauvilleGym_GreenBeamH3_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamH3_Off);
                break;
            case METATILE_MauvilleGym_GreenBeamH4_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_GreenBeamH4_Off);
                break;
            case METATILE_MauvilleGym_RedBeamH1_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamH1_Off);
                break;
            case METATILE_MauvilleGym_RedBeamH2_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamH2_Off);
                break;
            case METATILE_MauvilleGym_RedBeamH3_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamH3_Off);
                break;
            case METATILE_MauvilleGym_RedBeamH4_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_RedBeamH4_Off);
                break;
            case METATILE_MauvilleGym_GreenBeamV1_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_PoleBottom_On | MAPGRID_COLLISION_MASK);
                break;
            case METATILE_MauvilleGym_RedBeamV1_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_PoleBottom_Off | MAPGRID_COLLISION_MASK);
                break;
            case METATILE_MauvilleGym_GreenBeamV2_On:
            case METATILE_MauvilleGym_RedBeamV2_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_FloorTile);
                break;
            case METATILE_MauvilleGym_PoleTop_On:
                MapGridSetMetatileIdAt(x, y, METATILE_MauvilleGym_PoleTop_Off);
                break;
            }
        }
    }
}

static const bool8 sSlidingDoorNextFrameDelay[] = {0, 1, 1, 1, 1};

static const u16 sPetalburgGymSlidingDoorMetatiles[] = {
    METATILE_PetalburgGym_SlidingDoor_Frame0,
    METATILE_PetalburgGym_SlidingDoor_Frame1,
    METATILE_PetalburgGym_SlidingDoor_Frame2,
    METATILE_PetalburgGym_SlidingDoor_Frame3,
    METATILE_PetalburgGym_SlidingDoor_Frame4,
};

void PetalburgGymSlideOpenRoomDoors(void)
{
    sSlidingDoorNextFrameCounter = 0;
    sSlidingDoorFrame = 0;
    PlaySE(SE_UNLOCK);
    CreateTask(Task_PetalburgGymSlideOpenRoomDoors, 8);
}

static void Task_PetalburgGymSlideOpenRoomDoors(u8 taskId)
{
    if (sSlidingDoorNextFrameDelay[sSlidingDoorFrame] == sSlidingDoorNextFrameCounter)
    {
        PetalburgGymSetDoorMetatiles(gSpecialVar_0x8004, sPetalburgGymSlidingDoorMetatiles[sSlidingDoorFrame]);
        sSlidingDoorNextFrameCounter = 0;
        if ((++sSlidingDoorFrame) == ARRAY_COUNT(sPetalburgGymSlidingDoorMetatiles))
        {
            DestroyTask(taskId);
            ScriptContext_Enable();
        }
    }
    else
    {
        sSlidingDoorNextFrameCounter++;
    }
}

static void PetalburgGymSetDoorMetatiles(u8 roomNumber, u16 metatileId)
{
    u16 doorCoordsX[4];
    u16 doorCoordsY[4];
    u8 i;
    u8 nDoors = 0;
    switch (roomNumber)
    {
    case 1:
        nDoors = 2;
        doorCoordsX[0] = 1;
        doorCoordsX[1] = 7;
        doorCoordsY[0] = 104;
        doorCoordsY[1] = 104;
        break;
    case 2:
        nDoors = 2;
        doorCoordsX[0] = 1;
        doorCoordsX[1] = 7;
        doorCoordsY[0] = 78;
        doorCoordsY[1] = 78;
        break;
    case 3:
        nDoors = 2;
        doorCoordsX[0] = 1;
        doorCoordsX[1] = 7;
        doorCoordsY[0] = 91;
        doorCoordsY[1] = 91;
        break;
    case 4:
        nDoors = 1;
        doorCoordsX[0] = 7;
        doorCoordsY[0] = 39;
        break;
    case 5:
        nDoors = 2;
        doorCoordsX[0] = 1;
        doorCoordsX[1] = 7;
        doorCoordsY[0] = 52;
        doorCoordsY[1] = 52;
        break;
    case 6:
        nDoors = 1;
        doorCoordsX[0] = 1;
        doorCoordsY[0] = 65;
        break;
    case 7:
        nDoors = 1;
        doorCoordsX[0] = 7;
        doorCoordsY[0] = 13;
        break;
    case 8:
        nDoors = 1;
        doorCoordsX[0] = 1;
        doorCoordsY[0] = 26;
        break;
    }
    for (i = 0; i < nDoors; i++)
    {
        MapGridSetMetatileIdAt(doorCoordsX[i] + MAP_OFFSET, doorCoordsY[i] + MAP_OFFSET, metatileId | MAPGRID_COLLISION_MASK);
        MapGridSetMetatileIdAt(doorCoordsX[i] + MAP_OFFSET, doorCoordsY[i] + MAP_OFFSET + 1, (metatileId + METATILE_ROW_WIDTH) | MAPGRID_COLLISION_MASK);
    }
    DrawWholeMapView();
}

void PetalburgGymUnlockRoomDoors(void)
{
    PetalburgGymSetDoorMetatiles(gSpecialVar_0x8004, sPetalburgGymSlidingDoorMetatiles[4]);
}

void ShowFieldMessageStringVar4(void)
{
    ShowFieldMessage(gStringVar4);
}

void StorePlayerCoordsInVars(void)
{
    gSpecialVar_0x8004 = gSaveBlock1Ptr->pos.x;
    gSpecialVar_0x8005 = gSaveBlock1Ptr->pos.y;
}

u8 GetPlayerTrainerIdOnesDigit(void)
{
    return (u16)((gSaveBlock2Ptr->playerTrainerId[1] << 8) | gSaveBlock2Ptr->playerTrainerId[0]) % 10;
}

void GetPlayerBigGuyGirlString(void)
{
    if (gSaveBlock2Ptr->playerGender == MALE)
        StringCopy(gStringVar1, gText_BigGuy);
    else
        StringCopy(gStringVar1, gText_BigGirl);
}

void GetRivalSonDaughterString(void)
{
    if (gSaveBlock2Ptr->playerGender == MALE)
        StringCopy(gStringVar1, gText_Daughter);
    else
        StringCopy(gStringVar1, gText_Son);
}

u8 GetBattleOutcome(void)
{
    return gBattleOutcome;
}

void CableCarWarp(void)
{
    if (gSpecialVar_0x8004 != 0)
        SetWarpDestination(MAP_GROUP(ROUTE112_CABLE_CAR_STATION), MAP_NUM(ROUTE112_CABLE_CAR_STATION), WARP_ID_NONE, 6, 4);
    else
        SetWarpDestination(MAP_GROUP(MT_CHIMNEY_CABLE_CAR_STATION), MAP_NUM(MT_CHIMNEY_CABLE_CAR_STATION), WARP_ID_NONE, 6, 4);
}

void SetHiddenItemFlag(void)
{
    FlagSet(gSpecialVar_0x8004);
}

u16 GetWeekCount(void)
{
    u16 weekCount = gLocalTime.days / 7;
    if (weekCount > 9999)
        weekCount = 9999;

    return weekCount;
}

u8 GetLeadMonFriendshipScore(void)
{
    struct Pokemon *pokemon = &gPlayerParty[GetLeadMonIndex()];
    if (GetMonData(pokemon, MON_DATA_FRIENDSHIP) == MAX_FRIENDSHIP)
        return FRIENDSHIP_MAX;
    if (GetMonData(pokemon, MON_DATA_FRIENDSHIP) >= 200)
        return FRIENDSHIP_200_TO_254;
    if (GetMonData(pokemon, MON_DATA_FRIENDSHIP) >= 150)
        return FRIENDSHIP_150_TO_199;
    if (GetMonData(pokemon, MON_DATA_FRIENDSHIP) >= 100)
        return FRIENDSHIP_100_TO_149;
    if (GetMonData(pokemon, MON_DATA_FRIENDSHIP) >= 50)
        return FRIENDSHIP_50_TO_99;
    if (GetMonData(pokemon, MON_DATA_FRIENDSHIP) >= 1)
        return FRIENDSHIP_1_TO_49;

    return FRIENDSHIP_NONE;
}

static void CB2_FieldShowRegionMap(void)
{
    FieldInitRegionMap(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

void FieldShowRegionMap(void)
{
    SetMainCallback2(CB2_FieldShowRegionMap);
}

// Task data for Task_PCTurnOnEffect and Task_LotteryCornerComputerEffect
#define tPaused       data[0] // Never set
#define tTaskId       data[1]
#define tFlickerCount data[2]
#define tTimer        data[3]
#define tIsScreenOn   data[4]

// For this special, gSpecialVar_0x8004 is expected to be some PC_LOCATION_* value.
void DoPCTurnOnEffect(void)
{
    if (FuncIsActiveTask(Task_PCTurnOnEffect) != TRUE)
    {
        u8 taskId = CreateTask(Task_PCTurnOnEffect, 8);
        gTasks[taskId].tPaused = FALSE;
        gTasks[taskId].tTaskId = taskId;
        gTasks[taskId].tFlickerCount = 0;
        gTasks[taskId].tTimer = 0;
        gTasks[taskId].tIsScreenOn = FALSE;
    }
}

static void Task_PCTurnOnEffect(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    if (!task->tPaused)
        PCTurnOnEffect(task);
}

static void PCTurnOnEffect(struct Task *task)
{
    u8 playerDirection;
    s8 dx = 0;
    s8 dy = 0;
    if (task->tTimer == 6)
    {
        task->tTimer = 0;

        // Get where the PC should be, depending on where the player is looking.
        playerDirection = GetPlayerFacingDirection();
        switch (playerDirection)
        {
        case DIR_NORTH:
            dx = 0;
            dy = -1;
            break;
        case DIR_WEST:
            dx = -1;
            dy = -1;
            break;
        case DIR_EAST:
            dx = 1;
            dy = -1;
            break;
        }

        // Update map
        PCTurnOnEffect_SetMetatile(task->tIsScreenOn, dx, dy);
        DrawWholeMapView();

        // Screen flickers 5 times. Odd number and starting with the
        // screen off means the animation ends with the screen on.
        task->tIsScreenOn ^= 1;
        if (++task->tFlickerCount == 5)
            DestroyTask(task->tTaskId);
    }
    task->tTimer++;
}

static void PCTurnOnEffect_SetMetatile(s16 isScreenOn, s8 dx, s8 dy)
{
    u16 metatileId = 0;
    if (isScreenOn)
    {
        // Screen is on, set it off
        if (gSpecialVar_0x8004 == PC_LOCATION_OTHER)
            metatileId = METATILE_Building_PC_Off;
        else if (gSpecialVar_0x8004 == PC_LOCATION_BRENDANS_HOUSE)
            metatileId = METATILE_BrendansMaysHouse_BrendanPC_Off;
        else if (gSpecialVar_0x8004 == PC_LOCATION_MAYS_HOUSE)
            metatileId = METATILE_BrendansMaysHouse_MayPC_Off;
    }
    else
    {
        // Screen is off, set it on
        if (gSpecialVar_0x8004 == PC_LOCATION_OTHER)
            metatileId = METATILE_Building_PC_On;
        else if (gSpecialVar_0x8004 == PC_LOCATION_BRENDANS_HOUSE)
            metatileId = METATILE_BrendansMaysHouse_BrendanPC_On;
        else if (gSpecialVar_0x8004 == PC_LOCATION_MAYS_HOUSE)
            metatileId = METATILE_BrendansMaysHouse_MayPC_On;
    }
    MapGridSetMetatileIdAt(gSaveBlock1Ptr->pos.x + dx + MAP_OFFSET, gSaveBlock1Ptr->pos.y + dy + MAP_OFFSET, metatileId | MAPGRID_COLLISION_MASK);
}

// For this special, gSpecialVar_0x8004 is expected to be some PC_LOCATION_* value.
void DoPCTurnOffEffect(void)
{
    PCTurnOffEffect();
}

static void PCTurnOffEffect(void)
{
    s8 dx = 0;
    s8 dy = 0;
    u16 metatileId = 0;

    // Get where the PC should be, depending on where the player is looking.
    u8 playerDirection = GetPlayerFacingDirection();
    switch (playerDirection)
    {
    case DIR_NORTH:
        dx = 0;
        dy = -1;
        break;
    case DIR_WEST:
        dx = -1;
        dy = -1;
        break;
    case DIR_EAST:
        dx = 1;
        dy = -1;
        break;
    }

    if (gSpecialVar_0x8004 == PC_LOCATION_OTHER)
        metatileId = METATILE_Building_PC_Off;
    else if (gSpecialVar_0x8004 == PC_LOCATION_BRENDANS_HOUSE)
        metatileId = METATILE_BrendansMaysHouse_BrendanPC_Off;
    else if (gSpecialVar_0x8004 == PC_LOCATION_MAYS_HOUSE)
        metatileId = METATILE_BrendansMaysHouse_MayPC_Off;

    MapGridSetMetatileIdAt(gSaveBlock1Ptr->pos.x + dx + MAP_OFFSET, gSaveBlock1Ptr->pos.y + dy + MAP_OFFSET, metatileId | MAPGRID_COLLISION_MASK);
    DrawWholeMapView();
}

void DoLotteryCornerComputerEffect(void)
{
    if (FuncIsActiveTask(Task_LotteryCornerComputerEffect) != TRUE)
    {
        u8 taskId = CreateTask(Task_LotteryCornerComputerEffect, 8);
        gTasks[taskId].tPaused = FALSE;
        gTasks[taskId].tTaskId = taskId;
        gTasks[taskId].tFlickerCount = 0;
        gTasks[taskId].tTimer = 0;
        gTasks[taskId].tIsScreenOn = FALSE;
    }
}

static void Task_LotteryCornerComputerEffect(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    if (!task->tPaused)
        LotteryCornerComputerEffect(task);
}

static void LotteryCornerComputerEffect(struct Task *task)
{
    if (task->tTimer == 6)
    {
        task->tTimer = 0;
        if (task->tIsScreenOn)
        {
            // Screen is on, set it off
            MapGridSetMetatileIdAt(11 + MAP_OFFSET, 1 + MAP_OFFSET, METATILE_Shop_Laptop1_Normal | MAPGRID_COLLISION_MASK);
            MapGridSetMetatileIdAt(11 + MAP_OFFSET, 2 + MAP_OFFSET, METATILE_Shop_Laptop2_Normal | MAPGRID_COLLISION_MASK);
        }
        else
        {
            // Screen is off, set it on
            MapGridSetMetatileIdAt(11 + MAP_OFFSET, 1 + MAP_OFFSET, METATILE_Shop_Laptop1_Flash | MAPGRID_COLLISION_MASK);
            MapGridSetMetatileIdAt(11 + MAP_OFFSET, 2 + MAP_OFFSET, METATILE_Shop_Laptop2_Flash | MAPGRID_COLLISION_MASK);
        }
        DrawWholeMapView();

        // Screen flickers 5 times. Odd number and starting with the
        // screen off means the animation ends with the screen on.
        task->tIsScreenOn ^= 1;
        if (++task->tFlickerCount == 5)
            DestroyTask(task->tTaskId);
    }
    task->tTimer++;
}

void EndLotteryCornerComputerEffect(void)
{
    MapGridSetMetatileIdAt(11 + MAP_OFFSET, 1 + MAP_OFFSET, METATILE_Shop_Laptop1_Normal | MAPGRID_COLLISION_MASK);
    MapGridSetMetatileIdAt(11 + MAP_OFFSET, 2 + MAP_OFFSET, METATILE_Shop_Laptop2_Normal | MAPGRID_COLLISION_MASK);
    DrawWholeMapView();
}

#undef tPaused
#undef tTaskId
#undef tFlickerCount
#undef tTimer
#undef tIsScreenOn

void SetTrickHouseNuggetFlag(void)
{
    u16 *specVar = &gSpecialVar_0x8004;
    u16 flag = FLAG_HIDDEN_ITEM_TRICK_HOUSE_NUGGET;
    *specVar = flag;
    FlagSet(flag);
}

void ResetTrickHouseNuggetFlag(void)
{
    u16 *specVar = &gSpecialVar_0x8004;
    u16 flag = FLAG_HIDDEN_ITEM_TRICK_HOUSE_NUGGET;
    *specVar = flag;
    FlagClear(flag);
}

bool8 CheckLeadMonCool(void)
{
    if (GetMonData(&gPlayerParty[GetLeadMonIndex()], MON_DATA_COOL) < 200)
        return FALSE;

    return TRUE;
}

bool8 CheckLeadMonBeauty(void)
{
    if (GetMonData(&gPlayerParty[GetLeadMonIndex()], MON_DATA_BEAUTY) < 200)
        return FALSE;

    return TRUE;
}

bool8 CheckLeadMonCute(void)
{
    if (GetMonData(&gPlayerParty[GetLeadMonIndex()], MON_DATA_CUTE) < 200)
        return FALSE;

    return TRUE;
}

bool8 CheckLeadMonSmart(void)
{
    if (GetMonData(&gPlayerParty[GetLeadMonIndex()], MON_DATA_SMART) < 200)
        return FALSE;

    return TRUE;
}

bool8 CheckLeadMonTough(void)
{
    if (GetMonData(&gPlayerParty[GetLeadMonIndex()], MON_DATA_TOUGH) < 200)
        return FALSE;

    return TRUE;
}

void IsGrassTypeInParty(void)
{
    u8 i;
    u16 species;
    struct Pokemon *pokemon;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        pokemon = &gPlayerParty[i];
        if (GetMonData(pokemon, MON_DATA_SANITY_HAS_SPECIES) && !GetMonData(pokemon, MON_DATA_IS_EGG))
        {
            species = GetMonData(pokemon, MON_DATA_SPECIES);
            if (gSpeciesInfo[species].types[0] == TYPE_GRASS || gSpeciesInfo[species].types[1] == TYPE_GRASS)
            {
                gSpecialVar_Result = TRUE;
                return;
            }
        }
    }
    gSpecialVar_Result = FALSE;
}

void SpawnCameraObject(void)
{
    u8 obj = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_BOY_1,
                                                  MOVEMENT_TYPE_FACE_DOWN,
                                                  OBJ_EVENT_ID_CAMERA,
                                                  gSaveBlock1Ptr->pos.x + MAP_OFFSET,
                                                  gSaveBlock1Ptr->pos.y + MAP_OFFSET,
                                                  3); // elevation
    gObjectEvents[obj].invisible = TRUE;
    CameraObjectSetFollowedSpriteId(gObjectEvents[obj].spriteId);
}

void RemoveCameraObject(void)
{
    CameraObjectSetFollowedSpriteId(GetPlayerAvatarSpriteId());
    RemoveObjectEventByLocalIdAndMap(OBJ_EVENT_ID_CAMERA, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
}

u8 GetPokeblockNameByMonNature(void)
{
    return CopyMonFavoritePokeblockName(GetNature(&gPlayerParty[GetLeadMonIndex()], FALSE), gStringVar1);
}

void GetSecretBaseNearbyMapName(void)
{
    GetMapName(gStringVar1, VarGet(VAR_SECRET_BASE_MAP), 0);
}

u16 GetBattleTowerSinglesStreak(void)
{
    return GetGameStat(GAME_STAT_BATTLE_TOWER_SINGLES_STREAK);
}

void BufferEReaderTrainerName(void)
{
    GetEreaderTrainerName(gStringVar1);
}

u16 GetSlotMachineId(void)
{
    static const u8 sSlotMachineRandomSeeds[SLOT_MACHINE_COUNT] = {12, 2, 4, 5, 1, 8, 7, 11, 3, 10, 9, 6};
    static const u8 sSlotMachineIds[SLOT_MACHINE_COUNT] = {
        SLOT_MACHINE_UNLUCKIEST,
        SLOT_MACHINE_UNLUCKIER,
        SLOT_MACHINE_UNLUCKIER,
        SLOT_MACHINE_UNLUCKY,
        SLOT_MACHINE_UNLUCKY,
        SLOT_MACHINE_UNLUCKY,
        SLOT_MACHINE_LUCKY,
        SLOT_MACHINE_LUCKY,
        SLOT_MACHINE_LUCKY,
        SLOT_MACHINE_LUCKIER,
        SLOT_MACHINE_LUCKIER,
        SLOT_MACHINE_LUCKIEST
    };
    static const u8 sSlotMachineServiceDayIds[SLOT_MACHINE_COUNT] = {
        SLOT_MACHINE_LUCKY,
        SLOT_MACHINE_LUCKY,
        SLOT_MACHINE_LUCKY,
        SLOT_MACHINE_LUCKY,
        SLOT_MACHINE_LUCKY,
        SLOT_MACHINE_LUCKY,
        SLOT_MACHINE_LUCKIER,
        SLOT_MACHINE_LUCKIER,
        SLOT_MACHINE_LUCKIER,
        SLOT_MACHINE_LUCKIER,
        SLOT_MACHINE_LUCKIEST,
        SLOT_MACHINE_LUCKIEST
    };

    u32 rnd = gSaveBlock1Ptr->dewfordTrends[0].trendiness + gSaveBlock1Ptr->dewfordTrends[0].rand + sSlotMachineRandomSeeds[gSpecialVar_0x8004];
    if (IsPokeNewsActive(POKENEWS_GAME_CORNER))
        return sSlotMachineServiceDayIds[rnd % SLOT_MACHINE_COUNT];

    return sSlotMachineIds[rnd % SLOT_MACHINE_COUNT];
}

bool8 FoundAbandonedShipRoom1Key(void)
{
    u16 *specVar = &gSpecialVar_0x8004;
    u16 flag = FLAG_HIDDEN_ITEM_ABANDONED_SHIP_RM_1_KEY;
    *specVar = flag;
    if (!FlagGet(flag))
        return FALSE;

    return TRUE;
}

bool8 FoundAbandonedShipRoom2Key(void)
{
    u16 *specVar = &gSpecialVar_0x8004;
    u16 flag = FLAG_HIDDEN_ITEM_ABANDONED_SHIP_RM_2_KEY;
    *specVar = flag;
    if (!FlagGet(flag))
        return FALSE;

    return TRUE;
}

bool8 FoundAbandonedShipRoom4Key(void)
{
    u16 *specVar = &gSpecialVar_0x8004;
    u16 flag = FLAG_HIDDEN_ITEM_ABANDONED_SHIP_RM_4_KEY;
    *specVar = flag;
    if (!FlagGet(flag))
        return FALSE;

    return TRUE;
}

bool8 FoundAbandonedShipRoom6Key(void)
{
    u16 *specVar = &gSpecialVar_0x8004;
    u16 flag = FLAG_HIDDEN_ITEM_ABANDONED_SHIP_RM_6_KEY;
    *specVar = flag;
    if (!FlagGet(flag))
        return FALSE;

    return TRUE;
}

bool8 LeadMonHasEffortRibbon(void)
{
    return GetMonData(&gPlayerParty[GetLeadMonIndex()], MON_DATA_EFFORT_RIBBON, NULL);
}

void GiveLeadMonEffortRibbon(void)
{
    bool8 ribbonSet;
    struct Pokemon *leadMon;
    IncrementGameStat(GAME_STAT_RECEIVED_RIBBONS);
    FlagSet(FLAG_SYS_RIBBON_GET);
    ribbonSet = TRUE;
    leadMon = &gPlayerParty[GetLeadMonIndex()];
    SetMonData(leadMon, MON_DATA_EFFORT_RIBBON, &ribbonSet);
    if (GetRibbonCount(leadMon) > NUM_CUTIES_RIBBONS)
        TryPutSpotTheCutiesOnAir(leadMon, MON_DATA_EFFORT_RIBBON);
}

bool8 Special_AreLeadMonEVsMaxedOut(void)
{
    if (GetMonEVCount(&gPlayerParty[GetLeadMonIndex()]) >= MAX_TOTAL_EVS)
        return TRUE;

    return FALSE;
}

u8 TryUpdateRusturfTunnelState(void)
{
    if (!FlagGet(FLAG_RUSTURF_TUNNEL_OPENED)
        && gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(RUSTURF_TUNNEL)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(RUSTURF_TUNNEL))
    {
        if (FlagGet(FLAG_HIDE_RUSTURF_TUNNEL_ROCK_1))
        {
            VarSet(VAR_RUSTURF_TUNNEL_STATE, 4);
            return TRUE;
        }
        else if (FlagGet(FLAG_HIDE_RUSTURF_TUNNEL_ROCK_2))
        {
            VarSet(VAR_RUSTURF_TUNNEL_STATE, 5);
            return TRUE;
        }
    }
    return FALSE;
}

void SetShoalItemFlag(u16 unused)
{
    FlagSet(FLAG_SYS_SHOAL_ITEM);
}

void LoadWallyZigzagoon(void)
{
    u16 monData;
    CreateMon(&gPlayerParty[0], SPECIES_ZIGZAGOON, 7, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    monData = TRUE;
    SetMonData(&gPlayerParty[0], MON_DATA_ABILITY_NUM, &monData);
    monData = MOVE_TACKLE;
    SetMonData(&gPlayerParty[0], MON_DATA_MOVE1, &monData);
    monData = MOVE_NONE;
    SetMonData(&gPlayerParty[0], MON_DATA_MOVE2, &monData);
    SetMonData(&gPlayerParty[0], MON_DATA_MOVE3, &monData);
    SetMonData(&gPlayerParty[0], MON_DATA_MOVE4, &monData);
}

bool8 IsStarterInParty(void)
{
    u8 i;
    u16 starter = GetStarterPokemon(VarGet(VAR_STARTER_MON));
    u8 partyCount = CalculatePlayerPartyCount();
    for (i = 0; i < partyCount; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG, NULL) == starter)
            return TRUE;
    }
    return FALSE;
}

bool8 ScriptCheckFreePokemonStorageSpace(void)
{
    return CheckFreePokemonStorageSpace();
}

bool8 IsPokerusInParty(void)
{
    if (!CheckPartyPokerus(gPlayerParty, (1 << PARTY_SIZE) - 1))
        return FALSE;

    return TRUE;
}

// Task data for Task_ShakeCamera
#define tHorizontalPan  data[0]
#define tDelayCounter   data[1]
#define tNumShakes      data[2]
#define tDelay          data[3]
#define tVerticalPan    data[4]

void ShakeCamera(void)
{
    u8 taskId = CreateTask(Task_ShakeCamera, 9);
    gTasks[taskId].tHorizontalPan = gSpecialVar_0x8005;
    gTasks[taskId].tDelayCounter = 0;
    gTasks[taskId].tNumShakes = gSpecialVar_0x8006;
    gTasks[taskId].tDelay = gSpecialVar_0x8007;
    gTasks[taskId].tVerticalPan = gSpecialVar_0x8004;
    SetCameraPanningCallback(NULL);
    PlaySE(SE_M_STRENGTH);
}

static void Task_ShakeCamera(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    tDelayCounter++;
    if (tDelayCounter % tDelay == 0)
    {
        tDelayCounter = 0;
        tNumShakes--;
        tHorizontalPan = -tHorizontalPan;
        tVerticalPan = -tVerticalPan;
        SetCameraPanning(tHorizontalPan, tVerticalPan);
        if (tNumShakes == 0)
        {
            StopCameraShake(taskId);
            InstallCameraPanAheadCallback();
        }
    }
}

static void StopCameraShake(u8 taskId)
{
    DestroyTask(taskId);
    ScriptContext_Enable();
}

#undef tHorizontalPan
#undef tDelayCounter
#undef tNumShakes
#undef tDelay
#undef tVerticalPan

bool8 FoundBlackGlasses(void)
{
    return FlagGet(FLAG_HIDDEN_ITEM_ROUTE_116_BLACK_GLASSES);
}

void SetRoute119Weather(void)
{
    if (IsMapTypeOutdoors(GetLastUsedWarpMapType()) != TRUE)
        SetSavedWeather(WEATHER_ROUTE119_CYCLE);
}

void SetRoute123Weather(void)
{
    if (IsMapTypeOutdoors(GetLastUsedWarpMapType()) != TRUE)
        SetSavedWeather(WEATHER_ROUTE123_CYCLE);
}

u8 GetLeadMonIndex(void)
{
    u8 i;
    u8 partyCount = CalculatePlayerPartyCount();
    for (i = 0; i < partyCount; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG, NULL) != SPECIES_EGG
         && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG, NULL) != SPECIES_NONE)
            return i;
    }
    return 0;
}

u16 ScriptGetPartyMonSpecies(void)
{
    return GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_SPECIES_OR_EGG, NULL);
}

// Removed for Emerald
void TryInitBattleTowerAwardManObjectEvent(void)
{
    //TryInitLocalObjectEvent(6);
}

u16 GetDaysUntilPacifidlogTMAvailable(void)
{
    u16 tmReceivedDay = VarGet(VAR_PACIFIDLOG_TM_RECEIVED_DAY);
    if (gLocalTime.days - tmReceivedDay >= 7)
        return 0;
    else if (gLocalTime.days < 0)
        return 8;

    return 7 - (gLocalTime.days - tmReceivedDay);
}

u16 SetPacifidlogTMReceivedDay(void)
{
    VarSet(VAR_PACIFIDLOG_TM_RECEIVED_DAY, gLocalTime.days);
    return gLocalTime.days;
}

bool8 MonOTNameNotPlayer(void)
{
    if (GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_LANGUAGE) != GAME_LANGUAGE)
        return TRUE;

    GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_OT_NAME, gStringVar1);

    if (!StringCompare(gSaveBlock2Ptr->playerName, gStringVar1))
        return FALSE;

    return TRUE;
}

void BufferLottoTicketNumber(void)
{
    if (gSpecialVar_Result >= 10000)
    {
        ConvertIntToDecimalString(0, gSpecialVar_Result);
    }
    else if (gSpecialVar_Result >= 1000)
    {
        gStringVar1[0] = CHAR_0;
        ConvertIntToDecimalStringN(gStringVar1 + 1, gSpecialVar_Result, STR_CONV_MODE_LEFT_ALIGN, CountDigits(gSpecialVar_Result));
    }
    else if (gSpecialVar_Result >= 100)
    {
        gStringVar1[0] = CHAR_0;
        gStringVar1[1] = CHAR_0;
        ConvertIntToDecimalStringN(gStringVar1 + 2, gSpecialVar_Result, STR_CONV_MODE_LEFT_ALIGN, CountDigits(gSpecialVar_Result));
    }
    else if (gSpecialVar_Result >= 10)
    {
        gStringVar1[0] = CHAR_0;
        gStringVar1[1] = CHAR_0;
        gStringVar1[2] = CHAR_0;
        ConvertIntToDecimalStringN(gStringVar1 + 3, gSpecialVar_Result, STR_CONV_MODE_LEFT_ALIGN, CountDigits(gSpecialVar_Result));
    }
    else
    {
        gStringVar1[0] = CHAR_0;
        gStringVar1[1] = CHAR_0;
        gStringVar1[2] = CHAR_0;
        gStringVar1[3] = CHAR_0;
        ConvertIntToDecimalStringN(gStringVar1 + 4, gSpecialVar_Result, STR_CONV_MODE_LEFT_ALIGN, CountDigits(gSpecialVar_Result));
    }
}

u16 GetMysteryGiftCardStat(void)
{
    switch (gSpecialVar_Result)
    {
    case GET_NUM_STAMPS:
        return MysteryGift_GetCardStat(CARD_STAT_NUM_STAMPS);
    case GET_MAX_STAMPS:
        return MysteryGift_GetCardStat(CARD_STAT_MAX_STAMPS);
    case GET_CARD_BATTLES_WON:
        return MysteryGift_GetCardStat(CARD_STAT_BATTLES_WON);
    case GET_CARD_BATTLES_LOST: // Never occurs
        return MysteryGift_GetCardStat(CARD_STAT_BATTLES_LOST);
    case GET_CARD_NUM_TRADES: // Never occurs
        return MysteryGift_GetCardStat(CARD_STAT_NUM_TRADES);
    default:
        return 0;
    }
}

bool8 BufferTMHMMoveName(void)
{
    if (gSpecialVar_0x8004 >= ITEM_TM01 && gSpecialVar_0x8004 <= ITEM_HM08)
    {
        StringCopy(gStringVar2, gMoveNames[ItemIdToBattleMoveId(gSpecialVar_0x8004)]);
        return TRUE;
    }

    return FALSE;
}

bool8 IsBadEggInParty(void)
{
    u8 partyCount = CalculatePlayerPartyCount();
    u8 i;

    for (i = 0; i < partyCount; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_IS_BAD_EGG) == TRUE)
            return TRUE;
    }

    return FALSE;
}

bool8 InMultiPartnerRoom(void)
{
    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(BATTLE_FRONTIER_BATTLE_TOWER_MULTI_PARTNER_ROOM)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(BATTLE_FRONTIER_BATTLE_TOWER_MULTI_PARTNER_ROOM) &&
        VarGet(VAR_FRONTIER_BATTLE_MODE) == FRONTIER_MODE_MULTIS)
        return TRUE;
    return FALSE;
}

void OffsetCameraForBattle(void)
{
    SetCameraPanningCallback(NULL);
    SetCameraPanning(8, 0);
}

static const struct WindowTemplate sWindowTemplate_ElevatorFloor =
{
    .bg = 0,
    .tilemapLeft = 21,
    .tilemapTop = 1,
    .width = 8,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 8,
};

static const u8 *const sDeptStoreFloorNames[] =
{
    [DEPT_STORE_FLOORNUM_B4F] = gText_B4F,
    [DEPT_STORE_FLOORNUM_B3F] = gText_B3F,
    [DEPT_STORE_FLOORNUM_B2F] = gText_B2F,
    [DEPT_STORE_FLOORNUM_B1F] = gText_B1F,
    [DEPT_STORE_FLOORNUM_1F] = gText_1F,
    [DEPT_STORE_FLOORNUM_2F] = gText_2F,
    [DEPT_STORE_FLOORNUM_3F] = gText_3F,
    [DEPT_STORE_FLOORNUM_4F] = gText_4F,
    [DEPT_STORE_FLOORNUM_5F] = gText_5F,
    [DEPT_STORE_FLOORNUM_6F] = gText_6F,
    [DEPT_STORE_FLOORNUM_7F] = gText_7F,
    [DEPT_STORE_FLOORNUM_8F] = gText_8F,
    [DEPT_STORE_FLOORNUM_9F] = gText_9F,
    [DEPT_STORE_FLOORNUM_10F] = gText_10F,
    [DEPT_STORE_FLOORNUM_11F] = gText_11F,
    [DEPT_STORE_FLOORNUM_ROOFTOP] = gText_Rooftop
};

static const u16 sElevatorWindowTiles_Ascending[ELEVATOR_WINDOW_HEIGHT][ELEVATOR_LIGHT_STAGES] =
{
    {
        METATILE_BattleFrontier_Elevator_Top0,
        METATILE_BattleFrontier_Elevator_Top1,
        METATILE_BattleFrontier_Elevator_Top2
    },
    {
        METATILE_BattleFrontier_Elevator_Mid0,
        METATILE_BattleFrontier_Elevator_Mid1,
        METATILE_BattleFrontier_Elevator_Mid2
    },
    {
        METATILE_BattleFrontier_Elevator_Bottom0,
        METATILE_BattleFrontier_Elevator_Bottom1,
        METATILE_BattleFrontier_Elevator_Bottom2
    },
};

static const u16 sElevatorWindowTiles_Descending[ELEVATOR_WINDOW_HEIGHT][ELEVATOR_LIGHT_STAGES] =
{
    {
        METATILE_BattleFrontier_Elevator_Top0,
        METATILE_BattleFrontier_Elevator_Top2,
        METATILE_BattleFrontier_Elevator_Top1
    },
    {
        METATILE_BattleFrontier_Elevator_Mid0,
        METATILE_BattleFrontier_Elevator_Mid2,
        METATILE_BattleFrontier_Elevator_Mid1
    },
    {
        METATILE_BattleFrontier_Elevator_Bottom0,
        METATILE_BattleFrontier_Elevator_Bottom2,
        METATILE_BattleFrontier_Elevator_Bottom1
    },
};

void SetDeptStoreFloor(void)
{
    u8 deptStoreFloor;
    switch (gSaveBlock1Ptr->dynamicWarp.mapNum)
    {
    case MAP_NUM(LILYCOVE_CITY_DEPARTMENT_STORE_1F):
        deptStoreFloor = DEPT_STORE_FLOORNUM_1F;
        break;
    case MAP_NUM(LILYCOVE_CITY_DEPARTMENT_STORE_2F):
        deptStoreFloor = DEPT_STORE_FLOORNUM_2F;
        break;
    case MAP_NUM(LILYCOVE_CITY_DEPARTMENT_STORE_3F):
        deptStoreFloor = DEPT_STORE_FLOORNUM_3F;
        break;
    case MAP_NUM(LILYCOVE_CITY_DEPARTMENT_STORE_4F):
        deptStoreFloor = DEPT_STORE_FLOORNUM_4F;
        break;
    case MAP_NUM(LILYCOVE_CITY_DEPARTMENT_STORE_5F):
        deptStoreFloor = DEPT_STORE_FLOORNUM_5F;
        break;
    case MAP_NUM(LILYCOVE_CITY_DEPARTMENT_STORE_ROOFTOP):
        deptStoreFloor = DEPT_STORE_FLOORNUM_ROOFTOP;
        break;
    default:
        deptStoreFloor = DEPT_STORE_FLOORNUM_1F;
        break;
    }
    VarSet(VAR_DEPT_STORE_FLOOR, deptStoreFloor);
}

u16 GetDeptStoreDefaultFloorChoice(void)
{
    sLilycoveDeptStore_NeverRead = 0;
    sLilycoveDeptStore_DefaultFloorChoice = 0;

    if (gSaveBlock1Ptr->dynamicWarp.mapGroup == MAP_GROUP(LILYCOVE_CITY_DEPARTMENT_STORE_1F))
    {
        switch (gSaveBlock1Ptr->dynamicWarp.mapNum)
        {
        case MAP_NUM(LILYCOVE_CITY_DEPARTMENT_STORE_5F):
            sLilycoveDeptStore_NeverRead = 0;
            sLilycoveDeptStore_DefaultFloorChoice = 0;
            break;
        case MAP_NUM(LILYCOVE_CITY_DEPARTMENT_STORE_4F):
            sLilycoveDeptStore_NeverRead = 0;
            sLilycoveDeptStore_DefaultFloorChoice = 1;
            break;
        case MAP_NUM(LILYCOVE_CITY_DEPARTMENT_STORE_3F):
            sLilycoveDeptStore_NeverRead = 0;
            sLilycoveDeptStore_DefaultFloorChoice = 2;
            break;
        case MAP_NUM(LILYCOVE_CITY_DEPARTMENT_STORE_2F):
            sLilycoveDeptStore_NeverRead = 0;
            sLilycoveDeptStore_DefaultFloorChoice = 3;
            break;
        case MAP_NUM(LILYCOVE_CITY_DEPARTMENT_STORE_1F):
            sLilycoveDeptStore_NeverRead = 0;
            sLilycoveDeptStore_DefaultFloorChoice = 4;
            break;
        }
    }

    return sLilycoveDeptStore_DefaultFloorChoice;
}

// Task data for Task_MoveElevator
#define tTimer       data[1]
#define tMoveCounter data[2]
#define tVerticalPan data[4]
#define tTotalMoves  data[5]
#define tDescending  data[6]

// The maximum considered difference between floors.
// Elevator trips with a larger difference are treated the same
// (i.e. traveling 9 floors and 200 floors would take the same amount of time).
#define MAX_ELEVATOR_TRIP 9

// gSpecialVar_0x8005 here is expected to be the current floor number, and
// gSpecialVar_0x8006 is expected to be the destination floor number.
void MoveElevator(void)
{
    static const u8 sElevatorTripLength[MAX_ELEVATOR_TRIP] = { 8, 16, 24, 32, 38, 46, 52, 56, 57 };

    s16 *data = gTasks[CreateTask(Task_MoveElevator, 9)].data;
    u16 floorDelta;

    tTimer = 0;
    tMoveCounter = 0;
    tVerticalPan = 1;

    if (gSpecialVar_0x8005 > gSpecialVar_0x8006)
    {
        floorDelta = gSpecialVar_0x8005 - gSpecialVar_0x8006;
        tDescending = TRUE;
    }
    else
    {
        floorDelta = gSpecialVar_0x8006 - gSpecialVar_0x8005;
        tDescending = FALSE;
    }

    if (floorDelta > MAX_ELEVATOR_TRIP - 1)
        floorDelta = MAX_ELEVATOR_TRIP - 1;

    tTotalMoves = sElevatorTripLength[floorDelta];

    SetCameraPanningCallback(NULL);
    MoveElevatorWindowLights(floorDelta, tDescending);
    PlaySE(SE_ELEVATOR);
}

static void Task_MoveElevator(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    tTimer++;
    if (tTimer % 3 == 0)
    {
        tTimer = 0;
        tMoveCounter++;
        tVerticalPan = -tVerticalPan;
        SetCameraPanning(0, tVerticalPan);

        if (tMoveCounter == tTotalMoves)
        {
            // Arrived at floor
            PlaySE(SE_DING_DONG);
            DestroyTask(taskId);
            ScriptContext_Enable();
            InstallCameraPanAheadCallback();
        }
    }
}

#undef tTimer
#undef tMoveCounter
#undef tVerticalPan
#undef tTotalMoves
#undef tDescending

void ShowDeptStoreElevatorFloorSelect(void)
{
    int xPos;

    sTutorMoveAndElevatorWindowId = AddWindow(&sWindowTemplate_ElevatorFloor);
    SetStandardWindowBorderStyle(sTutorMoveAndElevatorWindowId, FALSE);

    xPos = GetStringCenterAlignXOffset(FONT_NORMAL, gText_ElevatorNowOn, 64);
    AddTextPrinterParameterized(sTutorMoveAndElevatorWindowId, FONT_NORMAL, gText_ElevatorNowOn, xPos, 1, TEXT_SKIP_DRAW, NULL);

    xPos = GetStringCenterAlignXOffset(FONT_NORMAL, sDeptStoreFloorNames[gSpecialVar_0x8005], 64);
    AddTextPrinterParameterized(sTutorMoveAndElevatorWindowId, FONT_NORMAL, sDeptStoreFloorNames[gSpecialVar_0x8005], xPos, 17, TEXT_SKIP_DRAW, NULL);

    PutWindowTilemap(sTutorMoveAndElevatorWindowId);
    CopyWindowToVram(sTutorMoveAndElevatorWindowId, COPYWIN_FULL);
}

void CloseDeptStoreElevatorWindow(void)
{
    ClearStdWindowAndFrameToTransparent(sTutorMoveAndElevatorWindowId, TRUE);
    RemoveWindow(sTutorMoveAndElevatorWindowId);
}

// Task data for Task_MoveElevatorWindowLights
#define tMoveCounter data[0]
#define tTimer       data[1]
#define tDescending  data[2]
#define tTotalMoves  data[3]

static void MoveElevatorWindowLights(u16 floorDelta, bool8 descending)
{
    static const u8 sElevatorLightCycles[MAX_ELEVATOR_TRIP] = { 3, 6, 9, 12, 15, 18, 21, 24, 27 };

    if (FuncIsActiveTask(Task_MoveElevatorWindowLights) != TRUE)
    {
        u8 taskId = CreateTask(Task_MoveElevatorWindowLights, 8);
        gTasks[taskId].tMoveCounter = 0;
        gTasks[taskId].tTimer = 0;
        gTasks[taskId].tDescending = descending;
        gTasks[taskId].tTotalMoves = sElevatorLightCycles[floorDelta];
    }
}

static void Task_MoveElevatorWindowLights(u8 taskId)
{
    u8 x, y;
    s16 *data = gTasks[taskId].data;

    if (tTimer == 6)
    {
        tMoveCounter++;

        if (!tDescending)
        {
            // Ascending
            for (y = 0; y < ELEVATOR_WINDOW_HEIGHT; y++)
            {
                for (x = 0; x < ELEVATOR_WINDOW_WIDTH; x++)
                    MapGridSetMetatileIdAt(x + MAP_OFFSET + 1, y + MAP_OFFSET, sElevatorWindowTiles_Ascending[y][tMoveCounter % ELEVATOR_LIGHT_STAGES] | MAPGRID_COLLISION_MASK);
            }
        }
        else
        {
            // Descending
            for (y = 0; y < ELEVATOR_WINDOW_HEIGHT; y++)
            {
                for (x = 0; x < ELEVATOR_WINDOW_WIDTH; x++)
                    MapGridSetMetatileIdAt(x + MAP_OFFSET + 1, y + MAP_OFFSET, sElevatorWindowTiles_Descending[y][tMoveCounter % ELEVATOR_LIGHT_STAGES] | MAPGRID_COLLISION_MASK);
            }
        }
        DrawWholeMapView();
        tTimer = 0;
        if (tMoveCounter == tTotalMoves)
            DestroyTask(taskId);
    }
    tTimer++;
}

#undef tMoveCounter
#undef tTimer
#undef tDescending
#undef tTotalMoves

void BufferVarsForIVRater(void)
{
    u8 i;
    u32 ivStorage[NUM_STATS];

    ivStorage[STAT_HP] = GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_HP_IV);
    ivStorage[STAT_ATK] = GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_ATK_IV);
    ivStorage[STAT_DEF] = GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_DEF_IV);
    ivStorage[STAT_SPEED] = GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_SPEED_IV);
    ivStorage[STAT_SPATK] = GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_SPATK_IV);
    ivStorage[STAT_SPDEF] = GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_SPDEF_IV);

    gSpecialVar_0x8005 = 0;

    for (i = 0; i < NUM_STATS; i++)
        gSpecialVar_0x8005 += ivStorage[i];

    gSpecialVar_0x8006 = 0;
    gSpecialVar_0x8007 = ivStorage[STAT_HP];

    for (i = 1; i < NUM_STATS; i++)
    {
        if (ivStorage[gSpecialVar_0x8006] < ivStorage[i])
        {
            gSpecialVar_0x8006 = i;
            gSpecialVar_0x8007 = ivStorage[i];
        }
        else if (ivStorage[gSpecialVar_0x8006] == ivStorage[i])
        {
            u16 randomNumber = Random();
            if (randomNumber & 1)
            {
                gSpecialVar_0x8006 = i;
                gSpecialVar_0x8007 = ivStorage[i];
            }
        }
    }
}

bool8 UsedPokemonCenterWarp(void)
{
    static const u16 sPokemonCenters[] =
    {
        MAP_OLDALE_TOWN_POKEMON_CENTER_1F,
        MAP_DEWFORD_TOWN_POKEMON_CENTER_1F,
        MAP_LAVARIDGE_TOWN_POKEMON_CENTER_1F,
        MAP_FALLARBOR_TOWN_POKEMON_CENTER_1F,
        MAP_VERDANTURF_TOWN_POKEMON_CENTER_1F,
        MAP_PACIFIDLOG_TOWN_POKEMON_CENTER_1F,
        MAP_PETALBURG_CITY_POKEMON_CENTER_1F,
        MAP_SLATEPORT_CITY_POKEMON_CENTER_1F,
        MAP_MAUVILLE_CITY_POKEMON_CENTER_1F,
        MAP_RUSTBORO_CITY_POKEMON_CENTER_1F,
        MAP_FORTREE_CITY_POKEMON_CENTER_1F,
        MAP_LILYCOVE_CITY_POKEMON_CENTER_1F,
        MAP_MOSSDEEP_CITY_POKEMON_CENTER_1F,
        MAP_SOOTOPOLIS_CITY_POKEMON_CENTER_1F,
        MAP_EVER_GRANDE_CITY_POKEMON_CENTER_1F,
        MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F,
        MAP_BATTLE_FRONTIER_POKEMON_CENTER_1F,
        MAP_UNION_ROOM,
        MAP_UNDEFINED
    };

    int i;
    u16 map = (gLastUsedWarp.mapGroup << 8) + gLastUsedWarp.mapNum;

    for (i = 0; sPokemonCenters[i] != MAP_UNDEFINED; i++)
    {
        if (sPokemonCenters[i] == map)
            return TRUE;
    }

    return FALSE;
}

bool32 PlayerNotAtTrainerHillEntrance(void)
{
    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(TRAINER_HILL_ENTRANCE) && gSaveBlock1Ptr->location.mapNum == MAP_NUM(TRAINER_HILL_ENTRANCE))
        return FALSE;

    return TRUE;
}

void UpdateFrontierManiac(u16 daysSince)
{
    u16 *var = GetVarPointer(VAR_FRONTIER_MANIAC_FACILITY);
    *var += daysSince;
    *var %= FRONTIER_MANIAC_FACILITY_COUNT;
}

void ShowFrontierManiacMessage(void)
{
    static const u8 *const sFrontierManiacMessages[][FRONTIER_MANIAC_MESSAGE_COUNT] =
    {
        [FRONTIER_MANIAC_TOWER_SINGLES] =
        {
            BattleFrontier_Lounge2_Text_SalonMaidenIsThere,
            BattleFrontier_Lounge2_Text_SalonMaidenSilverMons,
            BattleFrontier_Lounge2_Text_SalonMaidenGoldMons
        },
        [FRONTIER_MANIAC_TOWER_DOUBLES] =
        {
            BattleFrontier_Lounge2_Text_DoubleBattleAdvice1,
            BattleFrontier_Lounge2_Text_DoubleBattleAdvice2,
            BattleFrontier_Lounge2_Text_DoubleBattleAdvice3
        },
        [FRONTIER_MANIAC_TOWER_MULTIS] =
        {
            BattleFrontier_Lounge2_Text_MultiBattleAdvice,
            BattleFrontier_Lounge2_Text_MultiBattleAdvice,
            BattleFrontier_Lounge2_Text_MultiBattleAdvice
        },
        [FRONTIER_MANIAC_TOWER_LINK] =
        {
            BattleFrontier_Lounge2_Text_LinkMultiBattleAdvice,
            BattleFrontier_Lounge2_Text_LinkMultiBattleAdvice,
            BattleFrontier_Lounge2_Text_LinkMultiBattleAdvice
        },
        [FRONTIER_MANIAC_DOME] =
        {
            BattleFrontier_Lounge2_Text_DomeAceIsThere,
            BattleFrontier_Lounge2_Text_DomeAceSilverMons,
            BattleFrontier_Lounge2_Text_DomeAceGoldMons
        },
        [FRONTIER_MANIAC_FACTORY] =
        {
            BattleFrontier_Lounge2_Text_FactoryHeadIsThere,
            BattleFrontier_Lounge2_Text_FactoryHeadSilverMons,
            BattleFrontier_Lounge2_Text_FactoryHeadGoldMons
        },
        [FRONTIER_MANIAC_PALACE] =
        {
            BattleFrontier_Lounge2_Text_PalaceMavenIsThere,
            BattleFrontier_Lounge2_Text_PalaceMavenSilverMons,
            BattleFrontier_Lounge2_Text_PalaceMavenGoldMons
        },
        [FRONTIER_MANIAC_ARENA] =
        {
            BattleFrontier_Lounge2_Text_ArenaTycoonIsThere,
            BattleFrontier_Lounge2_Text_ArenaTycoonSilverMons,
            BattleFrontier_Lounge2_Text_ArenaTycoonGoldMons
        },
        [FRONTIER_MANIAC_PIKE] =
        {
            BattleFrontier_Lounge2_Text_PikeQueenIsThere,
            BattleFrontier_Lounge2_Text_PikeQueenSilverMons,
            BattleFrontier_Lounge2_Text_PikeQueenGoldMons
        },
        [FRONTIER_MANIAC_PYRAMID] =
        {
            BattleFrontier_Lounge2_Text_PyramidKingIsThere,
            BattleFrontier_Lounge2_Text_PyramidKingSilverMons,
            BattleFrontier_Lounge2_Text_PyramidKingGoldMons
        },
    };

    static const u8 sFrontierManiacStreakThresholds[][FRONTIER_MANIAC_MESSAGE_COUNT - 1] =
    {
        [FRONTIER_MANIAC_TOWER_SINGLES] = { 21, 56 },
        [FRONTIER_MANIAC_TOWER_DOUBLES] = { 21, 35 },
        [FRONTIER_MANIAC_TOWER_MULTIS]  = { 255, 255 },
        [FRONTIER_MANIAC_TOWER_LINK]    = { 255, 255 },
        [FRONTIER_MANIAC_DOME]          = { 2, 4 },
        [FRONTIER_MANIAC_FACTORY]       = { 7, 21 },
        [FRONTIER_MANIAC_PALACE]        = { 7, 21 },
        [FRONTIER_MANIAC_ARENA]         = { 14, 28 },
        [FRONTIER_MANIAC_PIKE]          = { 13, 112 }, //BUG: 112 (0x70) is probably a mistake; the Pike Queen is battled twice well before that
        [FRONTIER_MANIAC_PYRAMID]       = { 7, 56 }
    };

    u8 i;
    u16 winStreak = 0;
    u16 facility = VarGet(VAR_FRONTIER_MANIAC_FACILITY);

    switch (facility)
    {
    case FRONTIER_MANIAC_TOWER_SINGLES:
    case FRONTIER_MANIAC_TOWER_DOUBLES:
    case FRONTIER_MANIAC_TOWER_MULTIS:
    case FRONTIER_MANIAC_TOWER_LINK:
        if (gSaveBlock2Ptr->frontier.towerWinStreaks[facility][FRONTIER_LVL_50]
            >= gSaveBlock2Ptr->frontier.towerWinStreaks[facility][FRONTIER_LVL_OPEN])
            winStreak = gSaveBlock2Ptr->frontier.towerWinStreaks[facility][FRONTIER_LVL_50];
        else
            winStreak = gSaveBlock2Ptr->frontier.towerWinStreaks[facility][FRONTIER_LVL_OPEN];
        break;
    case FRONTIER_MANIAC_DOME:
        if (gSaveBlock2Ptr->frontier.domeWinStreaks[FRONTIER_MODE_SINGLES][FRONTIER_LVL_50]
            >= gSaveBlock2Ptr->frontier.domeWinStreaks[FRONTIER_MODE_SINGLES][FRONTIER_LVL_OPEN])
            winStreak = gSaveBlock2Ptr->frontier.domeWinStreaks[FRONTIER_MODE_SINGLES][FRONTIER_LVL_50];
        else
            winStreak = gSaveBlock2Ptr->frontier.domeWinStreaks[FRONTIER_MODE_SINGLES][FRONTIER_LVL_OPEN];
        break;
    case FRONTIER_MANIAC_FACTORY:
        if (gSaveBlock2Ptr->frontier.factoryWinStreaks[FRONTIER_MODE_SINGLES][FRONTIER_LVL_50]
            >= gSaveBlock2Ptr->frontier.factoryWinStreaks[FRONTIER_MODE_SINGLES][FRONTIER_LVL_OPEN])
            winStreak = gSaveBlock2Ptr->frontier.factoryWinStreaks[FRONTIER_MODE_SINGLES][FRONTIER_LVL_50];
        else
            winStreak = gSaveBlock2Ptr->frontier.factoryWinStreaks[FRONTIER_MODE_SINGLES][FRONTIER_LVL_OPEN];
        break;
    case FRONTIER_MANIAC_PALACE:
        if (gSaveBlock2Ptr->frontier.palaceWinStreaks[FRONTIER_MODE_SINGLES][FRONTIER_LVL_50]
            >= gSaveBlock2Ptr->frontier.palaceWinStreaks[FRONTIER_MODE_SINGLES][FRONTIER_LVL_OPEN])
            winStreak = gSaveBlock2Ptr->frontier.palaceWinStreaks[FRONTIER_MODE_SINGLES][FRONTIER_LVL_50];
        else
            winStreak = gSaveBlock2Ptr->frontier.palaceWinStreaks[FRONTIER_MODE_SINGLES][FRONTIER_LVL_OPEN];
        break;
    case FRONTIER_MANIAC_ARENA:
        if (gSaveBlock2Ptr->frontier.arenaWinStreaks[FRONTIER_LVL_50]
            >= gSaveBlock2Ptr->frontier.arenaWinStreaks[FRONTIER_LVL_OPEN])
            winStreak = gSaveBlock2Ptr->frontier.arenaWinStreaks[FRONTIER_LVL_50];
        else
            winStreak = gSaveBlock2Ptr->frontier.arenaWinStreaks[FRONTIER_LVL_OPEN];
        break;
    case FRONTIER_MANIAC_PIKE:
        if (gSaveBlock2Ptr->frontier.pikeWinStreaks[FRONTIER_LVL_50]
            >= gSaveBlock2Ptr->frontier.pikeWinStreaks[FRONTIER_LVL_OPEN])
            winStreak = gSaveBlock2Ptr->frontier.pikeWinStreaks[FRONTIER_LVL_50];
        else
            winStreak = gSaveBlock2Ptr->frontier.pikeWinStreaks[FRONTIER_LVL_OPEN];
        break;
    case FRONTIER_MANIAC_PYRAMID:
        if (gSaveBlock2Ptr->frontier.pyramidWinStreaks[FRONTIER_LVL_50]
            >= gSaveBlock2Ptr->frontier.pyramidWinStreaks[FRONTIER_LVL_OPEN])
            winStreak = gSaveBlock2Ptr->frontier.pyramidWinStreaks[FRONTIER_LVL_50];
        else
            winStreak = gSaveBlock2Ptr->frontier.pyramidWinStreaks[FRONTIER_LVL_OPEN];
        break;
    }

    for (i = 0; i < FRONTIER_MANIAC_MESSAGE_COUNT - 1 && sFrontierManiacStreakThresholds[facility][i] < winStreak; i++);

    ShowFieldMessage(sFrontierManiacMessages[facility][i]);
}

// gSpecialVar_0x8005 and 0x8006 here are used by MoveElevator
void BufferBattleTowerElevatorFloors(void)
{
    static const u16 sBattleTowerStreakThresholds[] = {
        7, 14, 21, 28, 35, 49, 63, 77, 91, 0
    };

    u8 i;
    u16 battleMode = VarGet(VAR_FRONTIER_BATTLE_MODE);
    u8 lvlMode = gSaveBlock2Ptr->frontier.lvlMode;

    if (battleMode == FRONTIER_MODE_MULTIS && !FlagGet(FLAG_CHOSEN_MULTI_BATTLE_NPC_PARTNER))
    {
        gSpecialVar_0x8005 = 5;
        gSpecialVar_0x8006 = 4;
        return;
    }

    for (i = 0; i < ARRAY_COUNT(sBattleTowerStreakThresholds) - 1; i++)
    {
        if (sBattleTowerStreakThresholds[i] > gSaveBlock2Ptr->frontier.towerWinStreaks[battleMode][lvlMode])
        {
            gSpecialVar_0x8005 = 4;
            gSpecialVar_0x8006 = i + 5;
            return;
        }
    }

    gSpecialVar_0x8005 = 4;
    gSpecialVar_0x8006 = 12;
}

// Scrollable Multichoice task data defines
#define tMaxItemsOnScreen    data[0]
#define tNumItems            data[1]
#define tLeft                data[2]
#define tTop                 data[3]
#define tWidth               data[4]
#define tHeight              data[5]
#define tKeepOpenAfterSelect data[6]
#define tScrollOffset        data[7]
#define tSelectedRow         data[8]
#define tScrollMultiId       data[11]
#define tScrollArrowId       data[12]
#define tWindowId            data[13]
#define tListTaskId          data[14]
#define tTaskId              data[15]
// data[9] and [10] unused

void ShowScrollableMultichoice(void)
{
    u8 taskId = CreateTask(Task_ShowScrollableMultichoice, 8);
    struct Task *task = &gTasks[taskId];
    task->tScrollMultiId = gSpecialVar_0x8004;

    switch (gSpecialVar_0x8004)
    {
    case SCROLL_MULTI_NONE:
        task->tMaxItemsOnScreen = 1;
        task->tNumItems = 1;
        task->tLeft = 1;
        task->tTop = 1;
        task->tWidth = 1;
        task->tHeight = 1;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_GLASS_WORKSHOP_VENDOR:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN - 1;
        task->tNumItems = 8;
        task->tLeft = 1;
        task->tTop = 1;
        task->tWidth = 9;
        task->tHeight = 10;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_POKEMON_FAN_CLUB_RATER:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 12;
        task->tLeft = 1;
        task->tTop = 1;
        task->tWidth = 7;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_BF_EXCHANGE_CORNER_DECOR_VENDOR_1:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 11;
        task->tLeft = 14;
        task->tTop = 1;
        task->tWidth = 15;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_BF_EXCHANGE_CORNER_DECOR_VENDOR_2:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 6;
        task->tLeft = 14;
        task->tTop = 1;
        task->tWidth = 15;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_BF_EXCHANGE_CORNER_VITAMIN_VENDOR:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 11; // + Golden Cap
        task->tLeft = 14;
        task->tTop = 1;
        task->tWidth = 15;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_BF_EXCHANGE_CORNER_VITAMIN_VENDOR_SILVER_SYMBOLS:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 12; // + Golden Cap
        task->tLeft = 14;
        task->tTop = 1;
        task->tWidth = 15;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_BF_EXCHANGE_CORNER_HOLD_ITEM_VENDOR:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 19;
        task->tLeft = 14;
        task->tTop = 1;
        task->tWidth = 15;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_BF_EXCHANGE_CORNER_BERRY_VENDOR:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 12;
        task->tLeft = 14;
        task->tTop = 1;
        task->tWidth = 15;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_BERRY_POWDER_VENDOR:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 12;
        task->tLeft = 15;
        task->tTop = 1;
        task->tWidth = 14;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_BF_RECEPTIONIST:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 10;
        task->tLeft = 17;
        task->tTop = 1;
        task->tWidth = 11;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_BF_MOVE_TUTOR_1:
    case SCROLL_MULTI_BF_MOVE_TUTOR_1_FREE:
        // Tutor 1 gained HEADBUTT (11 moves + Exit).
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 12;
        task->tLeft = 15;
        task->tTop = 1;
        task->tWidth = 14;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_BF_MOVE_TUTOR_2:
    case SCROLL_MULTI_BF_MOVE_TUTOR_2_FREE:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 11;
        task->tLeft = 15;
        task->tTop = 1;
        task->tWidth = 14;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_SS_TIDAL_DESTINATION:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 7;
        task->tLeft = 19;
        task->tTop = 1;
        task->tWidth = 10;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    case SCROLL_MULTI_BATTLE_TENT_RULES:
        task->tMaxItemsOnScreen = MAX_SCROLL_MULTI_ON_SCREEN;
        task->tNumItems = 7;
        task->tLeft = 17;
        task->tTop = 1;
        task->tWidth = 12;
        task->tHeight = 12;
        task->tKeepOpenAfterSelect = FALSE;
        task->tTaskId = taskId;
        break;
    default:
        gSpecialVar_Result = MULTI_B_PRESSED;
        DestroyTask(taskId);
        break;
    }
}

static const u8 *const sScrollableMultichoiceOptions[][MAX_SCROLL_MULTI_LENGTH] =
{
    [SCROLL_MULTI_NONE] =
    {
        gText_Exit
    },
    [SCROLL_MULTI_GLASS_WORKSHOP_VENDOR] =
    {
        gText_BlueFlute,
        gText_YellowFlute,
        gText_RedFlute,
        gText_WhiteFlute,
        gText_BlackFlute,
        gText_PrettyChair,
        gText_PrettyDesk,
        gText_Exit
    },
    [SCROLL_MULTI_POKEMON_FAN_CLUB_RATER] =
    {
        gText_0Pts,
        gText_10Pts,
        gText_20Pts,
        gText_30Pts,
        gText_40Pts,
        gText_50Pts,
        gText_60Pts,
        gText_70Pts,
        gText_80Pts,
        gText_90Pts,
        gText_100Pts,
        gText_QuestionMark
    },
    [SCROLL_MULTI_BF_EXCHANGE_CORNER_DECOR_VENDOR_1] =
    {
        gText_KissPosterCost,
        gText_KissCushionCost,
        gText_SmoochumDollCost,
        gText_TogepiDollCost,
        gText_MeowthDollCost,
        gText_ClefairyDollCost,
        gText_DittoDollCost,
        gText_CyndaquilDollCost,
        gText_ChikoritaDollCost,
        gText_TotodileDollCost,
        gText_Exit
    },
    [SCROLL_MULTI_BF_EXCHANGE_CORNER_DECOR_VENDOR_2] =
    {
        gText_LaprasDollCost,
        gText_SnorlaxDollCost,
        gText_VenusaurDollCost,
        gText_CharizardDollCost,
        gText_BlastoiseDollCost,
        gText_Exit
    },
    [SCROLL_MULTI_BF_EXCHANGE_CORNER_VITAMIN_VENDOR] =
    {
        gText_ProteinCost,
        gText_CalciumCost,
        gText_IronCost,
        gText_ZincCost,
        gText_CarbosCost,
        gText_HpUpCost,
        gText_PpUpCost,
        gText_RareCandyCost,
        gText_AbilityCapsuleCost,
        gText_GoldenCapCost,
        gText_Exit
    },
    [SCROLL_MULTI_BF_EXCHANGE_CORNER_VITAMIN_VENDOR_SILVER_SYMBOLS] =
    {
        gText_ProteinCost,
        gText_CalciumCost,
        gText_IronCost,
        gText_ZincCost,
        gText_CarbosCost,
        gText_HpUpCost,
        gText_PpUpCost,
        gText_PpMaxCost,
        gText_RareCandyCost,
        gText_AbilityCapsuleCost,
        gText_GoldenCapCost,
        gText_Exit
    },
    [SCROLL_MULTI_BF_EXCHANGE_CORNER_HOLD_ITEM_VENDOR] =
    {
        gText_LeftoversCost,
        gText_WhiteHerbCost,
        gText_QuickClawCost,
        gText_MentalHerbCost,
        gText_BrightPowderCost,
        gText_ChoiceBandCost,
        gText_KingsRockCost,
        gText_FocusBandCost,
        gText_ScopeLensCost,
        gText_MetalCoatCost,
        gText_AmuletCoinCost,
        gText_ExpShareCost,
        gText_LuckyEggCost,
        gText_MachoBraceCost,
        gText_CleanseTagCost,
        gText_SmokeBallCost,
        gText_SootheBellCost,
        gText_EverstoneCost,
        gText_Exit
    },
    [SCROLL_MULTI_BF_EXCHANGE_CORNER_BERRY_VENDOR] =
    {
        gText_LiechiCost,
        gText_GanlonCost,
        gText_SalacCost,
        gText_PetayaCost,
        gText_ApicotCost,
        gText_PomegCost,
        gText_KelpsyCost,
        gText_QualotCost,
        gText_HondewCost,
        gText_GrepaCost,
        gText_TamatoCost,
        gText_Exit
    },
    [SCROLL_MULTI_BERRY_POWDER_VENDOR] =
    {
        gText_EnergyPowder50,
        gText_EnergyRoot80,
        gText_HealPowder50,
        gText_RevivalHerb300,
        gText_Protein1000,
        gText_Iron1000,
        gText_Carbos1000,
        gText_Calcium1000,
        gText_Zinc1000,
        gText_HPUp1000,
        gText_PPUp3000,
        gText_Exit
    },
    [SCROLL_MULTI_BF_RECEPTIONIST] =
    {
        gText_BattleTower2,
        gText_BattleDome,
        gText_BattlePalace,
        gText_BattleArena,
        gText_BattleFactory,
        gText_BattlePike,
        gText_BattlePyramid,
        gText_RankingHall,
        gText_ExchangeService,
        gText_Exit
    },
    [SCROLL_MULTI_BF_MOVE_TUTOR_1] =
    {
        gText_SoftboiledCost,
        gText_SeismicTossCost,
        gText_DreamEaterCost,
        gText_MegaPunchCost,
        gText_MegaKickCost,
        gText_BodySlamCost,
        gText_RockSlideCost,
        gText_CounterCost,
        gText_ThunderWaveCost,
        gText_SwordsDanceCost,
        gText_HeadbuttCost,
        gText_Exit
    },
    [SCROLL_MULTI_BF_MOVE_TUTOR_2] =
    {
        gText_DefenseCurlCost,
        gText_SnoreCost,
        gText_MudSlapCost,
        gText_SwiftCost,
        gText_IcyWindCost,
        gText_EndureCost,
        gText_PsychUpCost,
        gText_IcePunchCost,
        gText_ThunderPunchCost,
        gText_FirePunchCost,
        gText_Exit
    },
    [SCROLL_MULTI_SS_TIDAL_DESTINATION] =
    {
        gText_SlateportCity,
        gText_BattleFrontier,
        gText_SouthernIsland,
        gText_NavelRock,
        gText_BirthIsland,
        gText_FarawayIsland,
        gText_Exit
    },
    [SCROLL_MULTI_BATTLE_TENT_RULES] =
    {
        gText_BattleTrainers,
        gText_BattleBasics,
        gText_PokemonNature,
        gText_PokemonMoves,
        gText_Underpowered,
        gText_WhenInDanger,
        gText_Exit
    },
    [SCROLL_MULTI_BF_MOVE_TUTOR_1_FREE] =
    {
        gText_SoftboiledFree,
        gText_SeismicTossFree,
        gText_DreamEaterFree,
        gText_MegaPunchFree,
        gText_MegaKickFree,
        gText_BodySlamFree,
        gText_RockSlideFree,
        gText_CounterFree,
        gText_ThunderWaveFree,
        gText_SwordsDanceFree,
        gText_HeadbuttFree,
        gText_Exit
    },
    [SCROLL_MULTI_BF_MOVE_TUTOR_2_FREE] =
    {
        gText_DefenseCurlFree,
        gText_SnoreFree,
        gText_MudSlapFree,
        gText_SwiftFree,
        gText_IcyWindFree,
        gText_EndureFree,
        gText_PsychUpFree,
        gText_IcePunchFree,
        gText_ThunderPunchFree,
        gText_FirePunchFree,
        gText_Exit
    }
};

static void Task_ShowScrollableMultichoice(u8 taskId)
{
    u32 width;
    u8 i, windowId;
    struct WindowTemplate template;
    struct Task *task = &gTasks[taskId];

    LockPlayerFieldControls();
    sScrollableMultichoice_ScrollOffset = 0;
    sScrollableMultichoice_ItemSpriteId = MAX_SPRITES;
    FillFrontierExchangeCornerWindowAndItemIcon(task->tScrollMultiId, 0);
    ShowBattleFrontierTutorWindow(task->tScrollMultiId, 0);
    sScrollableMultichoice_ListMenuItem = AllocZeroed(task->tNumItems * 8);
    sFrontierExchangeCorner_NeverRead = 0;
    InitScrollableMultichoice();

    for (width = 0, i = 0; i < task->tNumItems; i++)
    {
        const u8 *text = sScrollableMultichoiceOptions[gSpecialVar_0x8004][i];
        sScrollableMultichoice_ListMenuItem[i].name = text;
        sScrollableMultichoice_ListMenuItem[i].id = i;
        width = DisplayTextAndGetWidth(text, width);
    }

    task->tWidth = ConvertPixelWidthToTileWidth(width);

    if (task->tLeft + task->tWidth > MAX_MULTICHOICE_WIDTH + 1)
    {
        int adjustedLeft = MAX_MULTICHOICE_WIDTH + 1 - task->tWidth;
        if (adjustedLeft < 0)
            task->tLeft = 0;
        else
            task->tLeft = adjustedLeft;
    }

    template = CreateWindowTemplate(0, task->tLeft, task->tTop, task->tWidth, task->tHeight, 0xF, 0x64);
    windowId = AddWindow(&template);
    task->tWindowId = windowId;
    SetStandardWindowBorderStyle(windowId, FALSE);

    gScrollableMultichoice_ListMenuTemplate.totalItems = task->tNumItems;
    gScrollableMultichoice_ListMenuTemplate.maxShowed = task->tMaxItemsOnScreen;
    gScrollableMultichoice_ListMenuTemplate.windowId = task->tWindowId;

    ScrollableMultichoice_UpdateScrollArrows(taskId);
    task->tListTaskId = ListMenuInit(&gScrollableMultichoice_ListMenuTemplate, task->tScrollOffset, task->tSelectedRow);
    ScheduleBgCopyTilemapToVram(0);
    gTasks[taskId].func = ScrollableMultichoice_ProcessInput;
}

static void InitScrollableMultichoice(void)
{
    gScrollableMultichoice_ListMenuTemplate.items = sScrollableMultichoice_ListMenuItem;
    gScrollableMultichoice_ListMenuTemplate.moveCursorFunc = ScrollableMultichoice_MoveCursor;
    gScrollableMultichoice_ListMenuTemplate.itemPrintFunc = NULL;
    gScrollableMultichoice_ListMenuTemplate.totalItems = 1;
    gScrollableMultichoice_ListMenuTemplate.maxShowed = 1;
    gScrollableMultichoice_ListMenuTemplate.windowId = 0;
    gScrollableMultichoice_ListMenuTemplate.header_X = 0;
    gScrollableMultichoice_ListMenuTemplate.item_X = 8;
    gScrollableMultichoice_ListMenuTemplate.cursor_X = 0;
    gScrollableMultichoice_ListMenuTemplate.upText_Y = 1;
    gScrollableMultichoice_ListMenuTemplate.cursorPal = 2;
    gScrollableMultichoice_ListMenuTemplate.fillValue = 1;
    gScrollableMultichoice_ListMenuTemplate.cursorShadowPal = 3;
    gScrollableMultichoice_ListMenuTemplate.lettersSpacing = 0;
    gScrollableMultichoice_ListMenuTemplate.itemVerticalPadding = 0;
    gScrollableMultichoice_ListMenuTemplate.scrollMultiple = LIST_NO_MULTIPLE_SCROLL;
    gScrollableMultichoice_ListMenuTemplate.fontId = FONT_NORMAL;
    gScrollableMultichoice_ListMenuTemplate.cursorKind = CURSOR_BLACK_ARROW;
}

static void ScrollableMultichoice_MoveCursor(s32 itemIndex, bool8 onInit, struct ListMenu *list)
{
    u8 taskId;
    PlaySE(SE_SELECT);
    taskId = FindTaskIdByFunc(ScrollableMultichoice_ProcessInput);
    if (taskId != TASK_NONE)
    {
        u16 selection;
        struct Task *task = &gTasks[taskId];
        ListMenuGetScrollAndRow(task->tListTaskId, &selection, NULL);
        sScrollableMultichoice_ScrollOffset = selection;
        ListMenuGetCurrentItemArrayId(task->tListTaskId, &selection);
        HideFrontierExchangeCornerItemIcon(task->tScrollMultiId, sFrontierExchangeCorner_NeverRead);
        FillFrontierExchangeCornerWindowAndItemIcon(task->tScrollMultiId, selection);
        ShowBattleFrontierTutorMoveDescription(task->tScrollMultiId, selection);
        sFrontierExchangeCorner_NeverRead = selection;
    }
}

static void ScrollableMultichoice_ProcessInput(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    s32 input = ListMenu_ProcessInput(task->tListTaskId);

    switch (input)
    {
    case LIST_NOTHING_CHOSEN:
        break;
    case LIST_CANCEL:
        gSpecialVar_Result = MULTI_B_PRESSED;
        PlaySE(SE_SELECT);
        CloseScrollableMultichoice(taskId);
        break;
    default:
        gSpecialVar_Result = input;
        PlaySE(SE_SELECT);
        if (!task->tKeepOpenAfterSelect)
        {
            CloseScrollableMultichoice(taskId);
        }
        else if (input == task->tNumItems - 1)
        {
            // Selected option was the last one (Exit)
            CloseScrollableMultichoice(taskId);
        }
        else
        {
            // Handle selection while keeping the menu open
            ScrollableMultichoice_RemoveScrollArrows(taskId);
            task->func = Task_ScrollableMultichoice_WaitReturnToList;
            ScriptContext_Enable();
        }
        break;
    }
}

static void CloseScrollableMultichoice(u8 taskId)
{
    u16 selection;
    struct Task *task = &gTasks[taskId];
    ListMenuGetCurrentItemArrayId(task->tListTaskId, &selection);
    HideFrontierExchangeCornerItemIcon(task->tScrollMultiId, selection);
    ScrollableMultichoice_RemoveScrollArrows(taskId);
    DestroyListMenuTask(task->tListTaskId, NULL, NULL);
    Free(sScrollableMultichoice_ListMenuItem);
    ClearStdWindowAndFrameToTransparent(task->tWindowId, TRUE);
    FillWindowPixelBuffer(task->tWindowId, PIXEL_FILL(0));
    CopyWindowToVram(task->tWindowId, COPYWIN_GFX);
    RemoveWindow(task->tWindowId);
    DestroyTask(taskId);
    ScriptContext_Enable();
}

// Never run, tKeepOpenAfterSelect is FALSE for all scrollable multichoices.
static void Task_ScrollableMultichoice_WaitReturnToList(u8 taskId)
{
    switch (gTasks[taskId].tKeepOpenAfterSelect)
    {
    case 1:
    default:
        break;
    case 2:
        gTasks[taskId].tKeepOpenAfterSelect = 1;
        gTasks[taskId].func = Task_ScrollableMultichoice_ReturnToList;
        break;
    }
}

// Never called
void ScrollableMultichoice_TryReturnToList(void)
{
    u8 taskId = FindTaskIdByFunc(Task_ScrollableMultichoice_WaitReturnToList);
    if (taskId == TASK_NONE)
        ScriptContext_Enable();
    else
        gTasks[taskId].tKeepOpenAfterSelect++; // Return to list
}

static void Task_ScrollableMultichoice_ReturnToList(u8 taskId)
{
    LockPlayerFieldControls();
    ScrollableMultichoice_UpdateScrollArrows(taskId);
    gTasks[taskId].func = ScrollableMultichoice_ProcessInput;
}

static void ScrollableMultichoice_UpdateScrollArrows(u8 taskId)
{
    static const struct ScrollArrowsTemplate sScrollableMultichoice_ScrollArrowsTemplate = {
        .firstArrowType = SCROLL_ARROW_UP,
        .firstX = 0,
        .firstY = 0,
        .secondArrowType = SCROLL_ARROW_DOWN,
        .secondX = 0,
        .secondY = 0,
        .fullyUpThreshold = 0,
        .fullyDownThreshold = 0,
        .tileTag = GFXTAG_MULTICHOICE_SCROLL_ARROWS,
        .palTag = PALTAG_MULTICHOICE_SCROLL_ARROWS,
        .palNum = 0
    };

    struct Task *task = &gTasks[taskId];
    struct ScrollArrowsTemplate template = sScrollableMultichoice_ScrollArrowsTemplate;
    if (task->tMaxItemsOnScreen != task->tNumItems)
    {
        template.firstX = (task->tWidth / 2) * 8 + 12 + (task->tLeft - 1) * 8;
        template.firstY = 8;
        template.secondX = (task->tWidth / 2) * 8 + 12 + (task->tLeft - 1) * 8;
        template.secondY = task->tHeight * 8 + 10;
        template.fullyUpThreshold = 0;
        template.fullyDownThreshold = task->tNumItems - task->tMaxItemsOnScreen;
        task->tScrollArrowId = AddScrollIndicatorArrowPair(&template, &sScrollableMultichoice_ScrollOffset);
    }
}

static void ScrollableMultichoice_RemoveScrollArrows(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    if (task->tMaxItemsOnScreen != task->tNumItems)
        RemoveScrollIndicatorArrowPair(task->tScrollArrowId);
}

// Removed for Emerald (replaced by ShowScrollableMultichoice)
void ShowGlassWorkshopMenu(void)
{

}

void SetBattleTowerLinkPlayerGfx(void)
{
    u8 i;
    for (i = 0; i < 2; i++)
    {
        if (gLinkPlayers[i].gender == MALE)
            VarSet(VAR_OBJ_GFX_ID_F - i, OBJ_EVENT_GFX_BRENDAN_NORMAL);
        else
            VarSet(VAR_OBJ_GFX_ID_F - i, OBJ_EVENT_GFX_RIVAL_MAY_NORMAL);
    }
}

void ShowNatureGirlMessage(void)
{
    static const u8 *const sNatureGirlMessages[NUM_NATURES] = {
        [NATURE_HARDY]   = BattleFrontier_Lounge5_Text_NatureGirlHardy,
        [NATURE_LONELY]  = BattleFrontier_Lounge5_Text_NatureGirlLonely,
        [NATURE_BRAVE]   = BattleFrontier_Lounge5_Text_NatureGirlBrave,
        [NATURE_ADAMANT] = BattleFrontier_Lounge5_Text_NatureGirlAdamant,
        [NATURE_NAUGHTY] = BattleFrontier_Lounge5_Text_NatureGirlNaughty,
        [NATURE_BOLD]    = BattleFrontier_Lounge5_Text_NatureGirlBold,
        [NATURE_DOCILE]  = BattleFrontier_Lounge5_Text_NatureGirlDocileNaiveQuietQuirky,
        [NATURE_RELAXED] = BattleFrontier_Lounge5_Text_NatureGirlRelaxed,
        [NATURE_IMPISH]  = BattleFrontier_Lounge5_Text_NatureGirlImpish,
        [NATURE_LAX]     = BattleFrontier_Lounge5_Text_NatureGirlLax,
        [NATURE_TIMID]   = BattleFrontier_Lounge5_Text_NatureGirlTimid,
        [NATURE_HASTY]   = BattleFrontier_Lounge5_Text_NatureGirlHasty,
        [NATURE_SERIOUS] = BattleFrontier_Lounge5_Text_NatureGirlSerious,
        [NATURE_JOLLY]   = BattleFrontier_Lounge5_Text_NatureGirlJolly,
        [NATURE_NAIVE]   = BattleFrontier_Lounge5_Text_NatureGirlDocileNaiveQuietQuirky,
        [NATURE_MODEST]  = BattleFrontier_Lounge5_Text_NatureGirlModest,
        [NATURE_MILD]    = BattleFrontier_Lounge5_Text_NatureGirlMild,
        [NATURE_QUIET]   = BattleFrontier_Lounge5_Text_NatureGirlDocileNaiveQuietQuirky,
        [NATURE_BASHFUL] = BattleFrontier_Lounge5_Text_NatureGirlBashful,
        [NATURE_RASH]    = BattleFrontier_Lounge5_Text_NatureGirlRash,
        [NATURE_CALM]    = BattleFrontier_Lounge5_Text_NatureGirlCalm,
        [NATURE_GENTLE]  = BattleFrontier_Lounge5_Text_NatureGirlGentle,
        [NATURE_SASSY]   = BattleFrontier_Lounge5_Text_NatureGirlSassy,
        [NATURE_CAREFUL] = BattleFrontier_Lounge5_Text_NatureGirlCareful,
        [NATURE_QUIRKY]  = BattleFrontier_Lounge5_Text_NatureGirlDocileNaiveQuietQuirky,
    };

    u8 nature;

    if (gSpecialVar_0x8004 >= PARTY_SIZE)
        gSpecialVar_0x8004 = 0;

    nature = GetNature(&gPlayerParty[gSpecialVar_0x8004], FALSE);
    ShowFieldMessage(sNatureGirlMessages[nature]);
}

void UpdateFrontierGambler(u16 daysSince)
{
    u16 *var = GetVarPointer(VAR_FRONTIER_GAMBLER_CHALLENGE);
    *var += daysSince;
    *var %= FRONTIER_GAMBLER_CHALLENGE_COUNT;
}

void ShowFrontierGamblerLookingMessage(void)
{
    static const u8 *const sFrontierGamblerLookingMessages[] =
    {
        BattleFrontier_Lounge3_Text_ChallengeBattleTowerSingle,
        BattleFrontier_Lounge3_Text_ChallengeBattleTowerDouble,
        BattleFrontier_Lounge3_Text_ChallengeBattleTowerMulti,
        BattleFrontier_Lounge3_Text_ChallengeBattleDomeSingle,
        BattleFrontier_Lounge3_Text_ChallengeBattleDomeDouble,
        BattleFrontier_Lounge3_Text_ChallengeBattleFactorySingle,
        BattleFrontier_Lounge3_Text_ChallengeBattleFactoryDouble,
        BattleFrontier_Lounge3_Text_ChallengeBattlePalaceSingle,
        BattleFrontier_Lounge3_Text_ChallengeBattlePalaceDouble,
        BattleFrontier_Lounge3_Text_ChallengeBattleArena,
        BattleFrontier_Lounge3_Text_ChallengeBattlePike,
        BattleFrontier_Lounge3_Text_ChallengeBattlePyramid,
    };

    u16 challenge = VarGet(VAR_FRONTIER_GAMBLER_CHALLENGE);
    ShowFieldMessage(sFrontierGamblerLookingMessages[challenge]);
    VarSet(VAR_FRONTIER_GAMBLER_SET_CHALLENGE, challenge);
}

void ShowFrontierGamblerGoMessage(void)
{
    static const u8 *const sFrontierGamblerGoMessages[] =
    {
        BattleFrontier_Lounge3_Text_GetToBattleTowerSingle,
        BattleFrontier_Lounge3_Text_GetToBattleTowerDouble,
        BattleFrontier_Lounge3_Text_GetToBattleTowerMulti,
        BattleFrontier_Lounge3_Text_GetToBattleDomeSingle,
        BattleFrontier_Lounge3_Text_GetToBattleDomeDouble,
        BattleFrontier_Lounge3_Text_GetToBattleFactorySingle,
        BattleFrontier_Lounge3_Text_GetToBattleFactoryDouble,
        BattleFrontier_Lounge3_Text_GetToBattlePalaceSingle,
        BattleFrontier_Lounge3_Text_GetToBattlePalaceDouble,
        BattleFrontier_Lounge3_Text_GetToBattleArena,
        BattleFrontier_Lounge3_Text_GetToBattlePike,
        BattleFrontier_Lounge3_Text_GetToBattlePyramid,
    };

    ShowFieldMessage(sFrontierGamblerGoMessages[VarGet(VAR_FRONTIER_GAMBLER_SET_CHALLENGE)]);
}

void FrontierGamblerSetWonOrLost(bool8 won)
{
    static const u16 sFrontierChallenges[] =
    {
        FRONTIER_CHALLENGE(FRONTIER_FACILITY_TOWER,   FRONTIER_MODE_SINGLES),
        FRONTIER_CHALLENGE(FRONTIER_FACILITY_TOWER,   FRONTIER_MODE_DOUBLES),
        FRONTIER_CHALLENGE(FRONTIER_FACILITY_TOWER,   FRONTIER_MODE_MULTIS),
        FRONTIER_CHALLENGE(FRONTIER_FACILITY_DOME,    FRONTIER_MODE_SINGLES),
        FRONTIER_CHALLENGE(FRONTIER_FACILITY_DOME,    FRONTIER_MODE_DOUBLES),
        FRONTIER_CHALLENGE(FRONTIER_FACILITY_FACTORY, FRONTIER_MODE_SINGLES),
        FRONTIER_CHALLENGE(FRONTIER_FACILITY_FACTORY, FRONTIER_MODE_DOUBLES),
        FRONTIER_CHALLENGE(FRONTIER_FACILITY_PALACE,  FRONTIER_MODE_SINGLES),
        FRONTIER_CHALLENGE(FRONTIER_FACILITY_PALACE,  FRONTIER_MODE_DOUBLES),
        FRONTIER_CHALLENGE(FRONTIER_FACILITY_ARENA,   FRONTIER_MODE_SINGLES),
        FRONTIER_CHALLENGE(FRONTIER_FACILITY_PIKE,    FRONTIER_MODE_SINGLES),
        FRONTIER_CHALLENGE(FRONTIER_FACILITY_PYRAMID, FRONTIER_MODE_SINGLES)
    };

    u16 battleMode = VarGet(VAR_FRONTIER_BATTLE_MODE);
    u16 challenge = VarGet(VAR_FRONTIER_GAMBLER_SET_CHALLENGE);
    u16 frontierFacilityId = VarGet(VAR_FRONTIER_FACILITY);

    if (VarGet(VAR_FRONTIER_GAMBLER_STATE) == FRONTIER_GAMBLER_PLACED_BET)
    {
        if (sFrontierChallenges[challenge] ==  FRONTIER_CHALLENGE(frontierFacilityId, battleMode))
        {
            if (won)
                VarSet(VAR_FRONTIER_GAMBLER_STATE, FRONTIER_GAMBLER_WON);
            else
                VarSet(VAR_FRONTIER_GAMBLER_STATE, FRONTIER_GAMBLER_LOST);
        }
    }
}

void UpdateBattlePointsWindow(void)
{
    u8 string[32];
    u32 x;
    StringCopy(ConvertIntToDecimalStringN(string, gSaveBlock2Ptr->frontier.battlePoints, STR_CONV_MODE_RIGHT_ALIGN, 4), gText_BP);
    x = GetStringRightAlignXOffset(FONT_NORMAL, string, 48);
    AddTextPrinterParameterized(sBattlePointsWindowId, FONT_NORMAL, string, x, 1, 0, NULL);
}

void ShowBattlePointsWindow(void)
{
    static const struct WindowTemplate sBattlePoints_WindowTemplate =
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = 6,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 8,
    };

    sBattlePointsWindowId = AddWindow(&sBattlePoints_WindowTemplate);
    SetStandardWindowBorderStyle(sBattlePointsWindowId, FALSE);
    UpdateBattlePointsWindow();
    CopyWindowToVram(sBattlePointsWindowId, COPYWIN_GFX);
}

void CloseBattlePointsWindow(void)
{
    ClearStdWindowAndFrameToTransparent(sBattlePointsWindowId, TRUE);
    RemoveWindow(sBattlePointsWindowId);
}

void TakeFrontierBattlePoints(void)
{
    // Vanilla zeroed BP when cost > current — a silent data-loss path that bit
    // a tester (had 2 BP, ended up at 0 with no purchase). All Exchange Corner
    // scripts check BP before calling this, so reaching the underflow branch
    // means either a script-flow bug or a future regression. Refuse the
    // deduction instead of wiping the wallet.
    if (gSaveBlock2Ptr->frontier.battlePoints < gSpecialVar_0x8004)
        return;
    gSaveBlock2Ptr->frontier.battlePoints -= gSpecialVar_0x8004;
}

void GiveFrontierBattlePoints(void)
{
    if (gSaveBlock2Ptr->frontier.battlePoints + gSpecialVar_0x8004 > MAX_BATTLE_FRONTIER_POINTS)
        gSaveBlock2Ptr->frontier.battlePoints = MAX_BATTLE_FRONTIER_POINTS;
    else
        gSaveBlock2Ptr->frontier.battlePoints = gSaveBlock2Ptr->frontier.battlePoints + gSpecialVar_0x8004;
}

u16 GetFrontierBattlePoints(void)
{
    return gSaveBlock2Ptr->frontier.battlePoints;
}

void ShowFrontierExchangeCornerItemIconWindow(void)
{
    static const struct WindowTemplate sFrontierExchangeCorner_ItemIconWindowTemplate =
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 9,
        .width = 4,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 20,
    };

    sFrontierExchangeCorner_ItemIconWindowId = AddWindow(&sFrontierExchangeCorner_ItemIconWindowTemplate);
    SetStandardWindowBorderStyle(sFrontierExchangeCorner_ItemIconWindowId, FALSE);
    CopyWindowToVram(sFrontierExchangeCorner_ItemIconWindowId, COPYWIN_GFX);
}

void CloseFrontierExchangeCornerItemIconWindow(void)
{
    ClearStdWindowAndFrameToTransparent(sFrontierExchangeCorner_ItemIconWindowId, TRUE);
    RemoveWindow(sFrontierExchangeCorner_ItemIconWindowId);
}

static void FillFrontierExchangeCornerWindowAndItemIcon(u16 menu, u16 selection)
{
    #include "data/battle_frontier/battle_frontier_exchange_corner.h"

    if ((menu >= SCROLL_MULTI_BF_EXCHANGE_CORNER_DECOR_VENDOR_1 && menu <= SCROLL_MULTI_BF_EXCHANGE_CORNER_HOLD_ITEM_VENDOR) || menu == SCROLL_MULTI_BF_EXCHANGE_CORNER_BERRY_VENDOR || menu == SCROLL_MULTI_BF_EXCHANGE_CORNER_VITAMIN_VENDOR_SILVER_SYMBOLS)
    {
        FillWindowPixelRect(0, PIXEL_FILL(1), 0, 0, 216, 32);
        switch (menu)
        {
        case SCROLL_MULTI_BF_EXCHANGE_CORNER_DECOR_VENDOR_1:
            AddTextPrinterParameterized2(0, FONT_NORMAL, sFrontierExchangeCorner_Decor1Descriptions[selection], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
            if (sFrontierExchangeCorner_Decor1[selection] == ITEM_LIST_END)
            {
                ShowFrontierExchangeCornerItemIcon(sFrontierExchangeCorner_Decor1[selection]);
            }
            else
            {
                FreeSpriteTilesByTag(TAG_ITEM_ICON);
                FreeSpritePaletteByTag(TAG_ITEM_ICON);
                sScrollableMultichoice_ItemSpriteId = AddDecorationIconObject(sFrontierExchangeCorner_Decor1[selection], 33, 88, 0, TAG_ITEM_ICON, TAG_ITEM_ICON);
            }
            break;
        case SCROLL_MULTI_BF_EXCHANGE_CORNER_DECOR_VENDOR_2:
            AddTextPrinterParameterized2(0, FONT_NORMAL, sFrontierExchangeCorner_Decor2Descriptions[selection], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
            if (sFrontierExchangeCorner_Decor2[selection] == ITEM_LIST_END)
            {
                ShowFrontierExchangeCornerItemIcon(sFrontierExchangeCorner_Decor2[selection]);
            }
            else
            {
                FreeSpriteTilesByTag(TAG_ITEM_ICON);
                FreeSpritePaletteByTag(TAG_ITEM_ICON);
                sScrollableMultichoice_ItemSpriteId = AddDecorationIconObject(sFrontierExchangeCorner_Decor2[selection], 33, 88, 0, TAG_ITEM_ICON, TAG_ITEM_ICON);
            }
            break;
        case SCROLL_MULTI_BF_EXCHANGE_CORNER_VITAMIN_VENDOR:
            AddTextPrinterParameterized2(0, FONT_NORMAL, sFrontierExchangeCorner_VitaminsDescriptions[selection], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
            ShowFrontierExchangeCornerItemIcon(sFrontierExchangeCorner_Vitamins[selection]);
            break;
        case SCROLL_MULTI_BF_EXCHANGE_CORNER_VITAMIN_VENDOR_SILVER_SYMBOLS:
            AddTextPrinterParameterized2(0, FONT_NORMAL, sFrontierExchangeCorner_VitaminsDescriptions_SilverSymbols[selection], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
            ShowFrontierExchangeCornerItemIcon(sFrontierExchangeCorner_Vitamins_SilverSymbols[selection]);
            break;        
        case SCROLL_MULTI_BF_EXCHANGE_CORNER_HOLD_ITEM_VENDOR:
            AddTextPrinterParameterized2(0, FONT_NORMAL, sFrontierExchangeCorner_HoldItemsDescriptions[selection], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
            ShowFrontierExchangeCornerItemIcon(sFrontierExchangeCorner_HoldItems[selection]);
            break;
        case SCROLL_MULTI_BF_EXCHANGE_CORNER_BERRY_VENDOR:
            AddTextPrinterParameterized2(0, FONT_NORMAL, sFrontierExchangeCorner_BerryDescriptions[selection], 0, NULL, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY);
            ShowFrontierExchangeCornerItemIcon(sFrontierExchangeCorner_Berries[selection]);
            break;
        }
    }
}

static void ShowFrontierExchangeCornerItemIcon(u16 item)
{
    FreeSpriteTilesByTag(TAG_ITEM_ICON);
    FreeSpritePaletteByTag(TAG_ITEM_ICON);
    sScrollableMultichoice_ItemSpriteId = AddItemIconSprite(TAG_ITEM_ICON, TAG_ITEM_ICON, item);

    if (sScrollableMultichoice_ItemSpriteId != MAX_SPRITES)
    {
        gSprites[sScrollableMultichoice_ItemSpriteId].oam.priority = 0;
        gSprites[sScrollableMultichoice_ItemSpriteId].x = 36;
        gSprites[sScrollableMultichoice_ItemSpriteId].y = 92;
    }
}

static void HideFrontierExchangeCornerItemIcon(u16 menu, u16 unused)
{
    if (sScrollableMultichoice_ItemSpriteId != MAX_SPRITES)
    {
        switch (menu)
        {
        case SCROLL_MULTI_BF_EXCHANGE_CORNER_DECOR_VENDOR_1:
        case SCROLL_MULTI_BF_EXCHANGE_CORNER_DECOR_VENDOR_2:
        case SCROLL_MULTI_BF_EXCHANGE_CORNER_VITAMIN_VENDOR:
        case SCROLL_MULTI_BF_EXCHANGE_CORNER_VITAMIN_VENDOR_SILVER_SYMBOLS:
        case SCROLL_MULTI_BF_EXCHANGE_CORNER_HOLD_ITEM_VENDOR:
        case SCROLL_MULTI_BF_EXCHANGE_CORNER_BERRY_VENDOR:
            // This makes sure deleting the icon will not clear palettes in use by object events
            FieldEffectFreeGraphicsResources(&gSprites[sScrollableMultichoice_ItemSpriteId]);
            break;
        }
        sScrollableMultichoice_ItemSpriteId = MAX_SPRITES;
    }
}

static const u16 sBattleFrontier_TutorMoves1[] =
{
    MOVE_SOFT_BOILED,
    MOVE_SEISMIC_TOSS,
    MOVE_DREAM_EATER,
    MOVE_MEGA_PUNCH,
    MOVE_MEGA_KICK,
    MOVE_BODY_SLAM,
    MOVE_ROCK_SLIDE,
    MOVE_COUNTER,
    MOVE_THUNDER_WAVE,
    MOVE_SWORDS_DANCE,
    MOVE_HEADBUTT
};

static const u16 sBattleFrontier_TutorMoves2[] =
{
    MOVE_DEFENSE_CURL,
    MOVE_SNORE,
    MOVE_MUD_SLAP,
    MOVE_SWIFT,
    MOVE_ICY_WIND,
    MOVE_ENDURE,
    MOVE_PSYCH_UP,
    MOVE_ICE_PUNCH,
    MOVE_THUNDER_PUNCH,
    MOVE_FIRE_PUNCH
};

void BufferBattleFrontierTutorMoveName(void)
{
    if (gSpecialVar_0x8005 != 0)
        StringCopy(gStringVar1, gMoveNames[sBattleFrontier_TutorMoves2[gSpecialVar_0x8004]]);
    else
        StringCopy(gStringVar1, gMoveNames[sBattleFrontier_TutorMoves1[gSpecialVar_0x8004]]);
}

static void ShowBattleFrontierTutorWindow(u8 menu, u16 selection)
{
    static const struct WindowTemplate sBattleFrontierTutor_WindowTemplate =
    {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 7,
        .width = 12,
        .height = 6,
        .paletteNum = 15,
        .baseBlock = 28,
    };

    if (menu == SCROLL_MULTI_BF_MOVE_TUTOR_1 || menu == SCROLL_MULTI_BF_MOVE_TUTOR_2 || menu == SCROLL_MULTI_BF_MOVE_TUTOR_1_FREE || menu == SCROLL_MULTI_BF_MOVE_TUTOR_2_FREE)
    {
        if (gSpecialVar_0x8006 == 0)
        {
            sTutorMoveAndElevatorWindowId = AddWindow(&sBattleFrontierTutor_WindowTemplate);
            SetStandardWindowBorderStyle(sTutorMoveAndElevatorWindowId, FALSE);
        }
        ShowBattleFrontierTutorMoveDescription(menu, selection);
    }
}

static void ShowBattleFrontierTutorMoveDescription(u8 menu, u16 selection)
{
    static const u8 *const sBattleFrontier_TutorMoveDescriptions1[] =
    {
        BattleFrontier_Lounge7_Text_SoftboiledDesc,
        BattleFrontier_Lounge7_Text_SeismicTossDesc,
        BattleFrontier_Lounge7_Text_DreamEaterDesc,
        BattleFrontier_Lounge7_Text_MegaPunchDesc,
        BattleFrontier_Lounge7_Text_MegaKickDesc,
        BattleFrontier_Lounge7_Text_BodySlamDesc,
        BattleFrontier_Lounge7_Text_RockSlideDesc,
        BattleFrontier_Lounge7_Text_CounterDesc,
        BattleFrontier_Lounge7_Text_ThunderWaveDesc,
        BattleFrontier_Lounge7_Text_SwordsDanceDesc,
        BattleFrontier_Lounge7_Text_HeadbuttDesc,
        gText_Exit,
    };

    static const u8 *const sBattleFrontier_TutorMoveDescriptions2[] =
    {
        BattleFrontier_Lounge7_Text_DefenseCurlDesc,
        BattleFrontier_Lounge7_Text_SnoreDesc,
        BattleFrontier_Lounge7_Text_MudSlapDesc,
        BattleFrontier_Lounge7_Text_SwiftDesc,
        BattleFrontier_Lounge7_Text_IcyWindDesc,
        BattleFrontier_Lounge7_Text_EndureDesc,
        BattleFrontier_Lounge7_Text_PsychUpDesc,
        BattleFrontier_Lounge7_Text_IcePunchDesc,
        BattleFrontier_Lounge7_Text_ThunderPunchDesc,
        BattleFrontier_Lounge7_Text_FirePunchDesc,
        gText_Exit,
    };

    if (menu == SCROLL_MULTI_BF_MOVE_TUTOR_1 || menu == SCROLL_MULTI_BF_MOVE_TUTOR_2 || menu == SCROLL_MULTI_BF_MOVE_TUTOR_1_FREE || menu == SCROLL_MULTI_BF_MOVE_TUTOR_2_FREE)
    {
        FillWindowPixelRect(sTutorMoveAndElevatorWindowId, PIXEL_FILL(1), 0, 0, 96, 48);
        if (menu == SCROLL_MULTI_BF_MOVE_TUTOR_2 || menu == SCROLL_MULTI_BF_MOVE_TUTOR_2_FREE)
            AddTextPrinterParameterized(sTutorMoveAndElevatorWindowId, FONT_NORMAL, sBattleFrontier_TutorMoveDescriptions2[selection], 0, 1, 0, NULL);
        else
            AddTextPrinterParameterized(sTutorMoveAndElevatorWindowId, FONT_NORMAL, sBattleFrontier_TutorMoveDescriptions1[selection], 0, 1, 0, NULL);
    }
}

void CloseBattleFrontierTutorWindow(void)
{
    ClearStdWindowAndFrameToTransparent(sTutorMoveAndElevatorWindowId, TRUE);
    RemoveWindow(sTutorMoveAndElevatorWindowId);
}

// Never called
void ScrollableMultichoice_RedrawPersistentMenu(void)
{
    u16 scrollOffset, selectedRow;
    u8 i;
    u8 taskId = FindTaskIdByFunc(Task_ScrollableMultichoice_WaitReturnToList);
    if (taskId != TASK_NONE)
    {
        struct Task *task = &gTasks[taskId];
        ListMenuGetScrollAndRow(task->tListTaskId, &scrollOffset, &selectedRow);
        SetStandardWindowBorderStyle(task->tWindowId, FALSE);

        for (i = 0; i < MAX_SCROLL_MULTI_ON_SCREEN; i++)
            AddTextPrinterParameterized5(task->tWindowId, FONT_NORMAL, sScrollableMultichoiceOptions[gSpecialVar_0x8004][scrollOffset + i], 10, i * 16, TEXT_SKIP_DRAW, NULL, 0, 0);

        AddTextPrinterParameterized(task->tWindowId, FONT_NORMAL, gText_SelectorArrow, 0, selectedRow * 16, TEXT_SKIP_DRAW, NULL);
        PutWindowTilemap(task->tWindowId);
        CopyWindowToVram(task->tWindowId, COPYWIN_FULL);
    }
}

void GetBattleFrontierTutorMoveIndex(void)
{
    u8 i;
    u16 moveTutor = 0;
    u16 moveIndex = 0;
    gSpecialVar_0x8005 = 0;

    moveTutor = VarGet(VAR_TEMP_FRONTIER_TUTOR_ID);
    moveIndex = VarGet(VAR_TEMP_FRONTIER_TUTOR_SELECTION);

    if (moveTutor != 0)
    {
        i = 0;
        do
        {
            if (gTutorMoves[i] == sBattleFrontier_TutorMoves2[moveIndex])
            {
                gSpecialVar_0x8005 = i;
                break;
            }
            i++;
        } while (i < TUTOR_MOVE_COUNT);
    }
    else
    {
        i = 0;
        do
        {
            if (gTutorMoves[i] == sBattleFrontier_TutorMoves1[moveIndex])
            {
                gSpecialVar_0x8005 = i;
                break;
            }
            i++;
        } while (i < TUTOR_MOVE_COUNT);
    }
}

// Never called
// Close a scrollable multichoice that stays open after selection
void ScrollableMultichoice_ClosePersistentMenu(void)
{
    u8 taskId = FindTaskIdByFunc(Task_ScrollableMultichoice_WaitReturnToList);
    if (taskId != TASK_NONE)
    {
        struct Task *task = &gTasks[taskId];
        DestroyListMenuTask(task->tListTaskId, NULL, NULL);
        Free(sScrollableMultichoice_ListMenuItem);
        ClearStdWindowAndFrameToTransparent(task->tWindowId, TRUE);
        FillWindowPixelBuffer(task->tWindowId, PIXEL_FILL(0));
        ClearWindowTilemap(task->tWindowId);
        CopyWindowToVram(task->tWindowId, COPYWIN_GFX);
        RemoveWindow(task->tWindowId);
        DestroyTask(taskId);
    }
}

// Undefine Scrollable Multichoice task data macros
#undef tMaxItemsOnScreen
#undef tNumItems
#undef tLeft
#undef tTop
#undef tWidth
#undef tHeight
#undef tKeepOpenAfterSelect
#undef tScrollOffset
#undef tSelectedRow
#undef tScrollMultiId
#undef tScrollArrowId
#undef tWindowId
#undef tListTaskId
#undef tTaskId

#define DEOXYS_ROCK_LEVELS 11

void DoDeoxysRockInteraction(void)
{
    CreateTask(Task_DeoxysRockInteraction, 8);
}

static const u16 sDeoxysRockPalettes[DEOXYS_ROCK_LEVELS][16] = {
    INCBIN_U16("graphics/field_effects/palettes/deoxys_rock_1.gbapal"),
    INCBIN_U16("graphics/field_effects/palettes/deoxys_rock_2.gbapal"),
    INCBIN_U16("graphics/field_effects/palettes/deoxys_rock_3.gbapal"),
    INCBIN_U16("graphics/field_effects/palettes/deoxys_rock_4.gbapal"),
    INCBIN_U16("graphics/field_effects/palettes/deoxys_rock_5.gbapal"),
    INCBIN_U16("graphics/field_effects/palettes/deoxys_rock_6.gbapal"),
    INCBIN_U16("graphics/field_effects/palettes/deoxys_rock_7.gbapal"),
    INCBIN_U16("graphics/field_effects/palettes/deoxys_rock_8.gbapal"),
    INCBIN_U16("graphics/field_effects/palettes/deoxys_rock_9.gbapal"),
    INCBIN_U16("graphics/field_effects/palettes/deoxys_rock_10.gbapal"),
    INCBIN_U16("graphics/field_effects/palettes/deoxys_rock_11.gbapal"),
};

static const u8 sDeoxysRockCoords[DEOXYS_ROCK_LEVELS][2] = {
    { 15, 12 },
    { 11, 14 },
    { 15,  8 },
    { 19, 14 },
    { 12, 11 },
    { 18, 11 },
    { 15, 14 },
    { 11, 14 },
    { 19, 14 },
    { 15, 15 },
    { 15, 10 },
};

static void Task_DeoxysRockInteraction(u8 taskId)
{
    static const u8 sStoneMaxStepCounts[DEOXYS_ROCK_LEVELS - 1] = { 4, 8, 8, 8, 4, 4, 4, 6, 3, 3 };

    if (FlagGet(FLAG_DEOXYS_ROCK_COMPLETE) == TRUE)
    {
        gSpecialVar_Result = DEOXYS_ROCK_COMPLETE;
        ScriptContext_Enable();
        DestroyTask(taskId);
    }
    else
    {
        u16 rockLevel = VarGet(VAR_DEOXYS_ROCK_LEVEL);
        u16 stepCount = VarGet(VAR_DEOXYS_ROCK_STEP_COUNT);

        VarSet(VAR_DEOXYS_ROCK_STEP_COUNT, 0);
        if (rockLevel != 0 && sStoneMaxStepCounts[rockLevel - 1] < stepCount)
        {
            // Player failed to take the shortest path to the stone, so it resets.
            ChangeDeoxysRockLevel(0);
            VarSet(VAR_DEOXYS_ROCK_LEVEL, 0);
            gSpecialVar_Result = DEOXYS_ROCK_FAILED;
            DestroyTask(taskId);
        }
        else if (rockLevel == DEOXYS_ROCK_LEVELS - 1)
        {
            FlagSet(FLAG_DEOXYS_ROCK_COMPLETE);
            gSpecialVar_Result = DEOXYS_ROCK_SOLVED;
            ScriptContext_Enable();
            DestroyTask(taskId);
        }
        else
        {
            rockLevel++;
            ChangeDeoxysRockLevel(rockLevel);
            VarSet(VAR_DEOXYS_ROCK_LEVEL, rockLevel);
            gSpecialVar_Result = DEOXYS_ROCK_PROGRESSED;
            DestroyTask(taskId);
        }
    }
}

// duplicate of event_object_movement
#define OBJ_EVENT_PAL_TAG_BIRTH_ISLAND_STONE      0x111F

static void ChangeDeoxysRockLevel(u8 rockLevel)
{
    u8 paletteNum = IndexOfSpritePaletteTag(OBJ_EVENT_PAL_TAG_BIRTH_ISLAND_STONE);
    LoadPalette(&sDeoxysRockPalettes[rockLevel], OBJ_PLTT_ID(paletteNum), PLTT_SIZEOF(4));

    if (rockLevel == 0)
        PlaySE(SE_M_CONFUSE_RAY); // Failure sound
    else
        PlaySE(SE_RG_DEOXYS_MOVE); // Success sound

    CreateTask(WaitForDeoxysRockMovement, 8);
    gFieldEffectArguments[0] = LOCALID_BIRTH_ISLAND_EXTERIOR_ROCK;
    gFieldEffectArguments[1] = MAP_NUM(BIRTH_ISLAND_EXTERIOR);
    gFieldEffectArguments[2] = MAP_GROUP(BIRTH_ISLAND_EXTERIOR);
    gFieldEffectArguments[3] = sDeoxysRockCoords[rockLevel][0];
    gFieldEffectArguments[4] = sDeoxysRockCoords[rockLevel][1];

    // Set number of movement steps.
    // Resetting for failure is slow, successful movement is fast.
    if (rockLevel == 0)
        gFieldEffectArguments[5] = 60;
    else
        gFieldEffectArguments[5] = 5;

    FieldEffectStart(FLDEFF_MOVE_DEOXYS_ROCK);
    SetObjEventTemplateCoords(LOCALID_BIRTH_ISLAND_EXTERIOR_ROCK, sDeoxysRockCoords[rockLevel][0], sDeoxysRockCoords[rockLevel][1]);
}

static void WaitForDeoxysRockMovement(u8 taskId)
{
    if (FieldEffectActiveListContains(FLDEFF_MOVE_DEOXYS_ROCK) == FALSE)
    {
        ScriptContext_Enable();
        DestroyTask(taskId);
    }
}

void IncrementBirthIslandRockStepCount(void)
{
    u16 stepCount = VarGet(VAR_DEOXYS_ROCK_STEP_COUNT);
    if (gSaveBlock1Ptr->location.mapNum == MAP_NUM(BIRTH_ISLAND_EXTERIOR) && gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(BIRTH_ISLAND_EXTERIOR))
    {
        if (++stepCount > 99)
            VarSet(VAR_DEOXYS_ROCK_STEP_COUNT, 0);
        else
            VarSet(VAR_DEOXYS_ROCK_STEP_COUNT, stepCount);
    }
}

// called before fade-in
void SetDeoxysRockPalette(void)
{
    u32 paletteNum = IndexOfSpritePaletteTag(OBJ_EVENT_PAL_TAG_BIRTH_ISLAND_STONE);
    LoadPalette(&sDeoxysRockPalettes[(u8)VarGet(VAR_DEOXYS_ROCK_LEVEL)], OBJ_PLTT_ID(paletteNum), PLTT_SIZEOF(4));
    // Set faded to all black, weather blending handled during fade-in
    CpuFill16(0, &gPlttBufferFaded[OBJ_PLTT_ID(paletteNum)], 32);
}

void SetPCBoxToSendMon(u8 boxId)
{
    sPCBoxToSendMon = boxId;
}

u16 GetPCBoxToSendMon(void)
{
    return sPCBoxToSendMon;
}

bool8 ShouldShowBoxWasFullMessage(void)
{
    if (!FlagGet(FLAG_SHOWN_BOX_WAS_FULL_MESSAGE))
    {
        if (StorageGetCurrentBox() != VarGet(VAR_PC_BOX_TO_SEND_MON))
        {
            FlagSet(FLAG_SHOWN_BOX_WAS_FULL_MESSAGE);
            return TRUE;
        }
    }
    return FALSE;
}

bool8 IsDestinationBoxFull(void)
{
    int box;
    int i;
    SetPCBoxToSendMon(VarGet(VAR_PC_BOX_TO_SEND_MON));
    box = StorageGetCurrentBox();
    do
    {
        for (i = 0; i < IN_BOX_COUNT; i++)
        {
            if (GetBoxMonData(GetBoxedMonPtr(box, i), MON_DATA_SPECIES, 0) == SPECIES_NONE)
            {
                if (GetPCBoxToSendMon() != box)
                    FlagClear(FLAG_SHOWN_BOX_WAS_FULL_MESSAGE);
                VarSet(VAR_PC_BOX_TO_SEND_MON, box);
                return ShouldShowBoxWasFullMessage();
            }
        }

        if (++box == TOTAL_BOXES_COUNT)
            box = 0;
    } while (box != StorageGetCurrentBox());
    return FALSE;
}

void CreateAbnormalWeatherEvent(void)
{
    u16 randomValue = Random();
    VarSet(VAR_ABNORMAL_WEATHER_STEP_COUNTER, 0);

    if (FlagGet(FLAG_DEFEATED_KYOGRE) == TRUE)
    {
        VarSet(VAR_ABNORMAL_WEATHER_LOCATION, (randomValue % TERRA_CAVE_LOCATIONS) + TERRA_CAVE_LOCATIONS_START);
    }
    else if (FlagGet(FLAG_DEFEATED_GROUDON) == TRUE)
    {
        VarSet(VAR_ABNORMAL_WEATHER_LOCATION, (randomValue % MARINE_CAVE_LOCATIONS) + MARINE_CAVE_LOCATIONS_START);
    }
    else if ((randomValue & 1) == 0)
    {
        randomValue = Random();
        VarSet(VAR_ABNORMAL_WEATHER_LOCATION, (randomValue % TERRA_CAVE_LOCATIONS) + TERRA_CAVE_LOCATIONS_START);
    }
    else
    {
        randomValue = Random();
        VarSet(VAR_ABNORMAL_WEATHER_LOCATION, (randomValue % MARINE_CAVE_LOCATIONS) + MARINE_CAVE_LOCATIONS_START);
    }
}

// Saves the map name for the current abnormal weather location in gStringVar1, then
// returns TRUE if the weather is for Kyogre, and FALSE if it's for Groudon.
bool32 GetAbnormalWeatherMapNameAndType(void)
{
    static const u8 sAbnormalWeatherMapNumbers[] = {
        MAP_NUM(ROUTE114),
        MAP_NUM(ROUTE114),
        MAP_NUM(ROUTE115),
        MAP_NUM(ROUTE115),
        MAP_NUM(ROUTE116),
        MAP_NUM(ROUTE116),
        MAP_NUM(ROUTE118),
        MAP_NUM(ROUTE118),
        MAP_NUM(ROUTE105),
        MAP_NUM(ROUTE105),
        MAP_NUM(ROUTE125),
        MAP_NUM(ROUTE125),
        MAP_NUM(ROUTE127),
        MAP_NUM(ROUTE127),
        MAP_NUM(ROUTE129),
        MAP_NUM(ROUTE129)
    };

    u16 abnormalWeather = VarGet(VAR_ABNORMAL_WEATHER_LOCATION);

    GetMapName(gStringVar1, sAbnormalWeatherMapNumbers[abnormalWeather - 1], 0);

    if (abnormalWeather < MARINE_CAVE_LOCATIONS_START)
        return FALSE;
    else
        return TRUE;
}

bool8 AbnormalWeatherHasExpired(void)
{
    // Duplicate array.
    static const u8 sAbnormalWeatherMapNumbers[] =
    {
        MAP_NUM(ROUTE114),
        MAP_NUM(ROUTE114),
        MAP_NUM(ROUTE115),
        MAP_NUM(ROUTE115),
        MAP_NUM(ROUTE116),
        MAP_NUM(ROUTE116),
        MAP_NUM(ROUTE118),
        MAP_NUM(ROUTE118),
        MAP_NUM(ROUTE105),
        MAP_NUM(ROUTE105),
        MAP_NUM(ROUTE125),
        MAP_NUM(ROUTE125),
        MAP_NUM(ROUTE127),
        MAP_NUM(ROUTE127),
        MAP_NUM(ROUTE129),
        MAP_NUM(ROUTE129)
    };

    u16 steps = VarGet(VAR_ABNORMAL_WEATHER_STEP_COUNTER);
    u16 abnormalWeather = VarGet(VAR_ABNORMAL_WEATHER_LOCATION);

    if (abnormalWeather == ABNORMAL_WEATHER_NONE)
        return FALSE;

    if (++steps > 999)
    {
        VarSet(VAR_ABNORMAL_WEATHER_STEP_COUNTER, 0);
        if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(UNDERWATER_MARINE_CAVE))
        {
            switch (gSaveBlock1Ptr->location.mapNum)
            {
            case MAP_NUM(UNDERWATER_MARINE_CAVE):
            case MAP_NUM(MARINE_CAVE_ENTRANCE):
            case MAP_NUM(MARINE_CAVE_END):
            case MAP_NUM(TERRA_CAVE_ENTRANCE):
            case MAP_NUM(TERRA_CAVE_END):
                VarSet(VAR_SHOULD_END_ABNORMAL_WEATHER, 1);
                return FALSE;
            default:
                break;
            }
        }

        if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(UNDERWATER_ROUTE127))
        {
            switch (gSaveBlock1Ptr->location.mapNum)
            {
            case MAP_NUM(UNDERWATER_ROUTE127):
            case MAP_NUM(UNDERWATER_ROUTE129):
            case MAP_NUM(UNDERWATER_ROUTE105):
            case MAP_NUM(UNDERWATER_ROUTE125):
                VarSet(VAR_SHOULD_END_ABNORMAL_WEATHER, 1);
                return FALSE;
            default:
                break;
            }
        }

        if (gSaveBlock1Ptr->location.mapNum == sAbnormalWeatherMapNumbers[abnormalWeather - 1] &&
            gSaveBlock1Ptr->location.mapGroup == 0)
        {
            return TRUE;
        }
        else
        {
            VarSet(VAR_ABNORMAL_WEATHER_LOCATION, ABNORMAL_WEATHER_NONE);
            return FALSE;
        }
    }
    else
    {
        VarSet(VAR_ABNORMAL_WEATHER_STEP_COUNTER, steps);
        return FALSE;
    }
}

void Unused_SetWeatherSunny(void)
{
    SetCurrentAndNextWeather(WEATHER_SUNNY);
}

// All mart employees have a local id of 1, so function always returns 1
u32 GetMartEmployeeObjectEventId(void)
{
    static const u8 sPokeMarts[][3] =
    {
        { MAP_GROUP(OLDALE_TOWN_MART),     MAP_NUM(OLDALE_TOWN_MART),     LOCALID_OLDALE_MART_CLERK },
        { MAP_GROUP(LAVARIDGE_TOWN_MART),  MAP_NUM(LAVARIDGE_TOWN_MART),  LOCALID_LAVARIDGE_MART_CLERK },
        { MAP_GROUP(FALLARBOR_TOWN_MART),  MAP_NUM(FALLARBOR_TOWN_MART),  LOCALID_FALLARBOR_MART_CLERK },
        { MAP_GROUP(VERDANTURF_TOWN_MART), MAP_NUM(VERDANTURF_TOWN_MART), LOCALID_VERDANTURF_MART_CLERK },
        { MAP_GROUP(PETALBURG_CITY_MART),  MAP_NUM(PETALBURG_CITY_MART),  LOCALID_PETALBURG_MART_CLERK },
        { MAP_GROUP(SLATEPORT_CITY_MART),  MAP_NUM(SLATEPORT_CITY_MART),  LOCALID_SLATEPORT_MART_CLERK },
        { MAP_GROUP(MAUVILLE_CITY_MART),   MAP_NUM(MAUVILLE_CITY_MART),   LOCALID_MAUVILLE_MART_CLERK },
        { MAP_GROUP(RUSTBORO_CITY_MART),   MAP_NUM(RUSTBORO_CITY_MART),   LOCALID_RUSTBORO_MART_CLERK },
        { MAP_GROUP(FORTREE_CITY_MART),    MAP_NUM(FORTREE_CITY_MART),    LOCALID_FORTREE_MART_CLERK },
        { MAP_GROUP(MOSSDEEP_CITY_MART),   MAP_NUM(MOSSDEEP_CITY_MART),   LOCALID_MOSSDEEP_MART_CLERK },
        { MAP_GROUP(SOOTOPOLIS_CITY_MART), MAP_NUM(SOOTOPOLIS_CITY_MART), LOCALID_SOOTOPOLIS_MART_CLERK },
        { MAP_GROUP(BATTLE_FRONTIER_MART), MAP_NUM(BATTLE_FRONTIER_MART), LOCALID_BATTLE_FRONTIER_MART_CLERK }
    };

    u8 i;
    for (i = 0; i < ARRAY_COUNT(sPokeMarts); i++)
    {
        if (gSaveBlock1Ptr->location.mapGroup == sPokeMarts[i][0])
        {
            if (gSaveBlock1Ptr->location.mapNum == sPokeMarts[i][1])
                return sPokeMarts[i][2];
        }
    }
    return 1;
}

bool32 IsTrainerRegistered(void)
{
    int index = GetRematchIdxByTrainerIdx(gSpecialVar_0x8004);
    if (index >= 0)
    {
        if (FlagGet(FLAG_MATCH_CALL_REGISTERED + index) == TRUE)
            return TRUE;
    }
    return FALSE;
}

// Always returns FALSE
bool32 ShouldDistributeEonTicket(void)
{
    if (!VarGet(VAR_DISTRIBUTE_EON_TICKET))
        return FALSE;

    return TRUE;
}

#define tState data[0]

void BattleTowerReconnectLink(void)
{
    // Save battle type, restored at end
    // of Task_LinkRetireStatusWithBattleTowerPartner
    sBattleTowerMultiBattleTypeFlags = gBattleTypeFlags;
    gBattleTypeFlags = 0;

    if (!gReceivedRemoteLinkPlayers)
        CreateTask(Task_ReconnectWithLinkPlayers, 5);
}

void LinkRetireStatusWithBattleTowerPartner(void)
{
    CreateTask(Task_LinkRetireStatusWithBattleTowerPartner, 5);
}

// Communicate with a Battle Tower link partner to tell them
// whether or not the player chose to continue or retire,
// and determine what the partner chose to do
// gSpecialVar_0x8004: Player's choice
// gSpecialVar_0x8005: Partner's choice (read from gBlockRecvBuffer[1][0])
static void Task_LinkRetireStatusWithBattleTowerPartner(u8 taskId)
{
    switch (gTasks[taskId].tState)
    {
    case 0:
        if (!FuncIsActiveTask(Task_ReconnectWithLinkPlayers))
            gTasks[taskId].tState++;
        break;
    case 1:
        if (IsLinkTaskFinished() == TRUE)
        {
            if (GetMultiplayerId() == 0)
            {
                // Player is link leader, skip sending data
                gTasks[taskId].tState++;
            }
            else
            {
                // Send value of gSpecialVar_0x8004 to leader
                // Will either be BATTLE_TOWER_LINK_CONTINUE or BATTLE_TOWER_LINK_RETIRE
                SendBlock(BitmaskAllOtherLinkPlayers(), &gSpecialVar_0x8004, sizeof(gSpecialVar_0x8004));
                gTasks[taskId].tState++;
            }
        }
        break;
    case 2:
        if (GetBlockReceivedStatus() & 2)
        {
            if (GetMultiplayerId() == 0)
            {
                // Player is leader, read partner's choice
                // and determine if play should continue
                gSpecialVar_0x8005 = gBlockRecvBuffer[1][0];
                ResetBlockReceivedFlag(1);

                if (gSpecialVar_0x8004 == BATTLE_TOWER_LINK_RETIRE
                 && gSpecialVar_0x8005 == BATTLE_TOWER_LINK_RETIRE)
                    gSpecialVar_Result = BATTLE_TOWER_LINKSTAT_BOTH_RETIRE;
                else if (gSpecialVar_0x8004 == BATTLE_TOWER_LINK_CONTINUE
                      && gSpecialVar_0x8005 == BATTLE_TOWER_LINK_RETIRE)
                    gSpecialVar_Result = BATTLE_TOWER_LINKSTAT_MEMBER_RETIRE;
                else if (gSpecialVar_0x8004 == BATTLE_TOWER_LINK_RETIRE
                      && gSpecialVar_0x8005 == BATTLE_TOWER_LINK_CONTINUE)
                    gSpecialVar_Result = BATTLE_TOWER_LINKSTAT_LEADER_RETIRE;
                else
                    gSpecialVar_Result = BATTLE_TOWER_LINKSTAT_CONTINUE;
            }
            gTasks[taskId].tState++;
        }
        break;
    case 3:
        if (IsLinkTaskFinished() == TRUE)
        {
            if (GetMultiplayerId() != 0)
            {
                // Player is not link leader, wait for leader's response
                gTasks[taskId].tState++;
            }
            else
            {
                // Send whether or not play should continue
                SendBlock(BitmaskAllOtherLinkPlayers(), &gSpecialVar_Result, sizeof(gSpecialVar_Result));
                gTasks[taskId].tState++;
            }
        }
        break;
    case 4:
        if (GetBlockReceivedStatus() & 1)
        {
            if (GetMultiplayerId() != 0)
            {
                // Player is not link leader, read leader's response
                gSpecialVar_Result = gBlockRecvBuffer[0][0];
                ResetBlockReceivedFlag(0);
                gTasks[taskId].tState++;
            }
            else
            {
                gTasks[taskId].tState++;
            }
        }
        break;
    case 5:
        // Print message if partner chose to retire (and player didn't)
        if (GetMultiplayerId() == 0)
        {
            if (gSpecialVar_Result == BATTLE_TOWER_LINKSTAT_MEMBER_RETIRE)
                ShowFieldAutoScrollMessage(gText_YourPartnerHasRetired);
        }
        else
        {
            if (gSpecialVar_Result == BATTLE_TOWER_LINKSTAT_LEADER_RETIRE)
                ShowFieldAutoScrollMessage(gText_YourPartnerHasRetired);
        }
        gTasks[taskId].tState++;
        break;
    case 6:
        if (!IsTextPrinterActive(0))
            gTasks[taskId].tState++;
        break;
    case 7:
        if (IsLinkTaskFinished() == TRUE)
        {
            SetLinkStandbyCallback();
            gTasks[taskId].tState++;
        }
        break;
    case 8:
        if (IsLinkTaskFinished() == TRUE)
            gTasks[taskId].tState++;
        break;
    case 9:
        if (gWirelessCommType == 0)
            SetCloseLinkCallback();

        gBattleTypeFlags = sBattleTowerMultiBattleTypeFlags;
        ScriptContext_Enable();
        DestroyTask(taskId);
        break;
    }
}

#undef tState

void Script_DoRayquazaScene(void)
{
    if (!gSpecialVar_0x8004)
    {
        // Groudon/Kyogre fight scene
        DoRayquazaScene(0, TRUE, CB2_ReturnToFieldContinueScriptPlayMapMusic);
    }
    else
    {
        // Rayquaza arrives scene
        DoRayquazaScene(1, FALSE, CB2_ReturnToFieldContinueScriptPlayMapMusic);
    }
}

#define playCount data[0]
#define delay     data[1]

void LoopWingFlapSE(void)
{
    CreateTask(Task_LoopWingFlapSE, 8);
    PlaySE(SE_M_WING_ATTACK);
}

static void Task_LoopWingFlapSE(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    delay++;
    if (delay == gSpecialVar_0x8005)
    {
        playCount++;
        delay = 0;
        PlaySE(SE_M_WING_ATTACK);
    }

    if (playCount == gSpecialVar_0x8004 - 1)
        DestroyTask(taskId);
}

#undef playCount
#undef delay

#define CURTAIN_HEIGHT 4
#define CURTAIN_WIDTH 3
#define tFrameTimer   data
#define tCurrentFrame data[3]

void CloseBattlePikeCurtain(void)
{
    u8 taskId = CreateTask(Task_CloseBattlePikeCurtain, 8);
    gTasks[taskId].tFrameTimer[0] = 4;
    gTasks[taskId].tFrameTimer[1] = 4;
    gTasks[taskId].tFrameTimer[2] = 4;
    gTasks[taskId].tCurrentFrame = 0;
}

static void Task_CloseBattlePikeCurtain(u8 taskId)
{
    u8 x, y;
    s16 *data = gTasks[taskId].data;

    tFrameTimer[tCurrentFrame]--;
    if (tFrameTimer[tCurrentFrame] == 0)
    {
        for (y = 0; y < CURTAIN_HEIGHT; y++)
        {
            for (x = 0; x < CURTAIN_WIDTH; x++)
            {
                MapGridSetMetatileIdAt(gSaveBlock1Ptr->pos.x + x + MAP_OFFSET - 1,
                                       gSaveBlock1Ptr->pos.y + y + MAP_OFFSET - 3,
                                       (x + METATILE_BattlePike_CurtainFrames_Start) + (y * METATILE_ROW_WIDTH) + (tCurrentFrame * CURTAIN_HEIGHT * METATILE_ROW_WIDTH));
            }
        }
        DrawWholeMapView();
        tCurrentFrame++;
        if (tCurrentFrame == 3)
        {
            DestroyTask(taskId);
            ScriptContext_Enable();
        }
    }
}

#undef CURTAIN_HEIGHT
#undef CURTAIN_WIDTH
#undef tFrameTimer
#undef tCurrentFrame

void GetBattlePyramidHint(void)
{
    // gSpecialVar_0x8004 here is expected to be the current Battle Pyramid win streak.
    gSpecialVar_Result = gSpecialVar_0x8004 / FRONTIER_STAGES_PER_CHALLENGE;
    gSpecialVar_Result -= (gSpecialVar_Result / TOTAL_PYRAMID_ROUNDS) * TOTAL_PYRAMID_ROUNDS;
}

// Used to avoid a potential softlock if the player respawns on Dewford with no way off
void ResetHealLocationFromDewford(void)
{
    if (gSaveBlock1Ptr->lastHealLocation.mapGroup == MAP_GROUP(DEWFORD_TOWN) && gSaveBlock1Ptr->lastHealLocation.mapNum == MAP_NUM(DEWFORD_TOWN))
        SetLastHealLocationWarp(HEAL_LOCATION_PETALBURG_CITY);
}

bool8 InPokemonCenter(void)
{
    static const u16 sPokemonCenters[] =
    {
        MAP_OLDALE_TOWN_POKEMON_CENTER_1F,
        MAP_DEWFORD_TOWN_POKEMON_CENTER_1F,
        MAP_LAVARIDGE_TOWN_POKEMON_CENTER_1F,
        MAP_FALLARBOR_TOWN_POKEMON_CENTER_1F,
        MAP_VERDANTURF_TOWN_POKEMON_CENTER_1F,
        MAP_PACIFIDLOG_TOWN_POKEMON_CENTER_1F,
        MAP_PETALBURG_CITY_POKEMON_CENTER_1F,
        MAP_SLATEPORT_CITY_POKEMON_CENTER_1F,
        MAP_MAUVILLE_CITY_POKEMON_CENTER_1F,
        MAP_RUSTBORO_CITY_POKEMON_CENTER_1F,
        MAP_FORTREE_CITY_POKEMON_CENTER_1F,
        MAP_LILYCOVE_CITY_POKEMON_CENTER_1F,
        MAP_MOSSDEEP_CITY_POKEMON_CENTER_1F,
        MAP_SOOTOPOLIS_CITY_POKEMON_CENTER_1F,
        MAP_EVER_GRANDE_CITY_POKEMON_CENTER_1F,
        MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F,
        MAP_BATTLE_FRONTIER_POKEMON_CENTER_1F,
        MAP_BATTLE_COLOSSEUM_2P,
        MAP_TRADE_CENTER,
        MAP_RECORD_CORNER,
        MAP_BATTLE_COLOSSEUM_4P,
        MAP_UNDEFINED
    };

    int i;
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) + gSaveBlock1Ptr->location.mapNum;

    for (i = 0; sPokemonCenters[i] != MAP_UNDEFINED; i++)
    {
        if (sPokemonCenters[i] == map)
            return TRUE;
    }
    return FALSE;
}

/*  Summary of the Lilycove Trainer Fan Club, because it's a little messy

    ## The Fan Club room itself
    There are initially 4 members of the Fan Club (+ an interviewer), none of whom are fans of the player
    After becoming the champion there will be 8 members of the Fan Club, 3 of whom are automatically fans of the player
    After this point, if a club member is a fan of the player they will sit at the front table and comment on the player
    If they are not fans of the player, they will sit at the far table and can make comments about a different trainer (see BufferFanClubTrainerName)

    ## Gaining/losing fans
    After every link battle the player will gain a fan if they won, or lose a fan if they lost
    If the player has at least 3 fans, this is the only way to gain fans
    If the player has fewer than 3 fans, they may also gain fans by completing certain tasks enough times (see TryGainNewFanFromCounter)
    If the player has at least 5 fans, they can lose a fan every 12 real-time hours, or more often if the timer variable is reset (see TryLoseFansFromPlayTime)
    If the player has only 1 fan left it cannot be lost

    ## Variables
    VAR_FANCLUB_FAN_COUNTER, a bitfield for tracking the fans
      Bits  1-7: Counter for when to add new fans
      Bit     8: Flag set after receiving the initial 3 fans
      Bits 9-16: Flags for each of the 8 club members, set to 1 when theyre a fan of the player and 0 when theyre not

    VAR_FANCLUB_LOSE_FAN_TIMER, a timer for when to lose fans
      Compared against playTimeHours. When theyre equal, a fan is ready to be lost
      For every fan thats lost this way 12 hours are added to the timer

    VAR_LILYCOVE_FAN_CLUB_STATE
      0: Player is not the champion yet
      1: Player is the champion, ready to meet their initial fans
      2: Player has met their initial fans
*/

#define FANCLUB_BITFIELD (gSaveBlock1Ptr->vars[VAR_FANCLUB_FAN_COUNTER - VARS_START])
#define FANCLUB_COUNTER    0x007F

#define GET_TRAINER_FAN_CLUB_FLAG(flag) (FANCLUB_BITFIELD >> (flag) & 1)
#define SET_TRAINER_FAN_CLUB_FLAG(flag) (FANCLUB_BITFIELD |= 1 << (flag))
#define FLIP_TRAINER_FAN_CLUB_FLAG(flag)(FANCLUB_BITFIELD ^= 1 << (flag))

#define GET_TRAINER_FAN_CLUB_COUNTER        (FANCLUB_BITFIELD & FANCLUB_COUNTER)
#define SET_TRAINER_FAN_CLUB_COUNTER(count) (FANCLUB_BITFIELD = (FANCLUB_BITFIELD & ~FANCLUB_COUNTER) | (count))
#define INCR_TRAINER_FAN_CLUB_COUNTER(count)(FANCLUB_BITFIELD += (count))
#define CLEAR_TRAINER_FAN_CLUB_COUNTER      (FANCLUB_BITFIELD &= ~FANCLUB_COUNTER)

void ResetFanClub(void)
{
    gSaveBlock1Ptr->vars[VAR_FANCLUB_FAN_COUNTER - VARS_START] = 0;
    gSaveBlock1Ptr->vars[VAR_FANCLUB_LOSE_FAN_TIMER - VARS_START] = 0;
}

void TryLoseFansFromPlayTimeAfterLinkBattle(void)
{
    if (DidPlayerGetFirstFans())
    {
        TryLoseFansFromPlayTime();
        gSaveBlock1Ptr->vars[VAR_FANCLUB_LOSE_FAN_TIMER - VARS_START] = gSaveBlock2Ptr->playTimeHours;
    }
}

void UpdateTrainerFanClubGameClear(void)
{
    if (!GET_TRAINER_FAN_CLUB_FLAG(FANCLUB_GOT_FIRST_FANS))
    {
        SetPlayerGotFirstFans();
        SetInitialFansOfPlayer();
        gSaveBlock1Ptr->vars[VAR_FANCLUB_LOSE_FAN_TIMER - VARS_START] = gSaveBlock2Ptr->playTimeHours;
        FlagClear(FLAG_HIDE_FANCLUB_OLD_LADY);
        FlagClear(FLAG_HIDE_FANCLUB_BOY);
        FlagClear(FLAG_HIDE_FANCLUB_LITTLE_BOY);
        FlagClear(FLAG_HIDE_FANCLUB_LADY);
        FlagClear(FLAG_HIDE_LILYCOVE_FAN_CLUB_INTERVIEWER);
        VarSet(VAR_LILYCOVE_FAN_CLUB_STATE, 1);
    }
}

// If the player has < 3 fans, gain a new fan whenever the counter reaches 20+
// Defeating Drake or participating in a Contest increments the counter by 2
// Participating at Battle Tower or in a Secret Base battle increments the counter by 1
u8 TryGainNewFanFromCounter(u8 incrementId)
{
    static const u8 sCounterIncrements[] =
    {
        [FANCOUNTER_DEFEATED_DRAKE]    = 2,
        [FANCOUNTER_BATTLED_AT_BASE]   = 1,
        [FANCOUNTER_FINISHED_CONTEST]  = 2,
        [FANCOUNTER_USED_BATTLE_TOWER] = 1
    };

    if (VarGet(VAR_LILYCOVE_FAN_CLUB_STATE) == 2)
    {
        if (GET_TRAINER_FAN_CLUB_COUNTER + sCounterIncrements[incrementId] > 19)
        {
            if (GetNumFansOfPlayerInTrainerFanClub() < 3)
            {
                PlayerGainRandomTrainerFan();
                CLEAR_TRAINER_FAN_CLUB_COUNTER;
            }
            else
            {
                SET_TRAINER_FAN_CLUB_COUNTER(20);
            }
        }
        else
        {
            INCR_TRAINER_FAN_CLUB_COUNTER(sCounterIncrements[incrementId]);
        }
    }

    return GET_TRAINER_FAN_CLUB_COUNTER;
}


// Loop through the fan club members, and if theyre not a fan of the player there is a 50% chance for them to become a fan
// Stops when a fan is gained
// If no new fan was gained while looping, the last non-fan in the list becomes a fan
// If all the members are already fans of the player then this redundantly sets the first fan in the list to be a fan
static u16 PlayerGainRandomTrainerFan(void)
{
    static const u8 sFanClubMemberIds[NUM_TRAINER_FAN_CLUB_MEMBERS] =
    {
        FANCLUB_MEMBER1,
        FANCLUB_MEMBER2,
        FANCLUB_MEMBER3,
        FANCLUB_MEMBER4,
        FANCLUB_MEMBER5,
        FANCLUB_MEMBER6,
        FANCLUB_MEMBER7,
        FANCLUB_MEMBER8
    };

    u8 i;
    u8 idx = 0;

    for (i = 0; i < ARRAY_COUNT(sFanClubMemberIds); i++)
    {
        if (!GET_TRAINER_FAN_CLUB_FLAG(sFanClubMemberIds[i]))
        {
            idx = i;
            if (Random() & 1)
            {
                SET_TRAINER_FAN_CLUB_FLAG(sFanClubMemberIds[idx]);
                return idx;
            }
        }
    }
    SET_TRAINER_FAN_CLUB_FLAG(sFanClubMemberIds[idx]);
    return idx;
}

// Loops through the fan club members, and if theyre a fan of the player there is a 50% chance for them to stop being a fan
// Stops if a fan is removed, or if the player has only one fan left
// If no fan was lost while looping, the last current fan in the list will stop being a fan
static u16 PlayerLoseRandomTrainerFan(void)
{
    static const u8 sFanClubMemberIds[NUM_TRAINER_FAN_CLUB_MEMBERS] =
    {
        FANCLUB_MEMBER1,
        FANCLUB_MEMBER6,
        FANCLUB_MEMBER7,
        FANCLUB_MEMBER4,
        FANCLUB_MEMBER3,
        FANCLUB_MEMBER5,
        FANCLUB_MEMBER8,
        FANCLUB_MEMBER2
    };

    u8 i;
    u8 idx = 0;

    if (GetNumFansOfPlayerInTrainerFanClub() == 1)
        return 0;

    for (i = 0; i < ARRAY_COUNT(sFanClubMemberIds); i++)
    {
        if (GET_TRAINER_FAN_CLUB_FLAG(sFanClubMemberIds[i]))
        {
            idx = i;
            if (Random() & 1)
            {
                FLIP_TRAINER_FAN_CLUB_FLAG(sFanClubMemberIds[idx]);
                return idx;
            }
        }
    }

    if (GET_TRAINER_FAN_CLUB_FLAG(sFanClubMemberIds[idx]))
        FLIP_TRAINER_FAN_CLUB_FLAG(sFanClubMemberIds[idx]);

    return idx;
}

u16 GetNumFansOfPlayerInTrainerFanClub(void)
{
    u8 i;
    u8 numFans = 0;

    for (i = 0; i < NUM_TRAINER_FAN_CLUB_MEMBERS; i++)
    {
        if (GET_TRAINER_FAN_CLUB_FLAG(i + FANCLUB_MEMBER1))
            numFans++;
    }

    return numFans;
}

// If the player has > 5 fans in the Trainer Fan Club, then lose 1 fan for every 12 hours since the last fan loss / timer reset
void TryLoseFansFromPlayTime(void)
{
    u8 i = 0;
    if (gSaveBlock2Ptr->playTimeHours < 999)
    {
        while (TRUE)
        {
            if (GetNumFansOfPlayerInTrainerFanClub() < 5)
            {
                gSaveBlock1Ptr->vars[VAR_FANCLUB_LOSE_FAN_TIMER - VARS_START] = gSaveBlock2Ptr->playTimeHours;
                break;
            }
            else if (i == NUM_TRAINER_FAN_CLUB_MEMBERS)
            {
                break;
            }
            else if (gSaveBlock2Ptr->playTimeHours - gSaveBlock1Ptr->vars[VAR_FANCLUB_LOSE_FAN_TIMER - VARS_START] < 12)
            {
                return;
            }
            PlayerLoseRandomTrainerFan();
            gSaveBlock1Ptr->vars[VAR_FANCLUB_LOSE_FAN_TIMER - VARS_START] += 12;
            i++;
        }
    }
}

bool8 IsFanClubMemberFanOfPlayer(void)
{
    return GET_TRAINER_FAN_CLUB_FLAG(gSpecialVar_0x8004);
}

static void SetInitialFansOfPlayer(void)
{
    SET_TRAINER_FAN_CLUB_FLAG(FANCLUB_MEMBER6);
    SET_TRAINER_FAN_CLUB_FLAG(FANCLUB_MEMBER1);
    SET_TRAINER_FAN_CLUB_FLAG(FANCLUB_MEMBER3);
}

void BufferFanClubTrainerName(void)
{
    u8 whichLinkTrainer = 0;
    u8 whichNPCTrainer = 0;
    switch (gSpecialVar_0x8004)
    {
    case FANCLUB_MEMBER1:
        break;
    case FANCLUB_MEMBER2:
        break;
    case FANCLUB_MEMBER3:
        whichLinkTrainer = 0;
        whichNPCTrainer = 3;
        break;
    case FANCLUB_MEMBER4:
        whichLinkTrainer = 0;
        whichNPCTrainer = 1;
        break;
    case FANCLUB_MEMBER5:
        whichLinkTrainer = 1;
        whichNPCTrainer = 0;
        break;
    case FANCLUB_MEMBER6:
        whichLinkTrainer = 0;
        whichNPCTrainer = 4;
        break;
    case FANCLUB_MEMBER7:
        whichLinkTrainer = 1;
        whichNPCTrainer = 5;
        break;
    case FANCLUB_MEMBER8:
        break;
    }
    BufferFanClubTrainerName_(&gSaveBlock1Ptr->linkBattleRecords, whichLinkTrainer, whichNPCTrainer);
}

static void BufferFanClubTrainerName_(struct LinkBattleRecords *linkRecords, u8 whichLinkTrainer, u8 whichNPCTrainer)
{
    struct LinkBattleRecord *record = &linkRecords->entries[whichLinkTrainer];
    if (record->name[0] == EOS)
    {
        switch (whichNPCTrainer)
        {
        case 0:
            StringCopy(gStringVar1, gText_Wallace);
            break;
        case 1:
            StringCopy(gStringVar1, gText_Steven);
            break;
        case 2:
            StringCopy(gStringVar1, gText_Brawly);
            break;
        case 3:
            StringCopy(gStringVar1, gText_Winona);
            break;
        case 4:
            StringCopy(gStringVar1, gText_Phoebe);
            break;
        case 5:
            StringCopy(gStringVar1, gText_Glacia);
            break;
        default:
            StringCopy(gStringVar1, gText_Wallace);
            break;
        }
    }
    else
    {
        StringCopyN(gStringVar1, record->name, PLAYER_NAME_LENGTH);
        gStringVar1[PLAYER_NAME_LENGTH] = EOS;
        ConvertInternationalString(gStringVar1, linkRecords->languages[whichLinkTrainer]);
    }
}

void UpdateTrainerFansAfterLinkBattle(void)
{
    if (VarGet(VAR_LILYCOVE_FAN_CLUB_STATE) == 2)
    {
        TryLoseFansFromPlayTimeAfterLinkBattle();
        if (gBattleOutcome == B_OUTCOME_WON)
            PlayerGainRandomTrainerFan();
        else
            PlayerLoseRandomTrainerFan();
    }
}

static bool8 DidPlayerGetFirstFans(void)
{
    return GET_TRAINER_FAN_CLUB_FLAG(FANCLUB_GOT_FIRST_FANS);
}

void SetPlayerGotFirstFans(void)
{
    SET_TRAINER_FAN_CLUB_FLAG(FANCLUB_GOT_FIRST_FANS);
}

// return value is always ignored
u8 Script_TryGainNewFanFromCounter(void)
{
    return TryGainNewFanFromCounter(gSpecialVar_0x8004);
}

// Changes a Deoxys' form
void TryChangeDeoxysForm(void)
{
    u16 baseSpecies = GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_SPECIES);
    u16 targetSpecies;

    if (baseSpecies == SPECIES_DEOXYS)
    {
        targetSpecies = SPECIES_DEOXYS_ATTACK;
    }
    else if (baseSpecies == SPECIES_DEOXYS_ATTACK)
    {
        targetSpecies = SPECIES_DEOXYS_DEFENSE;
    }
    else if (baseSpecies == SPECIES_DEOXYS_DEFENSE)
    {
        targetSpecies = SPECIES_DEOXYS_SPEED;
    }
    else if (baseSpecies == SPECIES_DEOXYS_SPEED)
    {
        targetSpecies = SPECIES_DEOXYS;
    }
    else
    {
         gSpecialVar_Result = FALSE;
         return;
    }
    SetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_SPECIES, &targetSpecies);
    CalculateMonStats(&gPlayerParty[gSpecialVar_0x8004]);
    gSpecialVar_Result = TRUE;
    return;
}

void CheckSpecies(void)
{
    u8 i;
    u16 species;
    struct Pokemon *pokemon;
    for (i = 0; i < CalculatePlayerPartyCount(); i++)
    {
        pokemon = &gPlayerParty[i];
        if (GetMonData(pokemon, MON_DATA_SANITY_HAS_SPECIES) && !GetMonData(pokemon, MON_DATA_IS_EGG))
        {
            species = GetMonData(pokemon, MON_DATA_SPECIES);
            if (species == gSpecialVar_0x8005)
            {
                gSpecialVar_Result = TRUE;
                return;
            }
            else
            {
                gSpecialVar_Result = FALSE;
            }
        }
    }
}

void HasRematchBeenFought(void)
{
    if (FlagGet(TRAINER_FLAGS_START + gSpecialVar_0x8005))
    {
        gSpecialVar_Result = TRUE;
        return;
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}

void CheckFoundAllHeartScales(void)
{
    if (FlagGet(FLAG_HIDDEN_ITEM_UNDERWATER_124_HEART_SCALE_1) &&
        FlagGet(FLAG_HIDDEN_ITEM_UNDERWATER_126_HEART_SCALE) &&
        FlagGet(FLAG_HIDDEN_ITEM_UNDERWATER_127_HEART_SCALE) &&
        FlagGet(FLAG_HIDDEN_ITEM_LILYCOVE_CITY_HEART_SCALE) &&
        FlagGet(FLAG_HIDDEN_ITEM_UNDERWATER_124_HEART_SCALE_2) &&
        FlagGet(FLAG_HIDDEN_ITEM_ROUTE_106_HEART_SCALE) &&
        FlagGet(FLAG_HIDDEN_ITEM_ROUTE_109_HEART_SCALE_1) &&
        FlagGet(FLAG_HIDDEN_ITEM_ROUTE_118_HEART_SCALE) &&
        FlagGet(FLAG_HIDDEN_ITEM_ROUTE_104_HEART_SCALE) &&
        FlagGet(FLAG_HIDDEN_ITEM_ROUTE_105_HEART_SCALE) &&
        FlagGet(FLAG_HIDDEN_ITEM_ROUTE_109_HEART_SCALE_2) &&
        FlagGet(FLAG_HIDDEN_ITEM_ROUTE_109_HEART_SCALE_3) &&
        FlagGet(FLAG_HIDDEN_ITEM_ROUTE_128_HEART_SCALE_1) &&
        FlagGet(FLAG_HIDDEN_ITEM_ROUTE_128_HEART_SCALE_2) &&
        FlagGet(FLAG_HIDDEN_ITEM_ROUTE_128_HEART_SCALE_3) &&
        FlagGet(FLAG_HIDDEN_ITEM_ROUTE_115_HEART_SCALE))
    {
        gSpecialVar_Result = TRUE;
        return;
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}

void CheckFoundAllKecleon(void)
{
    if (FlagGet(FLAG_HIDE_FORTREE_CITY_KECLEON) &&
        FlagGet(FLAG_HIDE_ROUTE_120_KECLEON_BRIDGE) &&
        FlagGet(FLAG_HIDE_ROUTE_120_KECLEON_BRIDGE_SHADOW) &&
        FlagGet(FLAG_HIDE_ROUTE_120_KECLEON_1) &&
        FlagGet(FLAG_HIDE_ROUTE_120_KECLEON_2) &&
        FlagGet(FLAG_HIDE_ROUTE_120_KECLEON_3) &&
        FlagGet(FLAG_HIDE_ROUTE_120_KECLEON_4) &&
        FlagGet(FLAG_HIDE_ROUTE_120_KECLEON_5) &&
        FlagGet(FLAG_HIDE_ROUTE_119_KECLEON_1) &&
        FlagGet(FLAG_HIDE_ROUTE_119_KECLEON_2))
    {
        gSpecialVar_Result = TRUE;
        return;
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}

void CheckGotAllKantoStarters(void)
{
    if (FlagGet(FLAG_RIVAL_GOT_BULBASAUR) &&
        FlagGet(FLAG_RIVAL_GOT_CHARMANDER) &&
        FlagGet(FLAG_RIVAL_GOT_SQUIRTLE))
    {
        FlagSet(FLAG_RIVAL_GOT_ALL_KANTO_MONS);
        return;
    }
}

// Debug: mark the last party Pokemon as a true shiny (for sparkle testing)
void Debug_MarkLastPartyMonTrueShiny(void)
{
    u8 count = CalculatePlayerPartyCount();
    u8 trueShiny = 1;

    if (count > 0)
        SetMonData(&gPlayerParty[count - 1], MON_DATA_TRUE_SHINY, &trueShiny);
}

void SetLastGiftMonStevenOT(void)
{
    u32 otId = Random32();
    u8 gender = MALE;

    if (gSpecialVar_Result == MON_GIVEN_TO_PARTY)
    {
        u8 count = CalculatePlayerPartyCount();

        if (count > 0)
        {
            SetMonData(&gPlayerParty[count - 1], MON_DATA_OT_NAME, gText_Steven);
            SetMonData(&gPlayerParty[count - 1], MON_DATA_OT_GENDER, &gender);
            SetMonData(&gPlayerParty[count - 1], MON_DATA_OT_ID, &otId);
        }
    }
    else if (gSpecialVar_Result == MON_GIVEN_TO_PC)
    {
        struct BoxPokemon *boxMon = GetBoxedMonPtr(gSpecialVar_MonBoxId, gSpecialVar_MonBoxPos);

        SetBoxMonData(boxMon, MON_DATA_OT_NAME, gText_Steven);
        SetBoxMonData(boxMon, MON_DATA_OT_GENDER, &gender);
        SetBoxMonData(boxMon, MON_DATA_OT_ID, &otId);
    }
}

static bool8 IsValidNameByte(u8 byte)
{
    // Space
    if (byte == CHAR_SPACE)
        return TRUE;
    // Accented Latin block (European names): 0x01-0x2E
    if (byte >= 0x01 && byte <= 0x2E)
        return TRUE;
    // Scattered accented chars outside the main block
    if (byte == CHAR_I_ACUTE || byte == CHAR_a_CIRCUMFLEX || byte == CHAR_i_ACUTE)
        return TRUE;
    // Digits, punctuation, letters, diacriticals: 0xA1-0xF6
    if (byte >= CHAR_0 && byte <= CHAR_u_DIAERESIS)
        return TRUE;
    // Everything else (gaps, display-only chars like PK/MN,
    // inverted punctuation, arrows, control codes) is invalid
    return FALSE;
}

// Known player names for fuzzy repair of slightly-corrupted records
static const u8 sKnownName_PURPLE[] = {CHAR_P, CHAR_U, CHAR_R, CHAR_P, CHAR_L, CHAR_E, EOS};
static const u8 sKnownName_AILES[] = {CHAR_A, CHAR_I, CHAR_L, CHAR_E, CHAR_S, EOS};
static const u8 sKnownName_Alhya[] = {CHAR_A, CHAR_l, CHAR_h, CHAR_y, CHAR_a, EOS};
static const u8 sKnownName_Gabs[] = {CHAR_G, CHAR_a, CHAR_b, CHAR_s, EOS};
static const u8 sKnownName_Niji[] = {CHAR_N, CHAR_i, CHAR_j, CHAR_i, EOS};

static const u8 *const sKnownNames[] = {
    sKnownName_PURPLE,
    sKnownName_AILES,
    sKnownName_Alhya,
    sKnownName_Gabs,
    sKnownName_Niji,
};

// Try to fix a name where only the first 1-2 bytes are corrupted
// but the rest matches a known name. Returns TRUE if fixed.
static bool8 TryFuzzyNameRepair(u8 *name, u8 maxLen)
{
    u8 k;
    u8 startPos;

    for (k = 0; k < ARRAY_COUNT(sKnownNames); k++)
    {
        const u8 *known = sKnownNames[k];
        u8 knownLen = StringLength(known);

        // Try matching from position 1 (first byte corrupted)
        for (startPos = 1; startPos <= 2 && startPos < knownLen; startPos++)
        {
            u8 m;
            bool8 match = TRUE;

            for (m = startPos; m < knownLen; m++)
            {
                if (m >= maxLen || name[m] == EOS)
                {
                    match = FALSE;
                    break;
                }
                if (name[m] != known[m])
                {
                    match = FALSE;
                    break;
                }
            }
            if (match && (m >= maxLen || name[m] == EOS || m == knownLen))
            {
                // Fix the corrupted leading bytes
                u8 n;
                for (n = 0; n < startPos; n++)
                    name[n] = known[n];
                return TRUE;
            }
        }
    }
    return FALSE;
}

static bool8 IsTrainerNameInvalid(const u8 *name, u8 maxLen)
{
    u8 i;
    bool8 hasContent = FALSE;

    // Empty name (starts with EOS)
    if (name[0] == EOS)
        return TRUE;

    for (i = 0; i < maxLen; i++)
    {
        if (name[i] == EOS)
            break;
        // Check against whitelist of valid name characters
        if (!IsValidNameByte(name[i]))
            return TRUE;
        // Any non-space character counts as content
        if (name[i] != CHAR_SPACE)
            hasContent = TRUE;
    }

    // Name was all spaces
    if (!hasContent)
        return TRUE;

    return FALSE;
}

static bool8 IsSecretBasePartyInvalid(const struct SecretBaseParty *party)
{
    u8 i;

    // Lead Pokemon must exist
    if (party->species[0] == SPECIES_NONE || party->species[0] >= NUM_SPECIES)
        return TRUE;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (party->species[i] == SPECIES_NONE)
            continue;
        // Invalid species
        if (party->species[i] >= NUM_SPECIES)
            return TRUE;
        // Level must be 1-100
        if (party->levels[i] == 0 || party->levels[i] > MAX_LEVEL)
            return TRUE;
    }

    // Check all moves across all party slots
    for (i = 0; i < PARTY_SIZE * MAX_MON_MOVES; i++)
    {
        if (party->moves[i] != MOVE_NONE && party->moves[i] >= MOVES_COUNT)
            return TRUE;
    }

    return FALSE;
}

void CleanInvalidTrainerRecords(void)
{
    u8 i;
    u8 j;
    u8 dst;
    u8 count = 0;
    u8 linkCount = 0;
    u8 nameCount = 0;
    u8 baseCount = 0;

    if (sTrainerRecordsCleaned)
        return;
    sTrainerRecordsCleaned = TRUE;

    // Clear spurious Hall of Fame stats if player hasn't actually beaten the game
    if (!FlagGet(FLAG_SYS_GAME_CLEAR))
    {
        if (GetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME) != 0)
            SetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME, 0);
        if (GetGameStat(GAME_STAT_ENTERED_HOF) != 0)
            SetGameStat(GAME_STAT_ENTERED_HOF, 0);
    }

    // Clean link battle records with invalid trainer names
    for (i = 0; i < LINK_B_RECORDS_COUNT; i++)
    {
        if (gSaveBlock1Ptr->linkBattleRecords.entries[i].trainerId == 0
            && gSaveBlock1Ptr->linkBattleRecords.entries[i].wins == 0
            && gSaveBlock1Ptr->linkBattleRecords.entries[i].losses == 0
            && gSaveBlock1Ptr->linkBattleRecords.entries[i].draws == 0)
            continue; // Truly empty slot
        // Try fuzzy name repair first (fixes 1-2 corrupted leading bytes)
        TryFuzzyNameRepair(gSaveBlock1Ptr->linkBattleRecords.entries[i].name, PLAYER_NAME_LENGTH + 1);

        if (IsTrainerNameInvalid(gSaveBlock1Ptr->linkBattleRecords.entries[i].name, PLAYER_NAME_LENGTH + 1))
        {
            // Try to repair: find another record with same trainer ID and valid name
            for (j = 0; j < LINK_B_RECORDS_COUNT; j++)
            {
                if (j == i)
                    continue;
                if (gSaveBlock1Ptr->linkBattleRecords.entries[j].trainerId
                    == gSaveBlock1Ptr->linkBattleRecords.entries[i].trainerId
                    && !IsTrainerNameInvalid(gSaveBlock1Ptr->linkBattleRecords.entries[j].name, PLAYER_NAME_LENGTH + 1))
                {
                    // Merge W/L/D into the valid record, then wipe the corrupted one
                    gSaveBlock1Ptr->linkBattleRecords.entries[j].wins +=
                        gSaveBlock1Ptr->linkBattleRecords.entries[i].wins;
                    gSaveBlock1Ptr->linkBattleRecords.entries[j].losses +=
                        gSaveBlock1Ptr->linkBattleRecords.entries[i].losses;
                    gSaveBlock1Ptr->linkBattleRecords.entries[j].draws +=
                        gSaveBlock1Ptr->linkBattleRecords.entries[i].draws;
                    memset(&gSaveBlock1Ptr->linkBattleRecords.entries[i], 0, sizeof(struct LinkBattleRecord));
                    gSaveBlock1Ptr->linkBattleRecords.languages[i] = 0;
                    break;
                }
            }
            // If no match found, wipe the record
            if (j == LINK_B_RECORDS_COUNT)
            {
                memset(&gSaveBlock1Ptr->linkBattleRecords.entries[i], 0, sizeof(struct LinkBattleRecord));
                gSaveBlock1Ptr->linkBattleRecords.languages[i] = 0;
                linkCount++;
                count++;
            }
        }
        else
        {
            // Name is valid (possibly after fuzzy repair) — merge with duplicate if exists
            for (j = 0; j < i; j++)
            {
                if (gSaveBlock1Ptr->linkBattleRecords.entries[j].trainerId
                    == gSaveBlock1Ptr->linkBattleRecords.entries[i].trainerId
                    && !IsTrainerNameInvalid(gSaveBlock1Ptr->linkBattleRecords.entries[j].name, PLAYER_NAME_LENGTH + 1))
                {
                    gSaveBlock1Ptr->linkBattleRecords.entries[j].wins +=
                        gSaveBlock1Ptr->linkBattleRecords.entries[i].wins;
                    gSaveBlock1Ptr->linkBattleRecords.entries[j].losses +=
                        gSaveBlock1Ptr->linkBattleRecords.entries[i].losses;
                    gSaveBlock1Ptr->linkBattleRecords.entries[j].draws +=
                        gSaveBlock1Ptr->linkBattleRecords.entries[i].draws;
                    memset(&gSaveBlock1Ptr->linkBattleRecords.entries[i], 0, sizeof(struct LinkBattleRecord));
                    gSaveBlock1Ptr->linkBattleRecords.languages[i] = 0;
                    break;
                }
            }
        }
    }

    // Compact link battle records — shift non-empty entries to front
    dst = 0;
    for (i = 0; i < LINK_B_RECORDS_COUNT; i++)
    {
        if (gSaveBlock1Ptr->linkBattleRecords.entries[i].trainerId == 0
            && gSaveBlock1Ptr->linkBattleRecords.entries[i].wins == 0
            && gSaveBlock1Ptr->linkBattleRecords.entries[i].losses == 0
            && gSaveBlock1Ptr->linkBattleRecords.entries[i].draws == 0)
            continue;
        if (dst != i)
        {
            gSaveBlock1Ptr->linkBattleRecords.entries[dst] = gSaveBlock1Ptr->linkBattleRecords.entries[i];
            gSaveBlock1Ptr->linkBattleRecords.languages[dst] = gSaveBlock1Ptr->linkBattleRecords.languages[i];
            memset(&gSaveBlock1Ptr->linkBattleRecords.entries[i], 0, sizeof(struct LinkBattleRecord));
            gSaveBlock1Ptr->linkBattleRecords.languages[i] = 0;
        }
        dst++;
    }

    // Clean trainer name records with invalid trainer names
    for (i = 0; i < 20; i++)
    {
        if (gSaveBlock1Ptr->trainerNameRecords[i].trainerId == 0)
            continue; // Empty slot
        TryFuzzyNameRepair(gSaveBlock1Ptr->trainerNameRecords[i].trainerName, PLAYER_NAME_LENGTH + 1);
        if (IsTrainerNameInvalid(gSaveBlock1Ptr->trainerNameRecords[i].trainerName, PLAYER_NAME_LENGTH + 1))
        {
            // Try to find a matching trainer ID with valid name (just wipe duplicate)
            for (j = 0; j < 20; j++)
            {
                if (j == i)
                    continue;
                if (gSaveBlock1Ptr->trainerNameRecords[j].trainerId
                    == gSaveBlock1Ptr->trainerNameRecords[i].trainerId
                    && !IsTrainerNameInvalid(gSaveBlock1Ptr->trainerNameRecords[j].trainerName, PLAYER_NAME_LENGTH + 1))
                {
                    // Valid duplicate exists, just wipe the corrupted one
                    memset(&gSaveBlock1Ptr->trainerNameRecords[i], 0, sizeof(struct TrainerNameRecord));
                    break;
                }
            }
            if (j == 20)
            {
                // No match found, wipe the record
                memset(&gSaveBlock1Ptr->trainerNameRecords[i], 0, sizeof(struct TrainerNameRecord));
                nameCount++;
                count++;
            }
        }
    }

    // Clean secret base records with invalid names or party species (skip index 0 = player's base)
    for (i = 1; i < SECRET_BASES_COUNT; i++)
    {
        if (gSaveBlock1Ptr->secretBases[i].secretBaseId == 0)
            continue; // Empty slot
        TryFuzzyNameRepair(gSaveBlock1Ptr->secretBases[i].trainerName, PLAYER_NAME_LENGTH);
        if (IsTrainerNameInvalid(gSaveBlock1Ptr->secretBases[i].trainerName, PLAYER_NAME_LENGTH)
            || IsSecretBasePartyInvalid(&gSaveBlock1Ptr->secretBases[i].party))
        {
            memset(&gSaveBlock1Ptr->secretBases[i], 0, sizeof(struct SecretBase));
            baseCount++;
            count++;
        }
    }

    sInvalidRecordsCleaned = count;
    sInvalidLinkBattleRecords = linkCount;
    sInvalidTrainerNameRecords = nameCount;
    sInvalidSecretBaseRecords = baseCount;
}

bool8 TrySetupInvalidRecordCleanupMessage(void)
{
    u8 count;

    count = sInvalidRecordsCleaned;
    if (count == 0)
        return FALSE;

    sInvalidRecordsCleaned = 0;
    ConvertIntToDecimalStringN(gStringVar1, sInvalidLinkBattleRecords, STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, sInvalidTrainerNameRecords, STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar3, sInvalidSecretBaseRecords, STR_CONV_MODE_LEFT_ALIGN, 2);
    sInvalidLinkBattleRecords = 0;
    sInvalidTrainerNameRecords = 0;
    sInvalidSecretBaseRecords = 0;
    return TRUE;
}

// Veritas: special-case birthday gift for the OT name "PURPLE" / "Purple" /
// "purple". Fires once per save (gated by FLAG_RECEIVED_PURPLE_BIRTHDAY_GIFT) on the first
// frame that ProcessPlayerFieldInput is reached after load. Sets the flag
// before returning TRUE so the giveitem script can run safely.
bool8 TrySetupPurpleBirthdayGift(void)
{
    static const u8 sPurpleUpper[] = _("PURPLE");
    static const u8 sPurpleTitle[] = _("Purple");
    static const u8 sPurpleLower[] = _("purple");

    if (FlagGet(FLAG_RECEIVED_PURPLE_BIRTHDAY_GIFT))
        return FALSE;

    if (StringCompare(gSaveBlock2Ptr->playerName, sPurpleUpper) != 0
     && StringCompare(gSaveBlock2Ptr->playerName, sPurpleTitle) != 0
     && StringCompare(gSaveBlock2Ptr->playerName, sPurpleLower) != 0)
        return FALSE;

    FlagSet(FLAG_RECEIVED_PURPLE_BIRTHDAY_GIFT);
    return TRUE;
}

// Veritas: apology gift for the OT name "Alhya" / "ALHYA" / "ALHYIA" / "ALHIA"
// to compensate for the Battle Frontier Lv50 cap bug that overwrote levels.
// Fires once per save (gated by FLAG_RECEIVED_ALHYA_APOLOGY) on the first frame that
// ProcessPlayerFieldInput is reached after load. Sets the flag before
// returning TRUE so the giveitem script can run safely.
bool8 TrySetupAlhyaApology(void)
{
    static const u8 sAlhya[]  = _("Alhya");
    static const u8 sAlhyaU[] = _("ALHYA");
    static const u8 sAlhyia[] = _("ALHYIA");
    static const u8 sAlhia[]  = _("ALHIA");

    if (FlagGet(FLAG_RECEIVED_ALHYA_APOLOGY))
        return FALSE;

    if (StringCompare(gSaveBlock2Ptr->playerName, sAlhya)  != 0
     && StringCompare(gSaveBlock2Ptr->playerName, sAlhyaU) != 0
     && StringCompare(gSaveBlock2Ptr->playerName, sAlhyia) != 0
     && StringCompare(gSaveBlock2Ptr->playerName, sAlhia)  != 0)
        return FALSE;

    FlagSet(FLAG_RECEIVED_ALHYA_APOLOGY);
    return TRUE;
}

// Veritas: shiny check for the Battle Frontier "Cookie" Zigzagoon trade.
// The Beauty NPC accepts ANY shiny in trade rather than a fixed species,
// so the script asks this special after ChoosePartyMon and branches on
// the result. VAR_0x8004 holds the chosen party slot.
//
// MUST return its result: this codebase's ScrCmd_specialvar does
// `*var = gSpecials[...]()` (SpecialFunc is u16 (*)(void)) — it stores the
// RETURN VALUE, not gSpecialVar_Result. A void version left VAR_RESULT holding
// a garbage register value (usually non-zero), so the "not shiny" branch never
// fired and non-shiny mons passed the Cookie / Delibird trade gate.
u16 IsSelectedPartyMonShiny(void)
{
    struct Pokemon *mon;

    if (gSpecialVar_0x8004 >= PARTY_SIZE)
        return FALSE;

    // Reject eggs (a shiny egg reads shiny but looks like an egg) and empty
    // slots (which read OT id / personality 0, and GET_SHINY_VALUE(0, 0) == 0
    // would count as a false shiny). Only a real, hatched, visibly shiny mon
    // is a valid offer.
    mon = &gPlayerParty[gSpecialVar_0x8004];
    if (GetMonData(mon, MON_DATA_SPECIES, NULL) == SPECIES_NONE
     || GetMonData(mon, MON_DATA_IS_EGG, NULL))
        return FALSE;

    return IsMonShiny(mon);
}

// Veritas: builds gEnemyParty[0] as Cookie — the Beauty NPC's shiny
// Zigzagoon. Bypasses CreateInGameTradePokemon entirely so we can carry
// a custom moveset (Extremespeed / Guillotine / Thief / Yawn), force a
// shiny PID against the preset OT id, and skip the sIngameTrades table.
// The trade-scene UI (DoInGameTradeScene) reads from gEnemyParty[0] so
// the trade animation still plays normally after this runs.
//
// Cookie's level matches the player's offered shiny — same convention
// as vanilla in-game trades (see CreateInGameTradePokemonInternal in
// src/trade.c, which inherits the chosen mon's level).
//
// The forced ability (Baby Charm) is NOT stored on the Pokémon — see
// ResolveMonAbilityByName in src/pokemon.c, which detects Cookie by
// species + nickname + OT name at every ability-lookup site.
void CreateCookieZigzagoon(void)
{
    static const u8 sNickname[] = _("Cookie");
    static const u8 sOtName[] = _("Gabriel");
    static const u16 sMoves[MAX_MON_MOVES] = {
        MOVE_EXTREME_SPEED, MOVE_GUILLOTINE, MOVE_THIEF, MOVE_YAWN,
    };
    const u32 otId = 50135;
    u32 personality;
    u16 lo;
    u16 hi;
    u8 i;
    u8 level;
    u8 ivVal = MAX_PER_STAT_IVS;
    u8 abilityNum = 0;
    u8 otGender = MALE;
    u8 pp;
    u8 metLocation = METLOC_IN_GAME_TRADE;
    u16 heldItem = ITEM_LAVA_COOKIE;
    u16 move;

    // Force a shiny PID against the preset OT. Pick a random low half and
    // construct the high half so HIHALF ^ LOHALF ^ HIHALF_OT ^ LOHALF_OT
    // is < SHINY_ODDS (8) — the standard shiny-rebuild trick from
    // CreateBoxMon's FLAG_SHINY_CREATION path. Then loop until Cookie
    // is male (boyfriend's barking dog) AND has the Naughty nature.
    do
    {
        lo = Random();
        hi = ((u16)otId ^ (u16)(otId >> 16)) ^ lo ^ (u16)(Random() & (SHINY_ODDS - 1));
        personality = ((u32)hi << 16) | lo;
    } while ((personality & 0xFF) < 127
          || (personality % NUM_NATURES) != NATURE_NAUGHTY);

    level = (gSpecialVar_0x8005 < PARTY_SIZE)
          ? GetMonData(&gPlayerParty[gSpecialVar_0x8005], MON_DATA_LEVEL, NULL)
          : 20;

    CreateMon(&gEnemyParty[0], SPECIES_ZIGZAGOON, level, USE_RANDOM_IVS,
              TRUE, personality, OT_ID_PRESET, otId);

    // Max IVs across the board.
    SetMonData(&gEnemyParty[0], MON_DATA_HP_IV, &ivVal);
    SetMonData(&gEnemyParty[0], MON_DATA_ATK_IV, &ivVal);
    SetMonData(&gEnemyParty[0], MON_DATA_DEF_IV, &ivVal);
    SetMonData(&gEnemyParty[0], MON_DATA_SPEED_IV, &ivVal);
    SetMonData(&gEnemyParty[0], MON_DATA_SPATK_IV, &ivVal);
    SetMonData(&gEnemyParty[0], MON_DATA_SPDEF_IV, &ivVal);

    SetMonData(&gEnemyParty[0], MON_DATA_NICKNAME, sNickname);
    SetMonData(&gEnemyParty[0], MON_DATA_OT_NAME, sOtName);
    SetMonData(&gEnemyParty[0], MON_DATA_OT_GENDER, &otGender);
    SetMonData(&gEnemyParty[0], MON_DATA_ABILITY_NUM, &abilityNum);
    SetMonData(&gEnemyParty[0], MON_DATA_MET_LOCATION, &metLocation);
    SetMonData(&gEnemyParty[0], MON_DATA_HELD_ITEM, &heldItem);

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        move = sMoves[i];
        SetMonData(&gEnemyParty[0], MON_DATA_MOVE1 + i, &move);
        pp = gBattleMoves[move].pp;
        SetMonData(&gEnemyParty[0], MON_DATA_PP1 + i, &pp);
    }

    CalculateMonStats(&gEnemyParty[0]);
}

// Veritas: builds gEnemyParty[0] as the Gift Day present the rival trades over —
// SANTY, a shiny, perfect-IV, level 12 DELIBIRD that knows AEROBLAST (off its
// learnset, so the moves are written directly here, like CreateCookieZigzagoon).
// The trade scene (DoInGameTradeScene) reads gEnemyParty[0] and swaps it for the
// shiny the player offered. Its OT is SANTA, so it registers as a traded Pokémon.
void CreateGiftDelibird(void)
{
    static const u8 sNickname[] = _("Santy");
    static const u8 sOtName[] = _("SANTA");
    static const u16 sMoves[MAX_MON_MOVES] = {
        MOVE_AEROBLAST, MOVE_PRESENT, MOVE_POWDER_SNOW, MOVE_AURORA_BEAM,
    };
    const u32 otId = 25120; // festive preset OT id
    u32 personality;
    u16 lo;
    u16 hi;
    u8 i;
    u8 ivVal = MAX_PER_STAT_IVS;
    u8 metLocation = METLOC_IN_GAME_TRADE;
    u8 otGender = MALE; // SANTA
    u8 pp;
    u16 move;

    // Force a shiny PID against the preset OT — build the high half so
    // HIHALF ^ LOHALF ^ HIHALF_OT ^ LOHALF_OT < SHINY_ODDS (the standard
    // rebuild trick; no gender/nature constraint, so a single pass suffices).
    lo = Random();
    hi = ((u16)otId ^ (u16)(otId >> 16)) ^ lo ^ (u16)(Random() & (SHINY_ODDS - 1));
    personality = ((u32)hi << 16) | lo;

    CreateMon(&gEnemyParty[0], SPECIES_DELIBIRD, 12, USE_RANDOM_IVS,
              TRUE, personality, OT_ID_PRESET, otId);

    // Perfect IVs across the board.
    SetMonData(&gEnemyParty[0], MON_DATA_HP_IV, &ivVal);
    SetMonData(&gEnemyParty[0], MON_DATA_ATK_IV, &ivVal);
    SetMonData(&gEnemyParty[0], MON_DATA_DEF_IV, &ivVal);
    SetMonData(&gEnemyParty[0], MON_DATA_SPEED_IV, &ivVal);
    SetMonData(&gEnemyParty[0], MON_DATA_SPATK_IV, &ivVal);
    SetMonData(&gEnemyParty[0], MON_DATA_SPDEF_IV, &ivVal);

    SetMonData(&gEnemyParty[0], MON_DATA_NICKNAME, sNickname);
    SetMonData(&gEnemyParty[0], MON_DATA_OT_NAME, sOtName);
    SetMonData(&gEnemyParty[0], MON_DATA_OT_GENDER, &otGender);
    SetMonData(&gEnemyParty[0], MON_DATA_MET_LOCATION, &metLocation);

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        move = sMoves[i];
        SetMonData(&gEnemyParty[0], MON_DATA_MOVE1 + i, &move);
        pp = gBattleMoves[move].pp;
        SetMonData(&gEnemyParty[0], MON_DATA_PP1 + i, &pp);
    }

    CalculateMonStats(&gEnemyParty[0]);
}

// Veritas: capture the offered Pokémon's species into VAR_OBJ_GFX_ID_1
// (with the shiny overworld tag), so the floating sprite next to the
// Beauty trader becomes the shiny species the player just gave up.
// Must be called BEFORE the trade scene runs — the trade scene swaps
// gPlayerParty[gSpecialVar_0x8005] with Cookie, so the offered species
// is lost from gPlayerParty afterward. After the script re-spawns
// LOCALID_TRADEMON, the new sprite is the captured shiny species.
void SetCookieTradeOverworldToOfferedShiny(void)
{
    u16 species;
    if (gSpecialVar_0x8005 >= PARTY_SIZE)
        return;
    species = GetMonData(&gPlayerParty[gSpecialVar_0x8005], MON_DATA_SPECIES, NULL);
    if (species == SPECIES_NONE || species == SPECIES_EGG)
        return;
    VarSet(VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_MON_BASE + species + SPECIES_OVERWORLD_SHINY_TAG);
}

// Veritas: load the species currently set on VAR_OBJ_GFX_ID_1 (the
// floating-Pokémon sprite next to the Beauty trader) into gStringVar1
// and play its cry. Used by the post-trade _Mon handler so the floating
// sprite reacts to interaction regardless of which species the player
// traded. Strips both OBJ_EVENT_GFX_MON_BASE and SPECIES_OVERWORLD_SHINY_TAG
// before doing the species lookup.
void PrepareTradeFloatingMonGreeting(void)
{
    u16 gfx = VarGet(VAR_OBJ_GFX_ID_1);
    u16 species;

    if (gfx < OBJ_EVENT_GFX_MON_BASE)
        return;
    species = gfx - OBJ_EVENT_GFX_MON_BASE;
    if (species >= SPECIES_OVERWORLD_SHINY_TAG)
        species -= SPECIES_OVERWORLD_SHINY_TAG;
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return;
    StringCopy(gStringVar1, gSpeciesNames[species]);
    PlayCry_Normal(species, 0);
}

// Veritas: name-rater refusal check. Returns TRUE if the chosen party
// mon is Cookie (Gabriel's Zigzagoon) — in which case the Slateport
// Name Rater refuses to rename it. Checked AFTER the egg check and
// BEFORE the regular "fine name, want to change it?" prompt.
// MUST return its result (see IsSelectedPartyMonShiny) — invoked via specialvar,
// which stores the return value, not gSpecialVar_Result.
u16 IsSelectedMonGabrielsCookie(void)
{
    static const u8 sCookieName[] = _("Cookie");
    static const u8 sGabrielName[] = _("Gabriel");
    u8 nickname[POKEMON_NAME_BUFFER_SIZE];
    u8 otName[PLAYER_NAME_LENGTH + 1];
    u16 species;

    if (gSpecialVar_0x8004 >= PARTY_SIZE)
        return FALSE;

    species = GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_SPECIES, NULL);
    if (species != SPECIES_ZIGZAGOON)
        return FALSE;
    GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_NICKNAME, nickname);
    GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_OT_NAME, otName);
    return (StringCompare(nickname, sCookieName) == 0
         && StringCompare(otName, sGabrielName) == 0);
}

void CreateBossLugia(void)
{
    u16 move;
    u8 pp;

    ZeroEnemyPartyMons();
    CreateMon(&gEnemyParty[0], SPECIES_LUGIA, 99, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);

    /* Set custom moves */
    move = MOVE_THUNDER;
    SetMonData(&gEnemyParty[0], MON_DATA_MOVE1, &move);
    pp = gBattleMoves[MOVE_THUNDER].pp;
    SetMonData(&gEnemyParty[0], MON_DATA_PP1, &pp);

    move = MOVE_HYDRO_PUMP;
    SetMonData(&gEnemyParty[0], MON_DATA_MOVE2, &move);
    pp = gBattleMoves[MOVE_HYDRO_PUMP].pp;
    SetMonData(&gEnemyParty[0], MON_DATA_PP2, &pp);

    move = MOVE_CALM_MIND;
    SetMonData(&gEnemyParty[0], MON_DATA_MOVE3, &move);
    pp = gBattleMoves[MOVE_CALM_MIND].pp;
    SetMonData(&gEnemyParty[0], MON_DATA_PP3, &pp);

    move = MOVE_PSYCHO_BOOST;
    SetMonData(&gEnemyParty[0], MON_DATA_MOVE4, &move);
    pp = gBattleMoves[MOVE_PSYCHO_BOOST].pp;
    SetMonData(&gEnemyParty[0], MON_DATA_PP4, &pp);

    /* Set held item */
    move = ITEM_WHITE_HERB;
    SetMonData(&gEnemyParty[0], MON_DATA_HELD_ITEM, &move);
}

void SetVarToMultiplayerId(void)
{
    gSpecialVar_0x8005 = GetMultiplayerId();
}

// Route 117's garden wanderers (normally Zigzagoon / Azumarill) rotate by day:
// day & 3 == 0 -> both, 1 -> slot 0 only, 2 -> slot 1 only, 3 -> neither.
// 3 out of 4 days show one or none, easing object load on this dense map.
//
// The two objects use OBJ_EVENT_GFX_VAR_2 / VAR_3 (Route117 map.json), so their
// species is whatever VAR_OBJ_GFX_ID_2 / _3 hold when they spawn. On the fixed
// special calendar dates BOTH wanderers become a thematic pair; every day also
// carries a deterministic ~1% chance that one of the pair is MEW. On any of
// those days both are forced visible so the theme / Mew is actually seen;
// otherwise the pair stays Zigzagoon + Azumarill under the normal day rotation.
//
// Called from Route117_OnLoad — ON_LOAD runs on warps, connection crossings AND
// continue-save (ON_TRANSITION would miss continue and leave the state stale),
// and runs before the objects spawn, so both the hide flags and the gfx vars are
// fresh on every entry path. VAR_DAYS is current in all of them because
// DoTimeBasedEvents runs earlier in each load sequence.
void UpdateRoute117WandererFlags(void)
{
    u16 days = VarGet(VAR_DAYS);
    u16 daySel = days & 3;
    u16 s0 = SPECIES_ZIGZAGOON;
    u16 s1 = SPECIES_AZUMARILL;
    bool8 themed = TRUE;
    bool8 mewDay;
    u32 hash;

    // Thematic pair on the fixed special calendar dates.
    if (IsLuvdiscDayToday())        { s0 = SPECIES_LUVDISC; s1 = SPECIES_LUVDISC; }
    else if (IsSpookyDayToday())    { s0 = SPECIES_DUSKULL; s1 = SPECIES_SHUPPET; }
    else if (IsHoennDayToday())     { s0 = SPECIES_TORCHIC; s1 = SPECIES_MUDKIP;  }
    else if (IsTrickDayToday())     { s0 = SPECIES_DITTO;   s1 = SPECIES_KECLEON; }
    else if (IsGiftDayToday())      { s0 = SPECIES_SNORUNT; s1 = SPECIES_SPHEAL;  }
    else                              themed = FALSE;

    // ~1% per day that one wanderer is MEW. Hashed off the day count so it is
    // stable for the whole day and cannot be re-rolled by re-entering the map.
    // (days + 1 avoids the degenerate hash == 0 case making day 0 a guaranteed
    // Mew day.)
    hash = (u32)(days + 1) * 2654435761u;
    mewDay = ((hash >> 8) % 100u) == 0;
    if (mewDay)
    {
        if ((hash >> 3) & 1)
            s1 = SPECIES_MEW;
        else
            s0 = SPECIES_MEW;
    }

    VarSet(VAR_OBJ_GFX_ID_2, OBJ_EVENT_GFX_MON_BASE + s0);
    VarSet(VAR_OBJ_GFX_ID_3, OBJ_EVENT_GFX_MON_BASE + s1);

    // A themed pair or a Mew is worth always showing both; otherwise keep the
    // normal day rotation that thins this dense map.
    if (themed || mewDay)
    {
        FlagClear(FLAG_HIDE_ROUTE117_ZIGZAGOON);
        FlagClear(FLAG_HIDE_ROUTE117_AZUMARILL);
        return;
    }

    if (daySel == 0 || daySel == 1)
        FlagClear(FLAG_HIDE_ROUTE117_ZIGZAGOON);
    else
        FlagSet(FLAG_HIDE_ROUTE117_ZIGZAGOON);

    if (daySel == 0 || daySel == 2)
        FlagClear(FLAG_HIDE_ROUTE117_AZUMARILL);
    else
        FlagSet(FLAG_HIDE_ROUTE117_AZUMARILL);
}

// ============================================================
// Tournament prize codes
// ============================================================
// Redeemed at the Cable Club attendant (TOURNAMENT PRIZE menu option).
// Codes are generated offline with tools/make_prize_code.py.
//
// The code carries the whole prize — slot, item, amount, tournament and
// placement — so awarding a new item never needs a code change or a rebuild.
// The winner's visible trainer ID is NOT stored in the code: it keys both the
// payload whitening and the MAC, so a code only verifies on the save it was
// issued to. Each slot (1-15) redeems once per save (bitmask in
// VAR_TOURNEY_PRIZES_REDEEMED) — the slot is what makes a code single-use now
// that the item is arbitrary.
//
// Format: 11 characters from a 32-letter alphabet (5 bits each = 55 bits).
//   chars [0..4]  = 25-bit payload, XOR-whitened with a trainer-ID keystream:
//                     bits 0-1   placement  (0-3 = 1st/2nd/3rd/participation)
//                     bits 2-4   tournament (0-7, 0 = generic)
//                     bits 5-11  quantity   (1-99)
//                     bits 12-20 item id    (1..ITEMS_COUNT-1)
//                     bits 21-24 slot       (1-15)
//   chars [5..10] = 30-bit keyed MAC over the payload + trainer ID.
//
// The whitening hides what a code grants and makes the same prize look totally
// different per trainer; the MAC is what actually blocks forgery. This is
// client-side verification though — the salts ship inside the ROM, so a player
// who disassembles it can mint codes. It stops casual guessing and code
// sharing, not a determined reverse engineer.
//
// To add tournaments, extend sTournamentNames. To invalidate ALL outstanding
// codes, change the salts here and in the tool, then rebuild.

#define TOURNEY_CODE_LENGTH 11
#define TOURNEY_SLOT_COUNT 16 // slots 1-15; one bit each in VAR_TOURNEY_PRIZES_REDEEMED
#define TOURNEY_MAX_QUANTITY 99
#define TOURNEY_SALT_1 0x56455249 // "VERI" — must match tools/make_prize_code.py
#define TOURNEY_SALT_2 0x54415321 // "TAS!"
#define TOURNEY_SALT_3 0x504B4D4E // "PKMN" — payload whitening key

// Tournament IDs for payload bits 2-4. ID 0 = no specific tournament (generic
// congratulations). IDs 3-7 are open.
static const u8 sTourneyName_MagmaVsAqua[]    = _("MAGMA VS AQUA");
static const u8 sTourneyName_SouthernIsland[] = _("SOUTHERN ISLAND");
static const u8 *const sTournamentNames[8] =
{
    NULL,
    sTourneyName_MagmaVsAqua,
    sTourneyName_SouthernIsland,
};

// 32-character code alphabet (no I, O, 0 or 1 — too easy to misread).
// Must match ALPHABET in tools/make_prize_code.py.
static const u8 sTourneyCodeAlphabet[32] =
{
    CHAR_A, CHAR_B, CHAR_C, CHAR_D, CHAR_E, CHAR_F, CHAR_G, CHAR_H,
    CHAR_J, CHAR_K, CHAR_L, CHAR_M, CHAR_N, CHAR_P, CHAR_Q, CHAR_R,
    CHAR_S, CHAR_T, CHAR_U, CHAR_V, CHAR_W, CHAR_X, CHAR_Y, CHAR_Z,
    CHAR_2, CHAR_3, CHAR_4, CHAR_5, CHAR_6, CHAR_7, CHAR_8, CHAR_9,
};

enum
{
    TOURNEY_CODE_INVALID,      // bad code / unknown prize
    TOURNEY_CODE_OK,           // prize granted (item in 0x8004, qty in 0x8005)
    TOURNEY_CODE_ALREADY_USED, // this prize ID was already redeemed
    TOURNEY_CODE_NO_BAG_SPACE, // valid, but no room in the bag
    TOURNEY_CODE_CANCELLED,    // player backed out of the naming screen
};

static u32 TourneyMix(u32 x)
{
    x ^= x >> 16;
    x *= 0x7FEB352D;
    x ^= x >> 15;
    x *= 0x846CA68B;
    x ^= x >> 16;
    return x;
}

// Returns 0-31, or -1 if the character isn't in the code alphabet.
// Lowercase input is accepted and treated as uppercase.
static s32 TourneyCharToIndex(u8 c)
{
    s32 i;

    if (c >= CHAR_a && c <= CHAR_z)
        c -= CHAR_a - CHAR_A;
    for (i = 0; i < 32; i++)
    {
        if (sTourneyCodeAlphabet[i] == c)
            return i;
    }
    return -1;
}

static u16 RedeemTournamentCode(const u8 *code)
{
    s32 idx[TOURNEY_CODE_LENGTH];
    u32 h1, mac, enc, payload, mask;
    u32 slot, tournamentId, placement;
    u16 tid, redeemed, itemId, quantity;
    s32 i;

    if (code[0] == EOS)
        return TOURNEY_CODE_CANCELLED;

    for (i = 0; i < TOURNEY_CODE_LENGTH; i++)
    {
        idx[i] = TourneyCharToIndex(code[i]);
        if (idx[i] < 0)
            return TOURNEY_CODE_INVALID;
    }
    if (code[TOURNEY_CODE_LENGTH] != EOS)
        return TOURNEY_CODE_INVALID;

    // Visible trainer ID — the 5-digit number on the Trainer Card. Never stored
    // in the code; it keys both the whitening and the MAC, which is what binds
    // a code to this save.
    tid = gSaveBlock2Ptr->playerTrainerId[0] | (gSaveBlock2Ptr->playerTrainerId[1] << 8);

    enc = 0;
    for (i = 0; i < 5; i++)
        enc |= (u32)idx[i] << (5 * i);
    mac = 0;
    for (i = 0; i < 6; i++)
        mac |= (u32)idx[5 + i] << (5 * i);

    mask = TourneyMix(TOURNEY_SALT_3 ^ ((u32)tid * 0x2545F491)) & 0x01FFFFFF;
    payload = enc ^ mask;

    h1 = TourneyMix(TourneyMix(TOURNEY_SALT_1 ^ payload) ^ ((u32)tid << 8) ^ TOURNEY_SALT_2);
    if (mac != (h1 & 0x3FFFFFFF))
        return TOURNEY_CODE_INVALID;

    placement    = payload & 3;
    tournamentId = (payload >> 2) & 7;
    quantity     = (payload >> 5) & 0x7F;
    itemId       = (payload >> 12) & 0x1FF;
    slot         = (payload >> 21) & 0xF;

    if (slot == 0 || slot >= TOURNEY_SLOT_COUNT)
        return TOURNEY_CODE_INVALID;
    if (itemId == ITEM_NONE || itemId >= ITEMS_COUNT)
        return TOURNEY_CODE_INVALID;
    if (quantity == 0 || quantity > TOURNEY_MAX_QUANTITY)
        return TOURNEY_CODE_INVALID;
    if (tournamentId != 0 && sTournamentNames[tournamentId] == NULL)
        return TOURNEY_CODE_INVALID;

    redeemed = VarGet(VAR_TOURNEY_PRIZES_REDEEMED);
    if (redeemed & (1 << slot))
        return TOURNEY_CODE_ALREADY_USED;

    if (!CheckBagHasSpace(itemId, quantity))
        return TOURNEY_CODE_NO_BAG_SPACE;

    VarSet(VAR_TOURNEY_PRIZES_REDEEMED, redeemed | (1 << slot));
    gSpecialVar_0x8004 = itemId;
    gSpecialVar_0x8005 = quantity;
    // 0 = no tournament (generic congratulations); 1+ = the clerk names the
    // tournament (STR_VAR_1) and 0x8007 selects the dialog:
    // 1/2/3 = first/second/third place, 4 = took part.
    gSpecialVar_0x8006 = tournamentId;
    gSpecialVar_0x8007 = placement + 1;
    if (tournamentId != 0)
        StringCopy(gStringVar1, sTournamentNames[tournamentId]);
    return TOURNEY_CODE_OK;
}

static void CB2_HandleTournamentCode(void)
{
    gSpecialVar_Result = RedeemTournamentCode(gStringVar2);
    gFieldCallback = FieldCB_ContinueScriptHandleMusic;
    SetMainCallback2(CB2_ReturnToField);
}

// Opens the Walda-style naming screen for prize code entry. The waiting
// script resumes with VAR_RESULT set to a TOURNEY_CODE_* value.
void TournamentPrize_DoCodeEntry(void)
{
    gStringVar2[0] = EOS;
    DoNamingScreen(NAMING_SCREEN_WALDA, gStringVar2, 0, 0, 0, CB2_HandleTournamentCode);
}

// ============================================================
// Virtual calendar + player birthdays
// ============================================================
// The RTC in this hack tracks a day COUNTER, not a calendar, so the
// calendar date is player-declared: the date picker at the end of the
// wall-clock set flow (press START while checking the room clock) stores
// (month << 8 | day) in VAR_CALENDAR_BASE plus the local day counter at
// that moment in VAR_CALENDAR_BASE_DAYCOUNT. The current date is the base
// advanced by elapsed days, cycling a fixed 365-day year (Feb = 28; the
// year itself is deliberately not tracked — only the date matters).

static const u8 sCalendarMonthLengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Player birthdays, matched case-insensitively against the save's player
// name (same idea as the Purple/Alhya name gifts). Dates are day/month.
static const u8 sBdayName_Niji[]   = _("NIJI");   // 14/09
static const u8 sBdayName_Ailes[]  = _("AILES");  // 14/08
static const u8 sBdayName_Gabs[]   = _("GABS");   // 16/10
static const u8 sBdayName_Purple[] = _("PURPLE"); // 14/05
static const u8 sBdayName_Alhya[]  = _("ALHYA");  // 02/10 (accepted misspellings below)
static const u8 sBdayName_Alhyha[] = _("ALHYHA");
static const u8 sBdayName_Alhyia[] = _("ALHYIA");
static const u8 sBdayName_Alhia[]  = _("ALHIA");
static const u8 sBdayName_Luna[]   = _("LUNA");   // 27/07
static const u8 sBdayName_Mapu[]   = _("MAPU");   // 22/08
static const u8 sBdayName_Ania[]   = _("ANIA");   // 27/07
static const u8 sBdayName_Lucy[]   = _("LUCY");   // 14/03

static const struct
{
    const u8 *name;
    u8 month;
    u8 day;
} sPlayerBirthdays[] =
{
    {sBdayName_Niji,    9, 14},
    {sBdayName_Ailes,   8, 14},
    {sBdayName_Gabs,   10, 16},
    {sBdayName_Purple,  5, 14},
    {sBdayName_Alhya,  10,  2},
    {sBdayName_Alhyha, 10,  2},
    {sBdayName_Alhyia, 10,  2},
    {sBdayName_Alhia,  10,  2},
    {sBdayName_Luna,    7, 27},
    {sBdayName_Mapu,    8, 22},
    {sBdayName_Ania,    7, 27},
    {sBdayName_Lucy,    3, 14},
};

// Case-insensitive player-name compare in the game charset.
static bool8 BirthdayNameMatches(const u8 *a, const u8 *b)
{
    s32 i;

    for (i = 0; i <= PLAYER_NAME_LENGTH; i++)
    {
        u8 ca = a[i];
        u8 cb = b[i];

        if (ca >= CHAR_a && ca <= CHAR_z)
            ca -= CHAR_a - CHAR_A;
        if (cb >= CHAR_a && cb <= CHAR_z)
            cb -= CHAR_a - CHAR_A;
        if (ca != cb)
            return FALSE;
        if (ca == EOS)
            return TRUE;
    }
    return TRUE;
}

bool8 IsCalendarDateSet(void)
{
    return VarGet(VAR_CALENDAR_BASE) != 0;
}

// Current virtual month/day (1-based). month/day are 0 if no date is set.
void GetVirtualDate(u8 *month, u8 *day)
{
    u16 base = VarGet(VAR_CALENDAR_BASE);
    s32 m, d, elapsed;

    if (base == 0)
    {
        *month = 0;
        *day = 0;
        return;
    }
    m = ((base >> 8) & 0xFF) - 1; // 0-11
    d = (base & 0xFF) - 1;        // 0-based day within month
    RtcCalcLocalTime();
    elapsed = gLocalTime.days - (s16)VarGet(VAR_CALENDAR_BASE_DAYCOUNT);
    if (elapsed < 0)
        elapsed = 0;
    d += elapsed % 365;
    while (d >= sCalendarMonthLengths[m])
    {
        d -= sCalendarMonthLengths[m];
        m = (m + 1) % 12;
    }
    *month = m + 1;
    *day = d + 1;
}

// Returns TRUE and writes out the player's birthday if one is known:
// hardcoded names take priority, then the Fan-Club-registered custom
// birthday (VAR_PLAYER_BIRTHDAY, (month << 8) | day, 0 = unset).
static bool8 GetPlayerBirthdayDate(u8 *bMonth, u8 *bDay)
{
    u32 i;
    u16 custom;

    for (i = 0; i < ARRAY_COUNT(sPlayerBirthdays); i++)
    {
        if (BirthdayNameMatches(gSaveBlock2Ptr->playerName, sPlayerBirthdays[i].name))
        {
            *bMonth = sPlayerBirthdays[i].month;
            *bDay = sPlayerBirthdays[i].day;
            return TRUE;
        }
    }
    custom = VarGet(VAR_PLAYER_BIRTHDAY);
    if (custom != 0)
    {
        *bMonth = (custom >> 8) & 0xFF;
        *bDay = custom & 0xFF;
        return TRUE;
    }
    return FALSE;
}

// TRUE on the player's declared birthday (year ignored). Drives the
// Pokémon Center nurses' greeting, Mom's gift, doubled EXP/EVs, and
// bonus shiny rolls. Also exposed as a script special.
//
// Anti-abuse latch: a birthday arms at most once every 364 real days of
// the RTC day counter, so re-dialing the calendar date can't repeat the
// perks. VAR_LAST_BIRTHDAY_DAYCOUNT stores (day counter + 1) of the day
// it last armed; 0 = never armed, so a fresh save can always trigger.
// It stays TRUE for the whole day it armed on, and re-arms if the day
// counter ever goes backward (title-screen RTC reset).
u16 IsPlayerBirthdayToday(void)
{
    u8 month, day, bMonth, bDay;
    u16 stored, d;

    if (!IsCalendarDateSet())
        return FALSE;
    if (!GetPlayerBirthdayDate(&bMonth, &bDay))
        return FALSE;
    GetVirtualDate(&month, &day); // also refreshes gLocalTime
    if (month != bMonth || day != bDay)
        return FALSE;

    d = (u16)gLocalTime.days;
    stored = VarGet(VAR_LAST_BIRTHDAY_DAYCOUNT);
    if (stored != 0 && (u16)(d + 1) == stored)
        return TRUE; // the day it armed on — active all day
    if (stored != 0 && (u16)(d + 1) < stored)
    {
        // Day counter went backward (RTC reset) — re-arm cleanly.
        VarSet(VAR_LAST_BIRTHDAY_DAYCOUNT, 0);
        stored = 0;
    }
    if (stored == 0 || d >= (u16)(stored - 1 + 364))
    {
        VarSet(VAR_LAST_BIRTHDAY_DAYCOUNT, d + 1);
        return TRUE;
    }
    return FALSE;
}

// Shown once per boot after this update, until the player sets a date:
// existing saves have a time but no calendar date. New games set the date
// as part of the intro wall-clock flow, so they're gated out via
// FLAG_SET_WALL_CLOCK.
static EWRAM_DATA bool8 sShownDateReminder = FALSE;

bool8 TrySetupDateNotSetReminder(void)
{
    if (sShownDateReminder || IsCalendarDateSet())
        return FALSE;
    if (!FlagGet(FLAG_SET_WALL_CLOCK))
        return FALSE;
    sShownDateReminder = TRUE;
    return TRUE;
}

// ============================================================
// Save anniversary (the save file's own birthday)
// ============================================================
// Packed into VAR_SAVE_ANNIVERSARY as (yearsCelebrated << 9) | (month << 5)
// | day; 0 = untracked. Only saves created after this feature shipped are
// tracked — RecordSaveAnniversaryIfEligible runs from the intro clock flow
// (gated by FLAG_SAVE_ANNIV_ELIGIBLE, set only at new game), so existing
// saves stay 0 and never celebrate. yearsCelebrated doubles as the year
// number, incremented once per real anniversary via the same 364-day latch
// the player birthday uses (VAR_SAVE_ANNIV_ARMED) — re-dialing the calendar
// can't farm it, and it persists for the whole day it arms on.

#define SAVE_ANNIV_DAY(v)   ((v) & 0x1F)
#define SAVE_ANNIV_MONTH(v) (((v) >> 5) & 0xF)
#define SAVE_ANNIV_YEARS(v) (((v) >> 9) & 0x7F)
#define SAVE_ANNIV_PACK(years, month, day) (((u16)(years) << 9) | ((u16)(month) << 5) | (u8)(day))

// Called from the intro wall-clock date picker. Records this save's birth
// date + day counter the first time a NEW game sets its date.
void RecordSaveAnniversaryIfEligible(void)
{
    u8 month, day;

    if (!FlagGet(FLAG_SAVE_ANNIV_ELIGIBLE))
        return;
    if (VarGet(VAR_SAVE_ANNIVERSARY) != 0)
        return;
    GetVirtualDate(&month, &day); // refreshes gLocalTime from the just-set date
    if (month == 0)
        return;
    VarSet(VAR_SAVE_ANNIVERSARY, SAVE_ANNIV_PACK(0, month, day));
    VarSet(VAR_SAVE_ANNIV_ARMED, (u16)gLocalTime.days + 1); // creation day counts as armed
    FlagClear(FLAG_SAVE_ANNIV_ELIGIBLE);
}

// Returns the anniversary year number (>= 1) if today is a fresh save
// anniversary, else 0. Arms the once-per-364-days latch and advances the
// year counter on the first call of the day. Also a script special.
u16 IsSaveAnniversaryToday(void)
{
    u16 packed = VarGet(VAR_SAVE_ANNIVERSARY);
    u8 month, day;
    u16 celebrated, stored, d;

    if (packed == 0 || !IsCalendarDateSet())
        return 0;
    GetVirtualDate(&month, &day); // refreshes gLocalTime
    if (month != SAVE_ANNIV_MONTH(packed) || day != SAVE_ANNIV_DAY(packed))
        return 0;

    celebrated = SAVE_ANNIV_YEARS(packed);
    d = (u16)gLocalTime.days;
    stored = VarGet(VAR_SAVE_ANNIV_ARMED);
    if (stored != 0 && (u16)(d + 1) == stored)
        return celebrated; // armed today (0 on the creation day itself)
    if (stored != 0 && (u16)(d + 1) < stored)
    {
        VarSet(VAR_SAVE_ANNIV_ARMED, 0); // day counter went backward — re-arm
        stored = 0;
    }
    if (stored == 0 || d >= (u16)(stored - 1 + 364))
    {
        if (celebrated < 127)
            celebrated++;
        VarSet(VAR_SAVE_ANNIVERSARY, SAVE_ANNIV_PACK(celebrated, SAVE_ANNIV_MONTH(packed), SAVE_ANNIV_DAY(packed)));
        VarSet(VAR_SAVE_ANNIV_ARMED, d + 1);
        return celebrated;
    }
    return 0;
}

// If today is a hardcoded friend's birthday and the player carries a
// Pokémon originally caught by that friend (OT name match), buffer the
// mon's stored OT name into STR_VAR_1 for the nurse's reminder and
// return TRUE. Entries matching the player's own name are skipped —
// their own greeting handles those. No latch: it's only dialogue.
u16 BufferPartyBirthdayTrainerName(void)
{
    u8 month, day;
    u8 otName[PLAYER_NAME_LENGTH + 1];
    u32 i, j;

    if (!IsCalendarDateSet())
        return FALSE;
    GetVirtualDate(&month, &day);
    for (i = 0; i < ARRAY_COUNT(sPlayerBirthdays); i++)
    {
        if (sPlayerBirthdays[i].month != month || sPlayerBirthdays[i].day != day)
            continue;
        if (BirthdayNameMatches(gSaveBlock2Ptr->playerName, sPlayerBirthdays[i].name))
            continue;
        for (j = 0; j < gPlayerPartyCount; j++)
        {
            if (GetMonData(&gPlayerParty[j], MON_DATA_IS_EGG, NULL))
                continue;
            GetMonData(&gPlayerParty[j], MON_DATA_OT_NAME, otName);
            if (BirthdayNameMatches(otName, sPlayerBirthdays[i].name))
            {
                StringCopy(gStringVar1, otName);
                return TRUE;
            }
        }
    }
    return FALSE;
}

// ============================================================
// Fan Club birthday registration (custom birthdays)
// ============================================================
// A Lilycove Fan Club member records a birthday for players whose save
// name isn't in the hardcoded list. Stored in VAR_PLAYER_BIRTHDAY as
// (month << 8) | day (0 = unset) and honored by IsPlayerBirthdayToday()
// with the same once-per-364-days latch. UI: a single-number spinner
// window in the top-left corner (clear of the bottom message box) —
// Up/Down changes the number, A confirms the month then the day, B goes
// back / cancels.

#define BDAY_SPINNER_PHASE_MONTH 0
#define BDAY_SPINNER_PHASE_DAY   1

static const struct WindowTemplate sBirthdaySpinnerWinTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 4,
    .height = 2,
    .paletteNum = 15,
    .baseBlock = 8,
};

#define tPhase    data[0]
#define tMonth    data[1]
#define tDay      data[2]
#define tWindowId data[3]

static void PrintBirthdaySpinnerValue(u8 windowId, s32 value)
{
    u8 text[4];
    s32 xPos;

    ConvertIntToDecimalStringN(text, value, STR_CONV_MODE_LEADING_ZEROS, 2);
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    xPos = GetStringCenterAlignXOffset(FONT_NORMAL, text, 32);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, text, xPos, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void Task_BirthdaySpinner(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    s32 max;

    if (tPhase == BDAY_SPINNER_PHASE_MONTH)
        max = 12;
    else
        max = sCalendarMonthLengths[tMonth - 1];

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (tPhase == BDAY_SPINNER_PHASE_MONTH)
        {
            // Month locked in — clamp the day for short months (28/30).
            if (tDay > sCalendarMonthLengths[tMonth - 1])
                tDay = sCalendarMonthLengths[tMonth - 1];
            tPhase = BDAY_SPINNER_PHASE_DAY;
            PrintBirthdaySpinnerValue(tWindowId, tDay);
        }
        else
        {
            VarSet(VAR_PLAYER_BIRTHDAY, ((u16)tMonth << 8) | (u8)tDay);
            // Buffer "DD/MM" for the confirmation message.
            ConvertIntToDecimalStringN(gStringVar1, tDay, STR_CONV_MODE_LEADING_ZEROS, 2);
            gStringVar1[2] = CHAR_SLASH;
            ConvertIntToDecimalStringN(&gStringVar1[3], tMonth, STR_CONV_MODE_LEADING_ZEROS, 2);
            gSpecialVar_Result = TRUE;
            ClearStdWindowAndFrameToTransparent(tWindowId, TRUE);
            RemoveWindow(tWindowId);
            ScriptContext_Enable();
            DestroyTask(taskId);
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (tPhase == BDAY_SPINNER_PHASE_DAY)
        {
            tPhase = BDAY_SPINNER_PHASE_MONTH;
            PrintBirthdaySpinnerValue(tWindowId, tMonth);
        }
        else
        {
            gSpecialVar_Result = FALSE;
            ClearStdWindowAndFrameToTransparent(tWindowId, TRUE);
            RemoveWindow(tWindowId);
            ScriptContext_Enable();
            DestroyTask(taskId);
        }
    }
    else if (JOY_REPEAT(DPAD_UP))
    {
        if (tPhase == BDAY_SPINNER_PHASE_MONTH)
            tMonth = (tMonth % 12) + 1;
        else
            tDay = (tDay % max) + 1;
        PrintBirthdaySpinnerValue(tWindowId, (tPhase == BDAY_SPINNER_PHASE_MONTH) ? tMonth : tDay);
    }
    else if (JOY_REPEAT(DPAD_DOWN))
    {
        if (tPhase == BDAY_SPINNER_PHASE_MONTH)
            tMonth = (tMonth + 10) % 12 + 1;
        else
            tDay = (tDay + max - 2) % max + 1;
        PrintBirthdaySpinnerValue(tWindowId, (tPhase == BDAY_SPINNER_PHASE_MONTH) ? tMonth : tDay);
    }
}

// Special. Opens the spinner; the waiting script resumes with
// VAR_RESULT = 1 (stored, "DD/MM" buffered in STR_VAR_1) or 0 (cancelled).
void FanClub_StartBirthdayEntry(void)
{
    u8 taskId = CreateTask(Task_BirthdaySpinner, 80);
    u16 custom = VarGet(VAR_PLAYER_BIRTHDAY);

    gTasks[taskId].tPhase = BDAY_SPINNER_PHASE_MONTH;
    gTasks[taskId].tMonth = (custom != 0) ? ((custom >> 8) & 0xFF) : 1;
    gTasks[taskId].tDay = (custom != 0) ? (custom & 0xFF) : 1;
    gTasks[taskId].tWindowId = AddWindow(&sBirthdaySpinnerWinTemplate);
    SetStandardWindowBorderStyle(gTasks[taskId].tWindowId, FALSE);
    PrintBirthdaySpinnerValue(gTasks[taskId].tWindowId, gTasks[taskId].tMonth);
}

// Special. 1 = offer registration (no hardcoded birthday, none stored),
// 2 = player is on the hardcoded chart, 0 = custom already registered.
u16 FanClub_GetBirthdayAskState(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sPlayerBirthdays); i++)
    {
        if (BirthdayNameMatches(gSaveBlock2Ptr->playerName, sPlayerBirthdays[i].name))
            return 2;
    }
    if (VarGet(VAR_PLAYER_BIRTHDAY) != 0)
        return 0;
    return 1;
}

#undef tPhase
#undef tMonth
#undef tDay
#undef tWindowId

// ============================================================
// Special calendar dates
// ============================================================
// Unlike birthdays, special dates only activate once the National
// Pokédex has been unlocked.

static bool8 IsSpecialCalendarDate(u8 month, u8 day)
{
    u8 m, d;

    if (!IsCalendarDateSet() || !IsNationalPokedexEnabled())
        return FALSE;
    GetVirtualDate(&m, &d);
    return m == month && d == day;
}

// Spooky Day — October 31. Nurses get an eerie greeting, Mt. Pyre
// crawls with ghosts, and the day's mass outbreak is always a GHOST
// Pokémon (see wild_encounter.c and tv.c).
u16 IsSpookyDayToday(void)
{
    return IsSpecialCalendarDate(10, 31);
}

// Hoenn Day — November 21 (Ruby/Sapphire's launch date). Hoenn-native
// Pokémon give double EXP and EVs when defeated and shine twice as
// often when generated (see battle_script_commands.c and pokemon.c).
u16 IsHoennDayToday(void)
{
    return IsSpecialCalendarDate(11, 21);
}

// Trick Day — April 1. Half of all wild encounters, anywhere, are
// secretly DITTO — and prankster DITTO shine twice as often.
u16 IsTrickDayToday(void)
{
    return IsSpecialCalendarDate(4, 1);
}

// Luvdisc Day — February 14. LUVDISC swarm the ocean routes clutching
// HEART SCALES, eggs develop twice as fast, daycare couples get along
// twice as well — and a few familiar faces get a little flustered.
u16 IsLuvdiscDayToday(void)
{
    return IsSpecialCalendarDate(2, 14);
}

// Gift Day — December 25. A cold snap draws ICE Pokémon out across every
// route, every berry tree yields double, and the day grants double EXP/EVs
// /shiny odds. The nurse explains the chill and points to Prof Birch, who
// hands out a NEVERMELTICE and sends the player to Mom (a RARE CANDY) and
// the rival (a one-day shiny → shiny DELIBIRD trade). See wild_encounter.c,
// pokemon.c, battle_script_commands.c, berry.c and the house/lab scripts.
u16 IsGiftDayToday(void)
{
    return IsSpecialCalendarDate(12, 25);
}

// Delivery Day — the 1st of every month. The Slateport market's Energy
// Guru stocks one special item, one purchase per month.
u16 IsDeliveryDayToday(void)
{
    u8 m, d;

    if (!IsCalendarDateSet() || !IsNationalPokedexEnabled())
        return FALSE;
    GetVirtualDate(&m, &d);
    return d == 1;
}

// TRUE on any of the fixed special calendar dates (not birthdays/anniversary,
// which arm latches). Pure date reads — safe to call from map load. Used to
// keep Mirage Island visible on holidays.
bool8 IsAnySpecialDateToday(void)
{
    return IsSpookyDayToday() || IsHoennDayToday() || IsTrickDayToday()
        || IsLuvdiscDayToday() || IsGiftDayToday() || IsDeliveryDayToday();
}

// TRUE for species in the (expanded) Hoenn regional dex.
bool8 IsHoennNativeSpecies(u16 species)
{
    u16 hoennNum;

    if (species == SPECIES_NONE)
        return FALSE;
    hoennNum = NationalToHoennOrder(SpeciesToNationalPokedexNum(species));
    return hoennNum != 0 && hoennNum <= HOENN_DEX_COUNT;
}

// Delivery Day stock, one entry per month (January first).
static const struct
{
    u16 itemId;
    u16 price;
} sDeliveryDayStock[12] =
{
    {ITEM_LEFTOVERS,      9800}, // January
    {ITEM_PP_MAX,         9800}, // February
    {ITEM_KINGS_ROCK,     4800}, // March
    {ITEM_METAL_COAT,     4800}, // April
    {ITEM_SCOPE_LENS,     4800}, // May
    {ITEM_FOCUS_BAND,     4800}, // June
    {ITEM_DEEP_SEA_TOOTH, 3800}, // July
    {ITEM_DEEP_SEA_SCALE, 3800}, // August
    {ITEM_LUCKY_EGG,      9800}, // September
    {ITEM_MOON_STONE,     3000}, // October
    {ITEM_SUN_STONE,      3000}, // November
    {ITEM_STAR_PIECE,     2500}, // December
};

// VAR_RESULT: 0 = not Delivery Day, 1 = offer available (item id in
// 0x8004, name in STR_VAR_1, price in STR_VAR_2), 2 = already bought
// this month. The once-per-month latch stores (day counter + 1) at
// purchase; consecutive 1sts are 28-31 days apart, so a sub-25-day gap
// means "same month" without needing to track years.
void DeliveryDay_GetTodaysOffer(void)
{
    u8 month, day;
    u16 stored, d;

    gSpecialVar_Result = 0;
    if (!IsDeliveryDayToday())
        return;
    GetVirtualDate(&month, &day); // also refreshes gLocalTime
    d = (u16)gLocalTime.days;
    stored = VarGet(VAR_DELIVERY_DAY_LAST);
    if (stored != 0 && (u16)(d - (stored - 1)) < 25)
    {
        gSpecialVar_Result = 2;
        return;
    }
    gSpecialVar_0x8004 = sDeliveryDayStock[month - 1].itemId;
    CopyItemName(gSpecialVar_0x8004, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, sDeliveryDayStock[month - 1].price, STR_CONV_MODE_LEFT_ALIGN, 5);
    gSpecialVar_Result = 1;
}

// VAR_RESULT: 1 = paid (item in 0x8004 for the script's giveitem),
// 2 = not enough money, 3 = no room in the bag. Deducts the money and
// arms the monthly latch here; the script hands the item over.
void DeliveryDay_TryPurchase(void)
{
    u8 month, day;
    u16 price;

    GetVirtualDate(&month, &day);
    price = sDeliveryDayStock[month - 1].price;
    if (!IsEnoughMoney(&gSaveBlock1Ptr->money, price))
    {
        gSpecialVar_Result = 2;
        return;
    }
    if (!CheckBagHasSpace(sDeliveryDayStock[month - 1].itemId, 1))
    {
        gSpecialVar_Result = 3;
        return;
    }
    RemoveMoney(&gSaveBlock1Ptr->money, price);
    gSpecialVar_0x8004 = sDeliveryDayStock[month - 1].itemId;
    RtcCalcLocalTime();
    VarSet(VAR_DELIVERY_DAY_LAST, (u16)gLocalTime.days + 1);
    gSpecialVar_Result = 1;
}

// The visiting spirits of Spooky Day.
static const u16 sSpookySpecies[] =
{
    SPECIES_GASTLY,
    SPECIES_HAUNTER,
    SPECIES_DUSKULL,
    SPECIES_SHUPPET,
    SPECIES_MISDREAVUS,
};

u16 GetRandomSpookySpecies(void)
{
    return sSpookySpecies[Random() % ARRAY_COUNT(sSpookySpecies)];
}

// Gift Day's cold snap. Ocean surf turns up cold-water dwellers; land grass
// (and rock smash) turns up the mountain/ice crowd; DELIBIRD roams both, so
// it sits in each pool for its "basically everywhere" feel.
static const u16 sGiftDayWaterSpecies[] =
{
    SPECIES_LAPRAS,
    SPECIES_SPHEAL,
    SPECIES_SHELLDER,
    SPECIES_DELIBIRD,
};

static const u16 sGiftDayLandSpecies[] =
{
    SPECIES_SWINUB,
    SPECIES_SNEASEL,
    SPECIES_SNORUNT,
    SPECIES_SMOOCHUM,
    SPECIES_JYNX,
    SPECIES_DELIBIRD,
};

u16 GetRandomGiftDaySpecies(bool8 isWater)
{
    if (isWater)
        return sGiftDayWaterSpecies[Random() % ARRAY_COUNT(sGiftDayWaterSpecies)];
    return sGiftDayLandSpecies[Random() % ARRAY_COUNT(sGiftDayLandSpecies)];
}

