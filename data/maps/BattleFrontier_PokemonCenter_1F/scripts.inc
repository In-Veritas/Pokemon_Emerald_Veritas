.set LOCALID_NURSE, 1
.set LOCALID_TRADEMON, 6

BattleFrontier_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, BattleFrontier_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

BattleFrontier_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_BATTLE_FRONTIER_OUTSIDE_EAST
	@ Floating Pokémon next to the trader is Cookie the Zigzagoon. After
	@ the trade the LOCALID_TRADEMON object is removed permanently, so
	@ the setvar only matters on first visit (or pre-trade visits).
	call_if_unset FLAG_FRONTIER_SEEDOT_NPC_TRADE_COMPLETED, BattleFrontier_PokemonCenter_1F_EventScript_SetUpZigzagoon
	end

BattleFrontier_PokemonCenter_1F_EventScript_SetUpZigzagoon::
	setvar VAR_OBJ_GFX_ID_1, OBJ_EVENT_GFX_MON_BASE+SPECIES_ZIGZAGOON+SPECIES_OVERWORLD_SHINY_TAG
	return

BattleFrontier_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

BattleFrontier_PokemonCenter_1F_EventScript_CookieTrader::
	lock
	faceplayer
	goto_if_set FLAG_FRONTIER_SEEDOT_NPC_TRADE_COMPLETED, BattleFrontier_PokemonCenter_1F_EventScript_TradeCompleted
	msgbox BattleFrontier_PokemonCenter_1F_Text_CookieIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, BattleFrontier_PokemonCenter_1F_EventScript_DeclineTrade
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, BattleFrontier_PokemonCenter_1F_EventScript_DeclineTrade
	specialvar VAR_RESULT, IsSelectedPartyMonShiny
	goto_if_eq VAR_RESULT, FALSE, BattleFrontier_PokemonCenter_1F_EventScript_NotShiny
	@ Mirror the chosen slot into VAR_0x8005 so CreateCookieZigzagoon can
	@ inherit the player's level (matches vanilla trade behavior).
	copyvar VAR_0x8005, VAR_0x8004
	@ Capture the offered species into VAR_OBJ_GFX_ID_1 (with the shiny
	@ overworld tag) BEFORE the trade scene runs, since the trade scene
	@ replaces the offered mon in gPlayerParty with Cookie. The captured
	@ value is then used when LOCALID_TRADEMON is re-spawned post-trade
	@ — the floating sprite becomes the shiny species the player gave up.
	special SetCookieTradeOverworldToOfferedShiny
	special CreateCookieZigzagoon
	fadescreen FADE_TO_BLACK
	removeobject LOCALID_TRADEMON
	fadescreen FADE_FROM_BLACK
	special DoInGameTradeScene
	waitstate
	msgbox BattleFrontier_PokemonCenter_1F_Text_TakeCookie, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	addobject LOCALID_TRADEMON
	fadescreen FADE_FROM_BLACK
	setflag FLAG_FRONTIER_SEEDOT_NPC_TRADE_COMPLETED
	release
	end

BattleFrontier_PokemonCenter_1F_EventScript_DeclineTrade::
	msgbox BattleFrontier_PokemonCenter_1F_Text_DeclineCookie, MSGBOX_DEFAULT
	release
	end

BattleFrontier_PokemonCenter_1F_EventScript_NotShiny::
	msgbox BattleFrontier_PokemonCenter_1F_Text_NotShiny, MSGBOX_DEFAULT
	release
	end

BattleFrontier_PokemonCenter_1F_EventScript_TradeCompleted::
	msgbox BattleFrontier_PokemonCenter_1F_Text_AfterCookieGone, MSGBOX_DEFAULT
	release
	end

BattleFrontier_PokemonCenter_1F_EventScript_Man::
	msgbox BattleFrontier_PokemonCenter_1F_Text_NextStopBattleArena, MSGBOX_NPC
	end

BattleFrontier_PokemonCenter_1F_EventScript_Picnicker::
	msgbox BattleFrontier_PokemonCenter_1F_Text_GoingThroughEveryChallenge, MSGBOX_NPC
	end

