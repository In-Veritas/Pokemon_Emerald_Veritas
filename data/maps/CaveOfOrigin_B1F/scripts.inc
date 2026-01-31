.set LOCALID_WALLACE, 1
.set LOCALID_CRAIG, 2
.set LOCALID_SMITH, 3
.set LOCALID_WEEBRA, 4

CaveOfOrigin_B1F_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, CaveOfOrigin_B1F_OnFrame
	.byte 0

CaveOfOrigin_B1F_OnFrame:
	map_script_2 VAR_TEMP_1, 1, CaveOfOrigin_B1F_EventScript_CheckBeatSmith
	map_script_2 VAR_TEMP_1, 2, CaveOfOrigin_B1F_EventScript_CheckBeatCraig
	map_script_2 VAR_TEMP_1, 3, CaveOfOrigin_B1F_EventScript_CheckBeatWeebra
	.2byte 0

CaveOfOrigin_B1F_EventScript_Smith::
	setvar VAR_TEMP_1, 1
	trainerbattle_single TRAINER_SMITH, CaveOfOrigin_B1F_Text_SmithIntro, CaveOfOrigin_B1F_Text_SmithDefeated
CaveOfOrigin_B1F_EventScript_SmithPostBattle::
	msgbox CaveOfOrigin_B1F_Text_SmithPostBattle, MSGBOX_DEFAULT
	setflag FLAG_DEFEATED_SMITH
	goto_if_set FLAG_RECEIVED_REGIROCK_DOLL CaveOfOrigin_B1F_EventScript_End
	givedecoration DECOR_REGIROCK_DOLL
	goto_if_eq VAR_RESULT, FALSE, CaveOfOrigin_B1F_EventScript_NoRoomForDeco
	msgbox CaveOfOrigin_B1F_Text_SmithPostBattle2, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_REGIROCK_DOLL
	fadescreen FADE_TO_BLACK
	special Script_FadeOutMapMusic
	removeobject LOCALID_SMITH
	setflag FLAG_HIDE_SMITH
	fadescreen FADE_FROM_BLACK
	release
	end
CaveOfOrigin_B1F_EventScript_CheckBeatSmith::
	setvar VAR_TEMP_1, 0
	goto_if_set FLAG_DEFEATED_SMITH, CaveOfOrigin_B1F_EventScript_End
	setvar 0x8005, TRAINER_SMITH
	special HasRematchBeenFought
	goto_if_eq VAR_RESULT, TRUE, CaveOfOrigin_B1F_EventScript_SmithPostBattle
	release
	end

CaveOfOrigin_B1F_EventScript_Craig::
	setvar VAR_TEMP_1, 2
	trainerbattle_single TRAINER_CRAIG, CaveOfOrigin_B1F_Text_CraigIntro, CaveOfOrigin_B1F_Text_CraigDefeated
CaveOfOrigin_B1F_EventScript_CraigPostBattle::
	msgbox CaveOfOrigin_B1F_Text_CraigPostBattle, MSGBOX_DEFAULT
	setflag FLAG_DEFEATED_CRAIG
	goto_if_set FLAG_RECEIVED_REGISTEEL_DOLL CaveOfOrigin_B1F_EventScript_End
	givedecoration DECOR_REGISTEEL_DOLL
	goto_if_eq VAR_RESULT, FALSE, CaveOfOrigin_B1F_EventScript_NoRoomForDeco
	setflag FLAG_RECEIVED_REGISTEEL_DOLL
	fadescreen FADE_TO_BLACK
	special Script_FadeOutMapMusic
	removeobject LOCALID_CRAIG
	setflag FLAG_HIDE_CRAIG
	addobject LOCALID_WEEBRA
	clearflag FLAG_HIDE_WEEBRA
	fadescreen FADE_FROM_BLACK
	release
	end
CaveOfOrigin_B1F_EventScript_CheckBeatCraig::
	setvar VAR_TEMP_1, 0
	goto_if_set FLAG_DEFEATED_CRAIG, CaveOfOrigin_B1F_EventScript_End
	setvar 0x8005, TRAINER_CRAIG
	special HasRematchBeenFought
	goto_if_eq VAR_RESULT, TRUE, CaveOfOrigin_B1F_EventScript_CraigPostBattle
	release
	end

CaveOfOrigin_B1F_EventScript_Weebra::
	setvar VAR_TEMP_1, 3
	trainerbattle_single TRAINER_WEEBRA, CaveOfOrigin_B1F_Text_WeebraIntro, CaveOfOrigin_B1F_Text_WeebraDefeated
CaveOfOrigin_B1F_EventScript_WeebraPostBattle::
	msgbox CaveOfOrigin_B1F_Text_WeebraPostBattle, MSGBOX_DEFAULT
	setflag FLAG_DEFEATED_WEEBRA
	goto_if_set FLAG_RECEIVED_REGICE_DOLL CaveOfOrigin_B1F_EventScript_End
	givedecoration DECOR_REGICE_DOLL
	goto_if_eq VAR_RESULT, FALSE, CaveOfOrigin_B1F_EventScript_NoRoomForDeco
	setflag FLAG_RECEIVED_REGICE_DOLL
	fadescreen FADE_TO_BLACK
	special Script_FadeOutMapMusic
	removeobject LOCALID_WEEBRA
	setflag FLAG_HIDE_WEEBRA
	addobject LOCALID_SMITH
	clearflag FLAG_HIDE_SMITH
	fadescreen FADE_FROM_BLACK
	release
	end
