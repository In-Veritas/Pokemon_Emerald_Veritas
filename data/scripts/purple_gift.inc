EventScript_PurpleBirthdayGift::
	lockall
	msgbox Text_PurpleBirthday, MSGBOX_DEFAULT
	giveitem ITEM_CLAW_FOSSIL
	releaseall
	end

Text_PurpleBirthday:
	.string "Happy birthday! You are gay!\n"
	.string "Here's a Birthday Gift!$"

@ Shown once per boot on saves that predate the calendar-date feature,
@ until the player sets a date (START on the room clock, after the time).
EventScript_DateNotSetReminder::
	lockall
	msgbox Text_DateNotSetReminder, MSGBOX_DEFAULT
	releaseall
	end

Text_DateNotSetReminder:
	.string "The clock in your room seems to\n"
	.string "have lost track of today's date.\p"
	.string "Check the clock in your room and\n"
	.string "press START to set the time and\l"
	.string "today's date.$"
