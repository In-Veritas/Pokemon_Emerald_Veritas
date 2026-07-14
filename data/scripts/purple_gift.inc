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
@ until the player sets a date. Opens the date picker on the spot; the
@ date can still be changed later at the room clock (START after the time).
EventScript_DateNotSetReminder::
	lockall
	msgbox Text_DateNotSetReminder, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	special DateSetFromScript
	waitstate
	msgbox Text_DateSetThanks, MSGBOX_DEFAULT
	releaseall
	end

Text_DateNotSetReminder:
	.string "The clock in your room seems to\n"
	.string "have lost track of today's date.\p"
	.string "Let's set today's date right now!$"

Text_DateSetThanks:
	.string "All set!\p"
	.string "You can change the date any time\n"
	.string "by checking the clock in your\l"
	.string "room and pressing START.$"
