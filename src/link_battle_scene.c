#include "global.h"
#include "link_battle_scene.h"
#include "event_data.h"
#include "link.h"
#include "pokemon.h"
#include "random.h"
#include "rtc.h"
#include "constants/battle.h"
#include "constants/flags.h"
#include "constants/trainers.h"

// Per-player broadcast pipeline for cable-club link battles, shared by
// three features: the battle scene (terrain) picker, RS-style multi-battle
// sprites, and the mugshot cutscene BG selection. Each player's data rides
// in the trainer-card block exchange:
//   filler[0] (0x44) = scene pick (LINK_SCENE_OPT_* index, or LINK_SCENE_RANDOM)
//   filler[1] (0x45) = magic byte 0xA5 (presence marker for ALL fields)
//   filler[2] (0x46) = player style flag: 1 if FLAG_PLAYER_STYLE_RS set, else 0
//   filler[3] (0x47) = local RTC entropy low byte
//   filler[4] (0x48) = local RTC entropy high byte
// All five live inside the 100-byte SendBlockRequest(BLOCK_REQ_SIZE_100)
// window used by the cable club, so peers receive them via gBlockRecvBuffer.
//
// The scene picker: each player picks a terrain (or RANDOM) before the
// linkup; picks are broadcast, and every machine deterministically resolves
// the same winner (see ResolveAndGetTerrain). The resolved terrain overrides
// gBattleTerrain at battle init (battle_main.c) AND battle_bg.c falls
// through to the per-terrain art for link battles when a scene was
// negotiated — the missing half that made the original picker look broken.
//
// Why a fixed offset inside the trainer card instead of a struct field:
// TrainerCard is a save struct and any change to its size is forbidden
// (CLAUDE.md absolute rule). The card itself has 8 bytes of filler at
// offset 0x44 that nothing reads (verified via grep — zero references).
#define SCENE_BLOCK_OFFSET 0x44
#define STYLE_BLOCK_OFFSET 0x46
#define RTC_BLOCK_OFFSET   0x47

// Terrains exposed to the picker. Order matches the multichoice option
// indices in script_menu.c (sLinkBattleSceneNames).
static const u8 sPickableTerrains[] = {
    BATTLE_TERRAIN_PLAIN,
    BATTLE_TERRAIN_GRASS,
    BATTLE_TERRAIN_SAND,
    BATTLE_TERRAIN_WATER,
    BATTLE_TERRAIN_CAVE,
    BATTLE_TERRAIN_MOUNTAIN,
    BATTLE_TERRAIN_BUILDING,
};

EWRAM_DATA static u8 sLocalScenePick = LINK_SCENE_NONE;
EWRAM_DATA static u8 sFinalLinkScene = LINK_SCENE_NONE;
EWRAM_DATA static bool8 sFinalSceneResolved = FALSE;

EWRAM_DATA static u8 sCachedScenePicks[MAX_RFU_PLAYERS] = {0};
EWRAM_DATA static u8 sCachedStyleFlags[MAX_RFU_PLAYERS] = {0};
EWRAM_DATA static u16 sCachedRtcSnapshots[MAX_RFU_PLAYERS] = {0};
EWRAM_DATA static bool8 sCachedDataValid[MAX_RFU_PLAYERS] = {0};

void LinkBattleScene_Reset(void)
{
    u8 i;

    sLocalScenePick = LINK_SCENE_NONE;
    sFinalLinkScene = LINK_SCENE_NONE;
    sFinalSceneResolved = FALSE;
    for (i = 0; i < MAX_RFU_PLAYERS; i++)
    {
        sCachedScenePicks[i] = LINK_SCENE_NONE;
        sCachedStyleFlags[i] = 0;
        sCachedRtcSnapshots[i] = 0;
        sCachedDataValid[i] = FALSE;
    }
}

void LinkBattleScene_StoreLocalPick(u8 pick)
{
    sLocalScenePick = pick;
    // Pick changed, drop any cached resolution from a prior session.
    sFinalSceneResolved = FALSE;
    sFinalLinkScene = LINK_SCENE_NONE;
}

