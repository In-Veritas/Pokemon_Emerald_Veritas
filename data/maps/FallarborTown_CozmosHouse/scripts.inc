.set LOCALID_PROFCOZMO, 1

FallarborTown_CozmosHouse_MapScripts::
	.byte 0

FallarborTown_CozmosHouse_EventScript_ProfCozmo::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_TM_RETURN, FallarborTown_CozmosHouse_EventScript_GaveMeteorite
	checkitem ITEM_METEORITE
	goto_if_eq VAR_RESULT, TRUE, FallarborTown_CozmosHouse_EventScript_PlayerHasMeteorite
	msgbox FallarborTown_CozmosHouse_Text_MeteoriteWillNeverBeMineNow, MSGBOX_DEFAULT
	release
	end

FallarborTown_CozmosHouse_EventScript_PlayerHasMeteorite::
	call_if_unset FLAG_TEMP_2, FallarborTown_CozmosHouse_EventScript_NoticeMeteorite
	call_if_set FLAG_TEMP_2, FallarborTown_CozmosHouse_EventScript_AskForMeteorite
	goto_if_eq VAR_RESULT, NO, FallarborTown_CozmosHouse_EventScript_DeclineGiveMeteorite
	msgbox FallarborTown_CozmosHouse_Text_PleaseUseThisTM, MSGBOX_DEFAULT
	giveitem ITEM_TM_RETURN
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setvar VAR_0x8004, ITEM_METEORITE
	call Common_EventScript_PlayerHandedOverTheItem
	setflag FLAG_RECEIVED_TM_RETURN
	msgbox FallarborTown_CozmosHouse_Text_ReallyGoingToHelpMyResearch, MSGBOX_DEFAULT
	release
	end

FallarborTown_CozmosHouse_EventScript_NoticeMeteorite::
	msgbox FallarborTown_CozmosHouse_Text_MeteoriteWillNeverBeMineNow, MSGBOX_DEFAULT
	msgbox FallarborTown_CozmosHouse_Text_IsThatMeteoriteMayIHaveIt, MSGBOX_YESNO
	return

FallarborTown_CozmosHouse_EventScript_AskForMeteorite::
	msgbox FallarborTown_CozmosHouse_Text_MayIHaveMeteorite, MSGBOX_YESNO
	return

FallarborTown_CozmosHouse_EventScript_DeclineGiveMeteorite::
	setflag FLAG_TEMP_2
	msgbox FallarborTown_CozmosHouse_Text_CrushedWithDisappointment, MSGBOX_DEFAULT
	release
	end

FallarborTown_CozmosHouse_EventScript_GaveMeteorite::
	goto_if_set FLAG_RECEIVED_AURORA_TICKET, FallarborTown_CozmosHouse_EventScript_GaveMeteorite_Continued
	setvar 0x8005, SPECIES_RAYQUAZA
	special CheckSpecies
	goto_if_eq VAR_RESULT, TRUE, EventScript_GotRayquaza

FallarborTown_CozmosHouse_EventScript_GaveMeteorite_Continued::
	goto_if_set FLAG_BATTLED_DEOXYS, FallarborTown_CozmosHouse_EventScript_ChangeDeoxysForm
	goto_if_set FLAG_DEFEATED_DEOXYS, FallarborTown_CozmosHouse_EventScript_ChangeDeoxysForm
	msgbox FallarborTown_CozmosHouse_Text_ReallyGoingToHelpMyResearch, MSGBOX_DEFAULT
	release
	end

FallarborTown_CozmosHouse_EventScript_ChangeDeoxysForm::
	goto_if_set FLAG_COZMO_CAUGHT_DEOXYS, FallarborTown_CozmosHouse_EventScript_ChangeDeoxysForm2
	setflag FLAG_COZMO_CAUGHT_DEOXYS
	msgbox FallarborTown_CozmosHouse_Text_CozmoSurprised, MSGBOX_DEFAULT
FallarborTown_CozmosHouse_EventScript_ChangeDeoxysForm2::
	msgbox ChangeDeoxysForm_Text_Beginning, MSGBOX_YESNO
	compare VAR_RESULT, YES
	goto_if_eq ChangeDeoxysForm_Start
ChangeDeoxysForm_End:
	msgbox ChangeDeoxysForm_Text_End, MSGBOX_DEFAULT
	releaseall
	end

