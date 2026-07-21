#include "global.h"
#include "malloc.h"
#include "battle.h"
#include "battle_setup.h"
#include "bg.h"
#include "data.h"
#include "daycare.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "international_string_util.h"
#include "main.h"
#include "match_call.h"
#include "menu.h"
#include "new_game.h"
#include "overworld.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "region_map.h"
#include "rtc.h"
#include "script.h"
#include "script_movement.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "wild_encounter.h"
#include "window.h"
#include "constants/abilities.h"
#include "constants/battle_frontier.h"
#include "constants/event_objects.h"
#include "constants/game_stat.h"
#include "constants/opponents.h"
#include "constants/region_map_sections.h"
#include "constants/secret_bases.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/tv.h"

// In this file only the values normally associated with Battle Pike and Factory are swapped.
// Note that this is *not* a bug, because they are properly swapped consistently in this file.
// There would only be an issue if anything in this file interacted with something expecting
// the usual value order, or vice versa.
#define MATCH_CALL_FACTORY  FRONTIER_FACILITY_PIKE
#define MATCH_CALL_PIKE     FRONTIER_FACILITY_FACTORY

// Each match call message has variables that can be populated randomly or
// dependent on the trainer. The below are IDs for how to populate the vars
// in a given message.
// Each message may have up to 3 vars in it
enum {
    STR_TRAINER_NAME,
    STR_MAP_NAME,
    STR_SPECIES_IN_ROUTE,
    STR_SPECIES_IN_PARTY,
    STR_FACILITY_NAME,
    STR_FRONTIER_STREAK,
    STR_NONE = -1,
};
#define STRS_NORMAL_MSG      {STR_TRAINER_NAME, STR_NONE,             STR_NONE}
#define STRS_WILD_BATTLE     {STR_TRAINER_NAME, STR_SPECIES_IN_ROUTE, STR_NONE}
#define STRS_BATTLE_NEGATIVE {STR_TRAINER_NAME, STR_NONE,             STR_NONE}
#define STRS_BATTLE_POSITIVE {STR_TRAINER_NAME, STR_SPECIES_IN_PARTY, STR_NONE}
#define STRS_BATTLE_REQUEST  {STR_TRAINER_NAME, STR_MAP_NAME,         STR_NONE}
#define STRS_FRONTIER        {STR_TRAINER_NAME, STR_FACILITY_NAME,    STR_FRONTIER_STREAK}

#define NUM_STRVARS_IN_MSG 3

// Topic IDs for sMatchCallGeneralTopics
enum {
    GEN_TOPIC_PERSONAL = 1,
    GEN_TOPIC_STREAK,
    GEN_TOPIC_STREAK_RECORD,
    GEN_TOPIC_B_DOME,
    GEN_TOPIC_B_PIKE,
    GEN_TOPIC_B_PYRAMID,
};

// Topic IDs for sMatchCallBattleTopics
enum {
    B_TOPIC_WILD = 1,
    B_TOPIC_NEGATIVE,
    B_TOPIC_POSITIVE,
};

// Each trainer has a text id for 1 of each of the 3 battle topics
// The msgId is the index into the respective topic's message table
// For all but 2 trainers this index is the same for each topic
#define BATTLE_TEXT_IDS(msgId) {TEXT_ID(B_TOPIC_WILD, (msgId)), TEXT_ID(B_TOPIC_NEGATIVE, (msgId)), TEXT_ID(B_TOPIC_POSITIVE, (msgId))}

// Topic IDs for sMatchCallBattleRequestTopics
enum {
    REQ_TOPIC_SAME_ROUTE = 1,
    REQ_TOPIC_DIFF_ROUTE,
};

// Veritas: a call selected at trigger time can be forced into a special
// message instead of the vanilla rematch/chatter selection.
enum
{
    FORCED_CALL_NONE,     // vanilla behavior
    FORCED_CALL_BIRTHDAY, // first call on the player's birthday: a birthday wish
    FORCED_CALL_CASUAL,   // fallback call: random tip or idle chatter
    FORCED_CALL_SPECIAL,  // a fixed caller (Mom, Steven, a Frontier Brain, ...)
};

// Veritas "family B" callers: people who are NOT in the rematch table, so
// they have no matchCallId. Their name is baked into the message text (or
// written straight into gStringVar1) and the whole vanilla
// GetTrainerMatchCallId path is bypassed -- that function loops forever on
// a trainerId it doesn't know about.
enum
{
    SPECIAL_CALL_VERITAS,
    SPECIAL_CALL_HEART_RIVAL,
    SPECIAL_CALL_HEART_STEVEN,
    SPECIAL_CALL_HEART_WALLACE,
    SPECIAL_CALL_FRONTIER_BRAIN,
    SPECIAL_CALL_MOM,
    SPECIAL_CALL_BIRCH,
    SPECIAL_CALL_RIVAL,
    SPECIAL_CALL_STEVEN,
    SPECIAL_CALL_RIVAL_WEATHER, // "the Weather Institute needs trainers"
    SPECIAL_CALL_RIVAL_MIXED,   // "I met someone from far away…"
    SPECIAL_CALL_COUNT,
};

// Veritas "family A" casual calls: the caller IS a registered rematch
// trainer, so {STR_VAR_1} is filled by the normal PopulateTrainerName path
// and only the extra vars (if any) are written by hand.
enum
{
    CASUAL_CHATTER,      // vanilla battle/general chatter
    CASUAL_ABILITY_TIP,
    CASUAL_GENERAL_TIP,
    CASUAL_LORE_SOUTHERN,
    CASUAL_LORE_DRAGON,
    CASUAL_OUTBREAK,
    CASUAL_TIME_OF_DAY,
    CASUAL_RARE_CATCH,
    CASUAL_LEGEND_RUMOR,
    CASUAL_LEGEND_BIRDS,
    CASUAL_LEGEND_JIRACHI,
    CASUAL_QUEST_MUSEUM,
    CASUAL_QUEST_FASHIONISTA,
    CASUAL_MIXED_SIGHTING,
    CASUAL_MIXED_CATCH,
    CASUAL_MIXED_BASE,
    CASUAL_COUNT,
};

struct MatchCallState
{
    u32 minutes;
    u16 trainerId;
    u8 stepCounter;
    bool8 triggeredFromScript;
    u8 forcedCallType;
    u8 specialCallId;
    bool8 veritasCallActive; // the call in progress ends with a cry, not a goodbye
    u16 frozenRtcChecks;
};

struct MatchCallTrainerTextInfo
{
    u16 trainerId;
    u16 unused;
    u16 battleTopicTextIds[3];
    u16 generalTextId;
    u8 battleFrontierRecordStreakTextIndex;
    u16 sameRouteMatchCallTextId;
    u16 differentRouteMatchCallTextId;
};

struct MatchCallText
{
    const u8 *text;
    s8 stringVarFuncIds[NUM_STRVARS_IN_MSG];
};

struct MultiTrainerMatchCallText
{
    u16 trainerId;
    const u8 *text;
};

struct BattleFrontierStreakInfo
{
    u16 facilityId;
    u16 streak;
};

static EWRAM_DATA struct MatchCallState sMatchCallState = {0};
static EWRAM_DATA struct BattleFrontierStreakInfo sBattleFrontierStreakInfo = {0};

static u32 GetCurrentTotalMinutes(struct Time *);
static u32 GetNumRegisteredNPCs(void);
static u32 GetActiveMatchCallTrainerId(u32);
static int GetTrainerMatchCallId(int);
static u16 GetRematchTrainerLocation(int);
static bool32 TrainerIsEligibleForRematch(int);
static void StartMatchCall(void);
static void ExecuteMatchCall(u8);
static void DrawMatchCallTextBoxBorder_Internal(u32, u32, u32);
static void Task_SpinPokenavIcon(u8);
static void InitMatchCallTextPrinter(int, const u8 *);
static bool32 RunMatchCallTextPrinter(int);
static const struct MatchCallText *GetSameRouteMatchCallText(int, u8 *);
static const struct MatchCallText *GetDifferentRouteMatchCallText(int, u8 *);
static const struct MatchCallText *GetBattleMatchCallText(int, u8 *);
static const struct MatchCallText *GetGeneralMatchCallText(int, u8 *);
static bool32 ShouldTrainerRequestBattle(int);
static void BuildMatchCallString(int, const struct MatchCallText *, u8 *);
static u16 GetFrontierStreakInfo(u16, u32 *);
static void PopulateMatchCallStringVars(int, const s8 *);
static void PopulateMatchCallStringVar(int, int, u8 *);
static bool32 MatchCall_LoadGfx(u8);
static bool32 MatchCall_DrawWindow(u8);
static bool32 MatchCall_ReadyIntro(u8);
static bool32 MatchCall_SlideWindowIn(u8);
static bool32 MatchCall_PrintIntro(u8);
static bool32 MatchCall_PrintMessage(u8);
static bool32 MatchCall_SlideWindowOut(u8);
static bool32 MatchCall_EndCall(u8);
static void PopulateTrainerName(int, u8 *);
static void PopulateMapName(int, u8 *);
static void PopulateSpeciesFromTrainerLocation(int, u8 *);
static void PopulateSpeciesFromTrainerParty(int, u8 *);
static void PopulateBattleFrontierFacilityName(int, u8 *);
static void PopulateBattleFrontierStreak(int, u8 *);
// Veritas
static bool32 TrySelectSpecialCall(void);
static void BuildSpecialCallString(u8 *);
static const struct MatchCallText *SelectCasualMatchCallText(int, u8 *);
static bool32 TryPopulateOutbreakVars(bool8 *);
static bool32 TryPopulateRareCatchVars(bool8 *);
static bool32 PlayerHasAnyFrontierStreak(void);
static bool32 ShouldOfferWeatherInstituteCall(void);
static bool32 HasAnyRecordMixedTrainer(void);
static bool32 TryPopulateMixedTrainerForRival(u8 *, bool8 *);
static int SelectLegendRumor(void);
static bool32 ShouldOfferBirdRumor(void);

#define TEXT_ID(topic, id) (((topic) << 8) | ((id) & 0xFF))

