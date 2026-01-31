.set LOCALID_LITTLE_BOY, 5

Route119_WeatherInstitute_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route119_WeatherInstitute_1F_OnTransition
	.byte 0

Route119_WeatherInstitute_1F_OnTransition:
	call_if_eq VAR_WEATHER_INSTITUTE_STATE, 0, Route119_WeatherInstitute_1F_EventScript_SetLittleBoyPos
	end

Route119_WeatherInstitute_1F_EventScript_SetLittleBoyPos::
	setobjectxyperm LOCALID_LITTLE_BOY, 0, 5
	setobjectmovementtype LOCALID_LITTLE_BOY, MOVEMENT_TYPE_FACE_RIGHT
	return

Route119_WeatherInstitute_1F_EventScript_LittleBoy::
	lock
	faceplayer
	special GetPlayerBigGuyGirlString
	goto_if_eq VAR_WEATHER_INSTITUTE_STATE, 0, Route119_WeatherInstitute_1F_EventScript_LittleBoyTeamAquaHere
	msgbox Route119_WeatherInstitute_1F_Text_WowYoureStrong, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_1F_EventScript_LittleBoyTeamAquaHere::
	msgbox Route119_WeatherInstitute_1F_Text_EveryoneWentUpstairs, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_1F_EventScript_InstituteWorker1::
	lock
	faceplayer
	goto_if_unset FLAG_SYS_GAME_CLEAR, Route119_WeatherInstitute_1F_EventScript_StudyingRain
	setvar VAR_0x8004, 0
	call_if_set FLAG_DEFEATED_KYOGRE, Route119_WeatherInstitute_1F_EventScript_LegendaryDefeated
	call_if_set FLAG_DEFEATED_GROUDON, Route119_WeatherInstitute_1F_EventScript_LegendaryDefeated
	goto_if_eq VAR_0x8004, 2, Route119_WeatherInstitute_1F_EventScript_StudyingRain  @ Both defeated
	msgbox Route119_WeatherInstitute_1F_Text_NoticingAbnormalWeather, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_1F_EventScript_LegendaryDefeated::
	addvar VAR_0x8004, 1
	return

Route119_WeatherInstitute_1F_EventScript_StudyingRain::
	msgbox Route119_WeatherInstitute_1F_Text_ProfStudyingRain, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_1F_EventScript_InstituteWorker2::
	goto_if_unset FLAG_SYS_GAME_CLEAR, Route119_WeatherInstitute_1F_EventScriptWhatWereAquasUpTo
	goto_if_set FLAG_CAUGHT_SUICUNE, Route119_WeatherInstitute_1F_EventScriptCaughtSuicune
	goto_if_set FLAG_DEFEATED_SUICUNE, Route119_WeatherInstitute_1F_EventScriptCaughtSuicune
	goto_if_set FLAG_GOT_SECOND_PURITY_READING, Route119_WeatherInstitute_1F_EventScriptRoute108
	goto_if_set FLAG_GOT_FIRST_PURITY_READING, Route119_WeatherInstitute_1F_EventScriptRoute134
	goto_if_set FLAG_STARTED_FIRST_PURITY_READING, Route119_WeatherInstitute_1F_EventScriptRoute128
	msgbox Route119_WeatherInstitute_1F_Text_UnusuallyPureWater, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route119_WeatherInstitute_1F_EventScript_InstituteWorker2End
Route119_WeatherInstitute_1F_EventScriptRoute128::
	msgbox Route119_WeatherInstitute_1F_Text_Route128, MSGBOX_DEFAULT
	setflag FLAG_STARTED_FIRST_PURITY_READING
Route119_WeatherInstitute_1F_EventScript_InstituteWorker2End:
	release
	end

Route119_WeatherInstitute_1F_EventScriptRoute134::
	msgbox Route119_WeatherInstitute_1F_Text_Route134, MSGBOX_DEFAULT
	setflag FLAG_STARTED_SECOND_PURITY_READING
	release
	end

Route119_WeatherInstitute_1F_EventScriptRoute108::
	msgbox Route119_WeatherInstitute_1F_Text_Route108, MSGBOX_DEFAULT
	clearflag FLAG_HIDE_SUICUNE
	setflag FLAG_STARTED_FINAL_PURITY_READING
	release
	end

Route119_WeatherInstitute_1F_EventScriptCaughtSuicune::
	msgbox Route119_WeatherInstitute_1F_Text_FoundSuicune, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_1F_EventScriptWhatWereAquasUpTo::
	msgbox Route119_WeatherInstitute_1F_Text_WhatWereAquasUpTo, MSGBOX_NPC
	end

Route119_WeatherInstitute_1F_EventScript_Bed::
	lockall
	msgbox Route119_WeatherInstitute_1F_Text_TakeRestInBed, MSGBOX_DEFAULT
	closemessage
	call Common_EventScript_OutOfCenterPartyHeal
	releaseall
	end

