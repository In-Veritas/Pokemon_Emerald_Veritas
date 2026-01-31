Underwater_Route134_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, Underwater_Route134_OnResume
	.byte 0

Underwater_Route134_OnResume:
	setdivewarp MAP_ROUTE134, 60, 31
	call Underwater_Route134_CheckPurityReading
	end

Underwater_Route134_CheckPurityReading::
	goto_if_unset FLAG_STARTED_SECOND_PURITY_READING, Common_EventScript_NopReturn
	setflag FLAG_GOT_SECOND_PURITY_READING
	return
