Common_EventScript_PkmnCenterNurse::
	lock
	faceplayer
	setvar VAR_0x8004, 0
	@ Birthday greeting — every Pokémon Center nurse wishes the player a
	@ happy birthday on their declared birthday (see IsPlayerBirthdayToday).
	specialvar VAR_RESULT, IsPlayerBirthdayToday
	call_if_eq VAR_RESULT, TRUE, EventScript_PkmnCenterNurse_Birthday
	@ Friend's birthday — if a carried Pokémon's original TRAINER has their
	@ birthday today, remind the player to send wishes (dialogue only).
	specialvar VAR_RESULT, BufferPartyBirthdayTrainerName
	call_if_eq VAR_RESULT, TRUE, EventScript_PkmnCenterNurse_FriendBirthday
	@ Save anniversary — VAR_RESULT holds the year number (0 = not today).
	specialvar VAR_RESULT, IsSaveAnniversaryToday
	call_if_ne VAR_RESULT, 0, EventScript_PkmnCenterNurse_SaveAnniv
	@ Spooky Day (31/10, National Dex required) — an eerie greeting.
	specialvar VAR_RESULT, IsSpookyDayToday
	call_if_eq VAR_RESULT, TRUE, EventScript_PkmnCenterNurse_SpookyDay
	@ Hoenn Day (21/11) — announce the regional celebration and its perks.
	specialvar VAR_RESULT, IsHoennDayToday
	call_if_eq VAR_RESULT, TRUE, EventScript_PkmnCenterNurse_HoennDay
	@ Luvdisc Day (14/02) — announce the romance.
	specialvar VAR_RESULT, IsLuvdiscDayToday
	call_if_eq VAR_RESULT, TRUE, EventScript_PkmnCenterNurse_LuvdiscDay
	@ Delivery Day (1st of the month) — point at the Slateport market.
	specialvar VAR_RESULT, IsDeliveryDayToday
	call_if_eq VAR_RESULT, TRUE, EventScript_PkmnCenterNurse_DeliveryDay
	specialvar VAR_RESULT, CountPlayerTrainerStars
	goto_if_eq VAR_RESULT, 4, EventScript_PkmnCenterNurse_GoldCard
	@ If short dialog option is enabled, skip the yes/no question
	goto_if_set FLAG_ENABLE_SHORT_POKECENTER, Common_EventScript_PkmnCenterNurse_ShortDialog
	@ Vanilla behavior: always ask the yes/no question
	msgbox gText_WouldYouLikeToRestYourPkmn, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, EventScript_PkmnCenterNurse_HealPkmn
	goto_if_eq VAR_RESULT, NO, EventScript_PkmnCenterNurse_Goodbye
	end

Common_EventScript_PkmnCenterNurse_ShortDialog::
	@ Short dialog: skip straight to healing
	goto EventScript_PkmnCenterNurse_HealPkmn
	end

EventScript_PkmnCenterNurse_Goodbye::
	message gText_WeHopeToSeeYouAgain
	return

@ VAR_0x8004 is 1 when player has Gold Card; jumps are identical
EventScript_PkmnCenterNurse_HealPkmn::
	incrementgamestat GAME_STAT_USED_POKECENTER
	call_if_eq VAR_0x8004, 0, EventScript_PkmnCenterNurse_IllTakeYourPkmn
	call_if_eq VAR_0x8004, 1, EventScript_PkmnCenterNurse_IllTakeYourPkmn2
	waitmessage
	call EventScript_PkmnCenterNurse_TakeAndHealPkmn
	goto_if_unset FLAG_POKERUS_EXPLAINED, EventScript_PkmnCenterNurse_CheckPokerus
	goto EventScript_PkmnCenterNurse_CheckTrainerHillAndUnionRoom
	end

EventScript_PkmnCenterNurse_IllTakeYourPkmn::
	message gText_IllTakeYourPkmn
	return

