EventScript_InvalidRecordsCleaned::
	lockall
	msgbox Text_InvalidRecordsCleaned, MSGBOX_DEFAULT
	releaseall
	end

Text_InvalidRecordsCleaned:
	.string "Save data cleanup: removed\n"
	.string "{STR_VAR_1} invalid trainer record(s).$"
