GraniteCave_B2F_MapScripts::
	.byte 0

DewfordTown_EventScript_Tobin::
	trainerbattle_single TRAINER_TOBIN, GraniteCaveB2F_Text_TobinIntro, GraniteCaveB2F_Text_TobinDefeated
	msgbox GraniteCaveB2F_Text_TobinPostBattle, MSGBOX_AUTOCLOSE
	end
