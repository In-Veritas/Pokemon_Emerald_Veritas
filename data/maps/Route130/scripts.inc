.set LOCALID_CELEBI, 5

Route130_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route130_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Route130_OnFrame
	.byte 0

Route130_OnFrame:
	map_script_2 VAR_ROUTE130_STATE, 1, Route130_EventScript_Celebi1
	.2byte 0

Route130_EventScript_Celebi1::
	fadescreen FADE_TO_WHITE
	setobjectxyperm LOCALID_CELEBI, 41, 22
	addobject LOCALID_CELEBI
	special Script_FadeOutMapMusic
	applymovement LOCALID_CELEBI, Route130_Movement_Levitate
	fadescreen FADE_FROM_WHITE
	waitmovement 0
	playmoncry SPECIES_CELEBI, CRY_MODE_ENCOUNTER
	delay 80
	waitmoncry
	fadescreen FADE_TO_WHITE
	removeobject LOCALID_CELEBI
	special Script_FadeOutMapMusic
	fadescreen FADE_FROM_WHITE
	setvar VAR_ROUTE130_STATE, 2
	releaseall
	end

Route130_EventScript_GoToCelebiR::
	applymovement OBJ_EVENT_ID_PLAYER, Route130_Movement_WalkLeft
	waitmovement 0
Route130_EventScript_GoToCelebiL::
	applymovement OBJ_EVENT_ID_PLAYER, Route130_Movement_WalkToCelebi
	waitmovement 0
Route130_EventScript_Celebi2::
	fadescreen FADE_TO_WHITE
	setobjectxyperm LOCALID_CELEBI, 31, 11
	addobject LOCALID_CELEBI
	special Script_FadeOutMapMusic
	fadescreen FADE_FROM_WHITE
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_FaceLeft
	playmoncry SPECIES_CELEBI, CRY_MODE_ENCOUNTER
	delay 80
	waitmoncry
	fadescreen FADE_TO_WHITE
	removeobject LOCALID_CELEBI
	special Script_FadeOutMapMusic
	fadescreen FADE_FROM_WHITE
	delay 150

	fadescreen FADE_TO_WHITE
	setobjectxyperm LOCALID_CELEBI, 38, 9
	addobject LOCALID_CELEBI
	special Script_FadeOutMapMusic
	fadescreen FADE_FROM_WHITE
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_FaceUp
	playmoncry SPECIES_CELEBI, CRY_MODE_ENCOUNTER
	delay 80
	waitmoncry
	fadescreen FADE_TO_WHITE
	removeobject LOCALID_CELEBI
	special Script_FadeOutMapMusic
	fadescreen FADE_FROM_WHITE
	delay 150

	fadescreen FADE_TO_WHITE
	setobjectxyperm LOCALID_CELEBI, 40, 16
	addobject LOCALID_CELEBI
	special Script_FadeOutMapMusic
	fadescreen FADE_FROM_WHITE
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_FaceRight
	playmoncry SPECIES_CELEBI, CRY_MODE_ENCOUNTER
	delay 80
	waitmoncry
	fadescreen FADE_TO_WHITE
	removeobject LOCALID_CELEBI
	special Script_FadeOutMapMusic
	fadescreen FADE_FROM_WHITE
	delay 250

	fadescreen FADE_TO_WHITE
	setobjectxyperm LOCALID_CELEBI, 36, 11
	addobject LOCALID_CELEBI
	special Script_FadeOutMapMusic
	fadescreen FADE_FROM_WHITE
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_FaceUp
	playmoncry SPECIES_CELEBI, CRY_MODE_ENCOUNTER
	delay 80
	waitmoncry
	setvar VAR_LAST_TALKED, LOCALID_CELEBI
	seteventmon SPECIES_CELEBI, 30
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	special BattleSetup_StartLegendaryBattle
	waitstate
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, Route130_EventScript_CelebiDefeated
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, Route130_EventScript_CelebiRunsAway
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, Route130_EventScript_CelebiRunsAway
	setflag FLAG_BATTLED_CELEBI
	goto Route130_EventScript_EndCelebiEvent
	end

