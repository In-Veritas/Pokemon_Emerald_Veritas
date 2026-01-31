.set LOCALID_GRUNT_1, 1
.set LOCALID_GRUNT_2, 2
.set LOCALID_SHELLY, 3
.set LOCALID_SCIENTIST, 5
.set LOCALID_GRUNT_3, 7
.set LOCALID_GRUNT_4, 8

Route119_WeatherInstitute_2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route119_WeatherInstitute_2F_OnTransition
	.byte 0

Route119_WeatherInstitute_2F_OnTransition:
	call_if_eq VAR_WEATHER_INSTITUTE_STATE, 0, Route119_WeatherInstitute_2F_EventScript_SetScientistPosAquaHere
	call_if_eq VAR_WEATHER_INSTITUTE_STATE, 1, Route119_WeatherInstitute_2F_EventScript_SetScientistPosAquaGone
	call_if_set FLAG_SYS_GAME_CLEAR, Route119_WeatherInstitute_2F_EventScript_SetScientistPosGameClear
	end

Route119_WeatherInstitute_2F_EventScript_SetScientistPosAquaHere::
	setobjectxyperm LOCALID_SCIENTIST, 1, 6
	setobjectmovementtype LOCALID_SCIENTIST, MOVEMENT_TYPE_FACE_RIGHT
	return

Route119_WeatherInstitute_2F_EventScript_SetScientistPosAquaGone::
	setobjectxyperm LOCALID_SCIENTIST, 4, 6
	setobjectmovementtype LOCALID_SCIENTIST, MOVEMENT_TYPE_FACE_RIGHT
	return

Route119_WeatherInstitute_2F_EventScript_SetScientistPosGameClear::
	setobjectxyperm LOCALID_SCIENTIST, 2, 2
	setobjectmovementtype LOCALID_SCIENTIST, MOVEMENT_TYPE_FACE_UP
	return

Route119_WeatherInstitute_2F_EventScript_Grunt5::
	trainerbattle_single TRAINER_GRUNT_WEATHER_INST_5, Route119_WeatherInstitute_2F_Text_Grunt5Intro, Route119_WeatherInstitute_2F_Text_Grunt5Defeat
	msgbox Route119_WeatherInstitute_2F_Text_Grunt5PostBattle, MSGBOX_AUTOCLOSE
	end

Route119_WeatherInstitute_2F_EventScript_Grunt2::
	trainerbattle_single TRAINER_GRUNT_WEATHER_INST_2, Route119_WeatherInstitute_2F_Text_Grunt2Intro, Route119_WeatherInstitute_2F_Text_Grunt2Defeat
	msgbox Route119_WeatherInstitute_2F_Text_Grunt2PostBattle, MSGBOX_AUTOCLOSE
	end

Route119_WeatherInstitute_2F_EventScript_Grunt3::
	trainerbattle_single TRAINER_GRUNT_WEATHER_INST_3, Route119_WeatherInstitute_2F_Text_Grunt3Intro, Route119_WeatherInstitute_2F_Text_Grunt3Defeat
	msgbox Route119_WeatherInstitute_2F_Text_Grunt3PostBattle, MSGBOX_AUTOCLOSE
	end

Route119_WeatherInstitute_2F_EventScript_Shelly::
	trainerbattle_single TRAINER_SHELLY_WEATHER_INSTITUTE, Route119_WeatherInstitute_2F_Text_ShellyIntro, Route119_WeatherInstitute_2F_Text_ShellyDefeat, Route119_WeatherInstitute_2F_EventScript_ShellyDefeated
	msgbox Route119_WeatherInstitute_2F_Text_ShellyPostBattle, MSGBOX_AUTOCLOSE
	end