CaveOfOrigin_B1F_EventScript_CheckBeatWeebra::
	setvar VAR_TEMP_1, 0
	goto_if_set FLAG_DEFEATED_WEEBRA, CaveOfOrigin_B1F_EventScript_End
	setvar 0x8005, TRAINER_WEEBRA
	special HasRematchBeenFought
	goto_if_eq VAR_RESULT, TRUE, CaveOfOrigin_B1F_EventScript_WeebraPostBattle
	release
	end

CaveOfOrigin_B1F_EventScript_NoRoomForDeco::
	msgbox CaveOfOrigin_B1F_Text_NoRoomForDeco, MSGBOX_DEFAULT
CaveOfOrigin_B1F_EventScript_End::
	release
	end

CaveOfOrigin_B1F_Text_SmithIntro:
	.string "Hey! What's up?\n"
	.string "Let me see how much zest you've got!$"

CaveOfOrigin_B1F_Text_SmithDefeated:
	.string "You're a zest god!$"

CaveOfOrigin_B1F_Text_SmithPostBattle:
	.string "I'm impressed! You beat me with all my\n"
	.string "favorites!\p"
	.string "As a token of respect, I want you to\n"
	.string "have this. It doesn't do anything, but\l"
	.string "it looks cool!$"

CaveOfOrigin_B1F_Text_SmithPostBattle2:
	.string "Peace out, nerd!$"

CaveOfOrigin_B1F_Text_CraigIntro:
	.string "They say only a few can enter here, yet\n"
	.string "somehow… I was permitted.\p"
	.string "Why, you ask?\n"
	.string "Well, that's a secret.\p"
	.string "Instead, let's battle. I think I can\n"
	.string "gather some interesting data from you.$"

CaveOfOrigin_B1F_Text_CraigDefeated:
	.string "This is good data!\n"
	.string "I can use this.$"

CaveOfOrigin_B1F_Text_CraigPostBattle:
	.string "You know, most people wouldn't believe\n"
	.string "I was allowed in here.\p"
	.string "Here, take this as a token of our match.\n"
	.string "A little something from… places unseen.$"

CaveOfOrigin_B1F_Text_WeebraIntro:
	.string "Intruders! Sound the alarm! That's the\n"
	.string "line, right?$"

CaveOfOrigin_B1F_Text_WeebraDefeated:
	.string "Guess someone finally busted outta\n"
	.string "their cage…$"

CaveOfOrigin_B1F_Text_WeebraPostBattle:
	.string "Anyway - a cool cat like you is gonna\n"
	.string "need something fitting…$"

CaveOfOrigin_B1F_Text_NoRoomForDeco:
	.string "Looks like you don't have space for any\n"
	.string "more decorations right now.\p"
	.string "Better make some room and come back\n"
	.string "later!$"

CaveOfOrigin_B1F_EventScript_Wallace::
	lock
	faceplayer
	msgbox CaveOfOrigin_B1F_Text_WallaceStory, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_WALLACE, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	delay 60
	playse SE_PIN
	applymovement LOCALID_WALLACE, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_WALLACE, Common_Movement_Delay48
	waitmovement 0
	delay 30
	applymovement LOCALID_WALLACE, Common_Movement_FacePlayer
	waitmovement 0
	msgbox CaveOfOrigin_B1F_Text_WellHeadToSkyPillar, MSGBOX_DEFAULT
	closemessage
	playse SE_EXIT
	fadescreenspeed FADE_TO_BLACK, 4
	setflag FLAG_WALLACE_GOES_TO_SKY_PILLAR
	setvar VAR_SOOTOPOLIS_CITY_STATE, 3
	removeobject LOCALID_WALLACE
	clearflag FLAG_HIDE_SKY_PILLAR_WALLACE
	fadescreen FADE_FROM_BLACK
	release
	end

CaveOfOrigin_B1F_Text_WallaceStory:
	.string "Ah, so you are {PLAYER}{KUN}?\p"
	.string "I've heard such marvelous tales of your\n"
	.string "exploits.\p"
	.string "My name is WALLACE.\p"
	.string "As the GYM LEADER of SOOTOPOLIS, I've\n"
	.string "spent my life immersed in its mysteries,\l"
	.string "its beauty.\p"
	.string "And now, here we are…\n"
	.string "standing amidst a legend.\p"
	.string "… … … … … …\n"
	.string "… … … … … …\p"
	.string "GROUDON and KYOGRE… such power.\p"
	.string "But according to the ancient carvings\n"
	.string "here in the CAVE OF ORIGIN, there is yet\l"
	.string "another being.\p"
	.string "A sacred POKéMON, known in legends\n"
	.string "as the DRAGON LORD.\p"
	.string "It is said that in times of strife, this\n"
	.string "ethereal being descended from the\l"
	.string "heavens to quell the chaos below.\p"
	.string "I believe that seeking out this\n"
	.string "DRAGON LORD may be our only hope.\p"
	.string "The SKY PILLAR, a monument to its\n"
	.string "majesty, lies waiting on ROUTE 131.\p"
	.string "I trust that we will find the answers we\n"
	.string "seek there.\p"
	.string "I've studied the carvings, and at long\n"
	.string "last…\p"
	.string "I believe I know how to unlock its\n"
	.string "secrets.$"

CaveOfOrigin_B1F_Text_WellHeadToSkyPillar:
	.string "WALLACE: {PLAYER}{KUN}, will you\n"
	.string "accompany me?\p"
	.string "There's not a moment to lose!\p"
	.string "Let us make haste to the SKY PILLAR.\n"
	.string "Time is of the essence!\p"
	.string "I will go ahead and prepare to open\n"
	.string "its entrance.\p"
	.string "Please meet me there as soon as you can.$"
