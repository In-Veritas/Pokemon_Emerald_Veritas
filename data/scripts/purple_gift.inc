EventScript_PurpleBirthdayGift::
	lockall
	msgbox Text_PurpleBirthday, MSGBOX_DEFAULT
	giveitem ITEM_CLAW_FOSSIL
	releaseall
	end

Text_PurpleBirthday:
	.string "Happy birthday! You are gay!\n"
	.string "Here's a Birthday Gift!$"