Route119_WeatherInstitute_2F_EventScript_ShellyDefeated::
	msgbox Route119_WeatherInstitute_2F_Text_ShellyPostBattle, MSGBOX_DEFAULT
	closemessage
	addobject LOCALID_GRUNT_3
	applymovement LOCALID_GRUNT_3, Route119_WeatherInstitute_2F_Movement_GruntApproachShelly
	applymovement OBJ_EVENT_ID_PLAYER, Route119_WeatherInstitute_2F_Movement_ShovePlayerOutOfWay
	waitmovement 0
	msgbox Route119_WeatherInstitute_2F_Text_TeamMagmaJustPassedBy, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement LOCALID_SHELLY, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_SHELLY, Common_Movement_Delay48
	waitmovement 0
	msgbox Route119_WeatherInstitute_2F_Text_WeHaveToHurryToMtPyre, MSGBOX_DEFAULT
	closemessage
	setvar VAR_WEATHER_INSTITUTE_STATE, 1
	clearflag FLAG_HIDE_WEATHER_INSTITUTE_2F_WORKERS
	fadedefaultbgm
	fadescreen FADE_TO_BLACK
	setflag FLAG_HIDE_ROUTE_119_TEAM_AQUA
	setflag FLAG_HIDE_WEATHER_INSTITUTE_2F_AQUA_GRUNT_M
	removeobject LOCALID_GRUNT_1
	removeobject LOCALID_GRUNT_2
	removeobject LOCALID_SHELLY
	removeobject LOCALID_GRUNT_4
	removeobject LOCALID_GRUNT_3
	applymovement OBJ_EVENT_ID_PLAYER, Route119_WeatherInstitute_2F_Movement_PlayerReturnToPosition
	waitmovement 0
	fadescreen FADE_FROM_BLACK
	applymovement LOCALID_SCIENTIST, Route119_WeatherInstitute_2F_Movement_ScientistApproachPlayer
	waitmovement 0
	copyobjectxytoperm LOCALID_SCIENTIST
	goto Route119_WeatherInstitute_2F_EventScript_ReceiveCastform
	end

Route119_WeatherInstitute_2F_EventScript_ReceiveCastform::
	msgbox Route119_WeatherInstitute_2F_Text_ThanksPleaseTakePokemon, MSGBOX_DEFAULT
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_CASTFORM
	givemon SPECIES_CASTFORM, 25, ITEM_MYSTIC_WATER
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PARTY, Route119_WeatherInstitute_2F_EventScript_ReceiveCastformParty
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, Route119_WeatherInstitute_2F_EventScript_ReceiveCastformPC
	goto Common_EventScript_NoMoreRoomForPokemon
	end

Route119_WeatherInstitute_2F_EventScript_ReceiveCastformParty::
	call Route119_WeatherInstitute_2F_EventScript_ReceivedCastformFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route119_WeatherInstitute_2F_EventScript_ExplainCastform
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto Route119_WeatherInstitute_2F_EventScript_ExplainCastform
	end

Route119_WeatherInstitute_2F_EventScript_ReceiveCastformPC::
	call Route119_WeatherInstitute_2F_EventScript_ReceivedCastformFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route119_WeatherInstitute_2F_EventScript_SendCastformToPC
	call Common_EventScript_NameReceivedBoxMon
	goto Route119_WeatherInstitute_2F_EventScript_SendCastformToPC
	end

Route119_WeatherInstitute_2F_EventScript_SendCastformToPC::
	call Common_EventScript_TransferredToPC
	goto Route119_WeatherInstitute_2F_EventScript_ExplainCastform
	end

Route119_WeatherInstitute_2F_EventScript_ReceivedCastformFanfare::
	playfanfare MUS_OBTAIN_ITEM
	message Route119_WeatherInstitute_2F_Text_PlayerReceivedCastform
	waitmessage
	waitfanfare
	bufferspeciesname STR_VAR_1, SPECIES_CASTFORM
	return

Route119_WeatherInstitute_2F_EventScript_ExplainCastform::
	msgbox Route119_WeatherInstitute_2F_Text_PokemonChangesWithWeather, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_CASTFORM
	release
	end

