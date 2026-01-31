PacifidlogTown_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PacifidlogTown_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, PacifidlogTown_OnResume
	.byte 0

PacifidlogTown_OnTransition:
	setflag FLAG_VISITED_PACIFIDLOG_TOWN
	end

PacifidlogTown_OnResume:
	setstepcallback STEP_CB_PACIFIDLOG_BRIDGE
	end

PacifidlogTown_EventScript_NinjaBoy::
	msgbox PacifidlogTown_Text_NeatHousesOnWater, MSGBOX_NPC
	end

PacifidlogTown_EventScript_Girl::
	msgbox PacifidlogTown_Text_FastRunningCurrent, MSGBOX_DEFAULT
	release
	end

PacifidlogTown_EventScript_Fisherman::
	faceplayer
	call PacifidlogTown_EventScript_FishermanCheckFishingRod
	call PacifidlogTown_EventScript_FishermanCheckPokeballs
	msgbox PacifidlogTown_Text_SkyPillarTooScary, MSGBOX_NPC
	release
	end

PacifidlogTown_EventScript_FishermanCheckFishingRod::
	checkitem ITEM_OLD_ROD
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	checkitem ITEM_GOOD_ROD
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	checkitem ITEM_SUPER_ROD
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	msgbox PacifidlogTown_Text_NoFishingRod, MSGBOX_DEFAULT
	giveitem ITEM_OLD_ROD
	setflag FLAG_RECEIVED_OLD_ROD
	return
PacifidlogTown_EventScript_FishermanCheckPokeballs::
	checkitem ITEM_MASTER_BALL
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	checkitem ITEM_ULTRA_BALL
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	checkitem ITEM_GREAT_BALL
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	checkitem ITEM_POKE_BALL
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	checkitem ITEM_NET_BALL
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	checkitem ITEM_DIVE_BALL
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	checkitem ITEM_NEST_BALL
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	checkitem ITEM_REPEAT_BALL
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	checkitem ITEM_TIMER_BALL
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	checkitem ITEM_LUXURY_BALL
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	checkitem ITEM_PREMIER_BALL
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_EventScript_Return
	msgbox PacifidlogTown_Text_NoPokeballs, MSGBOX_DEFAULT
	giveitem ITEM_POKE_BALL, 5
PacifidlogTown_EventScript_Return::
	return

PacifidlogTown_EventScript_TownSign::
	msgbox PacifidlogTown_Text_TownSign, MSGBOX_SIGN
	end

PacifidlogTown_Text_FastRunningCurrent:
	.string "The sea between PACIFIDLOG and\n"
	.string "SLATEPORT has a fast-running tide.\p"
	.string "If you decide to SURF, you could end\n"
	.string "up swept away somewhere else.$"

PacifidlogTown_Text_NeatHousesOnWater:
	.string "See, isn't it neat?\n"
	.string "These houses are on water!\p"
	.string "I was born here!$"

PacifidlogTown_Text_SkyPillarTooScary:
	.string "The SKY PILLAR?\p"
	.string "…Oh, you must mean that tall, tall\n"
	.string "tower a little further out.\p"
	.string "If you asked me, I wouldn't climb it.\n"
	.string "It's too scary to get up that high.\p"
	.string "Life at sea level in PACIFIDLOG,\n"
	.string "that suits me fine.$"

PacifidlogTown_Text_NoFishingRod:
	.string "Hold on there, you don't even have a\n"
	.string "FISHING ROD? How did you even make it\l"
	.string "all the way here?\p"
	.string "Here, take one of mine.$"

PacifidlogTown_Text_NoPokeballs:
	.string "Wait a second… Do you have any POKé\n"
	.string "BALLS on you?\p"
	.string "You'll want some before you head any\n"
	.string "further. Take a few of mine.$"

PacifidlogTown_Text_TownSign:
	.string "PACIFIDLOG TOWN\p"
	.string "“Where the morning sun smiles upon\n"
	.string "the waters.”$"
