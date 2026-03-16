.set LOCALID_LATI, 2
.set LOCALID_EXCLSIOR, 3
.set LOCALID_TYPHLOSION, 4
.set LOCALID_SHINY_LATI, 5
SouthernIsland_Interior_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, SouthernIsland_Interior_OnResume
	map_script MAP_SCRIPT_ON_TRANSITION, SouthernIsland_Interior_OnTransition
	.byte 0

SouthernIsland_Interior_OnResume:
	call_if_set FLAG_SYS_CTRL_OBJ_DELETE, SouthernIsland_Interior_EventScript_TryRemoveLati
	end

SouthernIsland_Interior_EventScript_TryRemoveLati::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, Common_EventScript_NopReturn
	removeobject LOCALID_LATI
	removeobject LOCALID_SHINY_LATI
	return

SouthernIsland_Interior_OnTransition:
	call_if_eq VAR_ROAMER_POKEMON, 0, SouthernIsland_Interior_EventScript_SetUpLatios
	call_if_ne VAR_ROAMER_POKEMON, 0, SouthernIsland_Interior_EventScript_SetUpLatias
	call SouthernIsland_Interior_EventScript_SetUpPlayerGfx
	goto SouthernIsland_Interior_EventScript_CheckExclsior
	end

SouthernIsland_Interior_EventScript_SetUpLatios::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_LATIOS
	setvar VAR_TEMP_4, SPECIES_LATIOS
	return

SouthernIsland_Interior_EventScript_SetUpLatias::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_LATIAS
	setvar VAR_TEMP_4, SPECIES_LATIAS
	return

SouthernIsland_Interior_EventScript_SetUpPlayerGfx::
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, SouthernIsland_Interior_EventScript_SetBrendanGfx
	goto_if_eq VAR_RESULT, FEMALE, SouthernIsland_Interior_EventScript_SetMayGfx
	end

SouthernIsland_Interior_EventScript_CheckExclsior::
	goto_if_set FLAG_DEFEATED_EXCLSIOR, SouthernIsland_Interior_EventScript_HideExclsior
	goto_if_unset FLAG_DEFEATED_METEOR_FALLS_STEVEN, SouthernIsland_Interior_EventScript_HideExclsior
	goto_if_unset FLAG_CAUGHT_LATIAS_OR_LATIOS, SouthernIsland_Interior_EventScript_HideExclsior
	goto_if_unset FLAG_DEFEATED_ZINNIA, SouthernIsland_Interior_EventScript_HideExclsior
	@ All 3 requirements met — show Exclsior
	clearflag FLAG_TEMP_10
	end

SouthernIsland_Interior_EventScript_HideExclsior::
	setflag FLAG_TEMP_10
	end

SouthernIsland_Interior_EventScript_Typhlosion::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_FERALIGATR, CRY_MODE_NORMAL
	msgbox SouthernIsland_Interior_Text_Typhlosion, MSGBOX_DEFAULT
	waitmoncry
	release
	end

SouthernIsland_Interior_EventScript_SetBrendanGfx::
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL
	return

SouthernIsland_Interior_EventScript_SetMayGfx::
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_RIVAL_MAY_NORMAL
	return

SouthernIsland_Interior_EventScript_TryLatiEncounter::
	lockall
	setvar VAR_0x8008, 12  @ Player's Y coordinate. Not read
	goto SouthernIsland_Interior_EventScript_Lati
	end