Route119_WeatherInstitute_2F_EventScript_ScientistMentionWeather::
	goto_if_set FLAG_SYS_GAME_CLEAR, Route119_WeatherInstitute_2F_EventScript_TryStartAbnormalWeather
	msgbox Route119_WeatherInstitute_2F_Text_ChangingWeatherRidiculous, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_2F_EventScript_TryStartAbnormalWeather::
	setvar VAR_0x8004, 0
	call_if_set FLAG_DEFEATED_KYOGRE, Route119_WeatherInstitute_2F_EventScript_LegendaryDefeated
	call_if_set FLAG_DEFEATED_GROUDON, Route119_WeatherInstitute_2F_EventScript_LegendaryDefeated
	goto_if_eq VAR_0x8004, 2, Route119_WeatherInstitute_2F_EventScript_NoAbnormalWeather  @ Both defeated
	call_if_unset FLAG_TEMP_2, Route119_WeatherInstitute_2F_EventScript_CreateAbnormalWeather
	specialvar VAR_RESULT, GetAbnormalWeatherMapNameAndType
	goto_if_eq VAR_RESULT, 1, Route119_WeatherInstitute_2F_EventScript_KyogreWeather
	msgbox Route119_WeatherInstitute_2F_Text_GroudonWeather, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_2F_EventScript_KyogreWeather::
	msgbox Route119_WeatherInstitute_2F_Text_KyogreWeather, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_2F_EventScript_CreateAbnormalWeather::
	special CreateAbnormalWeatherEvent
	setflag FLAG_TEMP_2
	return

Route119_WeatherInstitute_2F_EventScript_LegendaryDefeated::
	addvar VAR_0x8004, 1
	return

Route119_WeatherInstitute_2F_EventScript_NoAbnormalWeather::
	msgbox Route119_WeatherInstitute_2F_Text_NoAbnormalWeather, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_2F_Movement_GruntApproachShelly:
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Route119_WeatherInstitute_2F_Movement_ShovePlayerOutOfWay:
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	ride_water_current_up
	walk_in_place_faster_down
	step_end

Route119_WeatherInstitute_2F_Movement_PlayerReturnToPosition:
	slide_down
	walk_in_place_faster_left
	step_end

Route119_WeatherInstitute_2F_Movement_ScientistApproachPlayer:
	walk_right
	walk_right
	walk_right
	step_end

Route119_WeatherInstitute_2F_EventScript_WeatherScientist::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_CASTFORM, Route119_WeatherInstitute_2F_EventScript_ScientistMentionWeather
	goto Route119_WeatherInstitute_2F_EventScript_ReceiveCastform
	end

Route119_WeatherInstitute_2F_EventScript_InstituteWorker3::
	faceplayer
	goto_if_set FLAG_CAUGHT_RAIKOU, Route119_WeatherInstitute_2F_EventScriptCaughtRaikou
	goto_if_set FLAG_DEFEATED_RAIKOU, Route119_WeatherInstitute_2F_EventScriptCaughtRaikou
	goto_if_set FLAG_GOT_SECOND_LIGHTNING_SEARCH, Route119_WeatherInstitute_2F_EventScriptRoute119
	goto_if_set FLAG_GOT_FIRST_LIGHTNING_SEARCH, Route119_WeatherInstitute_2F_EventScriptBattleFrontier
	goto_if_set FLAG_STARTED_FIRST_LIGHTNING_SEARCH, Route119_WeatherInstitute_2F_EventScriptRoute123
	msgbox Route119_WeatherInstitute_2F_Text_LightningPatterns, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route119_WeatherInstitute_2F_EventScript_InstituteWorker3End
Route119_WeatherInstitute_2F_EventScriptRoute123::
	msgbox Route119_WeatherInstitute_2F_Text_Route123, MSGBOX_DEFAULT
	clearflag FLAG_HIDE_RAIKOU1
	setflag FLAG_STARTED_FIRST_LIGHTNING_SEARCH
	Route119_WeatherInstitute_2F_EventScript_InstituteWorker3End:
	release
	end

