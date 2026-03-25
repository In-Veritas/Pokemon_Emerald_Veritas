@ Shared Fashionista NPC script
@ VAR_0x8006 = NPC team: 0=Magma M, 1=Magma F, 2=Aqua M, 3=Aqua F
@
@ Personalities:
@   Magma M (Slateport)  - Stoic perfectionist, serious craftsman
@   Magma F (Lavaridge)   - Cutie, flirty, bubbly
@   Aqua M (Lilycove)     - Informal, graffiti artist vibe, uses slang
@   Aqua F (Pacifidlog)   - Dreamy, poetic, nature-inspired

EventScript_Fashionista::
	lock
	faceplayer
	goto_if_eq VAR_0x8006, 0, EventScript_Fashionista_IntroMagmaM
	goto_if_eq VAR_0x8006, 1, EventScript_Fashionista_IntroMagmaF
	goto_if_eq VAR_0x8006, 2, EventScript_Fashionista_IntroAquaM
	goto_if_eq VAR_0x8006, 3, EventScript_Fashionista_IntroAquaF
	goto EventScript_Fashionista_AfterIntro
	end

EventScript_Fashionista_IntroMagmaM::
	msgbox EventScript_Fashionista_Text_MagmaM_Intro, MSGBOX_DEFAULT
	goto EventScript_Fashionista_AfterIntro
	end
EventScript_Fashionista_IntroMagmaF::
	msgbox EventScript_Fashionista_Text_MagmaF_Intro, MSGBOX_DEFAULT
	goto EventScript_Fashionista_AfterIntro
	end
EventScript_Fashionista_IntroAquaM::
	msgbox EventScript_Fashionista_Text_AquaM_Intro, MSGBOX_DEFAULT
	goto EventScript_Fashionista_AfterIntro
	end
EventScript_Fashionista_IntroAquaF::
	msgbox EventScript_Fashionista_Text_AquaF_Intro, MSGBOX_DEFAULT
	goto EventScript_Fashionista_AfterIntro
	end

@ === After intro: process passive unlocks first ===
EventScript_Fashionista_AfterIntro::
	special Fashionista_CountPassiveUnlocks
	goto_if_eq VAR_RESULT, 0, EventScript_Fashionista_CheckShow
	goto_if_eq VAR_0x8006, 0, EventScript_Fashionista_AnyIdeas_MagmaM
	goto_if_eq VAR_0x8006, 1, EventScript_Fashionista_AnyIdeas_MagmaF
	goto_if_eq VAR_0x8006, 2, EventScript_Fashionista_AnyIdeas_AquaM
	goto_if_eq VAR_0x8006, 3, EventScript_Fashionista_AnyIdeas_AquaF
	end

EventScript_Fashionista_AnyIdeas_MagmaM::
	msgbox EventScript_Fashionista_Text_MagmaM_AnyIdeas, MSGBOX_DEFAULT
	goto EventScript_Fashionista_PassiveLoop
	end
EventScript_Fashionista_AnyIdeas_MagmaF::
	msgbox EventScript_Fashionista_Text_MagmaF_AnyIdeas, MSGBOX_DEFAULT
	goto EventScript_Fashionista_PassiveLoop
	end
EventScript_Fashionista_AnyIdeas_AquaM::
	msgbox EventScript_Fashionista_Text_AquaM_AnyIdeas, MSGBOX_DEFAULT
	goto EventScript_Fashionista_PassiveLoop
	end
EventScript_Fashionista_AnyIdeas_AquaF::
	msgbox EventScript_Fashionista_Text_AquaF_AnyIdeas, MSGBOX_DEFAULT
	goto EventScript_Fashionista_PassiveLoop
	end

EventScript_Fashionista_PassiveLoop::
	special Fashionista_GetNextPassiveUnlock
	goto_if_eq VAR_RESULT, 0xFF, EventScript_Fashionista_CheckShow
	copyvar VAR_0x8005, VAR_RESULT
	msgbox EventScript_Fashionista_Text_DynamicMsg, MSGBOX_DEFAULT
	special Fashionista_UnlockOutfit
	playfanfare MUS_OBTAIN_ITEM
	goto_if_eq VAR_0x8006, 0, EventScript_Fashionista_GotOutfit_P_MagmaM
	goto_if_eq VAR_0x8006, 1, EventScript_Fashionista_GotOutfit_P_MagmaF
	goto_if_eq VAR_0x8006, 2, EventScript_Fashionista_GotOutfit_P_AquaM
	goto_if_eq VAR_0x8006, 3, EventScript_Fashionista_GotOutfit_P_AquaF
	end