EventScript_PkmnCenterNurse_IllTakeYourPkmn2::
	message gText_IllTakeYourPkmn2
	return

EventScript_PkmnCenterNurse_TakeAndHealPkmn::
	hidefollower 0
	applymovement VAR_0x800B, Movement_PkmnCenterNurse_Turn
	waitmovement 0
	dofieldeffect FLDEFF_POKECENTER_HEAL
	waitfieldeffect FLDEFF_POKECENTER_HEAL
	applymovement VAR_0x800B, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	special HealPlayerParty
	callnative UpdateFollowingPokemon
	return

EventScript_PkmnCenterNurse_CheckTrainerHillAndUnionRoom::
	specialvar VAR_RESULT, PlayerNotAtTrainerHillEntrance
	goto_if_eq VAR_RESULT, 0, EventScript_PkmnCenterNurse_ReturnPkmn
	specialvar VAR_RESULT, BufferUnionRoomPlayerName
	copyvar VAR_0x8008, VAR_RESULT
	goto_if_eq VAR_0x8008, 0, EventScript_PkmnCenterNurse_ReturnPkmn
	goto_if_eq VAR_0x8008, 1, EventScript_PkmnCenterNurse_PlayerWaitingInUnionRoom
	end

@ VAR_0x8004 is 1 when player has Gold Card
EventScript_PkmnCenterNurse_ReturnPkmn::
	goto_if_eq VAR_0x8004, 1, EventScript_PkmnCenterNurse_ReturnPkmn2
	message gText_RestoredPkmnToFullHealth
	waitmessage
	applymovement VAR_0x800B, Movement_PkmnCenterNurse_Bow
	waitmovement 0
	message gText_WeHopeToSeeYouAgain
	goto EventScript_PkmnCenterNurse_PlayerTurn
	return

EventScript_PkmnCenterNurse_ReturnPkmn2::
	message gText_ThankYouForWaiting
	waitmessage
	applymovement VAR_0x800B, Movement_PkmnCenterNurse_Bow
	waitmovement 0
	message gText_WeHopeToSeeYouAgain2
	goto EventScript_PkmnCenterNurse_PlayerTurn
	return

EventScript_PkmnCenterNurse_PlayerWaitingInUnionRoom::
	goto_if_set FLAG_NURSE_UNION_ROOM_REMINDER, EventScript_PkmnCenterNurse_ReturnPkmn
	msgbox gText_RestoredPkmnToFullHealth, MSGBOX_DEFAULT
	setflag FLAG_NURSE_UNION_ROOM_REMINDER
	message CableClub_Text_PlayerIsWaiting
	waitmessage
	applymovement VAR_0x800B, Movement_PkmnCenterNurse_Bow
	waitmovement 0
	message gText_WeHopeToSeeYouAgain
	return

EventScript_PkmnCenterNurse_CheckPokerus::
	specialvar VAR_RESULT, IsPokerusInParty
	goto_if_eq VAR_RESULT, TRUE, EventScript_PkmnCenterNurse_ExplainPokerus
	goto_if_eq VAR_RESULT, FALSE, EventScript_PkmnCenterNurse_CheckTrainerHillAndUnionRoom
	end

EventScript_PkmnCenterNurse_ExplainPokerus::
	message gText_PokerusExplanation
	setflag FLAG_POKERUS_EXPLAINED
	return

EventScript_PkmnCenterNurse_GoldCard::
	goto_if_set FLAG_NURSE_MENTIONS_GOLD_CARD, EventScript_PkmnCenterNurse_AskForUsual
	setflag FLAG_NURSE_MENTIONS_GOLD_CARD
	msgbox gText_WelcomeCutShort, MSGBOX_DEFAULT
	playse SE_PIN
	applymovement VAR_0x800B, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement VAR_0x800B, Common_Movement_Delay48
	waitmovement 0
	msgbox gText_NoticesGoldCard, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, EventScript_PkmnCenterNurse_GoldCardHealPkmn
	message gText_WeHopeToSeeYouAgain2
	return