Route130_EventScript_CelebiDefeated::
	setflag FLAG_BATTLED_CELEBI
	setvar VAR_0x8004, SPECIES_CELEBI
	fadescreenswapbuffers FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreenswapbuffers FADE_FROM_BLACK
	bufferspeciesname STR_VAR_1, VAR_0x8004
	msgbox gText_LegendaryFlewAway, MSGBOX_DEFAULT
	goto Route130_EventScript_EndCelebiEvent
	end

Route130_EventScript_CelebiRunsAway::
	setvar VAR_0x8004, SPECIES_CELEBI
	fadescreenswapbuffers FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreenswapbuffers FADE_FROM_BLACK
	bufferspeciesname STR_VAR_1, VAR_0x8004
	msgbox gText_LegendaryFlewAway, MSGBOX_DEFAULT
	goto Route130_EventScript_EndCelebiEvent
	end

Route130_EventScript_EndCelebiEvent::
	ClearFlag FLAG_SHOW_MIRAGE_ISLAND
	setvar VAR_ROUTE130_STATE, 0
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, Route130_EventScript_EndCelebiEventBrendansHouse
	goto_if_eq VAR_RESULT, FEMALE, Route130_EventScript_EndCelebiEventMaysHouse
Route130_EventScript_EndCelebiEventBrendansHouse::
	warp MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F, 5, 3
	waitstate
	releaseall
	end
Route130_EventScript_EndCelebiEventMaysHouse::
	warp MAP_LITTLEROOT_TOWN_MAYS_HOUSE_2F, 3, 3
	waitstate
	releaseall
	end

Route130_OnTransition:
	setflag FLAG_HIDE_CELEBI
	call_if_ge VAR_SOOTOPOLIS_CITY_STATE, 4, Route130_EventScript_CheckSetAbnormalWeather
	specialvar VAR_RESULT, IsMirageIslandPresent
	goto_if_eq VAR_RESULT, TRUE, Route130_EventScript_SetMirageIslandLayout
	goto_if_set FLAG_SHOW_MIRAGE_ISLAND, Route130_EventScript_SetMirageIslandLayout
@ Mirage Island isn't currently visible, hide any objects on the island. Only the first flag is actually associated with anything.
	setflag FLAG_TEMP_HIDE_MIRAGE_ISLAND_BERRY_TREE
	setflag FLAG_TEMP_12
	setflag FLAG_TEMP_13
	setflag FLAG_TEMP_14
	setflag FLAG_TEMP_15
	setflag FLAG_TEMP_16
	setflag FLAG_TEMP_17
	setflag FLAG_TEMP_18
	setflag FLAG_TEMP_19
	setflag FLAG_TEMP_1A
	setflag FLAG_TEMP_1B
	setflag FLAG_TEMP_1C
	setflag FLAG_TEMP_1D
	setflag FLAG_TEMP_1E
	setflag FLAG_TEMP_1F
	setmaplayoutindex LAYOUT_ROUTE130
	end

Route130_Movement_WalkLeft:
	walk_left
	step_end

Route130_Movement_WalkToCelebi:
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	step_end

Route130_Movement_Levitate:
	levitate
	step_end

Route130_EventScript_SetMirageIslandLayout::
	setmaplayoutindex LAYOUT_ROUTE130_MIRAGE_ISLAND
	end

Route130_EventScript_CheckSetAbnormalWeather::
	call_if_set FLAG_SYS_WEATHER_CTRL, Common_EventScript_SetAbnormalWeather
	return

Route130_EventScript_Rodney::
	trainerbattle_single TRAINER_RODNEY, Route130_Text_RodneyIntro, Route130_Text_RodneyDefeat
	msgbox Route130_Text_RodneyPostBattle, MSGBOX_AUTOCLOSE
	end

Route130_EventScript_Katie::
	trainerbattle_single TRAINER_KATIE, Route130_Text_KatieIntro, Route130_Text_KatieDefeat
	msgbox Route130_Text_KatiePostBattle, MSGBOX_AUTOCLOSE
	end

Route130_EventScript_Santiago::
	trainerbattle_single TRAINER_SANTIAGO, Route130_Text_SantiagoIntro, Route130_Text_SantiagoDefeat
	msgbox Route130_Text_SantiagoPostBattle, MSGBOX_AUTOCLOSE
	end