EventScript_Fashionista_GotOutfit_P_MagmaM::
	msgbox EventScript_Fashionista_Text_MagmaM_GotOutfit, MSGBOX_DEFAULT
	waitfanfare
	goto EventScript_Fashionista_PassiveLoop
	end
EventScript_Fashionista_GotOutfit_P_MagmaF::
	msgbox EventScript_Fashionista_Text_MagmaF_GotOutfit, MSGBOX_DEFAULT
	waitfanfare
	goto EventScript_Fashionista_PassiveLoop
	end
EventScript_Fashionista_GotOutfit_P_AquaM::
	msgbox EventScript_Fashionista_Text_AquaM_GotOutfit, MSGBOX_DEFAULT
	waitfanfare
	goto EventScript_Fashionista_PassiveLoop
	end
EventScript_Fashionista_GotOutfit_P_AquaF::
	msgbox EventScript_Fashionista_Text_AquaF_GotOutfit, MSGBOX_DEFAULT
	waitfanfare
	goto EventScript_Fashionista_PassiveLoop
	end

@ === Check if party has showable Pokemon ===
EventScript_Fashionista_CheckShow::
	special Fashionista_HasShowablePokemon
	goto_if_eq VAR_RESULT, 0, EventScript_Fashionista_NoNew
	goto_if_eq VAR_0x8006, 0, EventScript_Fashionista_ShowMe_MagmaM
	goto_if_eq VAR_0x8006, 1, EventScript_Fashionista_ShowMe_MagmaF
	goto_if_eq VAR_0x8006, 2, EventScript_Fashionista_ShowMe_AquaM
	goto_if_eq VAR_0x8006, 3, EventScript_Fashionista_ShowMe_AquaF
	end

EventScript_Fashionista_ShowMe_MagmaM::
	msgbox EventScript_Fashionista_Text_MagmaM_ShowMe, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, EventScript_Fashionista_NoNew
	goto EventScript_Fashionista_ShowPokemon
	end
EventScript_Fashionista_ShowMe_MagmaF::
	msgbox EventScript_Fashionista_Text_MagmaF_ShowMe, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, EventScript_Fashionista_NoNew
	goto EventScript_Fashionista_ShowPokemon
	end
EventScript_Fashionista_ShowMe_AquaM::
	msgbox EventScript_Fashionista_Text_AquaM_ShowMe, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, EventScript_Fashionista_NoNew
	goto EventScript_Fashionista_ShowPokemon
	end
EventScript_Fashionista_ShowMe_AquaF::
	msgbox EventScript_Fashionista_Text_AquaF_ShowMe, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, EventScript_Fashionista_NoNew
	goto EventScript_Fashionista_ShowPokemon
	end

EventScript_Fashionista_ShowPokemon::
	special ChoosePartyMon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, EventScript_Fashionista_Cancelled
	special Fashionista_CheckShownPokemon
	goto_if_eq VAR_RESULT, 0xFF, EventScript_Fashionista_NoMatch
	goto_if_eq VAR_RESULT, 0xFE, EventScript_Fashionista_Partial
	@ Full match - unlock!
	copyvar VAR_0x8005, VAR_RESULT
	msgbox EventScript_Fashionista_Text_DynamicMsg, MSGBOX_DEFAULT
	special Fashionista_UnlockOutfit
	playfanfare MUS_OBTAIN_ITEM
	goto_if_eq VAR_0x8006, 0, EventScript_Fashionista_GotOutfit_S_MagmaM
	goto_if_eq VAR_0x8006, 1, EventScript_Fashionista_GotOutfit_S_MagmaF
	goto_if_eq VAR_0x8006, 2, EventScript_Fashionista_GotOutfit_S_AquaM
	goto_if_eq VAR_0x8006, 3, EventScript_Fashionista_GotOutfit_S_AquaF
	end
