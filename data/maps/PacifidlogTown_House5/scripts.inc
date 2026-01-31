PacifidlogTown_House5_MapScripts::
	.byte 0

PacifidlogTown_House5_EventScript_MirageIslandWatcher::
	lock
	faceplayer
	specialvar VAR_RESULT, IsMirageIslandPresent
	goto_if_eq VAR_RESULT, TRUE, PacifidlogTown_House5_EventScript_MirageIslandPresent
	goto_if_set FLAG_SHOW_MIRAGE_ISLAND, PacifidlogTown_House5_EventScript_MirageIslandPresent
	msgbox PacifidlogTown_House5_Text_CantSeeMirageIslandToday, MSGBOX_DEFAULT
	release
	end

PacifidlogTown_House5_EventScript_MirageIslandPresent::
	msgbox PacifidlogTown_House5_Text_CanSeeMirageIslandToday, MSGBOX_DEFAULT
	release
	end

PacifidlogTown_House5_EventScript_Gentleman::
	msgbox PacifidlogTown_House5_Text_MirageIslandAppearDependingOnWeather, MSGBOX_NPC
	end

PacifidlogTown_House5_EventScript_OldWoman::
	msgbox PacifidlogTown_House5_Text_OldWoman, MSGBOX_NPC
	end

PacifidlogTown_House5_EventScript_OldMan::
	msgbox PacifidlogTown_House5_Text_OldMan, MSGBOX_NPC
	end

PacifidlogTown_House5_Text_CantSeeMirageIslandToday:
	.string "I can't see MIRAGE ISLAND today…$"

PacifidlogTown_House5_Text_CanSeeMirageIslandToday:
	.string "Oh! Oh my!\n"
	.string "I can see MIRAGE ISLAND today!$"

PacifidlogTown_House5_Text_MirageIslandAppearDependingOnWeather:
	.string "MIRAGE ISLAND…\p"
	.string "It must become visible and invisible\n"
	.string "depending on the weather conditions\l"
	.string "that make mirages appear.\p"
	.string "Or is it really appearing and\n"
	.string "disappearing?$"

PacifidlogTown_House5_Text_OldWoman:
	.string "I heard some people talking about how\n"
	.string "time doesn't flow the same everywhere.\p"
	.string "They say clocks can do funny things. I\n"
	.string "tried staring at my clock once, but\l"
	.string "nothing happened.$"

PacifidlogTown_House5_Text_OldMan:
	.string "The waves are quietest at sunrise…\n"
	.string "calm at the turn of day… and still again\l"
	.string "at night fall.\p"
	.string "It all feels like a peaceful dance,\n"
	.string "doesn't it?\p"
	.string "I've always thought MIRAGE ISLAND\n"
	.string "appears when the day feels at\l"
	.string "peace.$"
