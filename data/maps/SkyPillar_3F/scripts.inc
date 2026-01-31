.set LOCALID_ZINNIA, 1

SkyPillar_3F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SkyPillar_3F_OnTransition
	.byte 0

SkyPillar_3F_OnTransition:
	call_if_lt VAR_SKY_PILLAR_STATE, 2, SkyPillar_3F_EventScript_CleanFloor
	end

SkyPillar_3F_EventScript_CleanFloor::
	setmaplayoutindex LAYOUT_SKY_PILLAR_3F_CLEAN
	return

SkyPillar_3F_EventScript_Zinnia::
	applymovement LOCALID_ZINNIA, Common_Movement_FacePlayer
	waitmovement 0
	msgbox SkyPillar_3F_Text_Zinnia, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_ZINNIA, Common_Movement_FaceUp
	waitmovement 0
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_ZINNIA
	fadescreen FADE_FROM_BLACK
	setflag FLAG_SPOKE_TO_ZINNIA_3F
	setflag FLAG_HIDE_ZINNIA_3F
	clearflag FLAG_HIDE_ZINNIA_4F
	release
	end

SkyPillar_3F_Text_Zinnia::
	.string "After that fierce clash, once KYOGRE\n"
	.string "and GROUDON sank back into the depths,\l"
	.string "RAYQUAZA returned to the sky where it\l"
	.string "watches over us.\p"
	.string "That's when my people built the\n"
	.string "SKY PILLAR…\p"
	.string "A place where we can reach out to our\n"
	.string "DRAGON LORD when the world's in danger.\p"
	.string "It's like our own link to the heavens,\n"
	.string "where we can call to RAYQUAZA if we\l"
	.string "ever need it again.$"