static const struct MatchCallTrainerTextInfo sMatchCallTrainers[] =
{
    {
        .trainerId = TRAINER_ROSE_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(8),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 3),
        .battleFrontierRecordStreakTextIndex = 8,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 8),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 8),
    },
    {
        .trainerId = TRAINER_ANDRES_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(12),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 62),
        .battleFrontierRecordStreakTextIndex = 12,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 12),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 12),
    },
    {
        .trainerId = TRAINER_DUSTY_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(12),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 4),
        .battleFrontierRecordStreakTextIndex = 12,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 12),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 12),
    },
    {
        .trainerId = TRAINER_LOLA_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(2),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 5),
        .battleFrontierRecordStreakTextIndex = 2,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 2),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 2),
    },
    {
        .trainerId = TRAINER_RICKY_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(1),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 6),
        .battleFrontierRecordStreakTextIndex = 1,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 1),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 1),
    },
    {
        .trainerId = TRAINER_LILA_AND_ROY_1,
        .unused = 4,
        .battleTopicTextIds = BATTLE_TEXT_IDS(1),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 61),
        .battleFrontierRecordStreakTextIndex = 1,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 1),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 1),
    },
    {
        .trainerId = TRAINER_CRISTIN_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(10),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 64),
        .battleFrontierRecordStreakTextIndex = 10,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 10),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 10),
    },
    {
        .trainerId = TRAINER_BROOKE_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(9),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 8),
        .battleFrontierRecordStreakTextIndex = 9,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 9),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 9),
    },
    {
        .trainerId = TRAINER_WILTON_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(6),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 7),
        .battleFrontierRecordStreakTextIndex = 6,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 6),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 6),
    },
    {
        .trainerId = TRAINER_VALERIE_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(8),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 9),
        .battleFrontierRecordStreakTextIndex = 8,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 8),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 8),
    },
    {
        .trainerId = TRAINER_CINDY_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(8),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 10),
        .battleFrontierRecordStreakTextIndex = 8,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 8),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 8),
    },
    {
        .trainerId = TRAINER_THALIA_1,
        .unused = 0,
        // Thalia and Sawyer are the only ones who use different msg ids for their battle topics
        .battleTopicTextIds = { TEXT_ID(B_TOPIC_WILD, 8), TEXT_ID(B_TOPIC_NEGATIVE, 10), TEXT_ID(B_TOPIC_POSITIVE, 10) },
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 14),
        .battleFrontierRecordStreakTextIndex = 10,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 8),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 10),
    },
    {
        .trainerId = TRAINER_JESSICA_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(10),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 11),
        .battleFrontierRecordStreakTextIndex = 10,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 8),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 10),
    },
    {
        .trainerId = TRAINER_WINSTON_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(4),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 12),
        .battleFrontierRecordStreakTextIndex = 4,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 4),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 4),
    },
    {
        .trainerId = TRAINER_STEVE_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(7),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 13),
        .battleFrontierRecordStreakTextIndex = 7,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 7),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 7),
    },
    {
        .trainerId = TRAINER_TONY_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(5),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 15),
        .battleFrontierRecordStreakTextIndex = 5,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 5),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 5),
    },
    {
        .trainerId = TRAINER_NOB_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(3),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 16),
        .battleFrontierRecordStreakTextIndex = 3,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 3),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 3),
    },
    {
        .trainerId = TRAINER_KOJI_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(3),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 59),
        .battleFrontierRecordStreakTextIndex = 3,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 3),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 3),
    },
    {
        .trainerId = TRAINER_FERNANDO_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(6),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 17),
        .battleFrontierRecordStreakTextIndex = 6,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 6),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 6),
    },
    {
        .trainerId = TRAINER_DALTON_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(4),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 18),
        .battleFrontierRecordStreakTextIndex = 4,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 4),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 4),
    },
    {
        .trainerId = TRAINER_BERNIE_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(11),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 19),
        .battleFrontierRecordStreakTextIndex = 11,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 11),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 11),
    },
    {
        .trainerId = TRAINER_ETHAN_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(1),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 20),
        .battleFrontierRecordStreakTextIndex = 1,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 1),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 1),
    },
    {
        .trainerId = TRAINER_JOHN_AND_JAY_1,
        .unused = 3,
        .battleTopicTextIds = BATTLE_TEXT_IDS(12),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 60),
        .battleFrontierRecordStreakTextIndex = 12,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 12),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 12),
    },
    {
        .trainerId = TRAINER_JEFFREY_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(7),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 21),
        .battleFrontierRecordStreakTextIndex = 7,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 7),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 7),
    },
    {
        .trainerId = TRAINER_CAMERON_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(4),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 22),
        .battleFrontierRecordStreakTextIndex = 1,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 4),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 4),
    },
    {
        .trainerId = TRAINER_JACKI_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(8),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 23),
        .battleFrontierRecordStreakTextIndex = 8,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 8),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 8),
    },
    {
        .trainerId = TRAINER_WALTER_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(12),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 24),
        .battleFrontierRecordStreakTextIndex = 12,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 12),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 12),
    },
    {
        .trainerId = TRAINER_KAREN_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(2),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 26),
        .battleFrontierRecordStreakTextIndex = 2,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 2),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 2),
    },
    {
        .trainerId = TRAINER_JERRY_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(1),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 25),
        .battleFrontierRecordStreakTextIndex = 1,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 1),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 1),
    },
    {
        .trainerId = TRAINER_ANNA_AND_MEG_1,
        .unused = 6,
        .battleTopicTextIds = BATTLE_TEXT_IDS(9),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 27),
        .battleFrontierRecordStreakTextIndex = 9,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 9),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 9),
    },
    {
        .trainerId = TRAINER_ISABEL_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(14),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 29),
        .battleFrontierRecordStreakTextIndex = 14,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 14),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 14),
    },
    {
        .trainerId = TRAINER_MIGUEL_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(11),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 28),
        .battleFrontierRecordStreakTextIndex = 11,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 11),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 11),
    },
    {
        .trainerId = TRAINER_TIMOTHY_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(12),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 30),
        .battleFrontierRecordStreakTextIndex = 12,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 12),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 12),
    },
    {
        .trainerId = TRAINER_SHELBY_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(13),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 31),
        .battleFrontierRecordStreakTextIndex = 13,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 13),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 13),
    },
    {
        .trainerId = TRAINER_CALVIN_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(1),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 32),
        .battleFrontierRecordStreakTextIndex = 1,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 1),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 1),
    },
    {
        .trainerId = TRAINER_ELLIOT_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(3),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 33),
        .battleFrontierRecordStreakTextIndex = 3,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 3),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 3),
    },
    {
        .trainerId = TRAINER_ISAIAH_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(5),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 38),
        .battleFrontierRecordStreakTextIndex = 5,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 5),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 5),
    },
    {
        .trainerId = TRAINER_MARIA_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(9),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 37),
        .battleFrontierRecordStreakTextIndex = 9,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 9),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 9),
    },
    {
        .trainerId = TRAINER_ABIGAIL_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(9),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 35),
        .battleFrontierRecordStreakTextIndex = 9,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 9),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 9),
    },
    {
        .trainerId = TRAINER_DYLAN_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(5),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 36),
        .battleFrontierRecordStreakTextIndex = 5,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 5),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 5),
    },
    {
        .trainerId = TRAINER_KATELYN_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(9),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 40),
        .battleFrontierRecordStreakTextIndex = 9,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 9),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 9),
    },
    {
        .trainerId = TRAINER_BENJAMIN_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(5),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 34),
        .battleFrontierRecordStreakTextIndex = 5,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 5),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 5),
    },
    {
        .trainerId = TRAINER_PABLO_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(5),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 39),
        .battleFrontierRecordStreakTextIndex = 5,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 5),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 5),
    },
    {
        .trainerId = TRAINER_NICOLAS_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(4),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 41),
        .battleFrontierRecordStreakTextIndex = 4,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 4),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 4),
    },
    {
        .trainerId = TRAINER_ROBERT_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(6),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 42),
        .battleFrontierRecordStreakTextIndex = 6,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 6),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 6),
    },
    {
        .trainerId = TRAINER_LAO_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(1),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 43),
        .battleFrontierRecordStreakTextIndex = 1,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 1),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 1),
    },
    {
        .trainerId = TRAINER_CYNDY_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(9),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 44),
        .battleFrontierRecordStreakTextIndex = 9,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 9),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 9),
    },
    {
        .trainerId = TRAINER_MADELINE_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(8),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 45),
        .battleFrontierRecordStreakTextIndex = 8,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 8),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 8),
    },
    {
        .trainerId = TRAINER_JENNY_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(9),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 46),
        .battleFrontierRecordStreakTextIndex = 9,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 9),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 9),
    },
    {
        .trainerId = TRAINER_DIANA_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(2),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 47),
        .battleFrontierRecordStreakTextIndex = 2,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 2),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 2),
    },
    {
        .trainerId = TRAINER_AMY_AND_LIV_1,
        .unused = 2,
        .battleTopicTextIds = BATTLE_TEXT_IDS(2),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 48),
        .battleFrontierRecordStreakTextIndex = 1,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 2),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 2),
    },
    {
        .trainerId = TRAINER_ERNEST_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(3),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 49),
        .battleFrontierRecordStreakTextIndex = 3,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 3),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 3),
    },
    {
        .trainerId = TRAINER_CORY_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(3),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 63),
        .battleFrontierRecordStreakTextIndex = 3,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 3),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 3),
    },
    {
        .trainerId = TRAINER_EDWIN_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(7),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 50),
        .battleFrontierRecordStreakTextIndex = 7,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 7),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 7),
    },
    {
        .trainerId = TRAINER_LYDIA_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(8),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 52),
        .battleFrontierRecordStreakTextIndex = 8,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 8),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 8),
    },
    {
        .trainerId = TRAINER_ISAAC_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(5),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 51),
        .battleFrontierRecordStreakTextIndex = 5,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 5),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 5),
    },
    {
        .trainerId = TRAINER_GABRIELLE_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(8),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 2),
        .battleFrontierRecordStreakTextIndex = 8,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 8),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 8),
    },
    {
        .trainerId = TRAINER_CATHERINE_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(9),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 54),
        .battleFrontierRecordStreakTextIndex = 9,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 9),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 9),
    },
    {
        .trainerId = TRAINER_JACKSON_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(5),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 53),
        .battleFrontierRecordStreakTextIndex = 5,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 5),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 5),
    },
    {
        .trainerId = TRAINER_HALEY_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(2),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 55),
        .battleFrontierRecordStreakTextIndex = 2,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 2),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 2),
    },
    {
        .trainerId = TRAINER_JAMES_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(1),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 56),
        .battleFrontierRecordStreakTextIndex = 1,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 1),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 1),
    },
    {
        .trainerId = TRAINER_TRENT_1,
        .unused = 0,
        .battleTopicTextIds = BATTLE_TEXT_IDS(3),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 57),
        .battleFrontierRecordStreakTextIndex = 3,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 3),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 3),
    },
    {
        .trainerId = TRAINER_SAWYER_1,
        .unused = 0,
        // Thalia and Sawyer are the only ones who use different msg ids for their battle topics
        .battleTopicTextIds = { TEXT_ID(B_TOPIC_WILD, 15), TEXT_ID(B_TOPIC_NEGATIVE, 3), TEXT_ID(B_TOPIC_POSITIVE, 3) },
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 1),
        .battleFrontierRecordStreakTextIndex = 3,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 3),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 3),
    },
    {
        .trainerId = TRAINER_KIRA_AND_DAN_1,
        .unused = 1,
        .battleTopicTextIds = BATTLE_TEXT_IDS(9),
        .generalTextId = TEXT_ID(GEN_TOPIC_PERSONAL, 58),
        .battleFrontierRecordStreakTextIndex = 9,
        .sameRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_SAME_ROUTE, 9),
        .differentRouteMatchCallTextId = TEXT_ID(REQ_TOPIC_DIFF_ROUTE, 9),
    },
};

static const struct MatchCallText sMatchCallWildBattleTexts[] =
{
    { .text = MatchCall_WildBattleText1,  .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText2,  .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText3,  .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText4,  .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText5,  .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText6,  .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText7,  .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText8,  .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText9,  .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText10, .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText11, .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText12, .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText13, .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText14, .stringVarFuncIds = STRS_WILD_BATTLE },
    { .text = MatchCall_WildBattleText15, .stringVarFuncIds = STRS_WILD_BATTLE },
};