Route119_WeatherInstitute_2F_EventScriptBattleFrontier::
	msgbox Route119_WeatherInstitute_2F_Text_BattleFrontier, MSGBOX_DEFAULT
	clearflag FLAG_HIDE_RAIKOU2
	setflag FLAG_STARTED_SECOND_LIGHTNING_SEARCH
	release
	end

Route119_WeatherInstitute_2F_EventScriptRoute119::
	msgbox Route119_WeatherInstitute_2F_Text_Route119, MSGBOX_DEFAULT
	clearflag FLAG_HIDE_RAIKOU3
	setflag FLAG_STARTED_FINAL_LIGHTNING_SEARCH
	release
	end

Route119_WeatherInstitute_2F_EventScriptCaughtRaikou::
	msgbox Route119_WeatherInstitute_2F_Text_FoundRaikou, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_2F_EventScript_InstituteWorker4::
	faceplayer
	goto_if_set FLAG_CAUGHT_ENTEI, Route119_WeatherInstitute_2F_EventScriptCaughtEntei
	goto_if_set FLAG_DEFEATED_ENTEI, Route119_WeatherInstitute_2F_EventScriptCaughtEntei
	goto_if_set FLAG_GOT_ASH_SAMPLE, Route119_WeatherInstitute_2F_EventScriptScorchedSlab
	goto_if_set FLAG_GOT_FIRST_SEISMIC_READING, Route119_WeatherInstitute_2F_EventScriptRoute113
	goto_if_set FLAG_STARTED_FIRST_SEISMIC_READING, Route119_WeatherInstitute_2F_EventScriptMtChimney
	msgbox Route119_WeatherInstitute_2F_Text_SeismicActivity, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route119_WeatherInstitute_2F_EventScript_InstituteWorker4End
Route119_WeatherInstitute_2F_EventScriptMtChimney::
	msgbox Route119_WeatherInstitute_2F_Text_MtChimney, MSGBOX_DEFAULT
	setflag FLAG_STARTED_FIRST_SEISMIC_READING
	Route119_WeatherInstitute_2F_EventScript_InstituteWorker4End:
	release
	end

Route119_WeatherInstitute_2F_EventScriptRoute113::
	msgbox Route119_WeatherInstitute_2F_Text_Route113, MSGBOX_DEFAULT
	setflag FLAG_STARTED_ASH_SAMPLE
	release
	end

Route119_WeatherInstitute_2F_EventScriptScorchedSlab::
	msgbox Route119_WeatherInstitute_2F_Text_ScorchedSlab, MSGBOX_DEFAULT
	clearflag FLAG_HIDE_ENTEI
	setflag FLAG_STARTED_FINAL_SEISMIC_READING
	release
	end

Route119_WeatherInstitute_2F_EventScriptCaughtEntei::
	msgbox Route119_WeatherInstitute_2F_Text_FoundEntei, MSGBOX_DEFAULT
	release
	end

Route119_WeatherInstitute_2F_Text_Grunt2Intro:
	.string "The INSTITUTE created a type of\n"
	.string "POKéMON that has something to do with\l"
	.string "the weather. We're here to take them!$"

Route119_WeatherInstitute_2F_Text_Grunt2Defeat:
	.string "Our plan's being spoiled by a kid?$"

Route119_WeatherInstitute_2F_Text_Grunt2PostBattle:
	.string "If the POKéMON they made here can\n"
	.string "control the weather freely, then we of\l"
	.string "TEAM AQUA definitely need it!$"

Route119_WeatherInstitute_2F_Text_Grunt3Intro:
	.string "We're TEAM AQUA!\n"
	.string "We appear wherever anything rare is\l"
	.string "found!$"

Route119_WeatherInstitute_2F_Text_Grunt3Defeat:
	.string "You got me!$"

Route119_WeatherInstitute_2F_Text_Grunt3PostBattle:
	.string "You don't have any idea what we of\n"
	.string "TEAM AQUA are working towards!\l"
	.string "You stay out of our way!$"

