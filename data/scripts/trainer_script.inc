Std_RegisteredInMatchCall::
	buffertrainername STR_VAR_1, VAR_0x8000
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox gText_RegisteredTrainerInPokenav, MSGBOX_DEFAULT
	waitfanfare
	return

EventScript_TryGetTrainerScript::
	special ShouldTryGetTrainerScript
	goto_if_eq VAR_RESULT, TRUE, EventScript_GotoTrainerScript
	releaseall
	end

EventScript_GotoTrainerScript::
	gotobeatenscript
	releaseall
	end
