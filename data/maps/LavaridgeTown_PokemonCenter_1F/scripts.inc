.set LOCALID_NURSE, 1

LavaridgeTown_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, LavaridgeTown_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

LavaridgeTown_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_LAVARIDGE_TOWN
	call Common_EventScript_UpdateBrineyLocation
	end

LavaridgeTown_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

LavaridgeTown_PokemonCenter_1F_EventScript_Youngster::
	msgbox LavaridgeTown_PokemonCenter_1F_Text_HotSpringCanInvigorate, MSGBOX_NPC
	end

LavaridgeTown_PokemonCenter_1F_EventScript_Woman::
	msgbox LavaridgeTown_PokemonCenter_1F_Text_TrainersPokemonSpendTimeTogether, MSGBOX_NPC
	end

LavaridgeTown_PokemonCenter_1F_EventScript_Gentleman::
	msgbox LavaridgeTown_PokemonCenter_1F_Text_TrainersShouldRestToo, MSGBOX_NPC
	end

LavaridgeTown_PokemonCenter_1F_EventScript_May::
	lock
	faceplayer
	msgbox LavaridgeTown_PokemonCenter_1F_Text_MayTravelToPetalburg, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, LavaridgeTown_PokemonCenter_1F_EventScript_TravelToPetalburg
	goto_if_eq VAR_RESULT, NO, LavaridgeTown_PokemonCenter_1F_EventScript_RivalWaits
	end

LavaridgeTown_PokemonCenter_1F_EventScript_Brendan::
	lock
	faceplayer
	msgbox LavaridgeTown_PokemonCenter_1F_Text_BrendanTravelToPetalburg, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, LavaridgeTown_PokemonCenter_1F_EventScript_TravelToPetalburg
	goto_if_eq VAR_RESULT, NO, LavaridgeTown_PokemonCenter_1F_EventScript_RivalWaits
	end

LavaridgeTown_PokemonCenter_1F_EventScript_TravelToPetalburg::
	msgbox LavaridgeTown_PokemonCenter_1F_Text_RivalTravelToPetalburgLeaves, MSGBOX_DEFAULT
	setvar VAR_PETALBURG_CITY_STATE, 6
	setflag FLAG_HIDE_PETALBURG_CITY_MAY
	setflag FLAG_HIDE_PETALBURG_CITY_BRENDAN
	checkplayergender
	call_if_eq VAR_RESULT, MALE, LavaridgeTown_PokemonCenter_1F_EventScript_PetalburgCity_LoadMay
	call_if_eq VAR_RESULT, FEMALE, LavaridgeTown_PokemonCenter_1F_EventScript_PetalburgCity_LeadBrendan
LavaridgeTown_PokemonCenter_1F_EventScript_PetalburgCity_LoadMay::
	clearflag FLAG_HIDE_PETALBURG_CITY_MAY
	goto LavaridgeTown_PokemonCenter_1F_EventScript_PetalburgWarp
LavaridgeTown_PokemonCenter_1F_EventScript_PetalburgCity_LeadBrendan::
	clearflag FLAG_HIDE_PETALBURG_CITY_BRENDAN
LavaridgeTown_PokemonCenter_1F_EventScript_PetalburgWarp::
	special HealPlayerParty
	warp MAP_PETALBURG_CITY, 15, 12
	waitstate
	savebgm MUS_DUMMY
	fadedefaultbgm
	setflag FLAG_HIDE_LAVARIDGE_POKEMON_CENTER_BRENDAN
	setflag FLAG_HIDE_LAVARIDGE_POKEMON_CENTER_MAY
	releaseall
	end

LavaridgeTown_PokemonCenter_1F_EventScript_RivalWaits:
	msgbox LavaridgeTown_PokemonCenter_1F_Text_RivalWaits, MSGBOX_DEFAULT
	closemessage
	releaseall
	end

LavaridgeTown_PokemonCenter_1F_Text_TrainersPokemonSpendTimeTogether:
	.string "I think POKéMON get closer to their\n"
	.string "TRAINERS if they spend time together.\p"
	.string "The longer the better.\n"
	.string "That's what I think.$"

LavaridgeTown_PokemonCenter_1F_Text_HotSpringCanInvigorate:
	.string "It's sort of magical how just sitting\n"
	.string "in a hot-spring pool can invigorate.\p"
	.string "I wish I could let my POKéMON\n"
	.string "soak, too.$"

LavaridgeTown_PokemonCenter_1F_Text_TrainersShouldRestToo:
	.string "Hohoho! Hey, kid, you can reach\n"
	.string "the hot springs from here.\p"
	.string "If POKéMON are getting rest, so too\n"
	.string "should their TRAINERS.$"

LavaridgeTown_PokemonCenter_1F_Text_MayTravelToPetalburg:
	.string "MAY: I'm headed to PETALBURG'S GYM\n"
	.string "next…\p"
	.string "Do you want to go back together?\n"
	.string "Back to PETALBURG?$"

LavaridgeTown_PokemonCenter_1F_Text_BrendanTravelToPetalburg:
	.string "BRENDAN: I'm headed to PETALBURG'S GYM\n"
	.string "next…\p"
	.string "Do you want to go back together?\n"
	.string "Back to PETALBURG?$"

LavaridgeTown_PokemonCenter_1F_Text_RivalTravelToPetalburgLeaves:
	.string "Aye aye!\n"
	.string "Then let's hit the road!$"

LavaridgeTown_PokemonCenter_1F_Text_RivalWaits:
	.string "Oh, that's okay,\p"
	.string "I'll be here for a while, just in\n"
	.string "case you change your mind.\p"
	.string "See you around!$"
