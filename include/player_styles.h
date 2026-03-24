#ifndef GUARD_PLAYER_STYLES_H
#define GUARD_PLAYER_STYLES_H

#define PLAYER_STYLE_NONE 0

u16 GetPlayerStyle(void);
void ApplyPlayerStyleToOWPalette(u8 paletteSlot, bool8 isFemale);
void ApplyPlayerStyleToTrainerPalette(u16 paletteOffset, bool8 isFemale);
void RefreshPlayerOWPalette(void);
void PlayerStyleOverworldUpdate(void);
void ShowStyleMenu(u8 taskId);

#endif // GUARD_PLAYER_STYLES_H