Route119_WeatherInstitute_2F_Text_Grunt5Intro:
	.string "Don't tell me you're looking for that\n"
	.string "weather POKéMON, too?\p"
	.string "That's a no-no!\n"
	.string "We were here to get it first!$"

Route119_WeatherInstitute_2F_Text_Grunt5Defeat:
	.string "Oh, will you look at my POKéMON?$"

Route119_WeatherInstitute_2F_Text_Grunt5PostBattle:
	.string "Humph, so what?\n"
	.string "What we want…\p"
	.string "What we really want isn't here…\n"
	.string "Ihihihihi…$"

Route119_WeatherInstitute_2F_Text_ShellyIntro:
	.string "Ahahahaha!\p"
	.string "You're going to meddle in TEAM AQUA's\n"
	.string "affairs?\p"
	.string "You're either absolutely fearless,\n"
	.string "simply ignorant, or both!\p"
	.string "You're so cute, you're disgusting!\n"
	.string "I'll put you down, kiddy!$"

Route119_WeatherInstitute_2F_Text_ShellyDefeat:
	.string "Ahahahaha!\n"
	.string "You're disgustingly strong!$"

Route119_WeatherInstitute_2F_Text_ShellyPostBattle:
	.string "It's bad enough to have TEAM MAGMA\n"
	.string "blunder about, but now there's you!\p"
	.string "What makes you want to sniff around\n"
	.string "in our business, anyway?$"

Route119_WeatherInstitute_2F_Text_TeamMagmaJustPassedBy:
	.string "We have a situation here!\p"
	.string "A TEAM MAGMA mob just passed\n"
	.string "the WEATHER INSTITUTE.\p"
	.string "They appear to be headed for\n"
	.string "MT. PYRE!$"

Route119_WeatherInstitute_2F_Text_WeHaveToHurryToMtPyre:
	.string "What?!\p"
	.string "We can't waste any more time here!\n"
	.string "We have to hurry to MT. PYRE, too!\p"
	.string "Ahahahaha!\n"
	.string "TEAM MAGMA, just you wait!$"

Route119_WeatherInstitute_2F_Text_ThanksPleaseTakePokemon:
	.string "Thanks!\n"
	.string "Thanks to you, we're safe!\p"
	.string "It might be an odd way of thanking you,\n"
	.string "but take this POKéMON.$"

@ Unused
Route119_WeatherInstitute_2F_Text_NoRoomForPokemon:
	.string "Hm? You don't seem to have any room\n"
	.string "for this POKéMON.$"

Route119_WeatherInstitute_2F_Text_PlayerReceivedCastform:
	.string "{PLAYER} received CASTFORM!$"

Route119_WeatherInstitute_2F_Text_PokemonChangesWithWeather:
	.string "That POKéMON changes shape according\n"
	.string "to the weather conditions.\p"
	.string "There're plenty of them in the\n"
	.string "INSTITUTE--go ahead and take it.$"

Route119_WeatherInstitute_2F_Text_ChangingWeatherRidiculous:
	.string "I've been researching rain for many\n"
	.string "years, but it's ridiculous to think that\l"
	.string "humans can freely change the weather.$"

Route119_WeatherInstitute_2F_Text_GroudonWeather:
	.string "I track weather patterns over\n"
	.string "the HOENN region.\p"
	.string "Presently, a drought has been recorded\n"
	.string "in {STR_VAR_1}.\p"
	.string "Could that mean, somewhere near\n"
	.string "{STR_VAR_1}…$"

Route119_WeatherInstitute_2F_Text_KyogreWeather:
	.string "I track weather patterns over\n"
	.string "the HOENN region.\p"
	.string "Presently, heavy rainfall has been\n"
	.string "recorded over {STR_VAR_1}.\p"
	.string "Could that mean, somewhere near\n"
	.string "{STR_VAR_1}…$"

Route119_WeatherInstitute_2F_Text_NoAbnormalWeather:
	.string "Abnormal weather conditions are\n"
	.string "no longer being reported.\p"
	.string "The occasional rainfall is a blessing,\n"
	.string "really.$"