ChangeDeoxysForm_Start:
	msgbox ChangeDeoxysForm_Text_Select, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	compare VAR_0x8004, 255
	goto_if_eq ChangeDeoxysForm_End
	special TryChangeDeoxysForm
	compare VAR_RESULT, TRUE
	faceplayer
	goto_if_eq ChangeDeoxysForm_Success
	goto ChangeDeoxysForm_End

ChangeDeoxysForm_Success:
	playmoncry SPECIES_DEOXYS, 0
	msgbox ChangeDeoxysForm_Text_Success, MSGBOX_DEFAULT
	waitmoncry
	goto ChangeDeoxysForm_End

ChangeDeoxysForm_Text_Beginning:
	.string "Would you like to see what happens if\n"
	.string "we expose it to this meteorite?$"

ChangeDeoxysForm_Text_End:
	.string "Come back anytime.$"

ChangeDeoxysForm_Text_Select:
	.string "Show me your DEOXYS.$"

ChangeDeoxysForm_Text_Success:
	.string "The experiment was a success!\nYour DEOXYS has transformed!$"

FallarborTown_CozmosHouse_Text_CozmoSurprised:
	.string "PROF. COZMO: Incredible! You caught\n"
	.string "DEOXYS, didn't you?\p"
	.string "I've been studying the METEORITE while\n"
	.string "you were away, and I think there's a\l"
	.string "strange connection between it and\l"
	.string "DEOXYS.\p"
	.string "Can you feel it? It’s as if they…\n"
	.string "resonate.$"

EventScript_GotRayquaza::
	msgbox FallarborTown_CozmosHouse_Text_GotRayquaza, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, 1   @ vertical pan
	setvar VAR_0x8005, 3   @ horizontal pan
	setvar VAR_0x8006, 20  @ num shakes
	setvar VAR_0x8007, 5   @ shake delay
	special ShakeCamera
	waitstate
	applymovement LOCALID_PROFCOZMO, LavaridgeTown_Movement_ProfCozmoFaceLeft
	waitmovement 0
	applymovement LOCALID_PROFCOZMO, Common_Movement_Delay48
	waitmovement 0
	applymovement LOCALID_PROFCOZMO, LavaridgeTown_Movement_ProfCozmoFaceRight
	waitmovement 0
	applymovement LOCALID_PROFCOZMO, Common_Movement_Delay48
	waitmovement 0
	applymovement LOCALID_PROFCOZMO, LavaridgeTown_Movement_ProfCozmoFaceLeft
	waitmovement 0
	applymovement LOCALID_PROFCOZMO, Common_Movement_Delay48
	waitmovement 0
	applymovement LOCALID_PROFCOZMO, LavaridgeTown_Movement_ProfCozmoWalkToCabinet
	waitmovement 0
	applymovement LOCALID_PROFCOZMO, Common_Movement_Delay48
	waitmovement 0
	applymovement LOCALID_PROFCOZMO, LavaridgeTown_Movement_ProfCozmoWalkToPlayer
	waitmovement 0
	applymovement LOCALID_PROFCOZMO, Common_Movement_FacePlayer
	waitmovement 0
	msgbox FallarborTown_CozmosHouse_Text_GotRayquaza2, MSGBOX_DEFAULT
	checkitemspace ITEM_AURORA_TICKET
	vgoto_if_eq VAR_RESULT, FALSE, FallarborTown_CozmosHouse_EventScript_AuroraTicketBagFull
	giveitem ITEM_AURORA_TICKET
	setflag FLAG_RECEIVED_AURORA_TICKET
	msgbox FallarborTown_CozmosHouse_Text_GotRayquaza3, MSGBOX_DEFAULT
	releaseall
	end

FallarborTown_CozmosHouse_EventScript_AuroraTicketBagFull:
	vmessage FallarborTown_CozmosHouse_Text_AuroraTicketBagFull
	waitmessage
	waitbuttonpress
	release
	end

LavaridgeTown_Movement_ProfCozmoWalkToCabinet:
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	step_end

LavaridgeTown_Movement_ProfCozmoWalkToPlayer:
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	step_end

LavaridgeTown_Movement_ProfCozmoFaceLeft:
	face_left
	step_end

LavaridgeTown_Movement_ProfCozmoFaceRight:
	face_right
	step_end

FallarborTown_CozmosHouse_EventScript_CozmosWife::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_TM_RETURN, FallarborTown_CozmosHouse_EventScript_CozmoIsHappy
	goto_if_unset FLAG_HIDE_FALLARBOR_HOUSE_PROF_COZMO, FallarborTown_CozmosHouse_EventScript_CozmoIsSad
	msgbox FallarborTown_CozmosHouse_Text_CozmoWentToMeteorFalls, MSGBOX_DEFAULT
	release
	end