static const struct MatchCallText sMatchCallNegativeBattleTexts[] =
{
    { .text = MatchCall_NegativeBattleText1,  .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText2,  .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText3,  .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText4,  .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText5,  .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText6,  .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText7,  .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText8,  .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText9,  .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText10, .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText11, .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText12, .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText13, .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
    { .text = MatchCall_NegativeBattleText14, .stringVarFuncIds = STRS_BATTLE_NEGATIVE },
};

static const struct MatchCallText sMatchCallPositiveBattleTexts[] =
{
    { .text = MatchCall_PositiveBattleText1,  .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText2,  .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText3,  .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText4,  .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText5,  .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText6,  .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText7,  .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText8,  .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText9,  .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText10, .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText11, .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText12, .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText13, .stringVarFuncIds = STRS_BATTLE_POSITIVE },
    { .text = MatchCall_PositiveBattleText14, .stringVarFuncIds = STRS_BATTLE_POSITIVE },
};

static const struct MatchCallText sMatchCallSameRouteBattleRequestTexts[] =
{
    { .text = MatchCall_SameRouteBattleRequestText1,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText2,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText3,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText4,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText5,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText6,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText7,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText8,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText9,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText10, .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText11, .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText12, .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText13, .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_SameRouteBattleRequestText14, .stringVarFuncIds = STRS_BATTLE_REQUEST },
};

static const struct MatchCallText sMatchCallDifferentRouteBattleRequestTexts[] =
{
    { .text = MatchCall_DifferentRouteBattleRequestText1,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText2,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText3,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText4,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText5,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText6,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText7,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText8,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText9,  .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText10, .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText11, .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText12, .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText13, .stringVarFuncIds = STRS_BATTLE_REQUEST },
    { .text = MatchCall_DifferentRouteBattleRequestText14, .stringVarFuncIds = STRS_BATTLE_REQUEST },
};

static const struct MatchCallText sMatchCallPersonalizedTexts[] =
{
    { .text = MatchCall_PersonalizedText1,  .stringVarFuncIds = { STR_TRAINER_NAME, STR_MAP_NAME, STR_NONE } },
    { .text = MatchCall_PersonalizedText2,  .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText3,  .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText4,  .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText5,  .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText6,  .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText7,  .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText8,  .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText9,  .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText10, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText11, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText12, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText13, .stringVarFuncIds = { STR_TRAINER_NAME, STR_SPECIES_IN_ROUTE, STR_NONE } },
    { .text = MatchCall_PersonalizedText14, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText15, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText16, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText17, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText18, .stringVarFuncIds = { STR_TRAINER_NAME, STR_SPECIES_IN_PARTY, STR_NONE } },
    { .text = MatchCall_PersonalizedText19, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText20, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText21, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText22, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText23, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText24, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText25, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText26, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText27, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText28, .stringVarFuncIds = { STR_TRAINER_NAME, STR_SPECIES_IN_PARTY, STR_NONE } },
    { .text = MatchCall_PersonalizedText29, .stringVarFuncIds = { STR_TRAINER_NAME, STR_SPECIES_IN_PARTY, STR_NONE } },
    { .text = MatchCall_PersonalizedText30, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText31, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText32, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText33, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText34, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText35, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText36, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText37, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText38, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText39, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText40, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText41, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText42, .stringVarFuncIds = { STR_TRAINER_NAME, STR_SPECIES_IN_PARTY, STR_NONE } },
    { .text = MatchCall_PersonalizedText43, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText44, .stringVarFuncIds = { STR_TRAINER_NAME, STR_SPECIES_IN_PARTY, STR_NONE } },
    { .text = MatchCall_PersonalizedText45, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText46, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText47, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText48, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText49, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText50, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText51, .stringVarFuncIds = { STR_TRAINER_NAME, STR_MAP_NAME, STR_NONE } },
    { .text = MatchCall_PersonalizedText52, .stringVarFuncIds = { STR_TRAINER_NAME, STR_SPECIES_IN_PARTY, STR_NONE } },
    { .text = MatchCall_PersonalizedText53, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText54, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText55, .stringVarFuncIds = { STR_TRAINER_NAME, STR_MAP_NAME, STR_NONE } },
    { .text = MatchCall_PersonalizedText56, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText57, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText58, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText59, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText60, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText61, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText62, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText63, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_PersonalizedText64, .stringVarFuncIds = STRS_NORMAL_MSG },
};

static const struct MatchCallText sMatchCallBattleFrontierStreakTexts[] =
{
    { .text = MatchCall_BattleFrontierStreakText1,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText2,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText3,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText4,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText5,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText6,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText7,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText8,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText9,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText10, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText11, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText12, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText13, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierStreakText14, .stringVarFuncIds = STRS_FRONTIER },
};

static const struct MatchCallText sMatchCallBattleFrontierRecordStreakTexts[] =
{
    { .text = MatchCall_BattleFrontierRecordStreakText1,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText2,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText3,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText4,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText5,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText6,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText7,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText8,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText9,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText10, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText11, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText12, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText13, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleFrontierRecordStreakText14, .stringVarFuncIds = STRS_FRONTIER },
};

static const struct MatchCallText sMatchCallBattleDomeTexts[] =
{
    { .text = MatchCall_BattleDomeText1,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText2,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText3,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText4,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText5,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText6,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText7,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText8,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText9,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText10, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText11, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText12, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText13, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattleDomeText14, .stringVarFuncIds = STRS_FRONTIER },
};

static const struct MatchCallText sMatchCallBattlePikeTexts[] =
{
    { .text = MatchCall_BattlePikeText1,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText2,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText3,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText4,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText5,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText6,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText7,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText8,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText9,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText10, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText11, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText12, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText13, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePikeText14, .stringVarFuncIds = STRS_FRONTIER },
};

static const struct MatchCallText sMatchCallBattlePyramidTexts[] =
{
    { .text = MatchCall_BattlePyramidText1,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText2,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText3,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText4,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText5,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText6,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText7,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText8,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText9,  .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText10, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText11, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText12, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText13, .stringVarFuncIds = STRS_FRONTIER },
    { .text = MatchCall_BattlePyramidText14, .stringVarFuncIds = STRS_FRONTIER },
};

static const struct MatchCallText *const sMatchCallBattleTopics[] =
{
    [B_TOPIC_WILD - 1]     = sMatchCallWildBattleTexts,
    [B_TOPIC_NEGATIVE - 1] = sMatchCallNegativeBattleTexts,
    [B_TOPIC_POSITIVE - 1] = sMatchCallPositiveBattleTexts,
};

static const struct MatchCallText *const sMatchCallBattleRequestTopics[] =
{
    [REQ_TOPIC_SAME_ROUTE - 1] = sMatchCallSameRouteBattleRequestTexts,
    [REQ_TOPIC_DIFF_ROUTE - 1] = sMatchCallDifferentRouteBattleRequestTexts,
};

// Veritas: birthday wishes and gameplay tips. Both use only the caller's
// name ({STR_VAR_1}); {PLAYER} expands via StringExpandPlaceholders.
static const struct MatchCallText sMatchCallBirthdayTexts[] =
{
    { .text = MatchCall_BirthdayText1, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_BirthdayText2, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_BirthdayText3, .stringVarFuncIds = STRS_NORMAL_MSG },
};

// One tip per ability that actually changes something outside battle in this
// repo. Verified against the implementation, not against vanilla:
//   egg cycles   egg_hatch.c GetEggCyclesToSubtract (any party slot)
//   pickup       battle_script_commands.c (post-battle, any party slot)
//   encounters   wild_encounter.c
//   fishing      field_player_avatar.c
//   held items   pokemon.c SetWildMonHeldItem
//   ambient cry  overworld.c
//   cut radius   fldeff_cut.c
static const struct MatchCallText sMatchCallAbilityTipTexts[] =
{
    { .text = MatchCall_AbilityTip_LightningRod,  .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_AbilityTip_Synchronize,   .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_AbilityTip_CuteCharm,     .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_AbilityTip_MagnetPull,    .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_AbilityTip_EncounterRate, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_AbilityTip_KeenEye,       .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_AbilityTip_Hustle,        .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_AbilityTip_SuctionCups,   .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_AbilityTip_CompoundEyes,  .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_AbilityTip_FlameBody,     .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_AbilityTip_Pickup,        .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_AbilityTip_Swarm,         .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_AbilityTip_HyperCutter,   .stringVarFuncIds = STRS_NORMAL_MSG },
};

// Postgame legendary rumours. Offered only while `gate` is set and NEITHER
// `done` flag is set, so a rumour stops the moment that hunt is resolved --
// either by catching it or by knocking it out. For the ticket islands the
// "done" pair is (ticket received, legendary resolved), because once the
// player holds the ticket the rumour about how to get one is stale.
#define MATCH_CALL_NO_FLAG 0xFFFF

struct MatchCallRumor
{
    struct MatchCallText text;
    u16 gate;
    u16 done1;
    u16 done2;
};

#define RUMOR(txt, gate, d1, d2) { { txt, STRS_NORMAL_MSG }, gate, d1, d2 }

static const struct MatchCallRumor sMatchCallLegendRumors[] =
{
    RUMOR(MatchCall_Legend_Suicune,   FLAG_SYS_GAME_CLEAR,               FLAG_CAUGHT_SUICUNE,         FLAG_DEFEATED_SUICUNE),
    RUMOR(MatchCall_Legend_Raikou,    FLAG_SYS_GAME_CLEAR,               FLAG_CAUGHT_RAIKOU,          FLAG_DEFEATED_RAIKOU),
    RUMOR(MatchCall_Legend_Entei,     FLAG_SYS_GAME_CLEAR,               FLAG_CAUGHT_ENTEI,           FLAG_DEFEATED_ENTEI),
    RUMOR(MatchCall_Legend_Mewtwo,    FLAG_COLLECTED_ALL_SILVER_SYMBOLS, FLAG_CAUGHT_MEWTWO,          FLAG_DEFEATED_MEWTWO),
    RUMOR(MatchCall_Legend_Celebi,    FLAG_IS_CHAMPION,                  FLAG_BATTLED_CELEBI,         MATCH_CALL_NO_FLAG),
    RUMOR(MatchCall_Legend_Deoxys,    FLAG_IS_CHAMPION,                  FLAG_RECEIVED_AURORA_TICKET, FLAG_BATTLED_DEOXYS),
    RUMOR(MatchCall_Legend_NavelRock, FLAG_IS_CHAMPION,                  FLAG_RECEIVED_MYSTIC_TICKET, FLAG_DEFEATED_LUGIA),
    RUMOR(MatchCall_Legend_Mew,       FLAG_IS_CHAMPION,                  FLAG_RECEIVED_OLD_SEA_MAP,   FLAG_CAUGHT_MEW),
};

// The three birds share one rumour, so unlike the table above it has to stay
// available until ALL of them are resolved, not just the first.
static const u16 sMatchCallBirdDoneFlags[] =
{
    FLAG_CAUGHT_ARTICUNO, FLAG_DEFEATED_ARTICUNO,
    FLAG_CAUGHT_ZAPDOS,   FLAG_DEFEATED_ZAPDOS,
    FLAG_CAUGHT_MOLTRES,  FLAG_DEFEATED_MOLTRES,
};

static const struct MatchCallText sMatchCallLegendBirdsText =
    { .text = MatchCall_Legend_Birds, .stringVarFuncIds = STRS_NORMAL_MSG };
// Jirachi's gate is a var, not a flag, so it sits outside the table.
static const struct MatchCallText sMatchCallLegendJirachiText =
    { .text = MatchCall_Legend_Jirachi, .stringVarFuncIds = STRS_NORMAL_MSG };
static const struct MatchCallText sMatchCallQuestMuseumTicketText =
    { .text = MatchCall_Quest_MuseumTicket, .stringVarFuncIds = STRS_NORMAL_MSG };
static const struct MatchCallText sMatchCallMixedSightingText =
    { .text = MatchCall_Mixed_Sighting, .stringVarFuncIds = STRS_NORMAL_MSG };
static const struct MatchCallText sMatchCallMixedRareCatchText =
    { .text = MatchCall_Mixed_RareCatch, .stringVarFuncIds = STRS_NORMAL_MSG };
static const struct MatchCallText sMatchCallMixedSecretBaseTexts[2] =
{
    [MALE]   = { .text = MatchCall_Mixed_SecretBaseM, .stringVarFuncIds = STRS_NORMAL_MSG },
    [FEMALE] = { .text = MatchCall_Mixed_SecretBaseF, .stringVarFuncIds = STRS_NORMAL_MSG },
};
static const struct MatchCallText sMatchCallQuestFashionistaText =
    { .text = MatchCall_Quest_Fashionista, .stringVarFuncIds = STRS_NORMAL_MSG };

// The four Fashionista NPCs, one per Pokemon Center 1F. The call names
// whichever city it picks so the player knows where to go.
static const u16 sFashionistaCityMapSecs[] =
{
    MAPSEC_SLATEPORT_CITY,
    MAPSEC_LAVARIDGE_TOWN,
    MAPSEC_LILYCOVE_CITY,
    MAPSEC_SOOTOPOLIS_CITY,
};

// The Weather Institute's postgame beast hunts, in (caught, defeated) pairs.
static const u16 sWeatherInstituteBeastFlags[] =
{
    FLAG_CAUGHT_SUICUNE, FLAG_DEFEATED_SUICUNE,
    FLAG_CAUGHT_RAIKOU,  FLAG_DEFEATED_RAIKOU,
    FLAG_CAUGHT_ENTEI,   FLAG_DEFEATED_ENTEI,
};

static const struct MatchCallText sMatchCallGeneralTipTexts[] =
{
    { .text = MatchCall_Tip_FanClubBirthday, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_Tip_Pokerus,         .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_Tip_TradedExp,       .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_Tip_SecretBase,      .stringVarFuncIds = STRS_NORMAL_MSG },
};

// Late night (22:00-03:59) vs early morning (04:00-06:59).
static const struct MatchCallText sMatchCallLateNightTexts[] =
{
    { .text = MatchCall_Time_LateNight1, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_Time_LateNight2, .stringVarFuncIds = STRS_NORMAL_MSG },
};

static const struct MatchCallText sMatchCallEarlyMorningTexts[] =
{
    { .text = MatchCall_Time_EarlyMorning1, .stringVarFuncIds = STRS_NORMAL_MSG },
    { .text = MatchCall_Time_EarlyMorning2, .stringVarFuncIds = STRS_NORMAL_MSG },
};

// The extra vars in these are filled by hand before BuildMatchCallString,
// so the func ids only ask for the caller's name.
static const struct MatchCallText sMatchCallLoreSouthernIslandText =
    { .text = MatchCall_Lore_SouthernIsland, .stringVarFuncIds = STRS_NORMAL_MSG };
static const struct MatchCallText sMatchCallLoreDragonLadyText =
    { .text = MatchCall_Lore_DragonLady, .stringVarFuncIds = STRS_NORMAL_MSG };
static const struct MatchCallText sMatchCallOutbreakText =
    { .text = MatchCall_Rare_Outbreak, .stringVarFuncIds = STRS_NORMAL_MSG };
static const struct MatchCallText sMatchCallOutbreakNationalText =
    { .text = MatchCall_Rare_OutbreakNational, .stringVarFuncIds = STRS_NORMAL_MSG };
static const struct MatchCallText sMatchCallRareCatchLegendaryText =
    { .text = MatchCall_RareCatch_Legendary, .stringVarFuncIds = STRS_NORMAL_MSG };
static const struct MatchCallText sMatchCallRareCatchShinyText =
    { .text = MatchCall_RareCatch_Shiny, .stringVarFuncIds = STRS_NORMAL_MSG };

// Family B. Indexed by SPECIAL_CALL_*; the caller's name is either baked
// into the text or written into gStringVar1 by BuildSpecialCallString.
static const u8 *const sSpecialCallTexts[SPECIAL_CALL_COUNT] =
{
    [SPECIAL_CALL_VERITAS]        = MatchCall_Veritas,
    [SPECIAL_CALL_HEART_RIVAL]    = MatchCall_Heart_Rival,
    [SPECIAL_CALL_HEART_STEVEN]   = MatchCall_Heart_Steven,
    [SPECIAL_CALL_HEART_WALLACE]  = MatchCall_Heart_Wallace,
    // Placeholder: BuildSpecialCallString picks the actual Brain's own line
    // from sFrontierBrainTexts once it knows which facility.
    [SPECIAL_CALL_FRONTIER_BRAIN] = MatchCall_Frontier_Anabel,
    [SPECIAL_CALL_MOM]            = MatchCall_Leisure_Mom1,
    [SPECIAL_CALL_BIRCH]          = MatchCall_Leisure_Birch1,
    [SPECIAL_CALL_RIVAL]          = MatchCall_Leisure_Rival1,
    [SPECIAL_CALL_STEVEN]         = MatchCall_Leisure_Steven1,
    [SPECIAL_CALL_RIVAL_WEATHER]  = MatchCall_Quest_WeatherResearch,
    [SPECIAL_CALL_RIVAL_MIXED]    = MatchCall_Mixed_RivalMet,
};

// Second variant for the leisure callers, picked by a coin flip.
static const u8 *const sSpecialCallTextsAlt[SPECIAL_CALL_COUNT] =
{
    [SPECIAL_CALL_MOM]    = MatchCall_Leisure_Mom2,
    [SPECIAL_CALL_BIRCH]  = MatchCall_Leisure_Birch2,
    [SPECIAL_CALL_RIVAL]  = MatchCall_Leisure_Rival2,
    [SPECIAL_CALL_STEVEN] = MatchCall_Leisure_Steven2,
};

// Where each secret base entrance physically sits. The save only stores
// secretBaseId; the route is in the map's bg_events, which C can't reach at
// runtime, so this table is generated from data/maps/*/map.json. Every base
// interior reports MAPSEC_SECRET_BASE, which is why GetMapName can't be used
// for a friend's base.
struct SecretBaseLocation
{
    u8 baseId;
    u8 mapSec;
};

static const struct SecretBaseLocation sSecretBaseLocations[] =
{
    { SECRET_BASE_RED_CAVE1_1,    MAPSEC_ROUTE_118 },
    { SECRET_BASE_RED_CAVE1_2,    MAPSEC_ROUTE_125 },
    { SECRET_BASE_RED_CAVE1_3,    MAPSEC_ROUTE_113 },
    { SECRET_BASE_RED_CAVE2_1,    MAPSEC_ROUTE_118 },
    { SECRET_BASE_RED_CAVE2_2,    MAPSEC_ROUTE_121 },
    { SECRET_BASE_RED_CAVE2_3,    MAPSEC_ROUTE_111 },
    { SECRET_BASE_RED_CAVE3_1,    MAPSEC_ROUTE_115 },
    { SECRET_BASE_RED_CAVE3_2,    MAPSEC_ROUTE_121 },
    { SECRET_BASE_RED_CAVE3_3,    MAPSEC_ROUTE_119 },
    { SECRET_BASE_RED_CAVE4_1,    MAPSEC_ROUTE_127 },
    { SECRET_BASE_RED_CAVE4_2,    MAPSEC_ROUTE_125 },
    { SECRET_BASE_RED_CAVE4_3,    MAPSEC_ROUTE_111 },
    { SECRET_BASE_BROWN_CAVE1_1,  MAPSEC_ROUTE_114 },
    { SECRET_BASE_BROWN_CAVE1_2,  MAPSEC_ROUTE_115 },
    { SECRET_BASE_BROWN_CAVE1_3,  MAPSEC_ROUTE_115 },
    { SECRET_BASE_BROWN_CAVE2_1,  MAPSEC_ROUTE_114 },
    { SECRET_BASE_BROWN_CAVE2_2,  MAPSEC_ROUTE_115 },
    { SECRET_BASE_BROWN_CAVE2_3,  MAPSEC_ROUTE_115 },
    { SECRET_BASE_BROWN_CAVE3_1,  MAPSEC_ROUTE_114 },
    { SECRET_BASE_BROWN_CAVE3_2,  MAPSEC_ROUTE_115 },
    { SECRET_BASE_BROWN_CAVE3_3,  MAPSEC_ROUTE_115 },
    { SECRET_BASE_BROWN_CAVE4_1,  MAPSEC_ROUTE_114 },
    { SECRET_BASE_BROWN_CAVE4_2,  MAPSEC_ROUTE_115 },
    { SECRET_BASE_BROWN_CAVE4_3,  MAPSEC_ROUTE_114 },
    { SECRET_BASE_BLUE_CAVE1_1,   MAPSEC_ROUTE_116 },
    { SECRET_BASE_BLUE_CAVE1_2,   MAPSEC_ROUTE_123 },
    { SECRET_BASE_BLUE_CAVE1_3,   MAPSEC_ROUTE_123 },
    { SECRET_BASE_BLUE_CAVE2_1,   MAPSEC_ROUTE_116 },
    { SECRET_BASE_BLUE_CAVE2_2,   MAPSEC_ROUTE_123 },
    { SECRET_BASE_BLUE_CAVE2_3,   MAPSEC_ROUTE_120 },
    { SECRET_BASE_BLUE_CAVE3_1,   MAPSEC_ROUTE_120 },
    { SECRET_BASE_BLUE_CAVE3_2,   MAPSEC_ROUTE_116 },
    { SECRET_BASE_BLUE_CAVE3_3,   MAPSEC_ROUTE_119 },
    { SECRET_BASE_BLUE_CAVE4_1,   MAPSEC_ROUTE_120 },
    { SECRET_BASE_BLUE_CAVE4_2,   MAPSEC_ROUTE_116 },
    { SECRET_BASE_BLUE_CAVE4_3,   MAPSEC_ROUTE_119 },
    { SECRET_BASE_YELLOW_CAVE1_1, MAPSEC_ROUTE_111 },
    { SECRET_BASE_YELLOW_CAVE1_2, MAPSEC_ROUTE_118 },
    { SECRET_BASE_YELLOW_CAVE1_3, MAPSEC_ROUTE_127 },
    { SECRET_BASE_YELLOW_CAVE2_1, MAPSEC_ROUTE_111 },
    { SECRET_BASE_YELLOW_CAVE2_2, MAPSEC_ROUTE_125 },
    { SECRET_BASE_YELLOW_CAVE2_3, MAPSEC_ROUTE_115 },
    { SECRET_BASE_YELLOW_CAVE3_1, MAPSEC_ROUTE_111 },
    { SECRET_BASE_YELLOW_CAVE3_2, MAPSEC_ROUTE_127 },
    { SECRET_BASE_YELLOW_CAVE3_3, MAPSEC_ROUTE_127 },
    { SECRET_BASE_YELLOW_CAVE4_1, MAPSEC_ROUTE_127 },
    { SECRET_BASE_YELLOW_CAVE4_2, MAPSEC_ROUTE_125 },
    { SECRET_BASE_YELLOW_CAVE4_3, MAPSEC_ROUTE_111 },
    { SECRET_BASE_TREE1_1,        MAPSEC_ROUTE_111 },
    { SECRET_BASE_TREE1_2,        MAPSEC_ROUTE_121 },
    { SECRET_BASE_TREE1_3,        MAPSEC_ROUTE_118 },
    { SECRET_BASE_TREE1_4,        MAPSEC_ROUTE_111 },
    { SECRET_BASE_TREE2_1,        MAPSEC_ROUTE_118 },
    { SECRET_BASE_TREE2_2,        MAPSEC_ROUTE_121 },
    { SECRET_BASE_TREE2_3,        MAPSEC_ROUTE_119 },
    { SECRET_BASE_TREE2_4,        MAPSEC_ROUTE_115 },
    { SECRET_BASE_TREE3_1,        MAPSEC_ROUTE_110 },
    { SECRET_BASE_TREE3_2,        MAPSEC_ROUTE_114 },
    { SECRET_BASE_TREE3_3,        MAPSEC_ROUTE_115 },
    { SECRET_BASE_TREE4_1,        MAPSEC_ROUTE_110 },
    { SECRET_BASE_TREE4_2,        MAPSEC_ROUTE_114 },
    { SECRET_BASE_TREE4_3,        MAPSEC_ROUTE_119 },
    { SECRET_BASE_SHRUB1_1,       MAPSEC_ROUTE_119 },
    { SECRET_BASE_SHRUB1_2,       MAPSEC_ROUTE_119 },
    { SECRET_BASE_SHRUB1_3,       MAPSEC_ROUTE_120 },
    { SECRET_BASE_SHRUB1_4,       MAPSEC_ROUTE_120 },
    { SECRET_BASE_SHRUB2_1,       MAPSEC_ROUTE_119 },
    { SECRET_BASE_SHRUB2_2,       MAPSEC_ROUTE_119 },
    { SECRET_BASE_SHRUB2_3,       MAPSEC_ROUTE_120 },
    { SECRET_BASE_SHRUB3_1,       MAPSEC_ROUTE_119 },
    { SECRET_BASE_SHRUB3_2,       MAPSEC_ROUTE_120 },
    { SECRET_BASE_SHRUB3_3,       MAPSEC_ROUTE_119 },
    { SECRET_BASE_SHRUB4_1,       MAPSEC_ROUTE_119 },
    { SECRET_BASE_SHRUB4_2,       MAPSEC_ROUTE_120 },
    { SECRET_BASE_SHRUB4_3,       MAPSEC_ROUTE_119 },
};

// NOTE: all three tables below are indexed in this FILE'S facility space,
// where MATCH_CALL_PIKE/MATCH_CALL_FACTORY are deliberately swapped relative
// to the real FRONTIER_FACILITY_* values (see the comment at the top of this
// file). They must stay consistent with sBattleFrontierFacilityNames[] and
// with the index GetFrontierStreakInfo() switches on.
static const u16 sFrontierBrainTrainerIds[NUM_FRONTIER_FACILITIES] =
{
    [FRONTIER_FACILITY_TOWER]   = TRAINER_ANABEL,
    [FRONTIER_FACILITY_DOME]    = TRAINER_TUCKER,
    [FRONTIER_FACILITY_PALACE]  = TRAINER_SPENSER,
    [FRONTIER_FACILITY_ARENA]   = TRAINER_GRETA,
    [MATCH_CALL_PIKE]           = TRAINER_LUCY,
    [MATCH_CALL_FACTORY]        = TRAINER_NOLAND,
    [FRONTIER_FACILITY_PYRAMID] = TRAINER_BRANDON,
};

// A Brain only rings once the player has beaten them for the Silver Symbol.
// GetPlayerSymbolCountForFacility() can't be reused here: it indexes by the
// REAL facility id, and this file's ids are swapped for Pike/Factory.
static const u16 sFrontierSilverSymbolFlags[NUM_FRONTIER_FACILITIES] =
{
    [FRONTIER_FACILITY_TOWER]   = FLAG_SYS_TOWER_SILVER,
    [FRONTIER_FACILITY_DOME]    = FLAG_SYS_DOME_SILVER,
    [FRONTIER_FACILITY_PALACE]  = FLAG_SYS_PALACE_SILVER,
    [FRONTIER_FACILITY_ARENA]   = FLAG_SYS_ARENA_SILVER,
    [MATCH_CALL_PIKE]           = FLAG_SYS_PIKE_SILVER,
    [MATCH_CALL_FACTORY]        = FLAG_SYS_FACTORY_SILVER,
    [FRONTIER_FACILITY_PYRAMID] = FLAG_SYS_PYRAMID_SILVER,
};

// One voice per Brain rather than a shared line.
static const u8 *const sFrontierBrainTexts[NUM_FRONTIER_FACILITIES] =
{
    [FRONTIER_FACILITY_TOWER]   = MatchCall_Frontier_Anabel,
    [FRONTIER_FACILITY_DOME]    = MatchCall_Frontier_Tucker,
    [FRONTIER_FACILITY_PALACE]  = MatchCall_Frontier_Spenser,
    [FRONTIER_FACILITY_ARENA]   = MatchCall_Frontier_Greta,
    [MATCH_CALL_PIKE]           = MatchCall_Frontier_Lucy,
    [MATCH_CALL_FACTORY]        = MatchCall_Frontier_Noland,
    [FRONTIER_FACILITY_PYRAMID] = MatchCall_Frontier_Brandon,
};

static const struct MatchCallText *const sMatchCallGeneralTopics[] =
{
    [GEN_TOPIC_PERSONAL - 1]      = sMatchCallPersonalizedTexts,
    [GEN_TOPIC_STREAK - 1]        = sMatchCallBattleFrontierStreakTexts,
    [GEN_TOPIC_STREAK_RECORD - 1] = sMatchCallBattleFrontierRecordStreakTexts,
    [GEN_TOPIC_B_DOME - 1]        = sMatchCallBattleDomeTexts,
    [GEN_TOPIC_B_PIKE - 1]        = sMatchCallBattlePikeTexts,
    [GEN_TOPIC_B_PYRAMID - 1]     = sMatchCallBattlePyramidTexts,
};

extern const u8 gBirchDexRatingText_AreYouCurious[];
extern const u8 gBirchDexRatingText_SoYouveSeenAndCaught[];
extern const u8 gBirchDexRatingText_OnANationwideBasis[];

void InitMatchCallCounters(void)
{
    RtcCalcLocalTime();
    sMatchCallState.minutes = GetCurrentTotalMinutes(&gLocalTime) + 10;
    sMatchCallState.stepCounter = 0;
}

static u32 GetCurrentTotalMinutes(struct Time *time)
{
    return time->days * 24 * 60 + time->hours * 60 + time->minutes;
}

static bool32 UpdateMatchCallMinutesCounter(void)
{
    int curMinutes;
    RtcCalcLocalTime();
    curMinutes = GetCurrentTotalMinutes(&gLocalTime);
    if (sMatchCallState.minutes > curMinutes || curMinutes - sMatchCallState.minutes > 9)
    {
        sMatchCallState.minutes = curMinutes;
        sMatchCallState.frozenRtcChecks = 0;
        return TRUE;
    }

    // Veritas: on carts with a dead RTC battery (or bootlegs whose clock
    // never ticks), gLocalTime freezes, so the 10-minute gate above jams
    // shut PERMANENTLY and the PokeNav never rings again. This function
    // runs once per 10 steps; a working clock always crosses a minute
    // boundary within ~120 such checks (1200 steps), so zero movement for
    // that long means the clock is frozen. From then on, emulate the wait
    // with steps instead: one gate pass per 600 steps. Working-RTC players
    // are unaffected (their delta goes nonzero long before 120 checks).
    if (curMinutes == sMatchCallState.minutes)
    {
        if (++sMatchCallState.frozenRtcChecks >= 120)
        {
            sMatchCallState.frozenRtcChecks = 60;
            return TRUE;
        }
    }
    else
    {
        sMatchCallState.frozenRtcChecks = 0;
    }

    return FALSE;
}

static bool32 CheckMatchCallChance(void)
{
    int callChance = 1;

    // Veritas: never let the birthday call miss its roll.
    if (IsPlayerBirthdayToday() && !FlagGet(FLAG_DAILY_BIRTHDAY_MATCH_CALL))
        return TRUE;

    if (!GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG) && GetMonAbility(&gPlayerParty[0]) == ABILITY_LIGHTNING_ROD)
        callChance = 2;

    // Veritas: tuned so the PokeNav averages ~2 calls per hour of walking.
    // The 10-minute gate caps attempts at 6/hour and a lost roll consumes
    // the whole window, so this has to be generous: 50% base, 75% with a
    // lead Lightning Rod (vanilla was 30%/60%).
    if (Random() % 4 < callChance + 1)
        return TRUE;
    else
        return FALSE;
}