EventScript_Fashionista_GotOutfit_S_MagmaM::
	msgbox EventScript_Fashionista_Text_MagmaM_GotOutfit, MSGBOX_DEFAULT
	waitfanfare
	goto EventScript_Fashionista_AfterShowUnlock
	end
EventScript_Fashionista_GotOutfit_S_MagmaF::
	msgbox EventScript_Fashionista_Text_MagmaF_GotOutfit, MSGBOX_DEFAULT
	waitfanfare
	goto EventScript_Fashionista_AfterShowUnlock
	end
EventScript_Fashionista_GotOutfit_S_AquaM::
	msgbox EventScript_Fashionista_Text_AquaM_GotOutfit, MSGBOX_DEFAULT
	waitfanfare
	goto EventScript_Fashionista_AfterShowUnlock
	end
EventScript_Fashionista_GotOutfit_S_AquaF::
	msgbox EventScript_Fashionista_Text_AquaF_GotOutfit, MSGBOX_DEFAULT
	waitfanfare
	goto EventScript_Fashionista_AfterShowUnlock
	end

EventScript_Fashionista_AfterShowUnlock::
	special Fashionista_HasShowablePokemon
	goto_if_ne VAR_RESULT, 0, EventScript_Fashionista_ShowAnother
	goto EventScript_Fashionista_Done
	end

EventScript_Fashionista_ShowAnother::
	goto_if_eq VAR_0x8006, 0, EventScript_Fashionista_ShowMore_MagmaM
	goto_if_eq VAR_0x8006, 1, EventScript_Fashionista_ShowMore_MagmaF
	goto_if_eq VAR_0x8006, 2, EventScript_Fashionista_ShowMore_AquaM
	goto_if_eq VAR_0x8006, 3, EventScript_Fashionista_ShowMore_AquaF
	end
EventScript_Fashionista_ShowMore_MagmaM::
	msgbox EventScript_Fashionista_Text_MagmaM_ShowMore, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, EventScript_Fashionista_Done
	goto EventScript_Fashionista_ShowPokemon
	end
EventScript_Fashionista_ShowMore_MagmaF::
	msgbox EventScript_Fashionista_Text_MagmaF_ShowMore, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, EventScript_Fashionista_Done
	goto EventScript_Fashionista_ShowPokemon
	end
EventScript_Fashionista_ShowMore_AquaM::
	msgbox EventScript_Fashionista_Text_AquaM_ShowMore, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, EventScript_Fashionista_Done
	goto EventScript_Fashionista_ShowPokemon
	end
EventScript_Fashionista_ShowMore_AquaF::
	msgbox EventScript_Fashionista_Text_AquaF_ShowMore, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, 0, EventScript_Fashionista_Done
	goto EventScript_Fashionista_ShowPokemon
	end

EventScript_Fashionista_Partial::
	msgbox EventScript_Fashionista_Text_DynamicMsg, MSGBOX_DEFAULT
	goto EventScript_Fashionista_Done
	end

EventScript_Fashionista_NoMatch::
	goto_if_eq VAR_0x8006, 0, EventScript_Fashionista_NoMatch_MagmaM
	goto_if_eq VAR_0x8006, 1, EventScript_Fashionista_NoMatch_MagmaF
	goto_if_eq VAR_0x8006, 2, EventScript_Fashionista_NoMatch_AquaM
	goto_if_eq VAR_0x8006, 3, EventScript_Fashionista_NoMatch_AquaF
	end

EventScript_Fashionista_NoMatch_MagmaM::
	msgbox EventScript_Fashionista_Text_MagmaM_NoMatch, MSGBOX_DEFAULT
	goto EventScript_Fashionista_Done
	end
EventScript_Fashionista_NoMatch_MagmaF::
	msgbox EventScript_Fashionista_Text_MagmaF_NoMatch, MSGBOX_DEFAULT
	goto EventScript_Fashionista_Done
	end
EventScript_Fashionista_NoMatch_AquaM::
	msgbox EventScript_Fashionista_Text_AquaM_NoMatch, MSGBOX_DEFAULT
	goto EventScript_Fashionista_Done
	end
