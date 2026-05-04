#include "global.h"
#include "day_night.h"
#include "event_data.h"
#include "overworld.h"
#include "palette.h"
#include "rtc.h"
#include "constants/flags.h"
#include "constants/map_types.h"
#include "constants/rgb.h"

// Toggle is gated by FLAG_OPTION_DAYNIGHT_CYCLE (set from Options Plus).
// When enabled, the overworld and battle palettes get a time-of-day blend:
//   17:00 - 18:59 -> warm sunset orange
//   19:00 - 05:59 -> cool night blue
//   06:00 - 16:59 -> no tint (normal day)
// The blend is additive (BlendPalettes), not destructive — it composes on
// top of weather and other palette effects without overwriting them.
//
// IsTimeOfDayNight() is the public night check used by battle text overrides
// (e.g., Sunny Day -> Moonlight at night). It returns TRUE only when the
// option is enabled AND it's between 19:00 and 05:59.

#define DAYNIGHT_DAWN_END    6
#define DAYNIGHT_SUNSET      17
#define DAYNIGHT_NIGHT       19

#define DAYNIGHT_TINT_COEFF  6
#define DAYNIGHT_TINT_SUNSET RGB(31, 14, 4)
#define DAYNIGHT_TINT_NIGHT  RGB(4, 6, 18)

// Palette mask layout (low 16 bits = BG palettes 0-15, high 16 bits = OBJ
// palettes 0-15):
//   BG 0-13  (0x3FFF) — map tiles, overworld objects, weather sprites
//   BG 14    — STD_WINDOW_PALETTE_NUM (standard menu/window borders)
//   BG 15    — DLG_WINDOW_PALETTE_NUM (dialogue boxes, start menu text)
//   OBJ 0-15 — all OAM sprites (player, NPCs, follower, etc.)
//
// We tint BG 0-13 + OBJ 0-15 = 0x3FFF | 0xFFFF0000 = 0xFFFF3FFF.
//
// Why exclude BG 14-15: the weather system also marks them COLOR_MAP_NONE
// (sBasePaletteColorMapTypes in field_weather.c) so they stay neutral
// during weather effects. BlendPalettes does NOT honor that table on its
// own — we have to mirror the exclusion here, otherwise dialog text and
// the start menu pick up the tint.
#define DAYNIGHT_PALETTE_MASK 0xFFFF3FFF

bool8 DayNight_IsEnabled(void)
{
    return FlagGet(FLAG_OPTION_DAYNIGHT_CYCLE);
}

// Returns TRUE when day/night is enabled AND it's currently nighttime
// (19:00–05:59). Used by battle text to swap "sunlight" for "moonlight"
// during Sunny Day, etc.
bool8 IsTimeOfDayNight(void)
{
    if (!DayNight_IsEnabled())
        return FALSE;
    RtcCalcLocalTime();
    return gLocalTime.hours >= DAYNIGHT_NIGHT || gLocalTime.hours < DAYNIGHT_DAWN_END;
}

// Maps where the player can see the sky and feel time-of-day:
//   TOWN, CITY, ROUTE, OCEAN_ROUTE — outdoor with sky
// Maps where time-of-day visually doesn't apply:
//   INDOOR / SECRET_BASE — artificial light
//   UNDERGROUND — caves, no sun reaches
//   UNDERWATER — sky is replaced by water column
//   NONE / UNKNOWN — title screen, etc.
static bool8 MapTypeAllowsTint(u8 mapType)
{
    return mapType == MAP_TYPE_TOWN
        || mapType == MAP_TYPE_CITY
        || mapType == MAP_TYPE_ROUTE
        || mapType == MAP_TYPE_OCEAN_ROUTE;
}

bool8 DayNight_GetCurrentTint(u8 *coeff, u16 *rgb)
{
    u8 hour;

    if (!DayNight_IsEnabled())
        return FALSE;

    RtcCalcLocalTime();
    hour = gLocalTime.hours;

    if (hour >= DAYNIGHT_SUNSET && hour < DAYNIGHT_NIGHT)
    {
        *coeff = DAYNIGHT_TINT_COEFF;
        *rgb = DAYNIGHT_TINT_SUNSET;
        return TRUE;
    }
    if (hour >= DAYNIGHT_NIGHT || hour < DAYNIGHT_DAWN_END)
    {
        *coeff = DAYNIGHT_TINT_COEFF;
        *rgb = DAYNIGHT_TINT_NIGHT;
        return TRUE;
    }
    return FALSE;
}

void DayNight_ApplyOverworldTint(void)
{
    u8 coeff;
    u16 rgb;

    // Skip indoor / cave / underwater maps — the sky isn't visible there.
    if (!MapTypeAllowsTint(gMapHeader.mapType))
        return;

    if (!DayNight_GetCurrentTint(&coeff, &rgb))
        return;

    BlendPalettes(DAYNIGHT_PALETTE_MASK, coeff, rgb);
}

void DayNight_ApplyBattleTint(void)
{
    u8 coeff;
    u16 rgb;

    // gMapHeader is preserved across battle entry; if the player encountered
    // the battle indoors / in a cave / underwater, the battle background
    // also represents that environment and shouldn't pick up the time-of-day
    // tint. (The Pokémon Center heal animation and similar are also indoor.)
    if (!MapTypeAllowsTint(gMapHeader.mapType))
        return;

    if (!DayNight_GetCurrentTint(&coeff, &rgb))
        return;

    // Battle palettes use the same global palette buffer; reuse the same
    // mask. If the battle scene uses palette 15 for HP/exp text it stays
    // un-tinted via DAYNIGHT_PALETTE_MASK.
    BlendPalettes(DAYNIGHT_PALETTE_MASK, coeff, rgb);
}