BattleFrontier_PokemonCenter_1F_EventScript_Skitty::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_SKITTY, CRY_MODE_NORMAL
	msgbox BattleFrontier_PokemonCenter_1F_Text_Skitty, MSGBOX_DEFAULT
	waitmoncry
	release
	end

BattleFrontier_PokemonCenter_1F_EventScript_Mon::
	@ Pre-trade: floating sprite is the shiny Zigzagoon (Cookie).
	@ Post-trade: floating sprite is whatever shiny the player traded in,
	@ so the handler reads VAR_OBJ_GFX_ID_1 dynamically and plays the
	@ corresponding species cry / name in a generic greeting line.
	call_if_unset FLAG_FRONTIER_SEEDOT_NPC_TRADE_COMPLETED, BattleFrontier_PokemonCenter_1F_EventScript_Zigzagoon
	call_if_set FLAG_FRONTIER_SEEDOT_NPC_TRADE_COMPLETED, BattleFrontier_PokemonCenter_1F_EventScript_TradedShinyMon
	end

BattleFrontier_PokemonCenter_1F_EventScript_Zigzagoon::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_ZIGZAGOON, CRY_MODE_NORMAL
	msgbox BattleFrontier_PokemonCenter_1F_Text_Zigzagoon, MSGBOX_DEFAULT
	waitmoncry
	release
	end

BattleFrontier_PokemonCenter_1F_EventScript_TradedShinyMon::
	lock
	faceplayer
	waitse
	special PrepareTradeFloatingMonGreeting
	msgbox BattleFrontier_PokemonCenter_1F_Text_TradedShinyMon, MSGBOX_DEFAULT
	waitmoncry
	release
	end
	
BattleFrontier_PokemonCenter_1F_Text_CookieIntro:
	.string "Oh, hey! You seem like a TRAINER\n"
	.string "who loves shiny POKéMON!\p"
	.string "Listen, my boyfriend left me his\n"
	.string "ZIGZAGOON when he went off to\l"
	.string "train at the FRONTIER.\p"
	.string "He barks at me all the time and\n"
	.string "is always stealing my socks!\p"
	.string "I would TRADE this stinky pup\n"
	.string "for ANY shiny POKéMON.\p"
	.string "What do you say? Got anything\n"
	.string "you want to trade?$"

BattleFrontier_PokemonCenter_1F_Text_NotShiny:
	.string "Hmm… I want to get rid of him,\n"
	.string "but not THAT much.\p"
	.string "Come back when you find something\n"
	.string "with a little more… sparkle.$"

BattleFrontier_PokemonCenter_1F_Text_TakeCookie:
	.string "Oh… OHHH! That is EXACTLY what\n"
	.string "I'm talking about!\p"
	.string "Take COOKIE. And his sock\n"
	.string "collection. PLEASE.$"

BattleFrontier_PokemonCenter_1F_Text_DeclineCookie:
	.string "Oh… well, if you change your\n"
	.string "mind…\p"
	.string "COOKIE! Drop the sock!\n"
	.string "COOKIE, I said…$"

BattleFrontier_PokemonCenter_1F_Text_AfterCookieGone:
	.string "Hi again. No more lost socks.\n"
	.string "Life is good.\p"
	.string "Take good care of COOKIE for\n"
	.string "me, alright? He's a good boy.$"

BattleFrontier_PokemonCenter_1F_Text_NextStopBattleArena:
	.string "Okay! Next stop, the BATTLE ARENA!\n"
	.string "I'd better get the right POKéMON from\l"
	.string "the PC Storage System.$"

BattleFrontier_PokemonCenter_1F_Text_GoingThroughEveryChallenge:
	.string "Giggle… I'm going to go through every\n"
	.string "challenge with just this baby!$"

BattleFrontier_PokemonCenter_1F_Text_Skitty:
	.string "SKITTY: Mya myaaah!$"

BattleFrontier_PokemonCenter_1F_Text_Zigzagoon:
	.string "COOKIE: Wouf Wouf! Wouf!\p"
	.string "(There's a chewed-up sock at\n"
	.string "your feet.)$"

BattleFrontier_PokemonCenter_1F_Text_TradedShinyMon:
	.string "The shiny {STR_VAR_1} circles\n"
	.string "the spoiled girl.$"
