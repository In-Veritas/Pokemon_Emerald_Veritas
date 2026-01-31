SkyPillar_5F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SkyPillar_5F_OnTransition
	.byte 0

SkyPillar_5F_OnTransition:
	call_if_lt VAR_SKY_PILLAR_STATE, 2, SkyPillar_5F_EventScript_CleanFloor
	return

SkyPillar_5F_EventScript_CleanFloor::
	setmaplayoutindex LAYOUT_SKY_PILLAR_5F_CLEAN
	return

SkyPillar_5F_EventScript_Zinnia::
	applymovement LOCALID_ZINNIA, Common_Movement_FacePlayer
	waitmovement 0
	msgbox SkyPillar_5F_Text_Zinnia, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_ZINNIA, Common_Movement_FaceUp
	waitmovement 0
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_ZINNIA
	fadescreen FADE_FROM_BLACK
	setflag FLAG_SPOKE_TO_ZINNIA_5F
	setflag FLAG_HIDE_ZINNIA_5F
	clearflag FLAG_HIDE_ZINNIA_TOP
	release
	end

SkyPillar_5F_Text_Zinnia::
	.string "We knew there was a chance KYOGRE and\n"
	.string "GROUDON might wake up again, even\l"
	.string "after so many years of peace.\p"
	.string "So, as a safeguard, we created the RED\n"
	.string "and BLUE ORBS, hoping they'd keep the\l"
	.string "peace.\p"
	.string "But they were never really put to the\n"
	.string "test.\p"
	.string "No one knew for sure if they could\n"
	.string "control creatures that powerful.\p"
	.string "We hoped RAYQUAZA wouldn't have to be\n"
	.string "summoned again…\p"
	.string "But fate's a funny thing, isn't it?\p"
	.string "And here we are… with you being the one\n"
	.string "to summon RAYQUAZA when HOENN needed\l"
	.string "it most.$"