Route119_WeatherInstitute_2F_Text_LightningPatterns:
	.string "Hey there!\p"
	.string "We've brought on some extra hands here\n"
	.string "to monitor any unusual weather patterns.\p"
	.string "Actually, I've been tracking some\n"
	.string "strange thunder and lightning patterns\l"
	.string "across the region.\p"
	.string "It might be a lingering effect from the\n"
	.string "rainstorms, but we need more data to be\l"
	.string "sure.\p"
	.string "Could you investigate for us?$"

Route119_WeatherInstitute_2F_Text_Route123:
	.string "Great! We've seen activity around\n"
	.string "ROUTE 123.\p"
	.string "Head over there and see if you can find\n"
	.string "anything unusual.$"

Route119_WeatherInstitute_2F_Text_BattleFrontier:
	.string "Ah, you're back!\p"
	.string "While you were gone, the lightning\n"
	.string "around ROUTE 123 just stopped\l"
	.string "suddenly.\p"
	.string "But now, we're getting reports from the\n"
	.string "BATTLE FRONTIER!\p"
	.string "Could you head there next? We need to\n"
	.string "get to the bottom of this.$"

Route119_WeatherInstitute_2F_Text_Route119:
	.string "You're back already?\n"
	.string "Thanks, but just as before…\p"
	.string "The lightning at the BATTLE FRONTIER\n"
	.string "disappeared shortly after you arrived.\p"
	.string "Now, we're seeing reports near ROUTE\n"
	.string "119. Can you check that out for us?\p"
	.string "I don't know what's going on, but it's\n"
	.string "like the storms are running from you!$"

Route119_WeatherInstitute_2F_Text_FoundRaikou:
	.string "What!? You saw a RAIKOU!?\p"
	.string "That explains the strange thunder and\n"
	.string "why it kept moving around!\p"
	.string "Amazing… You've been tracking a\n"
	.string "LEGENDARY POKéMON this whole time!$"

Route119_WeatherInstitute_2F_Text_SeismicActivity:
	.string "Oh, hey!\p"
	.string "With all the recent weather anomalies,\n"
	.string "we've expanded our team to monitor any\l"
	.string "strange environmental activity.\p"
	.string "But listen, there's something worrying\n"
	.string "me. It's possible the weather incidents\l"
	.string "have irritated MT CHIMNEY, and we're\l"
	.string "worried it could lead to an eruption.\p"
	.string "Could you head up to the summit and\n"
	.string "check for signs of seismic activity?$"

Route119_WeatherInstitute_2F_Text_MtChimney:
	.string "If you notice anything unusual at MT\n"
	.string "CHIMNEY, please report back as soon as\l"
	.string "possible.$"

Route119_WeatherInstitute_2F_Text_Route113:
	.string "You're back! So, no major tremors…\n"
	.string "That's a relief, but I'd like to be sure.\p"
	.string "Could you collect an ash sample from\n"
	.string "Route 113 for us?\p"
	.string "It could tell us if any changes are\n"
	.string "happening beneath the surface.$"

Route119_WeatherInstitute_2F_Text_ScorchedSlab:
	.string "Ah, welcome back!\p"
	.string "That ash sample is normal, but we've\n"
	.string "just had reports of seismic activity\l"
	.string "near the SCORCHED SLAB.\p"
	.string "It could be the early formation of a\n"
	.string "parasitic cone.\p"
	.string "Can you investigate? Just be cautious.$"

Route119_WeatherInstitute_2F_Text_FoundEntei:
	.string "Wait, what!? You found an ENTEI?\p"
	.string "I can't believe a LEGENDARY POKéMON\n"
	.string "was behind this…\p"
	.string "ENTEI'S roar is said to cause eruptions.\n"
	.string "That explains the seismic readings!\p"
	.string "Fortunately, there's no parasitic cone\n"
	.string "forming.\p"
	.string "ENTEI must have been drawn here by the\n"
	.string "recent activity.\p"
	.string "Thank you for checking! It looks like\n"
	.string "everything's under control now.$"