static bool32 MapAllowsMatchCall(void)
{
    if (!Overworld_MapTypeAllowsTeleportAndFly(gMapHeader.mapType) || gMapHeader.regionMapSectionId == MAPSEC_SAFARI_ZONE)
        return FALSE;

    if (gMapHeader.regionMapSectionId == MAPSEC_SOOTOPOLIS_CITY
     && FlagGet(FLAG_HIDE_SOOTOPOLIS_CITY_RAYQUAZA) == TRUE
     && FlagGet(FLAG_NEVER_SET_0x0DC) == FALSE)
        return FALSE;

    if (gMapHeader.regionMapSectionId == MAPSEC_MT_CHIMNEY
     && FlagGet(FLAG_MET_ARCHIE_METEOR_FALLS) == TRUE
     && FlagGet(FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY) == FALSE)
        return FALSE;

    return TRUE;
}

static bool32 UpdateMatchCallStepCounter(void)
{
    if (++sMatchCallState.stepCounter >= 10)
    {
        sMatchCallState.stepCounter = 0;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static bool32 SelectMatchCallTrainer(void)
{
    u32 matchCallId;
    u32 numRegistered = GetNumRegisteredNPCs();

    sMatchCallState.forcedCallType = FORCED_CALL_NONE;
    sMatchCallState.triggeredFromScript = FALSE;

    // Veritas: family B callers aren't in the rematch table, so they're
    // offered first and don't need any registered trainer to exist.
    if (TrySelectSpecialCall())
    {
        sMatchCallState.forcedCallType = FORCED_CALL_SPECIAL;
        return TRUE;
    }

    if (numRegistered == 0)
        return FALSE;

    sMatchCallState.trainerId = GetActiveMatchCallTrainerId(Random() % numRegistered);
    if (sMatchCallState.trainerId == REMATCH_TABLE_ENTRIES)
        return FALSE;

    // Veritas: on the player's birthday, the first call of the day is a
    // birthday wish from whichever registered trainer got picked. It skips
    // the rematch-eligibility gate entirely (the daily flag auto-clears at
    // midnight, so it fires once per birthday).
    if (IsPlayerBirthdayToday() && !FlagGet(FLAG_DAILY_BIRTHDAY_MATCH_CALL))
    {
        sMatchCallState.forcedCallType = FORCED_CALL_BIRTHDAY;
        return TRUE;
    }

    matchCallId = GetTrainerMatchCallId(sMatchCallState.trainerId);
    if ((TrainerIsEligibleForRematch(matchCallId) && GetRematchTrainerLocation(matchCallId) == gMapHeader.regionMapSectionId)
     || ShouldTrainerRequestBattle(matchCallId))
        return TRUE;

    // Veritas: vanilla dropped the call entirely here, which is why players
    // barely got any -- before 5 badges ShouldTrainerRequestBattle is always
    // FALSE, so a call required a rematch-ready trainer on the CURRENT map.
    // Instead, half of these failed picks become casual calls: idle chatter,
    // a tip, a rumour, a sighting (see SelectCasualMatchCallText).
    if (Random() % 2)
    {
        sMatchCallState.forcedCallType = FORCED_CALL_CASUAL;
        return TRUE;
    }

    return FALSE;
}

static u32 GetNumRegisteredNPCs(void)
{
    u32 i, count;
    for (i = 0, count = 0; i < REMATCH_SPECIAL_TRAINER_START; i++)
    {
        if (FlagGet(FLAG_MATCH_CALL_REGISTERED + i))
            count++;
    }

    return count;
}

static u32 GetActiveMatchCallTrainerId(u32 activeMatchCallId)
{
    u32 i;
    for (i = 0; i < REMATCH_SPECIAL_TRAINER_START; i++)
    {
        if (FlagGet(FLAG_MATCH_CALL_REGISTERED + i))
        {
            if (!activeMatchCallId)
                return gRematchTable[i].trainerIds[0];

            activeMatchCallId--;
        }
    }

    return REMATCH_TABLE_ENTRIES;
}

/*
    From the function calls below, a call can only be triggered...
    - If the player has match call
    - Every 10th step
    - Every 10 minutes (Veritas: if the cart's RTC is frozen, which would
      jam this gate forever, the wait is emulated with steps instead)
    - 1/4 of the time (1/2 with a lead Lightning Rod Pokémon; always, for
      the once-per-day birthday call). Vanilla was 3/10 and 3/5.
    - If in a valid outdoor map (not Safari Zone, not underwater, not Mt Chimney with Team Magma, not Sootopolis with legendaries)
    - If a caller is selected. Veritas: a fixed caller who isn't in the
      rematch table (Mom, Steven, a Frontier Brain, Veritas himself) is
      offered first; then, on the player's birthday, the pick always
      succeeds; otherwise a third of the picks that fail vanilla's rematch
      gate become casual calls instead of silently dropping.
      See docs/POKENAV_CALLS.md.
*/
bool32 TryStartMatchCall(void)
{
    if (FlagGet(FLAG_HAS_MATCH_CALL)
        && UpdateMatchCallStepCounter()
        && UpdateMatchCallMinutesCounter()
        && CheckMatchCallChance()
        && MapAllowsMatchCall()
        && SelectMatchCallTrainer())
    {
        StartMatchCall();
        return TRUE;
    }

    return FALSE;
}

void StartMatchCallFromScript(const u8 *message)
{
    sMatchCallState.triggeredFromScript = TRUE;
    StartMatchCall();
}

bool32 IsMatchCallTaskActive(void)
{
    return FuncIsActiveTask(ExecuteMatchCall);
}

static void StartMatchCall(void)
{
    if (!sMatchCallState.triggeredFromScript)
    {
        LockPlayerFieldControls();
        FreezeObjectEvents();
        PlayerFreeze();
        StopPlayerAvatar();
    }

    PlaySE(SE_POKENAV_CALL);
    CreateTask(ExecuteMatchCall, 1);
}

static const u16 sMatchCallWindow_Pal[] = INCBIN_U16("graphics/pokenav/match_call/window.gbapal");
static const u8 sMatchCallWindow_Gfx[] = INCBIN_U8("graphics/pokenav/match_call/window.4bpp");
static const u16 sPokenavIcon_Pal[] = INCBIN_U16("graphics/pokenav/match_call/nav_icon.gbapal");
static const u32 sPokenavIcon_Gfx[] = INCBIN_U32("graphics/pokenav/match_call/nav_icon.4bpp.lz");

static const u8 sText_PokenavCallEllipsis[] = _("………………\p");

#define tState      data[0]
#define tCryState   data[1] // Veritas: only used by his mystery call
#define tWindowId   data[2]
#define tIconTaskId data[5]

static bool32 (*const sMatchCallTaskFuncs[])(u8) =
{
    MatchCall_LoadGfx,
    MatchCall_DrawWindow,
    MatchCall_ReadyIntro,
    MatchCall_SlideWindowIn,
    MatchCall_PrintIntro,
    MatchCall_PrintMessage,
    MatchCall_SlideWindowOut,
    MatchCall_EndCall,
};

static void ExecuteMatchCall(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (sMatchCallTaskFuncs[tState](taskId))
    {
        tState++;
        data[1] = 0; // Never read
        if ((u16)tState > 7)
            DestroyTask(taskId);
    }
}

static const struct WindowTemplate sMatchCallTextWindow =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 15,
    .width = 28,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x200
};

#define TILE_MC_WINDOW    0x270
#define TILE_POKENAV_ICON 0x279

static bool32 MatchCall_LoadGfx(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    tWindowId = AddWindow(&sMatchCallTextWindow);
    if (tWindowId == WINDOW_NONE)
    {
        DestroyTask(taskId);
        return FALSE;
    }

    if (LoadBgTiles(0, sMatchCallWindow_Gfx, sizeof(sMatchCallWindow_Gfx), TILE_MC_WINDOW) == 0xFFFF)
    {
        RemoveWindow(tWindowId);
        DestroyTask(taskId);
        return FALSE;
    }

    if (!DecompressAndCopyTileDataToVram(0, sPokenavIcon_Gfx, 0, TILE_POKENAV_ICON, 0))
    {
        RemoveWindow(tWindowId);
        DestroyTask(taskId);
        return FALSE;
    }

    FillWindowPixelBuffer(tWindowId, PIXEL_FILL(8));
    LoadPalette(sMatchCallWindow_Pal, BG_PLTT_ID(14), sizeof(sMatchCallWindow_Pal));
    LoadPalette(sPokenavIcon_Pal, BG_PLTT_ID(15), sizeof(sPokenavIcon_Pal));
    ChangeBgY(0, -0x2000, BG_COORD_SET);
    return TRUE;
}

static bool32 MatchCall_DrawWindow(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (FreeTempTileDataBuffersIfPossible())
        return FALSE;

    PutWindowTilemap(tWindowId);
    DrawMatchCallTextBoxBorder_Internal(tWindowId, TILE_MC_WINDOW, 14);
    WriteSequenceToBgTilemapBuffer(0, (0xF << 12) | TILE_POKENAV_ICON, 1, 15, 4, 4, 17, 1);
    tIconTaskId = CreateTask(Task_SpinPokenavIcon, 10);
    CopyWindowToVram(tWindowId, COPYWIN_GFX);
    CopyBgTilemapBufferToVram(0);
    return TRUE;
}

static bool32 MatchCall_ReadyIntro(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (!IsDma3ManagerBusyWithBgCopy())
    {
        // Note that "..." is not printed yet, just readied
        InitMatchCallTextPrinter(tWindowId, sText_PokenavCallEllipsis);
        return TRUE;
    }

    return FALSE;
}

static bool32 MatchCall_SlideWindowIn(u8 taskId)
{
    if (ChangeBgY(0, 0x600, BG_COORD_ADD) >= 0)
    {
        ChangeBgY(0, 0, BG_COORD_SET);
        return TRUE;
    }

    return FALSE;
}

static bool32 MatchCall_PrintIntro(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (!RunMatchCallTextPrinter(tWindowId))
    {
        FillWindowPixelBuffer(tWindowId, PIXEL_FILL(8));

        // Ready the message
        if (!sMatchCallState.triggeredFromScript)
            SelectMatchCallMessage(sMatchCallState.trainerId, gStringVar4);
        InitMatchCallTextPrinter(tWindowId, gStringVar4);
        return TRUE;
    }

    return FALSE;
}

static bool32 MatchCall_PrintMessage(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    // Veritas: his call signs off with a cry rather than a goodbye. Once the
    // last line has printed the cry starts on its own and the line drops as
    // soon as it finishes -- no button press, so it can't be skipped past.
    if (sMatchCallState.veritasCallActive)
    {
        if (RunMatchCallTextPrinter(tWindowId))
            return FALSE;

        switch (tCryState)
        {
        case 0:
            PlayCry_Normal(SPECIES_LUGIA, 0);
            tCryState = 1;
            return FALSE;
        case 1:
            if (IsCryFinished())
                tCryState = 2;
            return FALSE;
        default:
            sMatchCallState.veritasCallActive = FALSE;
            FillWindowPixelBuffer(tWindowId, PIXEL_FILL(8));
            CopyWindowToVram(tWindowId, COPYWIN_GFX);
            PlaySE(SE_POKENAV_HANG_UP);
            return TRUE;
        }
    }

    if (!RunMatchCallTextPrinter(tWindowId) && !IsSEPlaying() && JOY_NEW(A_BUTTON | B_BUTTON))
    {
        FillWindowPixelBuffer(tWindowId, PIXEL_FILL(8));
        CopyWindowToVram(tWindowId, COPYWIN_GFX);
        PlaySE(SE_POKENAV_HANG_UP);
        return TRUE;
    }

    return FALSE;
}

static bool32 MatchCall_SlideWindowOut(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (ChangeBgY(0, 0x600, BG_COORD_SUB) <= -0x2000)
    {
        FillBgTilemapBufferRect_Palette0(0, 0, 0, 14, 30, 6);
        DestroyTask(tIconTaskId);
        RemoveWindow(tWindowId);
        CopyBgTilemapBufferToVram(0);
        return TRUE;
    }

    return FALSE;
}

static bool32 MatchCall_EndCall(u8 taskId)
{
    u8 playerObjectId;
    if (!IsDma3ManagerBusyWithBgCopy() && !IsSEPlaying())
    {
        ChangeBgY(0, 0, BG_COORD_SET);
        if (!sMatchCallState.triggeredFromScript)
        {
            LoadMessageBoxAndBorderGfx();
            playerObjectId = GetObjectEventIdByLocalIdAndMap(OBJ_EVENT_ID_PLAYER, 0, 0);
            ObjectEventClearHeldMovementIfFinished(&gObjectEvents[playerObjectId]);
            ScriptMovement_UnfreezeObjectEvents();
            UnfreezeObjectEvents();
            UnlockPlayerFieldControls();
        }

        return TRUE;
    }

    return FALSE;
}

static void DrawMatchCallTextBoxBorder_Internal(u32 windowId, u32 tileOffset, u32 paletteId)
{
    int bg, x, y, width, height;
    int tileNum;

    bg = GetWindowAttribute(windowId, WINDOW_BG);
    x = GetWindowAttribute(windowId, WINDOW_TILEMAP_LEFT);
    y = GetWindowAttribute(windowId, WINDOW_TILEMAP_TOP);
    width = GetWindowAttribute(windowId, WINDOW_WIDTH);
    height = GetWindowAttribute(windowId, WINDOW_HEIGHT);
    tileNum = tileOffset + GetBgAttribute(bg, BG_ATTR_BASETILE);

    FillBgTilemapBufferRect_Palette0(bg, ((paletteId << 12) & 0xF000) | (tileNum + 0), x - 1, y - 1, 1, 1);
    FillBgTilemapBufferRect_Palette0(bg, ((paletteId << 12) & 0xF000) | (tileNum + 1), x, y - 1, width, 1);
    FillBgTilemapBufferRect_Palette0(bg, ((paletteId << 12) & 0xF000) | (tileNum + 2), x + width, y - 1, 1, 1);
    FillBgTilemapBufferRect_Palette0(bg, ((paletteId << 12) & 0xF000) | (tileNum + 3), x - 1, y, 1, height);
    FillBgTilemapBufferRect_Palette0(bg, ((paletteId << 12) & 0xF000) | (tileNum + 4), x + width, y, 1, height);
    FillBgTilemapBufferRect_Palette0(bg, ((paletteId << 12) & 0xF000) | (tileNum + 5), x - 1, y + height, 1, 1);
    FillBgTilemapBufferRect_Palette0(bg, ((paletteId << 12) & 0xF000) | (tileNum + 6), x, y + height, width, 1);
    FillBgTilemapBufferRect_Palette0(bg, ((paletteId << 12) & 0xF000) | (tileNum + 7), x + width, y + height, 1, 1);
}

static void InitMatchCallTextPrinter(int windowId, const u8 *str)
{
    struct TextPrinterTemplate printerTemplate;
    printerTemplate.currentChar = str;
    printerTemplate.windowId = windowId;
    printerTemplate.fontId = FONT_NORMAL;
    printerTemplate.x = 32;
    printerTemplate.y = 1;
    printerTemplate.currentX = 32;
    printerTemplate.currentY = 1;
    printerTemplate.letterSpacing = 0;
    printerTemplate.lineSpacing = 0;
    printerTemplate.unk = 0;
    printerTemplate.fgColor = TEXT_DYNAMIC_COLOR_1;
    printerTemplate.bgColor = TEXT_COLOR_BLUE;
    printerTemplate.shadowColor = TEXT_DYNAMIC_COLOR_5;
    gTextFlags.useAlternateDownArrow = FALSE;

    AddTextPrinter(&printerTemplate, GetPlayerTextSpeedDelay(), NULL);
}

static bool32 RunMatchCallTextPrinter(int windowId)
{
    if (JOY_HELD(A_BUTTON))
        gTextFlags.canABSpeedUpPrint = TRUE;
    else
        gTextFlags.canABSpeedUpPrint = FALSE;

    RunTextPrinters();
    return IsTextPrinterActive(windowId);
}

#define tTimer     data[0]
#define tSpinStage data[1]
#define tTileNum   data[2]

static void Task_SpinPokenavIcon(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (++tTimer > 8)
    {
        tTimer = 0;
        if (++tSpinStage > 7)
            tSpinStage = 0;

        tTileNum = (tSpinStage * 16) + TILE_POKENAV_ICON;
        WriteSequenceToBgTilemapBuffer(0, tTileNum | ~0xFFF, 1, 15, 4, 4, 17, 1);
        CopyBgTilemapBufferToVram(0);
    }
}

#undef tTimer
#undef tSpinStage
#undef tTileNum

static bool32 TrainerIsEligibleForRematch(int matchCallId)
{
    return gSaveBlock1Ptr->trainerRematches[matchCallId] > 0;
}

static u16 GetRematchTrainerLocation(int matchCallId)
{
    const struct MapHeader *mapHeader = Overworld_GetMapHeaderByGroupAndId(gRematchTable[matchCallId].mapGroup, gRematchTable[matchCallId].mapNum);
    return mapHeader->regionMapSectionId;
}

static u32 GetNumRematchTrainersFought(void)
{
    u32 i, count;
    for (i = 0, count = 0; i < REMATCH_SPECIAL_TRAINER_START; i++)
    {
        if (HasTrainerBeenFought(gRematchTable[i].trainerIds[0]))
            count++;
    }

    return count;
}

// Look through the rematch table for trainers that have been defeated once before.
// Return the index into the rematch table of the nth defeated trainer,
// or REMATCH_TABLE_ENTRIES if fewer than n rematch trainers have been defeated.
static u32 GetNthRematchTrainerFought(int n)
{
    u32 i, count;

    for (i = 0, count = 0; i < REMATCH_TABLE_ENTRIES; i++)
    {
        if (HasTrainerBeenFought(gRematchTable[i].trainerIds[0]))
        {
            if (count == n)
                return i;

            count++;
        }
    }

    return REMATCH_TABLE_ENTRIES;
}

bool32 SelectMatchCallMessage(int trainerId, u8 *str)
{
    u32 matchCallId;
    const struct MatchCallText *matchCallText;
    bool32 newRematchRequest = FALSE;

    sBattleFrontierStreakInfo.facilityId = 0;

    // Veritas: a family B caller has no entry in sMatchCallTrainers, so this
    // MUST come before GetTrainerMatchCallId -- that function scans the
    // table with no bound and never returns for an unknown trainerId.
    if (sMatchCallState.forcedCallType == FORCED_CALL_SPECIAL)
    {
        sMatchCallState.forcedCallType = FORCED_CALL_NONE;
        BuildSpecialCallString(str);
        return FALSE;
    }

    matchCallId = GetTrainerMatchCallId(trainerId);

    // Veritas: forced call types decided at trigger time (see
    // SelectMatchCallTrainer). Birthday wish: once per birthday, latched by
    // a daily flag that auto-clears at midnight.
    if (sMatchCallState.forcedCallType == FORCED_CALL_BIRTHDAY)
    {
        sMatchCallState.forcedCallType = FORCED_CALL_NONE;
        FlagSet(FLAG_DAILY_BIRTHDAY_MATCH_CALL);
        matchCallText = &sMatchCallBirthdayTexts[Random() % ARRAY_COUNT(sMatchCallBirthdayTexts)];
        BuildMatchCallString(matchCallId, matchCallText, str);
        return FALSE;
    }
    if (sMatchCallState.forcedCallType == FORCED_CALL_CASUAL)
    {
        sMatchCallState.forcedCallType = FORCED_CALL_NONE;
        matchCallText = SelectCasualMatchCallText(matchCallId, str);
        BuildMatchCallString(matchCallId, matchCallText, str);
        return FALSE;
    }

    // If the player is on the same route as the trainer
    // and they can be rematched, they will always request a battle
    if (TrainerIsEligibleForRematch(matchCallId)
     && GetRematchTrainerLocation(matchCallId) == gMapHeader.regionMapSectionId)
    {
        matchCallText = GetSameRouteMatchCallText(matchCallId, str);
    }
    // If the player is not on the same route as the trainer
    // and they can be rematched, there is a random chance for
    // the trainer to request a battle
    else if (ShouldTrainerRequestBattle(matchCallId))
    {
        matchCallText = GetDifferentRouteMatchCallText(matchCallId, str);
        newRematchRequest = TRUE;
        UpdateRematchIfDefeated(matchCallId);
    }
    else if (Random() % 3)
    {
        // Message talking about a battle the NPC had
        matchCallText = GetBattleMatchCallText(matchCallId, str);
    }
    else
    {
        // Message talking about something else
        matchCallText = GetGeneralMatchCallText(matchCallId, str);
    }

    BuildMatchCallString(matchCallId, matchCallText, str);
    return newRematchRequest;
}

// ===========================================================================
// Veritas PokeNav calls. Full write-up in docs/POKENAV_CALLS.md.
// ===========================================================================

// A Brain is willing to call only about a facility whose Silver Symbol the
// player already holds AND where they have a streak worth mentioning.
static bool32 IsFrontierBrainCallable(int facilityId)
{
    u32 topic;

    if (!FlagGet(sFrontierSilverSymbolFlags[facilityId]))
        return FALSE;

    return GetFrontierStreakInfo(facilityId, &topic) > 1;
}

static bool32 PlayerHasAnyFrontierStreak(void)
{
    int i;

    for (i = 0; i < NUM_FRONTIER_FACILITIES; i++)
    {
        if (IsFrontierBrainCallable(i))
            return TRUE;
    }

    return FALSE;
}

// The Weather Institute call is about the POSTGAME research quests -- the
// three roaming beasts its workers send the player after -- NOT the Team
// Aqua takeover during the story. The quest-givers themselves are gated on
// FLAG_SYS_GAME_CLEAR (Route119_WeatherInstitute_1F/2F scripts), so this
// matches. It stops once all three beasts are caught or knocked out.
static bool32 ShouldOfferWeatherInstituteCall(void)
{
    u32 i;

    if (!FlagGet(FLAG_SYS_GAME_CLEAR))
        return FALSE;

    for (i = 0; i < ARRAY_COUNT(sWeatherInstituteBeastFlags); i += 2)
    {
        if (!FlagGet(sWeatherInstituteBeastFlags[i]) && !FlagGet(sWeatherInstituteBeastFlags[i + 1]))
            return TRUE;
    }

    return FALSE;
}

// The outfit hint stops once every Fashionista outfit has been unlocked.
// The flags are contiguous from FLAG_STYLE_UNLOCKED_BRAZIL.
// NOTE: FLAG_SYS_NO_COLLISION shares a value with the last one
// (FLAG_STYLE_UNLOCKED_TREECKO, 0x2B5) -- see flags.h. In a debug session
// with no-collision on, this reads one outfit as already unlocked. Harmless
// here (it can only end the hint marginally early) but it is a real clash.
// PLAYER_STYLE_COUNT lives in src/data/player_styles.h, which isn't meant to
// be included from here, so the count comes from the flag range instead.
#define NUM_OUTFIT_UNLOCK_FLAGS (FLAG_STYLE_UNLOCKED_TREECKO - FLAG_STYLE_UNLOCKED_BRAZIL + 1)

static bool32 ShouldOfferFashionistaCall(void)
{
    u32 i;

    for (i = 0; i < NUM_OUTFIT_UNLOCK_FLAGS; i++)
    {
        if (!FlagGet(FLAG_STYLE_UNLOCKED_BRAZIL + i))
            return TRUE;
    }

    return FALSE;
}

static bool32 HasAnyRecordMixedTrainer(void)
{
    u8 i;

    for (i = 1; i < SECRET_BASES_COUNT; i++)
    {
        if (gSaveBlock1Ptr->secretBases[i].secretBaseId != 0)
            return TRUE;
    }

    for (i = NUM_NORMAL_TVSHOW_SLOTS; i < TV_SHOWS_COUNT; i++)
    {
        if (gSaveBlock1Ptr->tvShows[i].common.kind == TVSHOW_TODAYS_RIVAL_TRAINER
         && gSaveBlock1Ptr->tvShows[i].common.active)
            return TRUE;
    }

    return FALSE;
}

// Family B: callers with no rematch-table entry. Each gets its own roll and
// falls through to the next on a miss, rarest first.
static bool32 TrySelectSpecialCall(void)
{
    if (FlagGet(FLAG_DEFEATED_VERITAS)
     && !FlagGet(FLAG_DAILY_VERITAS_MATCH_CALL)
     && (Random() % 12) == 0)
    {
        sMatchCallState.specialCallId = SPECIAL_CALL_VERITAS;
        return TRUE;
    }

    if (FlagGet(FLAG_IS_CHAMPION) && (Random() % 24) == 0)
    {
        if (Random() % 2)
            sMatchCallState.specialCallId = SPECIAL_CALL_HEART_RIVAL;
        else
            sMatchCallState.specialCallId = SPECIAL_CALL_HEART_STEVEN;
        return TRUE;
    }

    // The rival telling the player about the Weather Institute. Two eras:
    // the Aqua takeover during the story, and the postgame research the
    // institute staff recruit for. Both stop once their work is finished.
    if ((Random() % 14) == 0 && ShouldOfferWeatherInstituteCall())
    {
        sMatchCallState.specialCallId = SPECIAL_CALL_RIVAL_WEATHER;
        return TRUE;
    }

    // The rival meeting somebody who came in through record mixing.
    if ((Random() % 14) == 0 && HasAnyRecordMixedTrainer())
    {
        sMatchCallState.specialCallId = SPECIAL_CALL_RIVAL_MIXED;
        return TRUE;
    }

    if (GetGameStat(GAME_STAT_ENTERED_HOF) != 0 && (Random() % 20) == 0)
    {
        sMatchCallState.specialCallId = SPECIAL_CALL_HEART_WALLACE;
        return TRUE;
    }

    if (PlayerHasAnyFrontierStreak() && (Random() % 16) == 0)
    {
        sMatchCallState.specialCallId = SPECIAL_CALL_FRONTIER_BRAIN;
        return TRUE;
    }

    if ((Random() % 8) == 0)
    {
        switch (Random() % 4)
        {
        case 0:
            sMatchCallState.specialCallId = SPECIAL_CALL_MOM;
            return TRUE;
        case 1:
            if (FlagGet(FLAG_SYS_POKEDEX_GET))
            {
                sMatchCallState.specialCallId = SPECIAL_CALL_BIRCH;
                return TRUE;
            }
            break;
        case 2:
            sMatchCallState.specialCallId = SPECIAL_CALL_RIVAL;
            return TRUE;
        case 3:
            if (FlagGet(FLAG_DEFEATED_METEOR_FALLS_STEVEN) || FlagGet(FLAG_IS_CHAMPION))
            {
                sMatchCallState.specialCallId = SPECIAL_CALL_STEVEN;
                return TRUE;
            }
            break;
        }
    }

    return FALSE;
}

// The first party slot that isn't empty and isn't an egg, or 0 if the whole
// party is eggs (a Hall of Fame team can't be, so this is just a guard).
static u8 GetFirstNonEggPartyIndex(void)
{
    u8 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL)
         && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG, NULL))
            return i;
    }

    return 0;
}

