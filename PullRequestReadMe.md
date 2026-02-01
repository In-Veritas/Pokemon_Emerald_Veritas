# Ruby/Sapphire Style Player Avatar Option

## Overview

This PR adds the ability for players to choose between Emerald-style or Ruby/Sapphire (Classic) style player sprites. The option is available during new game creation and can be changed anytime via the options menu.

## Feature Description

### Player Style Selection

Players can choose their avatar style at two points:

1. **New Game** - After selecting gender, a new "Look" selection screen appears
2. **Options Menu** - A "PLAYER STYLE" option is available in the World options

### Style Options

| Style | Description |
|-------|-------------|
| Emerald | Default Emerald protagonist sprites (Brendan/May) |
| Classic | Ruby/Sapphire style protagonist sprites |

The style affects:
- Overworld sprites (walking, running, biking, surfing, etc.)
- Battle back sprites
- Trainer card portrait
- Region map player icon

## Technical Implementation

### Storage

The player's style choice is stored using `FLAG_PLAYER_STYLE_RS` (repurposed from unused flag 0x296).

```c
// Check current style
if (FlagGet(FLAG_PLAYER_STYLE_RS))
    // Use RS/Classic sprites
else
    // Use Emerald sprites

// Set style
FlagSet(FLAG_PLAYER_STYLE_RS);   // RS/Classic
FlagClear(FLAG_PLAYER_STYLE_RS); // Emerald
```

Note: The initial implementation used a new SaveBlock2 field, but this was changed to use an existing unused flag to avoid modifying the save structure.

### Files Modified

#### Core Implementation

| File | Changes |
|------|---------|
| `include/constants/flags.h` | Added `FLAG_PLAYER_STYLE_RS` (0x296) |
| `include/constants/event_objects.h` | Added RS sprite object event IDs |
| `src/field_player_avatar.c` | RS sprite selection for overworld states |
| `src/main_menu.c` | New game style selection UI with proper window sizing |
| `src/option_plus_menu.c` | Post-game style toggle option |
| `src/pokemon.c` | Battle back sprite selection |
| `src/trainer_card.c` | Trainer card portrait selection |
| `src/region_map.c` | Region map player icon selection |
| `src/new_game.c` | Preserve style flag through game initialization |
| `src/field_effect_helpers.c` | Shadow palette fix for RS style |
| `src/event_object_movement.c` | RS palette reflection tags |
| `src/data/object_events/object_event_graphics_info.h` | RS sprite graphics info with correct animation tables |
| `src/data/object_events/object_event_pic_tables.h` | RS sprite frame tables including running frames |

#### Graphics Added

```
graphics/object_events/pics/people/ruby_sapphire_brendan/
├── walking.png
├── running.png
├── acro_bike.png
├── decorating.png
├── field_move.png
├── fishing.png
├── mach_bike.png
├── surfing.png
├── underwater.png
└── watering.png

graphics/object_events/pics/people/ruby_sapphire_may/
└── (same structure as brendan)

graphics/pokenav/region_map/
├── brendan_rs_icon.png
└── may_rs_icon.png
```

#### Data Files Modified

| File | Changes |
|------|---------|
| `src/data/object_events/object_event_graphics.h` | RS sprite graphics references |
| `src/data/object_events/object_event_graphics_info.h` | RS sprite metadata with `sAnimTable_BrendanMayNormal` |
| `src/data/object_events/object_event_graphics_info_pointers.h` | RS sprite pointers |
| `src/data/object_events/object_event_pic_tables.h` | RS sprite animation tables (walking + running) |
| `spritesheet_rules.mk` | Build rules for RS sprites |

### Option Menu Integration

The Player Style option:
- Located in: Options+ Menu -> World
- Available: Anytime (no unlock requirement)
- Options: "EMERALD" / "CLASSIC"

## New Game Flow

```
Gender Selection (Boy/Girl)
        |
Style Selection (Emerald/Classic)  <- NEW
        |
Difficulty Selection (Normal/Hard/Hardcore)
        |
National Dex Selection
        |
Name Entry
        |
Game Start
```

The style selection screen:
- Shows live preview of selected style
- Animated sprite transition when changing selection
- Press B to go back to gender selection
- Press A to confirm and continue