// Called by Task_LinkupAwaitTrainerCardData (src/cable_club.c) right after
// gTrainerCards is populated and before ResetBlockReceivedFlags clears the
// receive flags. Copies our broadcast bytes out of gBlockRecvBuffer into
// dedicated EWRAM so subsequent block exchanges (the wireless task's
// gLocalLinkPlayer broadcast in particular) don't wipe them.
void LinkBattleScene_StashPeerData(void)
{
    u8 numPlayers;
    u8 i;
    const u8 *recv;

    numPlayers = GetLinkPlayerCount();
    if (numPlayers > MAX_RFU_PLAYERS)
        numPlayers = MAX_RFU_PLAYERS;

    for (i = 0; i < numPlayers; i++)
    {
        recv = (const u8 *)gBlockRecvBuffer[i];
        if (recv[STYLE_BLOCK_OFFSET - 1] == 0xA5)
        {
            sCachedScenePicks[i] = recv[SCENE_BLOCK_OFFSET];
            sCachedStyleFlags[i] = recv[STYLE_BLOCK_OFFSET];
            sCachedRtcSnapshots[i] = ((u16)recv[RTC_BLOCK_OFFSET + 1] << 8) | recv[RTC_BLOCK_OFFSET];
            sCachedDataValid[i] = TRUE;
        }
        else
        {
            sCachedDataValid[i] = FALSE;
        }
    }
    // Mark unused slots as invalid (in case a previous session linked more
    // players than this one and left stale flags set).
    for (; i < MAX_RFU_PLAYERS; i++)
        sCachedDataValid[i] = FALSE;
}

// Called from cable_club.c right after TrainerCard_GenerateCardForLinkPlayer
// fills the send buffer. Appends RS-style flag + 2-byte RTC snapshot.
// Peers receive everything via gBlockRecvBuffer after link.
void LinkBattleScene_FillSendBuffer(void)
{
    u16 rtcSnapshot;

    gBlockSendBuffer[SCENE_BLOCK_OFFSET] = sLocalScenePick;
    // Magic byte at offset 0x45 so receivers can distinguish "data present"
    // from garbage left in the buffer by older clients.
    gBlockSendBuffer[STYLE_BLOCK_OFFSET - 1] = 0xA5;
    gBlockSendBuffer[STYLE_BLOCK_OFFSET] = FlagGet(FLAG_PLAYER_STYLE_RS) ? 1 : 0;

    // Capture the local RTC into 16 bits — minutes (0-59) in the low byte,
    // hours+seconds combined in the high byte. Anything that varies per
    // second works; the actual encoding doesn't matter as long as it
    // changes between battles. Used by GetSyncSeed to deterministically
    // pick the mugshot BG palette in a way that varies between battles.
    RtcCalcLocalTime();
    rtcSnapshot = ((u16)gLocalTime.minutes & 0xFF)
                | (((u16)gLocalTime.hours ^ ((u16)gLocalTime.seconds << 4)) << 8);
    gBlockSendBuffer[RTC_BLOCK_OFFSET]     = (u8)(rtcSnapshot & 0xFF);
    gBlockSendBuffer[RTC_BLOCK_OFFSET + 1] = (u8)((rtcSnapshot >> 8) & 0xFF);
}

// Returns TRUE if the linked player at playerIdx has FLAG_PLAYER_STYLE_RS
// set on their save (broadcast via the trainer-card buffer, then stashed
// into EWRAM by LinkBattleScene_StashPeerData). Falls back to FALSE
// (Emerald style) when the magic byte isn't present — covers older peers
// that don't broadcast a style or non-cable-club battles.
bool8 LinkBattleScene_IsPlayerRsStyle(u8 playerIdx)
{
    if (playerIdx >= MAX_RFU_PLAYERS || !sCachedDataValid[playerIdx])
        return FALSE;
    return sCachedStyleFlags[playerIdx] != 0;
}

// Returns the front trainer pic id for a linked Emerald-version player,
// honoring their broadcast RS style flag. Used by the link-battle sprite
// drawing code so each player's sprite reflects THEIR own style toggle
// (the local FlagGet path in PlayerGenderToFrontTrainerPicId would
// otherwise show every Emerald player with the local player's style,
// which is wrong in multi battles where each peer can pick differently).
u16 LinkBattleScene_GetTrainerPicId(u8 playerIdx, u8 playerGender)
{
    if (LinkBattleScene_IsPlayerRsStyle(playerIdx))
    {
        if (playerGender != MALE)
            return FacilityClassToPicIndex(FACILITY_CLASS_RS_MAY);
        return FacilityClassToPicIndex(FACILITY_CLASS_RS_BRENDAN);
    }
    if (playerGender != MALE)
        return FacilityClassToPicIndex(FACILITY_CLASS_MAY);
    return FacilityClassToPicIndex(FACILITY_CLASS_BRENDAN);
}

