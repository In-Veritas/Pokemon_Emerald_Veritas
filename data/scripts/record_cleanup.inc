EventScript_InvalidRecordsCleaned::
	lockall
	msgbox Text_InvalidRecordsCleaned, MSGBOX_DEFAULT
	releaseall
	end

Text_InvalidRecordsCleaned:
	.string "Save data cleanup:\n"
	.string "Corrupted records removed.\p"
	.string "Link battle records: {STR_VAR_1}\n"
	.string "Trainer name records: {STR_VAR_2}\n"
	.string "Secret bases: {STR_VAR_3}$"