// The rival is always the opposite gender to the player.
static void PopulateRivalName(u8 *destStr)
{
    if (gSaveBlock2Ptr->playerGender == MALE)
        StringCopy(destStr, gTrainers[TRAINER_MAY_ROUTE_103_TREECKO].trainerName);
    else
        StringCopy(destStr, gTrainers[TRAINER_BRENDAN_ROUTE_103_TREECKO].trainerName);
}

static void BuildSpecialCallString(u8 *str)
{
    const u8 *text;
    u16 streak, best;
    u32 topic;
    int i, bestId;
    u8 metGender = MALE;
    bool8 metGenderKnown = FALSE;

    text = sSpecialCallTexts[sMatchCallState.specialCallId];
    if (sSpecialCallTextsAlt[sMatchCallState.specialCallId] != NULL && (Random() % 2))
        text = sSpecialCallTextsAlt[sMatchCallState.specialCallId];

    switch (sMatchCallState.specialCallId)
    {
    case SPECIAL_CALL_VERITAS:
        // Latched so the mystery call is a once-a-day event, not a rerun.
        FlagSet(FLAG_DAILY_VERITAS_MATCH_CALL);
        sMatchCallState.veritasCallActive = TRUE;
        break;
    case SPECIAL_CALL_HEART_RIVAL:
    case SPECIAL_CALL_RIVAL:
        PopulateRivalName(gStringVar1);
        break;
    case SPECIAL_CALL_RIVAL_WEATHER:
        PopulateRivalName(gStringVar1);
        break;
    case SPECIAL_CALL_RIVAL_MIXED:
        PopulateRivalName(gStringVar1);
        // The crush variant only exists when the mixed-in trainer's gender is
        // known (secret bases record it, TV shows don't) AND is the opposite
        // of the rival's own -- the rival is always the player's opposite. It
        // stays a 1-in-3 so it reads as an occasional slip, not a running gag.
        if (TryPopulateMixedTrainerForRival(&metGender, &metGenderKnown)
         && metGenderKnown
         && metGender == gSaveBlock2Ptr->playerGender
         && (Random() % 3) == 0)
            text = MatchCall_Mixed_RivalCrush;
        break;
    case SPECIAL_CALL_HEART_WALLACE:
        // Reading the actual Hall of Fame means pulling flash sectors 28-29
        // into gDecompressionBuffer, which is not something to do in the
        // middle of the overworld. The lead Pokemon plus the HOF entry count
        // give the same "he remembers your run" beat for free.
        StringCopy(gStringVar2, gSpeciesNames[GetMonData(&gPlayerParty[GetFirstNonEggPartyIndex()], MON_DATA_SPECIES, NULL)]);
        ConvertIntToDecimalStringN(gStringVar3, GetGameStat(GAME_STAT_ENTERED_HOF), STR_CONV_MODE_LEFT_ALIGN, 5);
        break;
    case SPECIAL_CALL_FRONTIER_BRAIN:
        // Best streak among only those facilities the player has already
        // earned the Silver Symbol in, so a Brain never rings about a
        // facility the player hasn't beaten them in yet.
        best = 0;
        bestId = -1;
        for (i = 0; i < NUM_FRONTIER_FACILITIES; i++)
        {
            if (!IsFrontierBrainCallable(i))
                continue;

            streak = GetFrontierStreakInfo(i, &topic);
            if (bestId < 0 || streak > best)
            {
                best = streak;
                bestId = i;
            }
        }

        // TrySelectSpecialCall already proved one exists; this is a guard.
        if (bestId < 0)
            bestId = FRONTIER_FACILITY_TOWER;

        sBattleFrontierStreakInfo.facilityId = bestId;
        sBattleFrontierStreakInfo.streak = best;
        text = sFrontierBrainTexts[bestId];
        StringCopy(gStringVar1, gTrainers[sFrontierBrainTrainerIds[bestId]].trainerName);
        ConvertIntToDecimalStringN(gStringVar2, best, STR_CONV_MODE_LEFT_ALIGN, 4);
        break;
    }

    StringExpandPlaceholders(str, text);
}

