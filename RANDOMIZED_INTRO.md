# Randomized Intro Animation

This PR adds a randomized intro sequence that varies each time the game starts, bringing back elements from Ruby and Sapphire while maintaining Emerald's identity.

## Feature Overview

The intro now randomly selects between three different styles with equal probability (33% each):

| Style | Legendary | Scene | Player Sprites |
|-------|-----------|-------|----------------|
| **Emerald** | Flygon | Forest scenery | Emerald Brendan/May (50/50) |
| **Ruby** | Latios | Ocean/cloud scenery | RS Brendan/May (50/50) |
| **Sapphire** | Latias | Ocean/cloud scenery | RS Brendan/May (50/50) |

## How It Works

1. **RNG Seeding**: The random number generator is seeded with the Real-Time Clock (RTC) minute count at boot
2. **Style Selection**: `sIntroStyle = Random() % 3` determines which legendary and background to use
3. **Player Selection**: A second random roll determines which player sprite variant appears

Because the RNG is seeded from the RTC, the intro style changes approximately every minute of real time. Players who restart the game will see different intros depending on when they boot.

## Technical Changes

### Files Modified
- `src/intro.c` - Intro style selection logic and sprite loading
- `src/intro_credits_graphics.c` - RS player sprites, Latios/Latias sprite definitions
- `src/data/graphics/intro_scene.h` - INCBIN declarations for RS graphics
- `include/intro_credits_graphics.h` - Function declarations
- `include/graphics.h` - Extern declarations for RS sprites
- `src/main.c` - Removed BUGFIX wrapper from `SeedRngWithRtc()` for proper randomization

### Files Added
- `graphics/intro/rs_graphics/` - RS-style intro graphics including:
  - `intro2_brendan.png`, `intro2_may.png` (RS player sprites)
  - `intro2_latios.png`, `intro2_latias.png` (Lati sprites with corrected palettes)
  - Background and cloud assets for ocean scenery

## RNG Change Note

This PR also removes the `#ifdef BUGFIX` wrapper around `SeedRngWithRtc()`, making RTC-based RNG seeding always active. This prevents RNG manipulation exploits and ensures truly random results for:
- Intro style selection
- Wild encounters
- Shiny rolls
- Other RNG-dependent events

---

Feature ported from Pokémon Emerald Veritas by In-Veritas.