FallarborTown_CozmosHouse_EventScript_CozmoIsSad::
	msgbox FallarborTown_CozmosHouse_Text_FeelSorryForCozmo, MSGBOX_DEFAULT
	release
	end

FallarborTown_CozmosHouse_EventScript_CozmoIsHappy::
	msgbox FallarborTown_CozmosHouse_Text_CozmoIsSoHappy, MSGBOX_DEFAULT
	release
	end

FallarborTown_CozmosHouse_Text_MeteoriteWillNeverBeMineNow:
	.string "PROF. COZMO: Oh…\n"
	.string "I never should have let myself be\l"
	.string "conned into telling TEAM MAGMA where\l"
	.string "you can find METEORITES…\p"
	.string "That METEORITE from METEOR FALLS…\n"
	.string "It's never going to be mine now…$"

FallarborTown_CozmosHouse_Text_IsThatMeteoriteMayIHaveIt:
	.string "Oh!\n"
	.string "Hah?\p"
	.string "That item…\p"
	.string "Could it be?\p"
	.string "Is it the METEORITE that TEAM MAGMA\n"
	.string "took from METEOR FALLS?\p"
	.string "Please, may I have it?\p"
	.string "I'm not asking for it for free.\n"
	.string "How about in exchange for this TM?$"

FallarborTown_CozmosHouse_Text_PleaseUseThisTM:
	.string "PROF. COZMO: This TM, it represents\n"
	.string "my feeling of gratitude.\l"
	.string "Please use it!$"

FallarborTown_CozmosHouse_Text_ReallyGoingToHelpMyResearch:
	.string "PROF. COZMO: Oh, I can't believe it.\n"
	.string "This is really, really great!\p"
	.string "This is really going to help my research!$"

FallarborTown_CozmosHouse_Text_CrushedWithDisappointment:
	.string "PROF. COZMO: Oh, but…\n"
	.string "I'm crushed with disappointment…$"

FallarborTown_CozmosHouse_Text_MayIHaveMeteorite:
	.string "PROF. COZMO: Please, may I have that\n"
	.string "METEORITE?\p"
	.string "I'm not asking for it for free.\n"
	.string "How about in exchange for this TM?$"

FallarborTown_CozmosHouse_Text_GotRayquaza:
	.string "PROF. COZMO: Oh my…\n"
	.string "Is that… RAYQUAZA?!\p"
	.string "You're truly full of surprises,\n"
	.string "{PLAYER}{KUN}.\p"
	.string "Wait, what's this?\n"
	.string "The METEORITE… it's reacting…\l"
	.string "It’s shaking! But why?\p"
	.string "This has never happened before…\p"
	.string "Could it be that the presence of\n"
	.string "RAYQUAZA is triggering something?\p"
	.string "Hold on…$"

FallarborTown_CozmosHouse_Text_GotRayquaza2:
	.string "My equipment is picking up seismic\n"
	.string "activity coming from an area called\l"
	.string "BIRTH ISLAND.\p"
	.string "The timing can't be a coincidence…\n"
	.string "RAYQUAZA’s presence must be connected.\p"
	.string "Here, take this.$"

FallarborTown_CozmosHouse_Text_GotRayquaza3:
	.string "This TICKET should allow you to reach\n"
	.string "BIRTH ISLAND. You can use it at\l"
	.string "LILYCOVE CITY port.\p"
	.string "Please, {PLAYER}{KUN},\n"
	.string "investigate this for me!\p"
	.string "We may be on the verge of a\n"
	.string "groundbreaking discovery!$"

FallarborTown_CozmosHouse_Text_AuroraTicketBagFull:
	.string "Oh, I'm sorry, {PLAYER}.\n"
	.string "Your BAG's KEY ITEMS POCKET is full.\p"
	.string "Please store something on your PC,\n"
	.string "then come back for this.$"

FallarborTown_CozmosHouse_Text_CozmoWentToMeteorFalls:
	.string "PROF. COZMO went off to METEOR FALLS\n"
	.string "on ROUTE 114 with some people from\l"
	.string "TEAM MAGMA.$"

FallarborTown_CozmosHouse_Text_FeelSorryForCozmo:
	.string "Poor PROF. COZMO…\n"
	.string "He's so depressed… I feel sorry for him.$"

FallarborTown_CozmosHouse_Text_CozmoIsSoHappy:
	.string "Look at PROF. COZMO…\n"
	.string "He's so happy! I think it's cute.$"