SouthernIsland_Interior_EventScript_Lati::
	goto_if_set FLAG_TEMP_2, SouthernIsland_Interior_EventScript_Sign
	goto_if_set FLAG_DEFEATED_LATIAS_OR_LATIOS, SouthernIsland_Interior_EventScript_CloserLookAtStone
	goto_if_set FLAG_CAUGHT_LATIAS_OR_LATIOS, SouthernIsland_Interior_EventScript_CloserLookAtStone
	goto_if_unset FLAG_ENABLE_SHIP_SOUTHERN_ISLAND, SouthernIsland_Interior_EventScript_Sign
	setflag FLAG_ENCOUNTERED_LATIAS_OR_LATIOS
	setflag FLAG_TEMP_2
	special SpawnCameraObject
	applymovement OBJ_EVENT_ID_CAMERA, SouthernIsland_Interior_Movement_CameraPanUp
	waitmovement 0
	delay 50
	waitse
	playmoncry VAR_TEMP_4, CRY_MODE_NORMAL
	delay 30
	waitmoncry
	addobject LOCALID_LATI
	delay 30
	applymovement OBJ_EVENT_ID_CAMERA, SouthernIsland_Interior_Movement_CameraPanDown
	applymovement LOCALID_LATI, SouthernIsland_Interior_Movement_LatiApproach
	waitmovement 0
	delay 50
	special RemoveCameraObject
	setvar VAR_LAST_TALKED, LOCALID_LATI
	call_if_eq VAR_ROAMER_POKEMON, 0, SouthernIsland_Interior_EventScript_SetLatiosBattleVars
	call_if_ne VAR_ROAMER_POKEMON, 0, SouthernIsland_Interior_EventScript_SetLatiasBattleVars
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	special BattleSetup_StartLatiBattle
	waitstate
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, SouthernIsland_Interior_EventScript_LatiDefeated
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, SouthernIsland_Interior_EventScript_RanFromLati
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, SouthernIsland_Interior_EventScript_RanFromLati
	setflag FLAG_CAUGHT_LATIAS_OR_LATIOS
	releaseall
	end

SouthernIsland_Interior_EventScript_LatiDefeated::
	setflag FLAG_DEFEATED_LATIAS_OR_LATIOS
	copyvar VAR_0x8004, VAR_TEMP_4
	goto Common_EventScript_LegendaryFlewAway
	end

SouthernIsland_Interior_EventScript_RanFromLati::
	copyvar VAR_0x8004, VAR_TEMP_4
	goto Common_EventScript_LegendaryFlewAway
	end

SouthernIsland_Interior_EventScript_Sign::
	msgbox SouthernIsland_Interior_Text_Sign, MSGBOX_DEFAULT
	releaseall
	end

SouthernIsland_Interior_EventScript_SetLatiosBattleVars::
	seteventmon SPECIES_LATIOS, 50, ITEM_SOUL_DEW
	return

SouthernIsland_Interior_EventScript_SetLatiasBattleVars::
	seteventmon SPECIES_LATIAS, 50, ITEM_SOUL_DEW
	return

SouthernIsland_Interior_EventScript_Exclsior::
	goto_if_set FLAG_DEFEATED_EXCLSIOR, SouthernIsland_Interior_Text_ExclsiorPostBattle
	faceplayer
	applymovement LOCALID_TYPHLOSION, SouthernIsland_Interior_Movement_WalkSlowDown
	msgbox SouthernIsland_Interior_Text_Exclsior_ImCurious, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq SouthernIsland_Interior_EventScript_Exclsior_NoBattle
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC, TRAINER_EXCLSIOR, 0, SouthernIsland_Interior_Text_ExclsiorIntro, SouthernIsland_Interior_Text_ExclsiorDefeated, SouthernIsland_Interior_EventScript_ExclsiorPostBattle
	end

SouthernIsland_Interior_EventScript_Exclsior_NoBattle::
	msgbox SouthernIsland_Interior_Text_Exclsior_MaybeAnotherTime, MSGBOX_DEFAULT
	turnobject LOCALID_EXCLSIOR, DIR_NORTH
	release
	end

SouthernIsland_Interior_EventScript_ExclsiorPostBattle::
	msgbox SouthernIsland_Interior_Text_ExclsiorPostBattle, MSGBOX_DEFAULT
	setflag FLAG_DEFEATED_EXCLSIOR
	fadescreenswapbuffers FADE_TO_BLACK
	removeobject LOCALID_EXCLSIOR
	removeobject LOCALID_TYPHLOSION
	fadescreenswapbuffers FADE_FROM_BLACK
	release
	end

SouthernIsland_Interior_EventScript_CloserLookAtStone::
	goto_if_unset FLAG_DEFEATED_EXCLSIOR, SouthernIsland_Interior_EventScript_Sign
	goto_if_unset FLAG_ENABLE_SHIP_SOUTHERN_ISLAND, SouthernIsland_Interior_EventScript_Sign
	@ Boss Lugia must be dealt with before anything else
	goto_if_unset FLAG_DEFEATED_BOSS_LUGIA, SouthernIsland_Interior_EventScript_BossLugiaCheck
	@ Lugia was defeated — show sign (shiny Lati disabled for now)
	goto SouthernIsland_Interior_EventScript_Sign
	end
	@ --- Shiny Lati gift commented out for future use ---
	@ goto_if_set FLAG_DEFEATED_SHINY_LATIAS_OR_LATIOS, SouthernIsland_Interior_EventScript_Sign
	@ goto_if_set FLAG_CAUGHT_SHINY_LATIAS_OR_LATIOS, SouthernIsland_Interior_EventScript_Sign
	@ message SouthernIsland_Interior_Text_CloserLookAtStone
	@ waitmessage
	@ goto SouthernIsland_Interior_EventScript_ChooseShinyLati

