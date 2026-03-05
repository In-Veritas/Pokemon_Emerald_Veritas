.set LOCALID_MOVE_RELEARNER, 1
.set LOCALID_EGGMOVETUTOR, 2

FallarborTown_MoveRelearnersHouse_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, FallarborTown_MoveRelearnersHouse_OnLoad
	.byte 0

FallarborTown_MoveRelearnersHouse_OnLoad:
	call_if_unset FLAG_SYS_GAME_CLEAR, FallarborTown_MoveRelearnersHouse_EventScript_HideEggMoveTutor
	call_if_set FLAG_SYS_GAME_CLEAR, FallarborTown_MoveRelearnersHouse_EventScript_ShowEggMoveTutor
	end

FallarborTown_MoveRelearnersHouse_EventScript_MoveRelearner::
	clearflag FLAG_EGG_MOVES_TUTOR
	lockall
	applymovement LOCALID_MOVE_RELEARNER, Common_Movement_FacePlayer
	waitmovement 0
	goto_if_set FLAG_TEMP_1, FallarborTown_MoveRelearnersHouse_EventScript_AskTeachMove
	msgbox FallarborTown_MoveRelearnersHouse_Text_ImTheMoveTutor, MSGBOX_DEFAULT
	setflag FLAG_TEMP_1
	goto FallarborTown_MoveRelearnersHouse_EventScript_AskTeachMove
	end

FallarborTown_MoveRelearnersHouse_EventScript_AskTeachMove::
	checkitem ITEM_HEART_SCALE
	goto_if_eq VAR_RESULT, FALSE, FallarborTown_MoveRelearnersHouse_EventScript_ComeBackWithHeartScale
	msgbox FallarborTown_MoveRelearnersHouse_Text_ThatsAHeartScaleWantMeToTeachMove, MSGBOX_YESNO
	switch VAR_RESULT
	case NO, FallarborTown_MoveRelearnersHouse_EventScript_ComeBackWithHeartScale
	goto FallarborTown_MoveRelearnersHouse_EventScript_ChooseMon
	end

FallarborTown_MoveRelearnersHouse_EventScript_ChooseMon::
	msgbox FallarborTown_MoveRelearnersHouse_Text_TutorWhichMon, MSGBOX_DEFAULT
	special ChooseMonForMoveRelearner
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, FallarborTown_MoveRelearnersHouse_EventScript_ComeBackWithHeartScale
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, FallarborTown_MoveRelearnersHouse_EventScript_CantTeachEgg
	goto_if_eq VAR_0x8005, 0, FallarborTown_MoveRelearnersHouse_EventScript_NoMoveToTeachMon
	goto FallarborTown_MoveRelearnersHouse_EventScript_ChooseMove
	end

FallarborTown_MoveRelearnersHouse_EventScript_ChooseMove::
	msgbox FallarborTown_MoveRelearnersHouse_Text_TeachWhichMove, MSGBOX_DEFAULT
	special TeachMoveRelearnerMove
	waitstate
	goto_if_eq VAR_0x8004, 0, FallarborTown_MoveRelearnersHouse_EventScript_ChooseMon
	msgbox FallarborTown_MoveRelearnersHouse_Text_HandedOverHeartScale, MSGBOX_DEFAULT
	removeitem ITEM_HEART_SCALE
	goto FallarborTown_MoveRelearnersHouse_EventScript_ComeBackWithHeartScale
	end

FallarborTown_MoveRelearnersHouse_EventScript_NoMoveToTeachMon::
	msgbox FallarborTown_MoveRelearnersHouse_Text_DontHaveMoveToTeachPokemon, MSGBOX_DEFAULT
	goto FallarborTown_MoveRelearnersHouse_EventScript_ChooseMon
	end

FallarborTown_MoveRelearnersHouse_EventScript_CantTeachEgg::
	msgbox FallarborTown_MoveRelearnersHouse_Text_CantTeachEgg, MSGBOX_DEFAULT
	goto FallarborTown_MoveRelearnersHouse_EventScript_ChooseMon
	end

FallarborTown_MoveRelearnersHouse_EventScript_ComeBackWithHeartScale::
	msgbox FallarborTown_MoveRelearnersHouse_Text_ComeBackWithHeartScale, MSGBOX_DEFAULT
	releaseall
	return

