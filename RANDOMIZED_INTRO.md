# Randomized Intro Animation

This PR adds a randomized intro sequence that varies each time the game starts, bringing back elements from Ruby and Sapphire while maintaining Emerald's identity.

## Feature Overview

The intro now randomly selects between three different styles with equal probability (33% each):

| Style | Legendary | Scene | Player Sprites |
|-------|-----------|-------|----------------|
| **Emerald** | Flygon | Forest scenery | Emerald Brendan/May (50/50) |
| **Ruby** | Latios | Ocean scenery | RS Brendan/May (50/50) |
| **Sapphire** | Latias | Ocean scenery | RS Brendan/May (50/50) |

This gives 12 different possible animations.

## How It Works

1. **VBlank Counter**: The intro uses `gMain.vblankCounter1` for randomization, which increments every frame (60 times per second)
2. **Style Selection**: `sIntroStyle = gMain.vblankCounter1 % 3` determines which legendary and background to use
3. **Player Selection**: `(gMain.vblankCounter1 / 3) % 2` determines which player sprite variant appears

This approach ensures visual variety without affecting the main game RNG, preserving RNG manipulation compatibility for speedrunners and challenge players.

## Technical Changes

### Files Modified
- `src/intro.c` - Intro style selection logic and sprite loading
- `src/intro_credits_graphics.c` - RS player sprites, Latios/Latias sprite definitions
- `src/data/graphics/intro_scene.h` - INCBIN declarations for RS graphics
- `include/intro_credits_graphics.h` - Function declarations
- `include/graphics.h` - Extern declarations for RS sprites

### Files Added
- `graphics/intro/rs_graphics/` - RS-style intro graphics including:
  - `intro2_brendan.png`, `intro2_may.png` (RS player sprites)
  - `intro2_latios.png`, `intro2_latias.png` (Lati sprites with corrected palettes)
  - Background and cloud assets for ocean scenery

---

Feature ported from Pokémon Emerald Veritas by In-Veritas.