## Key Technical Details

### Animation Tables
RS sprites use `sAnimTable_BrendanMayNormal` instead of `sAnimTable_Standard` to properly support both walking (frames 0-8) and running (frames 9-17) animations.

### Shadow Fix
The shadow field effect copies the palette from the linked player sprite to ensure correct shadow colors regardless of which style is active.

### Flag Preservation
`FLAG_PLAYER_STYLE_RS` is preserved through `NewGameInitData()` alongside other new game flags (nuzlocke, hard mode, national dex mode).

# Random Legendary Title Screen

## Overview

This PR adds a dynamic title screen feature where the legendary Pokemon displayed rotates between Rayquaza, Groudon, and Kyogre based on the player's story progression.

## Feature Description

### Story-Progression Based Selection

The title screen legendary is no longer static. Instead, it unlocks progressively as the player advances through the story:

| Story Progress | Available Legendaries | Selection Method |
|----------------|----------------------|------------------|
| No save file | Rayquaza only | Fixed |
| New game (pre-Magma Hideout) | Rayquaza only | Fixed |
| After Groudon awakens | Rayquaza + Groudon | Random |
| After Kyogre escapes | All three | Random |

### Flag Triggers

The system uses existing story flags to determine progression:

- **`FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT`** (0x6F) - Set when the player witnesses Groudon awakening in Magma Hideout after defeating Maxie
- **`FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN`** (0x81) - Set when Kyogre escapes from Seafloor Cavern after the player defeats Archie

## Technical Implementation

### Files Modified

#### `src/title_screen.c`
- Added `#include "constants/flags.h"` for flag access
- Modified the legendary selection logic in `CB2_InitTitleScreen` (case 0)
- Selection now checks `FlagGet()` for story progression flags
- Random selection uses `Random() % 3` for all three, or `(Random() % 2) ? 2 : 0` for Rayquaza/Groudon only

### New Graphics Added

```
graphics/title_screen/
├── groudon.png
├── groudon_dark.pal
├── groudon_glow.pal
├── groudon_map.bin
├── kyogre.png
├── kyogre_dark.pal
├── kyogre_glow.pal
├── kyogre_map.bin
├── lava_map.bin
├── water_map.bin
└── rubysapphire/
    └── (RS-style variants of above)
```

### Legendary Values

The `sTitleLegendary` variable determines which legendary to display:
- `0` = Rayquaza (default, uses clouds background)
- `1` = Kyogre (uses water overlay background)
- `2` = Groudon (uses lava overlay background)

## Behavior Details

### Save File States

The system respects save file status before checking flags:
- `SAVE_STATUS_OK` - Normal save, check flags
- `SAVE_STATUS_UPDATED` - Updated save format, check flags
- `SAVE_STATUS_ERROR` - Corrupted but readable, check flags
- `SAVE_STATUS_EMPTY` / `SAVE_STATUS_CORRUPT` - No usable save, Rayquaza only

### Randomization

- Selection occurs once per title screen load (in `CB2_InitTitleScreen` case 0)
- Uses the game's built-in `Random()` function
- Player can soft-reset to see different legendaries (if multiple are unlocked)

## Testing

To test all three legendaries without playing through the story:

1. Enable debug mode in `include/debug.h`:
   ```c
   #define TX_DEBUG_SYSTEM_ENABLE TRUE
   ```

2. Add a test script to `data/scripts/debug.inc`:
   ```
   Debug_Script_1::
       lockall
       setflag FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT
       setflag FLAG_KYOGRE_ESCAPED_SEAFLOOR_CAVERN
       msgbox YourTestText, MSGBOX_DEFAULT
       releaseall
       end
   ```

3. In-game: Hold R + press START to open debug menu, run Script 1
4. Restart the game to see the title screen cycle through all three

**Remember to disable debug before release!**

## Compatibility

- No save file format changes
- No new flags introduced (uses existing story flags)
- Backwards compatible with existing saves
- Players with post-Seafloor saves will immediately have all three legendaries available

## Credits

Feature ported from Pokemon Emerald Veritas by AxolotlOfTruth (Gabriel)
