#include "global.h"
#include "event_data.h"
#include "rtc.h"
#include "time_events.h"
#include "field_specials.h"
#include "lottery_corner.h"
#include "dewford_trend.h"
#include "tv.h"
#include "field_weather.h"
#include "berry.h"
#include "main.h"
#include "overworld.h"
#include "wallclock.h"
#include "constants/game_stat.h"

// Veritas: on a dead-battery or bootleg cartridge the RTC never advances, so
// the vanilla per-minute berry update always sees a zero time delta and
// planted berries never grow. When the clock is detected as frozen (see
// RtcIsClockNotRunning, src/rtc.c), berry growth is driven by the player's
// step count instead: a synthetic clock ticks one minute every
// STEPS_PER_BERRY_MINUTE steps. This is calibrated so a typical 3-hour-per-
// stage berry (the most common) takes ~12000 steps per growth stage; faster
// and slower berries scale in proportion, preserving berry balance. All state
// lives in the existing gSaveBlock2Ptr->lastBerryTreeUpdate field (which is
// inert while the clock is frozen) plus FLAG_BERRY_STEP_BASELINE_SET, so this
// changes NO save structure.
#define STEPS_PER_BERRY_MINUTE 67

static void UpdatePerDay(struct Time *localTime);
static void UpdatePerMinute(struct Time *localTime);
static void UpdateBerriesByStep(void);
static void StepsToSyntheticBerryTime(u32 steps, struct Time *out);

static void InitTimeBasedEvents(void)
{
    FlagSet(FLAG_SYS_CLOCK_SET);
    RtcCalcLocalTime();
    gSaveBlock2Ptr->lastBerryTreeUpdate = gLocalTime;
    VarSet(VAR_DAYS, gLocalTime.days);
}

void DoTimeBasedEvents(void)
{
    if (FlagGet(FLAG_SYS_CLOCK_SET) && !InPokemonCenter())
    {
        RtcCalcLocalTime();
        UpdatePerDay(&gLocalTime);
        UpdatePerMinute(&gLocalTime);
    }
}

static void UpdatePerDay(struct Time *localTime)
{
    u16 *days = GetVarPointer(VAR_DAYS);
    u16 daysSince;

    if (*days != localTime->days && *days <= localTime->days)
    {
        daysSince = localTime->days - *days;
        ClearDailyFlags();
        UpdateDewfordTrendPerDay(daysSince);
        UpdateTVShowsPerDay(daysSince);
        UpdateWeatherPerDay(daysSince);
        UpdatePartyPokerusTime(daysSince);
        UpdateMirageRnd(daysSince);
        UpdateBirchState(daysSince);
        UpdateFrontierManiac(daysSince);
        UpdateFrontierGambler(daysSince);
        SetShoalItemFlag(daysSince);
        SetRandomLotteryNumber(daysSince);
        *days = localTime->days;
    }
}

// Convert a lifetime step count into a synthetic wall-clock time for berries.
// GAME_STAT_STEPS is capped at 0xFFFFFF (~16.7M); divided by the rate that is
// ~250k minutes = ~173 days, which fits struct Time's s16 day field.
static void StepsToSyntheticBerryTime(u32 steps, struct Time *out)
{
    u32 totalMinutes = steps / STEPS_PER_BERRY_MINUTE;

    out->days = totalMinutes / (24 * 60);
    out->hours = (totalMinutes / 60) % 24;
    out->minutes = totalMinutes % 60;
    out->seconds = 0;
}

// Dead-clock berry growth. Mirrors the vanilla per-minute update but derives
// elapsed time from steps instead of the RTC, storing its running position in
// the same lastBerryTreeUpdate field the RTC path uses.
static void UpdateBerriesByStep(void)
{
    struct Time synthetic;
    struct Time difference;
    int minutes;

    StepsToSyntheticBerryTime(GetGameStat(GAME_STAT_STEPS), &synthetic);

    // First frozen-clock update only captures the baseline, so an existing
    // save that already has a large step count does not dump one huge growth
    // (which would blow past BerryTreeTimeUpdate's death threshold and kill
    // every planted tree).
    if (!FlagGet(FLAG_BERRY_STEP_BASELINE_SET))
    {
        gSaveBlock2Ptr->lastBerryTreeUpdate = synthetic;
        FlagSet(FLAG_BERRY_STEP_BASELINE_SET);
        return;
    }

    CalcTimeDifference(&difference, &gSaveBlock2Ptr->lastBerryTreeUpdate, &synthetic);
    minutes = 24 * 60 * difference.days + 60 * difference.hours + difference.minutes;
    if (minutes > 0)
    {
        BerryTreeTimeUpdate(minutes);
        gSaveBlock2Ptr->lastBerryTreeUpdate = synthetic;
    }
}

static void UpdatePerMinute(struct Time *localTime)
{
    struct Time difference;
    int minutes;

    // Veritas: frozen clock -> grow berries by steps instead of by time.
    if (RtcIsClockNotRunning())
    {
        UpdateBerriesByStep();
        return;
    }

    // Clock is running. If we had been in step mode (e.g. a save that moved
    // from a dead-battery cart to a working RTC), drop the latch and rebaseline
    // to real time so the RTC path resumes cleanly. On a normal working cart
    // the latch is never set, so this is a no-op and behaviour is unchanged.
    if (FlagGet(FLAG_BERRY_STEP_BASELINE_SET))
    {
        FlagClear(FLAG_BERRY_STEP_BASELINE_SET);
        gSaveBlock2Ptr->lastBerryTreeUpdate = *localTime;
        return;
    }

    CalcTimeDifference(&difference, &gSaveBlock2Ptr->lastBerryTreeUpdate, localTime);
    minutes = 24 * 60 * difference.days + 60 * difference.hours + difference.minutes;
    if (minutes != 0)
    {
        if (minutes >= 0)
        {
            BerryTreeTimeUpdate(minutes);
            gSaveBlock2Ptr->lastBerryTreeUpdate = *localTime;
        }
    }
}

static void ReturnFromStartWallClock(void)
{
    InitTimeBasedEvents();
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

void StartWallClock(void)
{
    SetMainCallback2(CB2_StartWallClock);
    gMain.savedCallback = ReturnFromStartWallClock;
}