EventScript_Fashionista_NoMatch_AquaF::
	msgbox EventScript_Fashionista_Text_AquaF_NoMatch, MSGBOX_DEFAULT
	goto EventScript_Fashionista_Done
	end

EventScript_Fashionista_Cancelled::
	goto_if_eq VAR_0x8006, 0, EventScript_Fashionista_Cancel_MagmaM
	goto_if_eq VAR_0x8006, 1, EventScript_Fashionista_Cancel_MagmaF
	goto_if_eq VAR_0x8006, 2, EventScript_Fashionista_Cancel_AquaM
	goto_if_eq VAR_0x8006, 3, EventScript_Fashionista_Cancel_AquaF
	end
EventScript_Fashionista_Cancel_MagmaM::
	msgbox EventScript_Fashionista_Text_MagmaM_MaybeNext, MSGBOX_DEFAULT
	goto EventScript_Fashionista_Done
	end
EventScript_Fashionista_Cancel_MagmaF::
	msgbox EventScript_Fashionista_Text_MagmaF_MaybeNext, MSGBOX_DEFAULT
	goto EventScript_Fashionista_Done
	end
EventScript_Fashionista_Cancel_AquaM::
	msgbox EventScript_Fashionista_Text_AquaM_MaybeNext, MSGBOX_DEFAULT
	goto EventScript_Fashionista_Done
	end
EventScript_Fashionista_Cancel_AquaF::
	msgbox EventScript_Fashionista_Text_AquaF_MaybeNext, MSGBOX_DEFAULT
	goto EventScript_Fashionista_Done
	end

EventScript_Fashionista_NoNew::
	goto_if_eq VAR_0x8006, 0, EventScript_Fashionista_NoNew_MagmaM
	goto_if_eq VAR_0x8006, 1, EventScript_Fashionista_NoNew_MagmaF
	goto_if_eq VAR_0x8006, 2, EventScript_Fashionista_NoNew_AquaM
	goto_if_eq VAR_0x8006, 3, EventScript_Fashionista_NoNew_AquaF
	end

EventScript_Fashionista_NoNew_MagmaM::
	msgbox EventScript_Fashionista_Text_MagmaM_NoNew, MSGBOX_DEFAULT
	special Fashionista_GetTip
	msgbox EventScript_Fashionista_Text_MagmaM_TipWrapper, MSGBOX_DEFAULT
	release
	end
EventScript_Fashionista_NoNew_MagmaF::
	msgbox EventScript_Fashionista_Text_MagmaF_NoNew, MSGBOX_DEFAULT
	special Fashionista_GetTip
	msgbox EventScript_Fashionista_Text_MagmaF_TipWrapper, MSGBOX_DEFAULT
	release
	end
EventScript_Fashionista_NoNew_AquaM::
	msgbox EventScript_Fashionista_Text_AquaM_NoNew, MSGBOX_DEFAULT
	special Fashionista_GetTip
	msgbox EventScript_Fashionista_Text_AquaM_TipWrapper, MSGBOX_DEFAULT
	release
	end
EventScript_Fashionista_NoNew_AquaF::
	msgbox EventScript_Fashionista_Text_AquaF_NoNew, MSGBOX_DEFAULT
	special Fashionista_GetTip
	msgbox EventScript_Fashionista_Text_AquaF_TipWrapper, MSGBOX_DEFAULT
	release
	end

EventScript_Fashionista_Done::
	goto_if_eq VAR_0x8006, 0, EventScript_Fashionista_Done_MagmaM
	goto_if_eq VAR_0x8006, 1, EventScript_Fashionista_Done_MagmaF
	goto_if_eq VAR_0x8006, 2, EventScript_Fashionista_Done_AquaM
	goto_if_eq VAR_0x8006, 3, EventScript_Fashionista_Done_AquaF
	end

EventScript_Fashionista_Done_MagmaM::
	msgbox EventScript_Fashionista_Text_MagmaM_ComeBack, MSGBOX_DEFAULT
	release
	end
EventScript_Fashionista_Done_MagmaF::
	msgbox EventScript_Fashionista_Text_MagmaF_ComeBack, MSGBOX_DEFAULT
	release
	end
