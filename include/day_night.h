#ifndef GUARD_DAY_NIGHT_H
#define GUARD_DAY_NIGHT_H

bool8 DayNight_IsEnabled(void);
bool8 IsTimeOfDayNight(void);
bool8 DayNight_GetCurrentTint(u8 *coeff, u16 *rgb);
void DayNight_ApplyOverworldTint(void);
void DayNight_ApplyBattleTint(void);

#endif // GUARD_DAY_NIGHT_H
