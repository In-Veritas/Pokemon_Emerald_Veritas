#ifndef GUARD_FIELD_SPECIALS_H
#define GUARD_FIELD_SPECIALS_H

extern bool8 gBikeCyclingChallenge;
extern u8 gBikeCollisions;

u8 GetLeadMonIndex(void);
u8 IsDestinationBoxFull(void);
u16 GetPCBoxToSendMon(void);
bool8 InMultiPartnerRoom(void);
void UpdateTrainerFansAfterLinkBattle(void);
void IncrementBirthIslandRockStepCount(void);
bool8 AbnormalWeatherHasExpired(void);
bool8 ShouldDoBrailleRegicePuzzle(void);
bool32 ShouldDoWallyCall(void);
bool32 ShouldDoScottFortreeCall(void);
bool32 ShouldDoScottBattleFrontierCall(void);
bool32 ShouldDoRoxanneCall(void);
bool32 ShouldDoRivalRayquazaCall(void);
bool32 ShouldDoNormanRematchCall(void);
bool32 ShouldUpdateGymLeaderRematches(void);
bool32 CountSSTidalStep(u16 delta);
u8 GetSSTidalLocation(s8 *mapGroup, s8 *mapNum, s16 *x, s16 *y);
void ShowScrollableMultichoice(void);
void FrontierGamblerSetWonOrLost(bool8 won);
u8 TryGainNewFanFromCounter(u8 incrementId);
bool8 InPokemonCenter(void);
void SetShoalItemFlag(u16 unused);
void UpdateFrontierManiac(u16 daysSince);
void UpdateFrontierGambler(u16 daysSince);
void ResetCyclingRoadChallengeData(void);
bool8 UsedPokemonCenterWarp(void);
void ResetFanClub(void);
bool8 ShouldShowBoxWasFullMessage(void);
void SetPCBoxToSendMon(u8 boxId);
void CleanInvalidTrainerRecords(void);
bool8 TrySetupInvalidRecordCleanupMessage(void);
bool8 TrySetupPurpleBirthdayGift(void);
bool8 TrySetupAlhyaApology(void);
bool8 TrySetupDateNotSetReminder(void);
bool8 IsCalendarDateSet(void);
void GetVirtualDate(u8 *month, u8 *day);
u16 IsPlayerBirthdayToday(void);
u16 IsSpookyDayToday(void);
u16 GetRandomSpookySpecies(void);
u16 IsHoennDayToday(void);
u16 IsTrickDayToday(void);
u16 IsLuvdiscDayToday(void);
u16 IsGiftDayToday(void);
u16 GetRandomGiftDaySpecies(bool8 isWater);
void CreateGiftDelibird(void);
u16 IsDeliveryDayToday(void);
bool8 IsHoennNativeSpecies(u16 species);
u16 IsSaveAnniversaryToday(void);
void RecordSaveAnniversaryIfEligible(void);
bool8 IsAnySpecialDateToday(void);
void DeliveryDay_GetTodaysOffer(void);
void DeliveryDay_TryPurchase(void);
void IsSelectedPartyMonShiny(void);
void CreateCookieZigzagoon(void);
void SetCookieTradeOverworldToOfferedShiny(void);
void PrepareTradeFloatingMonGreeting(void);
void IsSelectedMonGabrielsCookie(void);

#endif // GUARD_FIELD_SPECIALS_H