EventScript_PkmnCenterNurse_AskForUsual::
	msgbox gText_YouWantTheUsual, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, EventScript_PkmnCenterNurse_GoldCardHealPkmn
	message gText_WeHopeToSeeYouAgain2
	return

EventScript_PkmnCenterNurse_GoldCardHealPkmn::
	setvar VAR_0x8004, 1
	goto EventScript_PkmnCenterNurse_HealPkmn
	end

Movement_PkmnCenterNurse_Turn:
  walk_in_place_faster_left
  step_end

Movement_PkmnCenterNurse_Bow:
	nurse_joy_bow
	delay_4
	step_end

EventScript_PkmnCenterNurse_PlayerTurn::
	closemessage
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_FaceDown
	waitmovement 0
	release
	end

@ Birthday: the May rival theme starts the moment she says it and keeps
@ playing until the player leaves the room (save_song FALSE, so any map
@ change loads that map's music normally). The family hint plays until
@ Mom's one-time Shiny Charm gift has been claimed.
EventScript_PkmnCenterNurse_Birthday::
	playbgm MUS_ENCOUNTER_MAY, FALSE
	msgbox gText_NurseHappyBirthday, MSGBOX_DEFAULT
	goto_if_set FLAG_RECEIVED_BIRTHDAY_SHINY_CHARM, EventScript_PkmnCenterNurse_BirthdayDone
	msgbox gText_NurseBirthdayFamilyHint, MSGBOX_DEFAULT
EventScript_PkmnCenterNurse_BirthdayDone::
	return

EventScript_PkmnCenterNurse_FriendBirthday::
	msgbox gText_NurseFriendBirthday, MSGBOX_DEFAULT
	return

EventScript_PkmnCenterNurse_SpookyDay::
	msgbox gText_NurseSpookyDay, MSGBOX_DEFAULT
	return

EventScript_PkmnCenterNurse_HoennDay::
	msgbox gText_NurseHoennDay, MSGBOX_DEFAULT
	return

EventScript_PkmnCenterNurse_LuvdiscDay::
	msgbox gText_NurseLuvdiscDay, MSGBOX_DEFAULT
	return

EventScript_PkmnCenterNurse_DeliveryDay::
	msgbox gText_NurseDeliveryDay, MSGBOX_DEFAULT
	return

@ Save anniversary greeting. VAR_RESULT holds the year number; the flavor
@ gets quirkier as the years pile up.
EventScript_PkmnCenterNurse_SaveAnniv::
	buffernumberstring STR_VAR_1, VAR_RESULT
	goto_if_eq VAR_RESULT, 1, EventScript_PkmnCenterNurse_SaveAnniv1
	goto_if_lt VAR_RESULT, 5, EventScript_PkmnCenterNurse_SaveAnnivFew
	goto_if_lt VAR_RESULT, 10, EventScript_PkmnCenterNurse_SaveAnnivMany
	goto_if_lt VAR_RESULT, 25, EventScript_PkmnCenterNurse_SaveAnnivLots
	msgbox gText_NurseSaveAnnivAncient, MSGBOX_DEFAULT
	return

EventScript_PkmnCenterNurse_SaveAnniv1::
	msgbox gText_NurseSaveAnniv1, MSGBOX_DEFAULT
	return

EventScript_PkmnCenterNurse_SaveAnnivFew::
	msgbox gText_NurseSaveAnnivFew, MSGBOX_DEFAULT
	return

EventScript_PkmnCenterNurse_SaveAnnivMany::
	msgbox gText_NurseSaveAnnivMany, MSGBOX_DEFAULT
	return

EventScript_PkmnCenterNurse_SaveAnnivLots::
	msgbox gText_NurseSaveAnnivLots, MSGBOX_DEFAULT
	return