EventScript_Fashionista_Done_AquaM::
	msgbox EventScript_Fashionista_Text_AquaM_ComeBack, MSGBOX_DEFAULT
	release
	end
EventScript_Fashionista_Done_AquaF::
	msgbox EventScript_Fashionista_Text_AquaF_ComeBack, MSGBOX_DEFAULT
	release
	end

@ ==========================================
@ TEXT - Shared (kept generic)
@ ==========================================

@ GotOutfit per NPC
EventScript_Fashionista_Text_MagmaM_GotOutfit:
	.string "The {STR_VAR_1} design is complete.\p"
	.string "Access any PC to equip it.$"
EventScript_Fashionista_Text_MagmaF_GotOutfit:
	.string "Yay! The {STR_VAR_1} outfit is all\n"
	.string "yours! Tee-hee!\p"
	.string "You can try it on at any PC!$"
EventScript_Fashionista_Text_AquaM_GotOutfit:
	.string "Yo, the {STR_VAR_1} look is locked\n"
	.string "in!\p"
	.string "Hit up any PC to rock it!$"
EventScript_Fashionista_Text_AquaF_GotOutfit:
	.string "U-um... the {STR_VAR_1} outfit is\n"
	.string "ready...!\p"
	.string "Y-you can try it on at any\n"
	.string "PC...!$"

@ ShowMore per NPC
EventScript_Fashionista_Text_MagmaM_ShowMore:
	.string "Do you have anything else\n"
	.string "worth showing?$"
EventScript_Fashionista_Text_MagmaF_ShowMore:
	.string "Ooh, got anything else?\n"
	.string "Show me, show me!$"
EventScript_Fashionista_Text_AquaM_ShowMore:
	.string "Got more heat to show me?$"
EventScript_Fashionista_Text_AquaF_ShowMore:
	.string "D-do you have anything else\n"
	.string "to show me...?$"

@ MaybeNext per NPC
EventScript_Fashionista_Text_MagmaM_MaybeNext:
	.string "Very well. Another time, then.$"
EventScript_Fashionista_Text_MagmaF_MaybeNext:
	.string "Aww, okay! Next time then!$"
EventScript_Fashionista_Text_AquaM_MaybeNext:
	.string "No worries, catch you later.$"
EventScript_Fashionista_Text_AquaF_MaybeNext:
	.string "O-okay... maybe next time...$"

@ Wrapper for dynamic text in gStringVar2
EventScript_Fashionista_Text_DynamicMsg:
	.string "{STR_VAR_2}$"

@ Tip wrappers per personality
EventScript_Fashionista_Text_MagmaM_TipWrapper:
	.string "Hmm... {STR_VAR_1}$"
EventScript_Fashionista_Text_MagmaF_TipWrapper:
	.string "Ooh! {STR_VAR_1}$"
EventScript_Fashionista_Text_AquaM_TipWrapper:
	.string "Yo, check it... {STR_VAR_1}$"
EventScript_Fashionista_Text_AquaF_TipWrapper:
	.string "W-well... {STR_VAR_1}$"

@ ==========================================
@ TEXT - Magma Male (Stoic perfectionist)
@ ==========================================

EventScript_Fashionista_Text_MagmaM_Intro:
	.string "I am TEAM MAGMA's head designer.\p"
	.string "Every stitch in our uniform was\n"
	.string "calculated for precision.\p"
	.string "I seek only the finest\n"
	.string "inspiration for my craft.$"

EventScript_Fashionista_Text_MagmaM_AnyIdeas:
	.string "Hold on. I'm picking up on\n"
	.string "something from you...$"

EventScript_Fashionista_Text_MagmaM_ShowMe:
	.string "There's something about your\n"
	.string "team that interests me.\p"
	.string "Show me a POKéMON.$"

EventScript_Fashionista_Text_MagmaM_NoMatch:
	.string "Adequate, but it doesn't meet\n"
	.string "my standards for inspiration.$"

EventScript_Fashionista_Text_MagmaM_NoNew:
	.string "Do you know anything that might\n"
	.string "meet my exacting standards?$"

EventScript_Fashionista_Text_MagmaM_ComeBack:
	.string "Your contributions are noted.\p"
	.string "Return when you have something\n"
	.string "worthy of my attention.$"