// Writes {STR_VAR_2} (species) and {STR_VAR_3} (place) for an outbreak call.
static bool32 TryPopulateOutbreakVars(bool8 *useNationalVariant)
{
    const struct MapHeader *header;
    u16 species = gSaveBlock1Ptr->outbreakPokemonSpecies;

    *useNationalVariant = FALSE;
    if (gSaveBlock1Ptr->outbreakDaysLeft == 0 || species == SPECIES_NONE)
        return FALSE;

    // Never name a Pokemon the player's Pokedex can't even list yet: if it
    // has no Hoenn number and the National Dex is still locked, stay quiet.
    if (NationalToHoennOrder(SpeciesToNationalPokedexNum(species)) == 0)
    {
        if (!IsNationalPokedexEnabled())
            return FALSE;

        *useNationalVariant = TRUE;
    }

    header = Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->outbreakLocationMapGroup,
                                                gSaveBlock1Ptr->outbreakLocationMapNum);
    if (header == NULL)
        return FALSE;

    StringCopy(gStringVar2, gSpeciesNames[species]);
    GetMapName(gStringVar3, header->regionMapSectionId, 0);
    return TRUE;
}

// Writes {STR_VAR_2} (species) and {STR_VAR_3} (OT) for the "I saw your
// catch on TV" call. Shinies win ties because they're the rarer sight.
static bool32 TryPopulateRareCatchVars(bool8 *wasShiny)
{
    u8 slots[PARTY_SIZE];
    u8 shiny[PARTY_SIZE];
    u8 otName[PLAYER_NAME_LENGTH + 1];
    u8 count = 0;
    u8 i, pick;
    u16 species;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL);
        if (species == SPECIES_NONE || GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG, NULL))
            continue;

        if (IsMonShiny(&gPlayerParty[i]))
            shiny[count] = TRUE;
        else if (IsSpeciesLegendary(species))
            shiny[count] = FALSE;
        else
            continue;

        slots[count] = i;
        count++;
    }

    if (count == 0)
        return FALSE;

    pick = Random() % count;
    *wasShiny = shiny[pick];
    species = GetMonData(&gPlayerParty[slots[pick]], MON_DATA_SPECIES, NULL);
    GetMonData(&gPlayerParty[slots[pick]], MON_DATA_OT_NAME, otName);
    StringCopy(gStringVar2, gSpeciesNames[species]);
    StringCopy(gStringVar3, otName);
    return TRUE;
}

// Secret base and Apprentice names are PLAYER_NAME_LENGTH bytes with NO room
// for a terminator, unlike TV show names. Copying one with StringCopy runs
// off the end into the adjacent fields.
static void CopyUnterminatedPlayerName(u8 *dest, const u8 *src, u8 language)
{
    StringCopyN(dest, src, PLAYER_NAME_LENGTH);
    dest[PLAYER_NAME_LENGTH] = EOS;
    ConvertInternationalString(dest, language);
}

// Record-mix TV shows live in slots NUM_NORMAL_TVSHOW_SLOTS..TV_SHOWS_COUNT-1;
// slots below that are the player's own. Returns a random live show of the
// requested kind, or NULL.
static TVShow *FindRecordMixedShow(u8 kind)
{
    TVShow *matches[TV_SHOWS_COUNT];
    u8 count = 0;
    u8 i;

    for (i = NUM_NORMAL_TVSHOW_SLOTS; i < TV_SHOWS_COUNT; i++)
    {
        if (gSaveBlock1Ptr->tvShows[i].common.kind == kind
         && gSaveBlock1Ptr->tvShows[i].common.active)
            matches[count++] = &gSaveBlock1Ptr->tvShows[i];
    }

    if (count == 0)
        return NULL;

    return matches[Random() % count];
}

// "I ran into {STR_VAR_2} over on {STR_VAR_3}."
static bool32 TryPopulateMixedSightingVars(void)
{
    TVShow *show = FindRecordMixedShow(TVSHOW_TODAYS_RIVAL_TRAINER);

    if (show != NULL)
    {
        TVShowConvertInternationalString(gStringVar2, show->rivalTrainer.playerName, show->rivalTrainer.language);
        GetMapName(gStringVar3, show->rivalTrainer.location, 0);
        return TRUE;
    }

    show = FindRecordMixedShow(TVSHOW_BREAKING_NEWS);
    if (show != NULL)
    {
        TVShowConvertInternationalString(gStringVar2, show->breakingNews.playerName, show->breakingNews.language);
        GetMapName(gStringVar3, show->breakingNews.location, 0);
        return TRUE;
    }

    return FALSE;
}

// "{STR_VAR_2} caught a {STR_VAR_3}!" Note the mixed-in data records WHAT was
// caught but not whether it was shiny -- no TV show carries that -- so this
// call never claims a shiny. The player's own shinies are covered by
// MatchCall_RareCatch_Shiny instead.
static bool32 TryPopulateMixedCatchVars(void)
{
    TVShow *show = FindRecordMixedShow(TVSHOW_POKEMON_TODAY_CAUGHT);

    if (show == NULL || show->pokemonToday.species == SPECIES_NONE)
        return FALSE;

    TVShowConvertInternationalString(gStringVar2, show->pokemonToday.playerName, show->pokemonToday.language);
    StringCopy(gStringVar3, gSpeciesNames[show->pokemonToday.species]);
    return TRUE;
}

static u16 GetSecretBaseMapSec(u8 baseId)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sSecretBaseLocations); i++)
    {
        if (sSecretBaseLocations[i].baseId == baseId)
            return sSecretBaseLocations[i].mapSec;
    }

    return MAPSEC_NONE;
}