Route119_WeatherInstitute_1F_EventScript_Grunt1::
	trainerbattle_single TRAINER_GRUNT_WEATHER_INST_1, Route119_WeatherInstitute_1F_Text_Grunt1Intro, Route119_WeatherInstitute_1F_Text_Grunt1Defeat
	msgbox Route119_WeatherInstitute_1F_Text_Grunt1PostBattle, MSGBOX_AUTOCLOSE
	end

Route119_WeatherInstitute_1F_EventScript_Grunt4::
	trainerbattle_single TRAINER_GRUNT_WEATHER_INST_4, Route119_WeatherInstitute_1F_Text_Grunt4Intro, Route119_WeatherInstitute_1F_Text_Grunt4Defeat
	msgbox Route119_WeatherInstitute_1F_Text_Grunt4PostBattle, MSGBOX_AUTOCLOSE
	end

Route119_WeatherInstitute_1F_Text_Grunt1Intro:
	.string "The BOSS got interested in\n"
	.string "the research they have going here,\l"
	.string "so he sent us out.\p"
	.string "You quit meddling!$"

Route119_WeatherInstitute_1F_Text_Grunt1Defeat:
	.string "Blast it…\n"
	.string "Blasted by a kid…$"

Route119_WeatherInstitute_1F_Text_Grunt1PostBattle:
	.string "Our BOSS knows everything.\p"
	.string "But I'm just a GRUNT. What would I know\n"
	.string "about what he's thinking?$"

Route119_WeatherInstitute_1F_Text_Grunt4Intro:
	.string "Huh?\n"
	.string "What's a kid doing here?$"

Route119_WeatherInstitute_1F_Text_Grunt4Defeat:
	.string "Huh?\n"
	.string "I lost?!$"

Route119_WeatherInstitute_1F_Text_Grunt4PostBattle:
	.string "Oh, no…\n"
	.string "I'll catch an earful for losing to a kid…\p"
	.string "I should just take a nap in the bed…$"

Route119_WeatherInstitute_1F_Text_EveryoneWentUpstairs:
	.string "While I was sleeping, everyone went\n"
	.string "upstairs!$"

Route119_WeatherInstitute_1F_Text_WowYoureStrong:
	.string "Wow, you're really strong!\p"
	.string "I wish I could be a POKéMON TRAINER\n"
	.string "like you!$"

Route119_WeatherInstitute_1F_Text_ProfStudyingRain:
	.string "The PROFESSOR loves rain.\n"
	.string "That's a fact.\p"
	.string "But if it keeps raining, people will be in\n"
	.string "trouble. That's another fact.\p"
	.string "And thus, the PROFESSOR is studying\n"
	.string "if the rain can be put to good use.$"

Route119_WeatherInstitute_1F_Text_NoticingAbnormalWeather:
	.string "On the 2nd floor of the INSTITUTE,\n"
	.string "we study the weather patterns over\l"
	.string "the HOENN region.\p"
	.string "We've been noticing temporary and\n"
	.string "isolated cases of droughts and\l"
	.string "heavy rain lately…$"

Route119_WeatherInstitute_1F_Text_WhatWereAquasUpTo:
	.string "Hello!\n"
	.string "We've been saved by your actions!\p"
	.string "What I don't understand is what on\n"
	.string "earth the AQUAS were up to.$"

Route119_WeatherInstitute_1F_Text_UnusuallyPureWater:
	.string "Hello! Thank you again for rescuing us\n"
	.string "from those AQUA goons!\p"
	.string "Actually, I could use your help with\n"
	.string "something else.\p"
	.string "We've noticed unusually pure water in\n"
	.string "certain areas across the region.\p"
	.string "Could you take some water purity\n"
	.string "readings for us?$"

Route119_WeatherInstitute_1F_Text_Route128:
	.string "Great! First, try ROUTE 128 by diving\n"
	.string "into the deeper spots.$"

Route119_WeatherInstitute_1F_Text_Route134:
	.string "Ah, you're back!\p"
	.string "Interesting… ROUTE 128's water is only\n"
	.string "slightly purer than usual.\p"
	.string "But the readings are incomplete. I'd\n"
	.string "like you to dive in ROUTE 134 next.\p"
	.string "We need more data to pinpoint what's\n"
	.string "causing this.$"

Route119_WeatherInstitute_1F_Text_Route108:
	.string "Hmm, ROUTE 134 shows even higher levels\n"
	.string "of purity.\p"
	.string "It looks like the source of this pure\n"
	.string "water is concentrated somewhere\l"
	.string "around ROUTE 108!\p"
	.string "Could you head there and find out\n"
	.string "what's behind all this?\p"
	.string "I'm counting on you!$"

Route119_WeatherInstitute_1F_Text_FoundSuicune:
	.string "What!? A SUICUNE!?\p"
	.string "I never imagined a LEGENDARY POKéMON\n"
	.string "was behind this!\l"
	.string "That explains everything…\p"
	.string "SUICUNE must be purifying the waters\n"
	.string "across the region.$"

Route119_WeatherInstitute_1F_Text_TakeRestInBed:
	.string "There's a bed…\n"
	.string "Let's take a rest.$"