@ ==========================================
@ TEXT - Magma Female (Cutie, flirty, bubbly)
@ ==========================================

EventScript_Fashionista_Text_MagmaF_Intro:
	.string "Hiii! I'm TEAM MAGMA's cutest\n"
	.string "designer! Tee-hee!\p"
	.string "I made all the little details on\n"
	.string "our uniforms! Cute, right?\p"
	.string "I'm always looking for something\n"
	.string "adorable to inspire me!$"

EventScript_Fashionista_Text_MagmaF_AnyIdeas:
	.string "Ooh! Wait wait wait!\n"
	.string "I'm getting ideas just looking\lat you!$"

EventScript_Fashionista_Text_MagmaF_ShowMe:
	.string "I can tell you have something\n"
	.string "special with you!\p"
	.string "Pretty please, show me one of\n"
	.string "your POKéMON?$"

EventScript_Fashionista_Text_MagmaF_NoMatch:
	.string "Aww, it's super cute! But it\n"
	.string "doesn't quite give me that spark...$"

EventScript_Fashionista_Text_MagmaF_NoNew:
	.string "Heard any fun gossip lately?\n"
	.string "Anything juicy works for me!$"

EventScript_Fashionista_Text_MagmaF_ComeBack:
	.string "Thank you sooo much!\p"
	.string "Come see me again, okay?\n"
	.string "I'll miss you!$"

@ ==========================================
@ TEXT - Aqua Male (Informal, graffiti artist)
@ ==========================================

EventScript_Fashionista_Text_AquaM_Intro:
	.string "Yo! Name's the style guy for\n"
	.string "TEAM AQUA.\p"
	.string "I tagged our whole uniform\n"
	.string "design. Pretty sick, right?\p"
	.string "Always scoutin' for fresh\n"
	.string "ideas, ya feel me?$"

EventScript_Fashionista_Text_AquaM_AnyIdeas:
	.string "Yo hold up! I'm vibin' with\n"
	.string "somethin' here...$"

EventScript_Fashionista_Text_AquaM_ShowMe:
	.string "Ay, I can tell you got somethin'\n"
	.string "fire on your team.\p"
	.string "Lemme see one of your POKéMON,\n"
	.string "yeah?$"

EventScript_Fashionista_Text_AquaM_NoMatch:
	.string "Nah, that ain't it, fam.\n"
	.string "Cool POKéMON though, no cap.$"

EventScript_Fashionista_Text_AquaM_NoNew:
	.string "Yo, know anything dope that\n"
	.string "might get the gears turnin'?$"

EventScript_Fashionista_Text_AquaM_ComeBack:
	.string "Aight, respect! You brought the\n"
	.string "heat today.\p"
	.string "Swing by anytime, yeah?$"

@ ==========================================
@ TEXT - Aqua Female (Dreamy, poetic)
@ ==========================================

EventScript_Fashionista_Text_AquaF_Intro:
	.string "O-oh! H-hello...\p"
	.string "I'm... um... I design things\n"
	.string "for TEAM AQUA...\p"
	.string "I made the little patterns on\n"
	.string "our uniforms... D-do you like\lthem?\p"
	.string "I'm always trying to come up\n"
	.string "with new ideas... but it's hard...$"

EventScript_Fashionista_Text_AquaF_AnyIdeas:
	.string "W-wait! I just... I think I'm\n"
	.string "getting an idea from you...!$"

EventScript_Fashionista_Text_AquaF_ShowMe:
	.string "Um... I think you might have\n"
	.string "something really cool...\p"
	.string "C-could you maybe show me one\n"
	.string "of your POKéMON...?$"

EventScript_Fashionista_Text_AquaF_NoMatch:
	.string "Oh, it's r-really cute...!\n"
	.string "But um... it's not quite what\lI was looking for... S-sorry...$"

EventScript_Fashionista_Text_AquaF_NoNew:
	.string "Um... d-do you know anything\n"
	.string "that might help me get ideas...?$"

EventScript_Fashionista_Text_AquaF_ComeBack:
	.string "Th-thank you so much...!\n"
	.string "That really helped...\p"
	.string "Please come back... I mean,\n"
	.string "only if you want to...!$"