SouthernIsland_Interior_EventScript_BossLugiaCheck::
	@ Already caught — skip to shiny Lati
	goto_if_set FLAG_CAUGHT_BOSS_LUGIA, SouthernIsland_Interior_EventScript_BossLugiaDone
	@ Not yet fought — start boss fight
	message SouthernIsland_Interior_Text_RockWarning
	waitmessage
	waitbuttonpress
	closemessage
	setflag FLAG_SHINY_CREATION
	special CreateBossLugia
	clearflag FLAG_SHINY_CREATION
	waitse
	playmoncry SPECIES_LUGIA, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	special BattleSetup_StartLugiaBossBattle
	waitstate
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, SouthernIsland_Interior_EventScript_BossLugiaDefeated
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, SouthernIsland_Interior_EventScript_BossLugiaFled
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, SouthernIsland_Interior_EventScript_BossLugiaFled
	@ Caught
	setflag FLAG_CAUGHT_BOSS_LUGIA
	setflag FLAG_DEFEATED_BOSS_LUGIA
	releaseall
	end

SouthernIsland_Interior_EventScript_BossLugiaDefeated::
	setflag FLAG_DEFEATED_BOSS_LUGIA
	msgbox SouthernIsland_Interior_Text_LugiaDefeated, MSGBOX_DEFAULT
	releaseall
	end

SouthernIsland_Interior_EventScript_BossLugiaFled::
	msgbox SouthernIsland_Interior_Text_LugiaFled, MSGBOX_DEFAULT
	releaseall
	end

SouthernIsland_Interior_EventScript_BossLugiaDone::
	@ Lugia was caught — show sign (shiny Lati disabled for now)
	goto SouthernIsland_Interior_EventScript_Sign
	end
	@ --- Shiny Lati after Lugia catch commented out ---
	@ goto_if_set FLAG_DEFEATED_SHINY_LATIAS_OR_LATIOS, SouthernIsland_Interior_EventScript_Sign
	@ goto_if_set FLAG_CAUGHT_SHINY_LATIAS_OR_LATIOS, SouthernIsland_Interior_EventScript_Sign
	@ message SouthernIsland_Interior_Text_CloserLookAtStone
	@ waitmessage
	@ goto SouthernIsland_Interior_EventScript_ChooseShinyLati

SouthernIsland_Interior_EventScript_ChooseShinyLati::
	multichoice 20, 8, MULTI_SHINY_LATI, TRUE
	switch VAR_RESULT
	case 0, SouthernIsland_Interior_EventScript_SetUpShinyLatias
	case 1, SouthernIsland_Interior_EventScript_SetUpShinyLatios
	end
	
SouthernIsland_Interior_EventScript_SetUpShinyLatios::
	closemessage
	setvar VAR_OBJ_GFX_ID_2, OBJ_EVENT_GFX_SPECIES(LATIOS)
	setvar VAR_TEMP_4, SPECIES_LATIOS
	setflag FLAG_SHINY_CREATION
	call SouthernIsland_Interior_EventScript_SetLatiosBattleVars
	goto SouthernIsland_Interior_EventScript_ShinyLati
	end

SouthernIsland_Interior_EventScript_SetUpShinyLatias::
	closemessage
	setvar VAR_OBJ_GFX_ID_2, OBJ_EVENT_GFX_SPECIES(LATIAS)
	setvar VAR_TEMP_4, SPECIES_LATIAS
	setflag FLAG_SHINY_CREATION
	call SouthernIsland_Interior_EventScript_SetLatiasBattleVars
	goto SouthernIsland_Interior_EventScript_ShinyLati
	end

