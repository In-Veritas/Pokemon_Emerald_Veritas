EventScript_AlhyaApology::
	lockall
	msgbox Text_AlhyaApology, MSGBOX_DEFAULT
	giveitem ITEM_RARE_CANDY, 80
	giveitem ITEM_SHINY_CHARM
	releaseall
	end

Text_AlhyaApology:
	.string "Ooops, sorry\n"
	.string "for the bug.$"