FallarborTown_MoveRelearnersHouse_EventScript_ShowEggMoveTutor:
	clearflag FLAG_HIDE_EGG_MOVE_TUTOR
	end

FallarborTown_MoveRelearnersHouse_EventScript_HideEggMoveTutor:
	setflag FLAG_HIDE_EGG_MOVE_TUTOR
	end

FallarborTown_MoveRelearnersHouse_EventScript_EggMovetutor::
	lockall
	applymovement LOCALID_EGGMOVETUTOR, Common_Movement_FacePlayer
	waitmovement 0
	goto_if_set FLAG_TEMP_2, FallarborTown_MoveRelearnersHouse_EventScript_AskTeachEggMove
	msgbox FallarborTown_MoveRelearnersHouse_Text_ImTheEggMoveTutor, MSGBOX_DEFAULT
	setflag FLAG_TEMP_2
	goto FallarborTown_MoveRelearnersHouse_EventScript_AskTeachEggMove
	end

FallarborTown_MoveRelearnersHouse_EventScript_AskTeachEggMove::
	checkitem ITEM_NUGGET
	goto_if_eq VAR_RESULT, TRUE, FallarborTown_MoveRelearnersHouse_EventScript_HasNugget
	checkitem ITEM_PEARL
	goto_if_eq VAR_RESULT, TRUE, FallarborTown_MoveRelearnersHouse_EventScript_HasPearl
	goto FallarborTown_MoveRelearnersHouse_EventScript_ComeBackLater
	end

FallarborTown_MoveRelearnersHouse_EventScript_HasNugget::
	setvar VAR_0x8006, ITEM_NUGGET
	goto FallarborTown_MoveRelearnersHouse_EventScript_OfferTeachEggMove
	end

FallarborTown_MoveRelearnersHouse_EventScript_HasPearl::
	setvar VAR_0x8006, ITEM_PEARL
	goto FallarborTown_MoveRelearnersHouse_EventScript_OfferTeachEggMove
	end

FallarborTown_MoveRelearnersHouse_EventScript_OfferTeachEggMove::
	msgbox FallarborTown_MoveRelearnersHouse_Text_WantMeToTeachEggMove, MSGBOX_YESNO
	switch VAR_RESULT
	case NO, FallarborTown_MoveRelearnersHouse_EventScript_ComeBackLater
	goto FallarborTown_MoveRelearnersHouse_EventScript_ChooseMonEggMove
	end

FallarborTown_MoveRelearnersHouse_EventScript_ChooseMonEggMove::
	msgbox FallarborTown_MoveRelearnersHouse_Text_TutorWhichMon, MSGBOX_DEFAULT
	setflag FLAG_EGG_MOVES_TUTOR
	special ChooseMonForMoveRelearner
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, FallarborTown_MoveRelearnersHouse_EventScript_ComeBackLater
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, FallarborTown_MoveRelearnersHouse_EventScript_CantTeachEggMoveToEgg
	goto_if_eq VAR_0x8005, 0, FallarborTown_MoveRelearnersHouse_EventScript_NoEggMoveToTeachMon
	goto FallarborTown_MoveRelearnersHouse_EventScript_ChooseEggMove
	end

FallarborTown_MoveRelearnersHouse_EventScript_ChooseEggMove::
	msgbox FallarborTown_MoveRelearnersHouse_Text_TeachWhichMove, MSGBOX_DEFAULT
	special TeachMoveRelearnerMove
	waitstate
	goto_if_eq VAR_0x8004, 0, FallarborTown_MoveRelearnersHouse_EventScript_ChooseMonEggMove
	compare VAR_0x8006, ITEM_NUGGET
	goto_if_eq FallarborTown_MoveRelearnersHouse_EventScript_RemoveNugget
	msgbox FallarborTown_MoveRelearnersHouse_Text_HandedOverPearl, MSGBOX_DEFAULT
	removeitem ITEM_PEARL
	goto FallarborTown_MoveRelearnersHouse_EventScript_ComeBackLater
	end

