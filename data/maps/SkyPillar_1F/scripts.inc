.set LOCALID_ZINNIA, 1

SkyPillar_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, SkyPillar_1F_OnTransition
	.byte 0

SkyPillar_1F_OnTransition:
	setflag FLAG_HIDE_ZINNIA_1F
	setvar VAR_SHOW_ZINNIA, 0
	call_if_lt VAR_SKY_PILLAR_STATE, 2, SkyPillar_1F_EventScript_CleanFloor
	call_if_set FLAG_IS_CHAMPION, SkyPillar_1F_EventScript_TryShowZinnia
	end

SkyPillar_1F_EventScript_CleanFloor::
	setmaplayoutindex LAYOUT_SKY_PILLAR_1F_CLEAN
	return

SkyPillar_1F_EventScript_TryShowZinnia::
	goto_if_set FLAG_SPOKE_TO_ZINNIA_1F, SkyPillar_1F_EventScript_TryShowZinniaReturn
	setvar VAR_SHOW_ZINNIA, 1
SkyPillar_1F_EventScript_TryShowZinniaReturn::
	return

SkyPillar_1F_EventScript_ZinniaAppears::
	addobject LOCALID_ZINNIA
	fadescreen FADE_TO_BLACK
	delay 20
	setobjectxy LOCALID_ZINNIA, 3, 6
	delay 20
	applymovement LOCALID_ZINNIA, Common_Movement_FacePlayer
	waitmovement 0
	fadescreen FADE_FROM_BLACK
	delay 20
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_FaceUp
	@ fallthrough
SkyPillar_1F_EventScript_Zinnia::
	msgbox SkyPillar_1F_Text_Zinnia, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_ZINNIA, SkyPillar_1F_Movement_Zinnia
	waitmovement 0
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_ZINNIA
	fadescreen FADE_FROM_BLACK
	setflag FLAG_SPOKE_TO_ZINNIA_1F
	setflag FLAG_HIDE_ZINNIA_1F
	clearflag FLAG_HIDE_ZINNIA_2F
	setvar VAR_SHOW_ZINNIA, 0
	release
	end

SkyPillar_1F_Movement_Zinnia:
	walk_up
	step_end

SkyPillar_1F_Text_Zinnia::
	.string "So, you're the one, huh?\p"
	.string "The one who called RAYQUAZA down from\n"
	.string "the heavens and put KYOGRE and\l"
	.string "GROUDON to rest.\p"
	.string "My people, the DRACONIDS - we've always\n"
	.string "looked to RAYQUAZA as our DRAGON LORD,\l"
	.string "the great guardian of balance.\p"
	.string "But, that balance… it's so, so delicate.\p"
	.string "Like, one little push and… whoosh!\n"
	.string "It could all just tip over.$"
