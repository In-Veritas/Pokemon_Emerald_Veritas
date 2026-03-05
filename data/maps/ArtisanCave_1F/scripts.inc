ArtisanCave_1F_MapScripts::
	.byte 0

ArtisanCave_1F_EventScript_MoveRelearner::
	lockall
	setflag FLAG_EGG_MOVES_TUTOR
	applymovement LOCALID_MOVE_RELEARNER, Common_Movement_FacePlayer
	waitmovement 0
	goto_if_set FLAG_TEMP_1, ArtisanCave_1F_EventScript_AskTeachMove
	msgbox ArtisanCave_1F_Text_ImTheMoveTutor, MSGBOX_DEFAULT
	setflag FLAG_TEMP_1
	goto ArtisanCave_1F_EventScript_AskTeachMove
	end

ArtisanCave_1F_EventScript_AskTeachMove::
	checkitem ITEM_NUGGET
	goto_if_eq VAR_RESULT, TRUE, ArtisanCave_1F_EventScript_HasNugget
	checkitem ITEM_PEARL
	goto_if_eq VAR_RESULT, TRUE, ArtisanCave_1F_EventScript_HasPearl
	goto ArtisanCave_1F_EventScript_ComeBackWithPayment
	end

ArtisanCave_1F_EventScript_HasNugget::
	setvar VAR_0x8006, ITEM_NUGGET
	goto ArtisanCave_1F_EventScript_OfferTeachMove
	end

ArtisanCave_1F_EventScript_HasPearl::
	setvar VAR_0x8006, ITEM_PEARL
	goto ArtisanCave_1F_EventScript_OfferTeachMove
	end

ArtisanCave_1F_EventScript_OfferTeachMove::
	msgbox ArtisanCave_1F_Text_ThatsATreasureWantMeToTeachMove, MSGBOX_YESNO
	switch VAR_RESULT
	case NO, ArtisanCave_1F_EventScript_ComeBackWithPayment
	goto ArtisanCave_1F_EventScript_ChooseMon
	end

ArtisanCave_1F_EventScript_ChooseMon::
	msgbox ArtisanCave_1F_Text_TutorWhichMon, MSGBOX_DEFAULT
	special ChooseMonForMoveRelearner
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, ArtisanCave_1F_EventScript_ComeBackWithPayment
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, ArtisanCave_1F_EventScript_CantTeachEgg
	goto_if_eq VAR_0x8005, 0, ArtisanCave_1F_EventScript_NoMoveToTeachMon
	goto ArtisanCave_1F_EventScript_ChooseMove
	end

ArtisanCave_1F_EventScript_ChooseMove::
	msgbox ArtisanCave_1F_Text_TeachWhichMove, MSGBOX_DEFAULT
	special TeachMoveRelearnerMove
	waitstate
	goto_if_eq VAR_0x8004, 0, ArtisanCave_1F_EventScript_ChooseMon
	compare VAR_0x8006, ITEM_NUGGET
	goto_if_eq ArtisanCave_1F_EventScript_RemoveNugget
	msgbox ArtisanCave_1F_Text_HandedOverPearl, MSGBOX_DEFAULT
	removeitem ITEM_PEARL
	goto ArtisanCave_1F_EventScript_ComeBackWithPayment
	end

ArtisanCave_1F_EventScript_RemoveNugget::
	msgbox ArtisanCave_1F_Text_HandedOverNugget, MSGBOX_DEFAULT
	removeitem ITEM_NUGGET
	goto ArtisanCave_1F_EventScript_ComeBackWithPayment
	end

ArtisanCave_1F_EventScript_NoMoveToTeachMon::
	msgbox ArtisanCave_1F_Text_DontHaveMoveToTeachPokemon, MSGBOX_DEFAULT
	goto ArtisanCave_1F_EventScript_ChooseMon
	end

ArtisanCave_1F_EventScript_CantTeachEgg::
	msgbox ArtisanCave_1F_Text_CantTeachEgg, MSGBOX_DEFAULT
	goto ArtisanCave_1F_EventScript_ChooseMon
	end

ArtisanCave_1F_EventScript_ComeBackWithPayment::
	msgbox ArtisanCave_1F_Text_ComeBackWithPayment, MSGBOX_DEFAULT
	releaseall
	clearflag FLAG_EGG_MOVES_TUTOR
	end

ArtisanCave_1F_Text_ImTheMoveTutor:
	.string "I'm the EGG MOVE ARTIST.\p"
	.string "I study moves with an artist's eye,\n"
	.string "and I sketch them like SMEARGLE.\l"
	.string "I teach moves meant for newborns.\p"
	.string "I can gift an EGG move to your POKéMON\n"
	.string "if you'd like.\p"
	.string "All for a NUGGET or PEARL.\n"
	.string "An artist needs to fund supplies!$"

ArtisanCave_1F_Text_ThatsATreasureWantMeToTeachMove:
	.string "Oh! What a fine treasure you carry!\n"
	.string "It sparkles with inspiration!\p"
	.string "Let me guess, you want me to\n"
	.string "sketch a move?$"

ArtisanCave_1F_Text_TutorWhichMon:
	.string "Which POKéMON needs an EGG move?$"

ArtisanCave_1F_Text_TeachWhichMove:
	.string "Which move should I teach?$"

ArtisanCave_1F_Text_DontHaveMoveToTeachPokemon:
	.string "Sorry…\p"
	.string "I haven't mastered that move's art\n"
	.string "for that POKéMON.$"

ArtisanCave_1F_Text_HandedOverNugget:
	.string "{PLAYER} handed over one NUGGET\n"
	.string "in exchange.$"

ArtisanCave_1F_Text_HandedOverPearl:
	.string "{PLAYER} handed over one PEARL\n"
	.string "in exchange.$"

ArtisanCave_1F_Text_ComeBackWithPayment:
	.string "If your POKéMON need to learn an EGG\n"
	.string "move, bring me a NUGGET or PEARL.$"

ArtisanCave_1F_Text_CantTeachEgg:
	.string "Hunh? No move in my sketchbook\n"
	.string "fits an EGG.$"