FallarborTown_MoveRelearnersHouse_EventScript_RemoveNugget::
	msgbox FallarborTown_MoveRelearnersHouse_Text_HandedOverNugget, MSGBOX_DEFAULT
	removeitem ITEM_NUGGET
	goto FallarborTown_MoveRelearnersHouse_EventScript_ComeBackLater
	end

FallarborTown_MoveRelearnersHouse_EventScript_NoEggMoveToTeachMon::
	msgbox FallarborTown_MoveRelearnersHouse_Text_DontHaveMoveToTeachPokemon, MSGBOX_DEFAULT
	goto FallarborTown_MoveRelearnersHouse_EventScript_ChooseMonEggMove
	end

FallarborTown_MoveRelearnersHouse_EventScript_CantTeachEggMoveToEgg::
	msgbox FallarborTown_MoveRelearnersHouse_Text_CantTeachEgg, MSGBOX_DEFAULT
	goto FallarborTown_MoveRelearnersHouse_EventScript_ChooseMonEggMove
	end

FallarborTown_MoveRelearnersHouse_EventScript_ComeBackLater::
	msgbox FallarborTown_MoveRelearnersHouse_Text_ComeBackLater, MSGBOX_DEFAULT
	clearflag FLAG_EGG_MOVES_TUTOR
	releaseall
	end

FallarborTown_MoveRelearnersHouse_Text_ImTheMoveTutor:
	.string "I'm the MOVE TUTOR.\p"
	.string "I know all the moves that POKéMON\n"
	.string "learn--every one of them--and I can\l"
	.string "teach POKéMON those moves.\p"
	.string "I can teach a move to a POKéMON\n"
	.string "of yours if you'd like.\p"
	.string "I'll do it for a HEART SCALE.\n"
	.string "I'm collecting those now.$"

FallarborTown_MoveRelearnersHouse_Text_ThatsAHeartScaleWantMeToTeachMove:
	.string "Oh! That's it! That's an honest to\n"
	.string "goodness HEART SCALE!\p"
	.string "Let me guess, you want me to teach\n"
	.string "a move?$"

FallarborTown_MoveRelearnersHouse_Text_TutorWhichMon:
	.string "Which POKéMON needs tutoring?$"

FallarborTown_MoveRelearnersHouse_Text_TeachWhichMove:
	.string "Which move should I teach?$"

FallarborTown_MoveRelearnersHouse_Text_DontHaveMoveToTeachPokemon:
	.string "Sorry…\p"
	.string "It doesn't appear as if I have any move\n"
	.string "I can teach that POKéMON.$"

FallarborTown_MoveRelearnersHouse_Text_HandedOverHeartScale:
	.string "{PLAYER} handed over one HEART SCALE\n"
	.string "in exchange.$"

FallarborTown_MoveRelearnersHouse_Text_ComeBackWithHeartScale:
	.string "If your POKéMON need to learn a move,\n"
	.string "come back with a HEART SCALE.$"

FallarborTown_MoveRelearnersHouse_Text_CantTeachEgg:
	.string "Hunh? There isn't a single move that\n"
	.string "I can teach an EGG.$"

FallarborTown_MoveRelearnersHouse_Text_ImTheEggMoveTutor:
	.string "I'm the EGG MOVE MANIAC!\p"
	.string "I know all the moves that POKéMON\n"
	.string "learn by breeding--every last one!\l"
	.string "I can teach those moves to yours.\p"
	.string "All I ask is a NUGGET or a PEARL.\n"
	.string "A man's got to fund his research!$"

FallarborTown_MoveRelearnersHouse_Text_WantMeToTeachEggMove:
	.string "Oh! Is that a little treasure\n"
	.string "I spot in your BAG?\p"
	.string "Want me to teach an EGG MOVE\n"
	.string "in exchange?$"

FallarborTown_MoveRelearnersHouse_Text_HandedOverNugget:
	.string "{PLAYER} handed over one NUGGET\n"
	.string "in exchange.$"

FallarborTown_MoveRelearnersHouse_Text_HandedOverPearl:
	.string "{PLAYER} handed over one PEARL\n"
	.string "in exchange.$"

FallarborTown_MoveRelearnersHouse_Text_ComeBackLater:
	.string "If your POKéMON need to learn an\n"
	.string "EGG MOVE, bring me a NUGGET or PEARL.$"