SouthernIsland_Interior_EventScript_ShinyLati::
	special SpawnCameraObject
	applymovement OBJ_EVENT_ID_CAMERA, SouthernIsland_Interior_Movement_CameraPanUp
	waitmovement 0
	delay 50
	waitse
	playmoncry VAR_TEMP_4, CRY_MODE_NORMAL
	delay 30
	waitmoncry
	fadescreen FADE_TO_WHITE
	addobject LOCALID_SHINY_LATI
	fadescreen FADE_FROM_WHITE
	delay 30
	applymovement OBJ_EVENT_ID_CAMERA, SouthernIsland_Interior_Movement_CameraPanDown
	applymovement LOCALID_SHINY_LATI, SouthernIsland_Interior_Movement_LatiApproach
	waitmovement 0
	delay 50
	special RemoveCameraObject
	setvar VAR_LAST_TALKED, LOCALID_SHINY_LATI
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	special BattleSetup_StartLatiBattle
	waitstate
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_WON, SouthernIsland_Interior_EventScript_ShinyLatiDefeated
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, SouthernIsland_Interior_EventScript_RanFromLati
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, SouthernIsland_Interior_EventScript_RanFromLati
	setflag FLAG_CAUGHT_SHINY_LATIAS_OR_LATIOS
	releaseall
	end

SouthernIsland_Interior_EventScript_ShinyLatiDefeated::
	setflag FLAG_DEFEATED_SHINY_LATIAS_OR_LATIOS
	copyvar VAR_0x8004, VAR_TEMP_4
	goto Common_EventScript_LegendaryFlewAway
	end

SouthernIsland_Interior_Text_Exclsior_ImCurious:
	.string "Hmm.. I wonder what made her come here\p"
	.string "Sorry.. I was lost in thoughts.\p"
	.string "Few trainers know of this place..\n"
	.string "Did she tell you- Wait...\p"
	.string "Do I know you? I feel like we already...\p"
	.string "No.\n" 
	.string "I lost that part of me long ago.\p"
	.string "Yet....\p"
	.string "Maybe I'm not as alone as I used to\n" 
	.string "be.\p" 
	.string "Only one way to find out.\p"
	.string "Are you ready?$"

SouthernIsland_Interior_Text_ExclsiorIntro:
	.string "Show me where your friendships\n"
	.string "led you.$"

SouthernIsland_Interior_Text_ExclsiorDefeated:
	.string "Perhaps I am not... as alone as\n"
	.string "I thought..$"

SouthernIsland_Interior_Text_Exclsior_MaybeAnotherTime::
	.string "Take your time, I'll be waiting.$"

SouthernIsland_Interior_Text_ExclsiorPostBattle:
	.string "Thank you for coming all this way.\p"
	.string "And for keeping my memory alive...\p"
	.string "You may want to have a closer look at\n"
	.string "this stone...\p"
	.string "But be careful.\n"
	.string "Something powerful sleeps within.\p"
	.string "Goodbye.$"

SouthernIsland_Interior_Text_Typhlosion:
	.string "FERALIGATR: Graaah!$"

SouthernIsland_Interior_Text_CloserLookAtStone:
	.string "You look closer at the strange stone.\p"
	.string "There seems to be a color shining\n"
	.string "through...\p"
	.string "Which color is shining through?$"

SouthernIsland_Interior_Movement_CameraPanUp:
	walk_up
	walk_up
	walk_up
	step_end

SouthernIsland_Interior_Movement_CameraPanDown:
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	walk_down
	walk_down
	walk_down
	walk_in_place_faster_up
	step_end

SouthernIsland_Interior_Movement_LatiApproach:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	delay_16
	delay_16
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

SouthernIsland_Interior_Movement_WalkSlowDown:
	walk_in_place_slow_down
	step_end

SouthernIsland_Interior_Movement_FaceUp:
	face_down
	step_end

SouthernIsland_Interior_Text_RockWarning:
	.string "You reach toward the strange stone...\p"
	.string "The air grows heavy.\n"
	.string "Something stirs from deep within...$"

SouthernIsland_Interior_Text_LugiaDefeated:
	.string "The light fades from the stone.\p"
	.string "Whatever was sealed inside has\n"
	.string "vanished...$"

SouthernIsland_Interior_Text_LugiaFled:
	.string "The presence retreats back into\n"
	.string "the stone...\p"
	.string "It seems to be waiting for you\n"
	.string "to try again.$"
