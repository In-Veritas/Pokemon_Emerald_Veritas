#ifndef GUARD_PLAYER_STYLES_H
#define GUARD_PLAYER_STYLES_H

#define PLAYER_STYLE_NONE 0

u16 GetPlayerStyle(void);
void ApplyPlayerStyleToOWPalette(u8 paletteSlot, bool8 isFemale);
void ApplyStyleToOWPaletteById(u8 styleId, u8 paletteSlot, bool8 isFemale);
void ApplyStyleToTrainerPaletteById(u8 styleId, u16 paletteOffset, bool8 isFemale);

// Decode packed link player style byte
#define LINK_STYLE_ID(packed)    ((packed) & 0x3F)
#define LINK_STYLE_IS_RS(packed) (((packed) & 0x40) != 0)
void ApplyPlayerStyleToTrainerPalette(u16 paletteOffset, bool8 isFemale);
void RefreshPlayerOWPalette(void);
void PlayerStyleOverworldUpdate(void);
void ShowStyleMenu(u8 taskId);

// Fashionista NPC specials
void Fashionista_CountPassiveUnlocks(void);
void Fashionista_GetNextPassiveUnlock(void);
void Fashionista_HasShowablePokemon(void);
void Fashionista_CheckShownPokemon(void);
void Fashionista_UnlockOutfit(void);
void Fashionista_GetTip(void);

// Hook functions
void MarkContestWonForOutfit(void);
void MarkShinyCaughtForOutfit(void);
void CheckPartyForStarterUnlocks(void);

#endif // GUARD_PLAYER_STYLES_H