// "{STR_VAR_2}'s base is on {STR_VAR_3}." Slot 0 is always the player's own
// base, so it's skipped.
static bool32 TryPopulateMixedBaseVars(u8 *gender)
{
    u8 slots[SECRET_BASES_COUNT];
    u8 count = 0;
    u8 i;
    u16 mapSec;

    for (i = 1; i < SECRET_BASES_COUNT; i++)
    {
        if (gSaveBlock1Ptr->secretBases[i].secretBaseId != 0
         && GetSecretBaseMapSec(gSaveBlock1Ptr->secretBases[i].secretBaseId) != MAPSEC_NONE)
            slots[count++] = i;
    }

    if (count == 0)
        return FALSE;

    i = slots[Random() % count];
    mapSec = GetSecretBaseMapSec(gSaveBlock1Ptr->secretBases[i].secretBaseId);
    *gender = gSaveBlock1Ptr->secretBases[i].gender;
    CopyUnterminatedPlayerName(gStringVar2, gSaveBlock1Ptr->secretBases[i].trainerName,
                               gSaveBlock1Ptr->secretBases[i].language);
    GetMapName(gStringVar3, mapSec, 0);
    return TRUE;
}

// A mixed-in trainer for the rival to have met. Secret bases are preferred
// because they're the only mixed source that records the owner's GENDER,
// which the rival's crush line needs; TV shows are the nameless-gender
// fallback and can only produce the neutral variant.
static bool32 TryPopulateMixedTrainerForRival(u8 *gender, bool8 *genderKnown)
{
    TVShow *show;
    u8 slots[SECRET_BASES_COUNT];
    u8 count = 0;
    u8 i;

    *genderKnown = FALSE;

    for (i = 1; i < SECRET_BASES_COUNT; i++)
    {
        if (gSaveBlock1Ptr->secretBases[i].secretBaseId != 0)
            slots[count++] = i;
    }

    if (count != 0)
    {
        i = slots[Random() % count];
        *gender = gSaveBlock1Ptr->secretBases[i].gender;
        *genderKnown = TRUE;
        CopyUnterminatedPlayerName(gStringVar2, gSaveBlock1Ptr->secretBases[i].trainerName,
                                   gSaveBlock1Ptr->secretBases[i].language);
        return TRUE;
    }

    show = FindRecordMixedShow(TVSHOW_TODAYS_RIVAL_TRAINER);
    if (show != NULL)
    {
        TVShowConvertInternationalString(gStringVar2, show->rivalTrainer.playerName, show->rivalTrainer.language);
        return TRUE;
    }

    return FALSE;
}

// Index into sMatchCallLegendRumors of a rumour whose gate is open and whose
// hunt isn't resolved yet, or -1 if there's nothing worth gossiping about.
static int SelectLegendRumor(void)
{
    u8 usable[ARRAY_COUNT(sMatchCallLegendRumors)];
    u8 count = 0;
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sMatchCallLegendRumors); i++)
    {
        if (!FlagGet(sMatchCallLegendRumors[i].gate))
            continue;
        if (FlagGet(sMatchCallLegendRumors[i].done1))
            continue;
        if (sMatchCallLegendRumors[i].done2 != MATCH_CALL_NO_FLAG
         && FlagGet(sMatchCallLegendRumors[i].done2))
            continue;

        usable[count++] = i;
    }

    if (count == 0)
        return -1;

    return usable[Random() % count];
}

// The birds share one rumour, so it stays available until every one of them
// has been caught or knocked out.
static bool32 ShouldOfferBirdRumor(void)
{
    u32 i;

    if (!FlagGet(FLAG_BEAT_ALL_3RD_REMATCHES))
        return FALSE;

    for (i = 0; i < ARRAY_COUNT(sMatchCallBirdDoneFlags); i += 2)
    {
        if (!FlagGet(sMatchCallBirdDoneFlags[i]) && !FlagGet(sMatchCallBirdDoneFlags[i + 1]))
            return TRUE;
    }

    return FALSE;
}

// Family A: the caller is a registered rematch trainer, so {STR_VAR_1} is
// filled by the normal PopulateTrainerName path. Categories that don't pass
// their gate simply aren't offered.
static const struct MatchCallText *SelectCasualMatchCallText(int matchCallId, u8 *str)
{
    // +4 covers the doubled CASUAL_CHATTER plus slack. Every new category
    // added below must stay within this.
    u8 candidates[CASUAL_COUNT + 4];
    u8 count = 0;
    bool8 useNationalVariant = FALSE;
    bool8 catchWasShiny = FALSE;
    u8 baseOwnerGender = MALE;
    u8 hours;
    int rumorId;

    RtcCalcLocalTime();
    hours = gLocalTime.hours;
    rumorId = SelectLegendRumor();

    // Vanilla chatter is weighted double so the new material stays a garnish
    // rather than replacing the trainers' own voices.
    candidates[count++] = CASUAL_CHATTER;
    candidates[count++] = CASUAL_CHATTER;
    candidates[count++] = CASUAL_ABILITY_TIP;
    candidates[count++] = CASUAL_GENERAL_TIP;
    candidates[count++] = CASUAL_LORE_SOUTHERN;

    if (FlagGet(FLAG_DEFEATED_RAYQUAZA))
        candidates[count++] = CASUAL_LORE_DRAGON;

    if (hours >= 22 || hours < 7)
        candidates[count++] = CASUAL_TIME_OF_DAY;

    if (TryPopulateOutbreakVars(&useNationalVariant))
        candidates[count++] = CASUAL_OUTBREAK;

    if (!FlagGet(FLAG_DAILY_RARE_CATCH_MATCH_CALL) && TryPopulateRareCatchVars(&catchWasShiny))
        candidates[count++] = CASUAL_RARE_CATCH;

    if (rumorId >= 0)
        candidates[count++] = CASUAL_LEGEND_RUMOR;

    if (ShouldOfferBirdRumor())
        candidates[count++] = CASUAL_LEGEND_BIRDS;

    // Jirachi's gate is the National Dex upgrade cutscene, which sets this var.
    if (VarGet(VAR_MOSSDEEP_CITY_STATE) == 4 && !FlagGet(FLAG_BATTLED_JIRACHI))
        candidates[count++] = CASUAL_LEGEND_JIRACHI;

    // The Lilycove Museum curator hands out the Mystic Ticket for filling his
    // gallery. Stops the moment the player has the ticket.
    if (FlagGet(FLAG_IS_CHAMPION) && !FlagGet(FLAG_RECEIVED_MYSTIC_TICKET))
        candidates[count++] = CASUAL_QUEST_MUSEUM;

    if (ShouldOfferFashionistaCall())
        candidates[count++] = CASUAL_QUEST_FASHIONISTA;

    if (TryPopulateMixedSightingVars())
        candidates[count++] = CASUAL_MIXED_SIGHTING;

    if (TryPopulateMixedCatchVars())
        candidates[count++] = CASUAL_MIXED_CATCH;

    if (TryPopulateMixedBaseVars(&baseOwnerGender))
        candidates[count++] = CASUAL_MIXED_BASE;

    // Every TryPopulate* above writes gStringVar2/gStringVar3, so by now they
    // hold whichever candidate was tested LAST. Each case below re-populates
    // for the category that actually won. A re-call can't fail: the probe
    // already proved the data is there.
    switch (candidates[Random() % count])
    {
    case CASUAL_ABILITY_TIP:
        return &sMatchCallAbilityTipTexts[Random() % ARRAY_COUNT(sMatchCallAbilityTipTexts)];
    case CASUAL_GENERAL_TIP:
        return &sMatchCallGeneralTipTexts[Random() % ARRAY_COUNT(sMatchCallGeneralTipTexts)];
    case CASUAL_LORE_SOUTHERN:
        return &sMatchCallLoreSouthernIslandText;
    case CASUAL_LORE_DRAGON:
        return &sMatchCallLoreDragonLadyText;
    case CASUAL_TIME_OF_DAY:
        if (hours >= 22 || hours < 4)
            return &sMatchCallLateNightTexts[Random() % ARRAY_COUNT(sMatchCallLateNightTexts)];
        return &sMatchCallEarlyMorningTexts[Random() % ARRAY_COUNT(sMatchCallEarlyMorningTexts)];
    case CASUAL_OUTBREAK:
        TryPopulateOutbreakVars(&useNationalVariant);
        if (useNationalVariant)
            return &sMatchCallOutbreakNationalText;
        return &sMatchCallOutbreakText;
    case CASUAL_RARE_CATCH:
        TryPopulateRareCatchVars(&catchWasShiny);
        FlagSet(FLAG_DAILY_RARE_CATCH_MATCH_CALL);
        if (catchWasShiny)
            return &sMatchCallRareCatchShinyText;
        return &sMatchCallRareCatchLegendaryText;
    case CASUAL_LEGEND_RUMOR:
        return &sMatchCallLegendRumors[rumorId].text;
    case CASUAL_LEGEND_BIRDS:
        return &sMatchCallLegendBirdsText;
    case CASUAL_LEGEND_JIRACHI:
        return &sMatchCallLegendJirachiText;
    case CASUAL_QUEST_MUSEUM:
        return &sMatchCallQuestMuseumTicketText;
    case CASUAL_QUEST_FASHIONISTA:
        GetMapName(gStringVar2, sFashionistaCityMapSecs[Random() % ARRAY_COUNT(sFashionistaCityMapSecs)], 0);
        return &sMatchCallQuestFashionistaText;
    case CASUAL_MIXED_SIGHTING:
        TryPopulateMixedSightingVars();
        return &sMatchCallMixedSightingText;
    case CASUAL_MIXED_CATCH:
        TryPopulateMixedCatchVars();
        return &sMatchCallMixedRareCatchText;
    case CASUAL_MIXED_BASE:
        TryPopulateMixedBaseVars(&baseOwnerGender);
        return &sMatchCallMixedSecretBaseTexts[baseOwnerGender];
    case CASUAL_CHATTER:
    default:
        if (Random() % 3)
            return GetBattleMatchCallText(matchCallId, str);
        return GetGeneralMatchCallText(matchCallId, str);
    }
}

static int GetTrainerMatchCallId(int trainerId)
{
    int i = 0;
    while (1)
    {
        if (sMatchCallTrainers[i].trainerId == trainerId)
            return i;
        else
            i++;
    }
}

static const struct MatchCallText *GetSameRouteMatchCallText(int matchCallId, u8 *str)
{
    u16 textId = sMatchCallTrainers[matchCallId].sameRouteMatchCallTextId;
    int mask = 0xFF;
    u32 topic = (textId >> 8) - 1;
    u32 id = (textId & mask) - 1;
    return &sMatchCallBattleRequestTopics[topic][id];
}

static const struct MatchCallText *GetDifferentRouteMatchCallText(int matchCallId, u8 *str)
{
    u16 textId = sMatchCallTrainers[matchCallId].differentRouteMatchCallTextId;
    int mask = 0xFF;
    u32 topic = (textId >> 8) - 1;
    u32 id = (textId & mask) - 1;
    return &sMatchCallBattleRequestTopics[topic][id];
}

static const struct MatchCallText *GetBattleMatchCallText(int matchCallId, u8 *str)
{
    int mask;
    u32 textId, topic, id;

    topic = Random() % 3;
    textId = sMatchCallTrainers[matchCallId].battleTopicTextIds[topic];
    if (!textId)
        SpriteCallbackDummy(NULL); // leftover debugging ???

    mask = 0xFF;
    topic = (textId >> 8) - 1;
    id = (textId & mask) - 1;
    return &sMatchCallBattleTopics[topic][id];
}

static const struct MatchCallText *GetGeneralMatchCallText(int matchCallId, u8 *str)
{
    int i;
    int count;
    u32 topic, id;
    u16 rand;

    rand = Random();
    if (!(rand & 1))
    {
        // Count the number of facilities with a win streak
        for (count = 0, i = 0; i < NUM_FRONTIER_FACILITIES; i++)
        {
            if (GetFrontierStreakInfo(i, &topic) > 1)
                count++;
        }

        if (count)
        {
            // At least one facility with a win streak
            // Randomly choose one to have a call about
            count = Random() % count;
            for (i = 0; i < NUM_FRONTIER_FACILITIES; i++)
            {
                sBattleFrontierStreakInfo.streak = GetFrontierStreakInfo(i, &topic);
                if (sBattleFrontierStreakInfo.streak < 2)
                    continue;

                if (!count)
                    break;

                count--;
            }

            sBattleFrontierStreakInfo.facilityId = i;
            id = sMatchCallTrainers[matchCallId].battleFrontierRecordStreakTextIndex - 1;
            return &sMatchCallGeneralTopics[topic][id];
        }
    }

    topic = (sMatchCallTrainers[matchCallId].generalTextId >> 8) - 1;
    id = (sMatchCallTrainers[matchCallId].generalTextId & 0xFF) - 1;
    return &sMatchCallGeneralTopics[topic][id];
}

static void BuildMatchCallString(int matchCallId, const struct MatchCallText *matchCallText, u8 *str)
{
    PopulateMatchCallStringVars(matchCallId, matchCallText->stringVarFuncIds);
    StringExpandPlaceholders(str, matchCallText->text);
}

static u8 *const sMatchCallTextStringVars[] = { gStringVar1, gStringVar2, gStringVar3 };

static void PopulateMatchCallStringVars(int matchCallId, const s8 *stringVarFuncIds)
{
    int i;
    for (i = 0; i < NUM_STRVARS_IN_MSG; i++)
    {
        if (stringVarFuncIds[i] >= 0)
            PopulateMatchCallStringVar(matchCallId, stringVarFuncIds[i], sMatchCallTextStringVars[i]);
    }
}

static void (*const sPopulateMatchCallStringVarFuncs[])(int, u8 *) =
{
    [STR_TRAINER_NAME]     = PopulateTrainerName,
    [STR_MAP_NAME]         = PopulateMapName,
    [STR_SPECIES_IN_ROUTE] = PopulateSpeciesFromTrainerLocation,
    [STR_SPECIES_IN_PARTY] = PopulateSpeciesFromTrainerParty,
    [STR_FACILITY_NAME]    = PopulateBattleFrontierFacilityName,
    [STR_FRONTIER_STREAK]  = PopulateBattleFrontierStreak,
};

static void PopulateMatchCallStringVar(int matchCallId, int funcId, u8 *destStr)
{
    sPopulateMatchCallStringVarFuncs[funcId](matchCallId, destStr);
}

static const struct MultiTrainerMatchCallText sMultiTrainerMatchCallTexts[] =
{
    { .trainerId = TRAINER_KIRA_AND_DAN_1, .text = gText_Kira },
    { .trainerId = TRAINER_AMY_AND_LIV_1,  .text = gText_Amy },
    { .trainerId = TRAINER_JOHN_AND_JAY_1, .text = gText_John },
    { .trainerId = TRAINER_LILA_AND_ROY_1, .text = gText_Roy },
    { .trainerId = TRAINER_GABBY_AND_TY_1, .text = gText_Gabby },
    { .trainerId = TRAINER_ANNA_AND_MEG_1, .text = gText_Anna },
};