// Back pic equivalent for the player-side trainer slide (each peer's own
// trainer back sprite when entering the battle). Same broadcast-style
// rules as the front pic above. Adding playerGender (MALE=0, FEMALE=1)
// to the base id picks BRENDAN/MAY (Emerald) or RS_BRENDAN/RS_MAY pairs.
u16 LinkBattleScene_GetTrainerBackPicId(u8 playerIdx, u8 playerGender)
{
    if (LinkBattleScene_IsPlayerRsStyle(playerIdx))
        return playerGender + TRAINER_BACK_PIC_RUBY_SAPPHIRE_BRENDAN;
    return playerGender + TRAINER_BACK_PIC_BRENDAN;
}

// Deterministic seed shared by the mugshot cutscene's BG-palette picker.
// Mixes trainer IDs + per-peer RTC snapshots so repeat battles between the
// same player pair pick different colors as time advances; every peer reads
// the same broadcast data so the choice still matches across screens.
//
// Iterates over the stash directly (sCachedDataValid) instead of asking
// GetLinkPlayerCount, because by the time this runs at battle init the
// cable lobby link has already been torn down (Task_StartWiredCableClubBattle
// case 2 fires SetCloseLinkCallback long before CB2_InitBattle), so
// GetLinkPlayerCount returns 0 and would zero the seed. The stash was
// populated during the lobby while the count was still valid.
u32 LinkBattleScene_GetSyncSeed(void)
{
    u8 i;
    u32 seed = 0;

    for (i = 0; i < MAX_RFU_PLAYERS; i++)
    {
        if (sCachedDataValid[i])
        {
            seed ^= (u32)gLinkPlayers[i].trainerId << ((i & 1) ? 0 : 16);
            seed ^= (u32)sCachedScenePicks[i] << ((i * 7) & 0x1F);
            seed ^= (u32)sCachedRtcSnapshots[i] << ((i * 11) & 0x1F);
        }
    }
    return seed;
}

// Returns LINK_SCENE_NONE if the player didn't pick (option disabled or
// non-cable-club battle), otherwise the broadcast pick value. Reads from
// the EWRAM stash populated by LinkBattleScene_StashPeerData — NOT from
// gBlockRecvBuffer directly, since that buffer gets overwritten by later
// block exchanges (notably the wireless lobby's gLocalLinkPlayer broadcast).
static u8 ResolveOnePick(u8 pick, u32 seed)
{
    u8 idx;

    if (pick == LINK_SCENE_RANDOM)
    {
        idx = seed % ARRAY_COUNT(sPickableTerrains);
        return sPickableTerrains[idx];
    }
    if (pick < ARRAY_COUNT(sPickableTerrains))
        return sPickableTerrains[pick];
    return LINK_SCENE_NONE;
}

// Deterministically resolve every peer's broadcast pick to one final
// terrain, identical on every machine. Iterates over the stash directly
// (sCachedDataValid) — by the time this runs at battle init the cable
// lobby link is gone (SetCloseLinkCallback fires before CB2_InitBattle),
// so GetLinkPlayerCount would be 0 and we'd bail on a perfectly-good
// stash. The stash was populated during the lobby; here we just trust it.
u8 LinkBattleScene_ResolveAndGetTerrain(void)
{
    u8 picks[MAX_RFU_PLAYERS];
    u8 numValidPicks = 0;
    u8 i;
    u8 pick;
    u32 seed;
    u8 chosenIdx;

    if (sFinalSceneResolved)
        return sFinalLinkScene;

    for (i = 0; i < MAX_RFU_PLAYERS; i++)
    {
        if (!sCachedDataValid[i])
            continue;
        pick = sCachedScenePicks[i];
        if (pick != LINK_SCENE_NONE)
            picks[numValidPicks++] = pick;
    }

    if (numValidPicks == 0)
        return LINK_SCENE_NONE;

    seed = LinkBattleScene_GetSyncSeed();
    // Defensive: GetSyncSeed could theoretically return 0 if every
    // valid peer's trainer id and RTC bytes XOR to zero. Fall back to
    // local trainer id so chosenIdx isn't always 0 % N = 0.
    if (seed == 0)
        seed = gSaveBlock2Ptr->playerTrainerId[0]
             | ((u32)gSaveBlock2Ptr->playerTrainerId[1] << 8);
    chosenIdx = seed % numValidPicks;
    sFinalLinkScene = ResolveOnePick(picks[chosenIdx], seed >> 4);
    sFinalSceneResolved = TRUE;
    return sFinalLinkScene;
}