static void PopulateTrainerName(int matchCallId, u8 *destStr)
{
    u32 i;
    u16 trainerId = sMatchCallTrainers[matchCallId].trainerId;
    for (i = 0; i < ARRAY_COUNT(sMultiTrainerMatchCallTexts); i++)
    {
        if (sMultiTrainerMatchCallTexts[i].trainerId == trainerId)
        {
            StringCopy(destStr, sMultiTrainerMatchCallTexts[i].text);
            return;
        }
    }

    StringCopy(destStr, gTrainers[trainerId].trainerName);
}

static void PopulateMapName(int matchCallId, u8 *destStr)
{
    GetMapName(destStr, GetRematchTrainerLocation(matchCallId), 0);
}

static u8 GetLandEncounterSlot(void)
{
    int rand = Random() % 100;
    if (rand < 20)
        return 0;
    else if (rand >= 20 && rand < 40)
        return 1;
    else if (rand >= 40 && rand < 50)
        return 2;
    else if (rand >= 50 && rand < 60)
        return 3;
    else if (rand >= 60 && rand < 70)
        return 4;
    else if (rand >= 70 && rand < 80)
        return 5;
    else if (rand >= 80 && rand < 85)
        return 6;
    else if (rand >= 85 && rand < 90)
        return 7;
    else if (rand >= 90 && rand < 94)
        return 8;
    else if (rand >= 94 && rand < 98)
        return 9;
    else if (rand >= 98 && rand < 99)
        return 10;
    else
        return 11;
}

static u8 GetWaterEncounterSlot(void)
{
    int rand = Random() % 100;
    if (rand < 60)
        return 0;
    else if (rand >= 60 && rand < 90)
        return 1;
    else if (rand >= 90 && rand < 95)
        return 2;
    else if (rand >= 95 && rand < 99)
        return 3;
    else
        return 4;
}

static void PopulateSpeciesFromTrainerLocation(int matchCallId, u8 *destStr)
{
    u16 species[2];
    int numSpecies;
    u8 slot;
    int i = 0;

    if (gWildMonHeaders[i].mapGroup != MAP_GROUP(UNDEFINED)) // ??? This check is nonsense.
    {
        while (gWildMonHeaders[i].mapGroup != MAP_GROUP(UNDEFINED))
        {
            if (gWildMonHeaders[i].mapGroup == gRematchTable[matchCallId].mapGroup
             && gWildMonHeaders[i].mapNum == gRematchTable[matchCallId].mapNum)
                break;

            i++;
        }

        if (gWildMonHeaders[i].mapGroup != MAP_GROUP(UNDEFINED))
        {
            numSpecies = 0;
            if (gWildMonHeaders[i].landMonsInfo)
            {
                slot = GetLandEncounterSlot();
                species[numSpecies] = gWildMonHeaders[i].landMonsInfo->wildPokemon[slot].species;
                numSpecies++;
            }

            if (gWildMonHeaders[i].waterMonsInfo)
            {
                slot = GetWaterEncounterSlot();
                species[numSpecies] = gWildMonHeaders[i].waterMonsInfo->wildPokemon[slot].species;
                numSpecies++;
            }

            if (numSpecies)
            {
                StringCopy(destStr, gSpeciesNames[species[Random() % numSpecies]]);
                return;
            }
        }
    }

    destStr[0] = EOS;
}

static void PopulateSpeciesFromTrainerParty(int matchCallId, u8 *destStr)
{
    u16 trainerId;
    union TrainerMonPtr party;
    u8 monId;
    const u8 *speciesName;

    trainerId = GetLastBeatenRematchTrainerId(sMatchCallTrainers[matchCallId].trainerId);
    party = gTrainers[trainerId].party;
    monId = Random() % gTrainers[trainerId].partySize;

    speciesName = gSpeciesNames[party.TrainerMon[monId].species];

    StringCopy(destStr, speciesName);
}

static const u8 *const sBattleFrontierFacilityNames[NUM_FRONTIER_FACILITIES] =
{
    [FRONTIER_FACILITY_TOWER]   = gText_BattleTower2,
    [FRONTIER_FACILITY_DOME]    = gText_BattleDome,
    [FRONTIER_FACILITY_PALACE]  = gText_BattlePalace,
    [FRONTIER_FACILITY_ARENA]   = gText_BattleArena,
    [MATCH_CALL_PIKE]           = gText_BattlePike,
    [MATCH_CALL_FACTORY]        = gText_BattleFactory,
    [FRONTIER_FACILITY_PYRAMID] = gText_BattlePyramid,
};

static void PopulateBattleFrontierFacilityName(int matchCallId, u8 *destStr)
{
    StringCopy(destStr, sBattleFrontierFacilityNames[sBattleFrontierStreakInfo.facilityId]);
}

static void PopulateBattleFrontierStreak(int matchCallId, u8 *destStr)
{
    int i = 0;
    int streak = sBattleFrontierStreakInfo.streak;
    while (streak != 0)
    {
        streak /= 10;
        i++;
    }

    ConvertIntToDecimalStringN(destStr, sBattleFrontierStreakInfo.streak, STR_CONV_MODE_LEFT_ALIGN, i);
}

static const u16 sBadgeFlags[NUM_BADGES] =
{
    FLAG_BADGE01_GET,
    FLAG_BADGE02_GET,
    FLAG_BADGE03_GET,
    FLAG_BADGE04_GET,
    FLAG_BADGE05_GET,
    FLAG_BADGE06_GET,
    FLAG_BADGE07_GET,
    FLAG_BADGE08_GET,
};

static int GetNumOwnedBadges(void)
{
    u32 i;

    for (i = 0; i < NUM_BADGES; i++)
    {
        if (!FlagGet(sBadgeFlags[i]))
            break;
    }

    return i;
}

// Whether or not a trainer calling the player from a different route should request a battle
static bool32 ShouldTrainerRequestBattle(int matchCallId)
{
    int dayCount;
    int otId;
    u16 dewfordRand;
    int numRematchTrainersFought;
    int max, rand, n;

    if (GetNumOwnedBadges() < 5)
        return FALSE;

    dayCount = RtcGetLocalDayCount();
    otId = GetTrainerId(gSaveBlock2Ptr->playerTrainerId) & 0xFFFF;

    dewfordRand = gSaveBlock1Ptr->dewfordTrends[0].rand;
    numRematchTrainersFought = GetNumRematchTrainersFought();
    max = (numRematchTrainersFought * 13) / 10;
    rand = ((dayCount ^ dewfordRand) + (dewfordRand ^ GetGameStat(GAME_STAT_TRAINER_BATTLES))) ^ otId;
    n = rand % max;
    if (n < numRematchTrainersFought)
    {
        if (GetNthRematchTrainerFought(n) == matchCallId)
            return TRUE;
    }

    return FALSE;
}

static u16 GetFrontierStreakInfo(u16 facilityId, u32 *topicTextId)
{
    int i;
    int j;
    u16 streak = 0;

    switch (facilityId)
    {
    case FRONTIER_FACILITY_DOME:
        for (i = 0; i < (int)ARRAY_COUNT(gSaveBlock2Ptr->frontier.domeRecordWinStreaks); i++)
        {
            for (j = 0; j < FRONTIER_LVL_MODE_COUNT; j++)
            {
                if (streak < gSaveBlock2Ptr->frontier.domeRecordWinStreaks[i][j])
                    streak = gSaveBlock2Ptr->frontier.domeRecordWinStreaks[i][j];
            }
        }
        *topicTextId = GEN_TOPIC_B_DOME - 1;
        break;
    case MATCH_CALL_PIKE:
        for (i = 0; i < FRONTIER_LVL_MODE_COUNT; i++)
        {
            if (streak < gSaveBlock2Ptr->frontier.pikeRecordStreaks[i])
                streak = gSaveBlock2Ptr->frontier.pikeRecordStreaks[i];
        }
        *topicTextId = GEN_TOPIC_B_PIKE - 1;
        break;
    case FRONTIER_FACILITY_TOWER:
        for (i = 0; i < (int)ARRAY_COUNT(gSaveBlock2Ptr->frontier.towerRecordWinStreaks); i++)
        {
            for (j = 0; j < FRONTIER_LVL_MODE_COUNT; j++)
            {
                if (streak < gSaveBlock2Ptr->frontier.towerRecordWinStreaks[i][j])
                    streak = gSaveBlock2Ptr->frontier.towerRecordWinStreaks[i][j];
            }
        }
        *topicTextId = GEN_TOPIC_STREAK_RECORD - 1;
        break;
    case FRONTIER_FACILITY_PALACE:
        for (i = 0; i < (int)ARRAY_COUNT(gSaveBlock2Ptr->frontier.palaceRecordWinStreaks); i++)
        {
            for (j = 0; j < FRONTIER_LVL_MODE_COUNT; j++)
            {
                if (streak < gSaveBlock2Ptr->frontier.palaceRecordWinStreaks[i][j])
                    streak = gSaveBlock2Ptr->frontier.palaceRecordWinStreaks[i][j];
            }
        }
        *topicTextId = GEN_TOPIC_STREAK_RECORD - 1;
        break;
    case MATCH_CALL_FACTORY:
        for (i = 0; i < (int)ARRAY_COUNT(gSaveBlock2Ptr->frontier.factoryRecordWinStreaks); i++)
        {
            for (j = 0; j < FRONTIER_LVL_MODE_COUNT; j++)
            {
                if (streak < gSaveBlock2Ptr->frontier.factoryRecordWinStreaks[i][j])
                    streak = gSaveBlock2Ptr->frontier.factoryRecordWinStreaks[i][j];
            }
        }
        *topicTextId = GEN_TOPIC_STREAK_RECORD - 1;
        break;
    case FRONTIER_FACILITY_ARENA:
        for (i = 0; i < FRONTIER_LVL_MODE_COUNT; i++)
        {
            if (streak < gSaveBlock2Ptr->frontier.arenaRecordStreaks[i])
                streak = gSaveBlock2Ptr->frontier.arenaRecordStreaks[i];
        }
        *topicTextId = GEN_TOPIC_STREAK_RECORD - 1;
        break;
    case FRONTIER_FACILITY_PYRAMID:
        for (i = 0; i < FRONTIER_LVL_MODE_COUNT; i++)
        {
            if (streak < gSaveBlock2Ptr->frontier.pyramidRecordStreaks[i])
                streak = gSaveBlock2Ptr->frontier.pyramidRecordStreaks[i];
        }
        *topicTextId = GEN_TOPIC_B_PYRAMID - 1;
        break;
    }

    return streak;
}

static u8 GetPokedexRatingLevel(u16 numSeen)
{
    if (numSeen < 10)
        return 0;
    if (numSeen < 20)
        return 1;
    if (numSeen < 30)
        return 2;
    if (numSeen < 40)
        return 3;
    if (numSeen < 50)
        return 4;
    if (numSeen < 60)
        return 5;
    if (numSeen < 70)
        return 6;
    if (numSeen < 80)
        return 7;
    if (numSeen < 90)
        return 8;
    if (numSeen < 100)
        return 9;
    if (numSeen < 110)
        return 10;
    if (numSeen < 120)
        return 11;
    if (numSeen < 130)
        return 12;
    if (numSeen < 140)
        return 13;
    if (numSeen < 150)
        return 14;
    if (numSeen < 160)
        return 15;
    if (numSeen < 170)
        return 16;
    if (numSeen < 180)
        return 17;
    if (numSeen < 190)
        return 18;
    if (numSeen < 200)
        return 19;

    if (GetSetPokedexFlag(SpeciesToNationalPokedexNum(SPECIES_DEOXYS), FLAG_GET_CAUGHT))
        numSeen--;
    if (GetSetPokedexFlag(SpeciesToNationalPokedexNum(SPECIES_JIRACHI), FLAG_GET_CAUGHT))
        numSeen--;

    if (numSeen < 200)
        return 19;
    else
        return 20;
}

static const u8 *const sBirchDexRatingTexts[] =
{
    gBirchDexRatingText_LessThan10,
    gBirchDexRatingText_LessThan20,
    gBirchDexRatingText_LessThan30,
    gBirchDexRatingText_LessThan40,
    gBirchDexRatingText_LessThan50,
    gBirchDexRatingText_LessThan60,
    gBirchDexRatingText_LessThan70,
    gBirchDexRatingText_LessThan80,
    gBirchDexRatingText_LessThan90,
    gBirchDexRatingText_LessThan100,
    gBirchDexRatingText_LessThan110,
    gBirchDexRatingText_LessThan120,
    gBirchDexRatingText_LessThan130,
    gBirchDexRatingText_LessThan140,
    gBirchDexRatingText_LessThan150,
    gBirchDexRatingText_LessThan160,
    gBirchDexRatingText_LessThan170,
    gBirchDexRatingText_LessThan180,
    gBirchDexRatingText_LessThan190,
    gBirchDexRatingText_LessThan200,
    gBirchDexRatingText_DexCompleted,
};

void BufferPokedexRatingForMatchCall(u8 *destStr)
{
    int numSeen, numCaught;
    u8 *str;
    u8 dexRatingLevel;

    u8 *buffer = Alloc(sizeof(gStringVar4));
    if (!buffer)
    {
        destStr[0] = EOS;
        return;
    }

    numSeen = GetHoennPokedexCount(FLAG_GET_SEEN);
    numCaught = GetHoennPokedexCount(FLAG_GET_CAUGHT);
    ConvertIntToDecimalStringN(gStringVar1, numSeen, STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar2, numCaught, STR_CONV_MODE_LEFT_ALIGN, 3);
    dexRatingLevel = GetPokedexRatingLevel(numCaught);
    str = StringCopy(buffer, gBirchDexRatingText_AreYouCurious);
    *(str++) = CHAR_PROMPT_CLEAR;
    str = StringCopy(str, gBirchDexRatingText_SoYouveSeenAndCaught);
    *(str++) = CHAR_PROMPT_CLEAR;
    StringCopy(str, sBirchDexRatingTexts[dexRatingLevel]);
    str = StringExpandPlaceholders(destStr, buffer);

    if (IsNationalPokedexEnabled())
    {
        *(str++) = CHAR_PROMPT_CLEAR;
        numSeen = GetNationalPokedexCount(FLAG_GET_SEEN);
        numCaught = GetNationalPokedexCount(FLAG_GET_CAUGHT);
        ConvertIntToDecimalStringN(gStringVar1, numSeen, STR_CONV_MODE_LEFT_ALIGN, 3);
        ConvertIntToDecimalStringN(gStringVar2, numCaught, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(str, gBirchDexRatingText_OnANationwideBasis);
    }

    Free(buffer);
}

void LoadMatchCallWindowGfx(u32 windowId, u32 destOffset, u32 paletteId)
{
    u8 bg = GetWindowAttribute(windowId, WINDOW_BG);
    LoadBgTiles(bg, sMatchCallWindow_Gfx, 0x100, destOffset);
    LoadPalette(sMatchCallWindow_Pal, BG_PLTT_ID(paletteId), sizeof(sMatchCallWindow_Pal));
}

void DrawMatchCallTextBoxBorder(u32 windowId, u32 tileOffset, u32 paletteId)
{
    DrawMatchCallTextBoxBorder_Internal(windowId, tileOffset, paletteId);
}
