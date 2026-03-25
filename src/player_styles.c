#include "global.h"
#include "player_styles.h"
#include "constants/flags.h"
#include "constants/game_stat.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "global.fieldmap.h"
#include "gpu_regs.h"
#include "item.h"
#include "list_menu.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "player_pc.h"
#include "pokedex.h"
#include "pokemon.h"
#include "rtc.h"
#include "script.h"
#include "script_menu.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "window.h"
#include "international_string_util.h"

#define PAL_TAG_BRENDAN    0x1100
#define PAL_TAG_MAY        0x1110
#define PAL_TAG_RS_BRENDAN 0x1122
#define PAL_TAG_RS_MAY     0x1123

#define STYLE_MENU_MAX_VISIBLE 8

#define COND_CONTEST_WON   (1 << 0)
#define COND_SHINY_CAUGHT  (1 << 1)
#define COND_HOF_TORCHIC   (1 << 2)
#define COND_HOF_MUDKIP    (1 << 3)
#define COND_HOF_TREECKO   (1 << 4)

#include "data/player_styles.h"

// Style indices (must match order in generate_style_data.py)
enum {
    STYLE_BRAZIL, STYLE_CHIKORITA, STYLE_CYNDAQUIL, STYLE_DARK,
    STYLE_DIVER, STYLE_ENIGMA, STYLE_FABULOUS, STYLE_FOREST,
    STYLE_GROUDON, STYLE_HISTORIC, STYLE_HOOH, STYLE_KYOGRE,
    STYLE_LUGIA, STYLE_MAGMA, STYLE_MASTER, STYLE_MUDKIP,
    STYLE_OCEAN, STYLE_OLD, STYLE_RED, STYLE_REDMOON,
    STYLE_ROYAL, STYLE_SAKURA, STYLE_AQUA, STYLE_SILVER,
    STYLE_TORCHIC, STYLE_TOTODILE, STYLE_TREECKO,
};

static const u16 sStyleUnlockFlags[PLAYER_STYLE_COUNT] = {
    FLAG_STYLE_UNLOCKED_BRAZIL, FLAG_STYLE_UNLOCKED_CHIKORITA,
    FLAG_STYLE_UNLOCKED_CYNDAQUIL, FLAG_STYLE_UNLOCKED_DARK,
    FLAG_STYLE_UNLOCKED_DIVER, FLAG_STYLE_UNLOCKED_ENIGMA,
    FLAG_STYLE_UNLOCKED_FABULOUS, FLAG_STYLE_UNLOCKED_FOREST,
    FLAG_STYLE_UNLOCKED_GROUDON, FLAG_STYLE_UNLOCKED_HISTORIC,
    FLAG_STYLE_UNLOCKED_HOOH, FLAG_STYLE_UNLOCKED_KYOGRE,
    FLAG_STYLE_UNLOCKED_LUGIA, FLAG_STYLE_UNLOCKED_MAGMA,
    FLAG_STYLE_UNLOCKED_MASTER, FLAG_STYLE_UNLOCKED_MUDKIP,
    FLAG_STYLE_UNLOCKED_OCEAN, FLAG_STYLE_UNLOCKED_OLD,
    FLAG_STYLE_UNLOCKED_RED, FLAG_STYLE_UNLOCKED_REDMOON,
    FLAG_STYLE_UNLOCKED_ROYAL, FLAG_STYLE_UNLOCKED_SAKURA,
    FLAG_STYLE_UNLOCKED_AQUA, FLAG_STYLE_UNLOCKED_SILVER,
    FLAG_STYLE_UNLOCKED_TORCHIC, FLAG_STYLE_UNLOCKED_TOTODILE,
    FLAG_STYLE_UNLOCKED_TREECKO,
};

// ============================================================
// Unlock dialog strings
// ============================================================
static const u8 sUnlock_Brazil[] = _("A TROPIUS! That reminds me of a\ntrip I went on to this wonderful\lplace!\pThis is perfect! How about\nthis... Here you go!");
static const u8 sUnlock_Chikorita[] = _("A MEGANIUM! That gentle green\ninspires a leafy design!");
static const u8 sUnlock_Cyndaquil[] = _("A TYPHLOSION! That fiery spirit\ninspires a bold new look!");
static const u8 sUnlock_Dark[] = _("You lost a link battle? Even in\ndefeat, there is fierce style!");
static const u8 sUnlock_Diver[] = _("All those deep-sea POKéMON!\nYou're a true diver!");
static const u8 sUnlock_Enigma[] = _("You caught an UNOWN! Those\nmysterious symbols inspire me!");
static const u8 sUnlock_Fabulous[] = _("You won a CONTEST! Style and\nfashion go hand in hand!");
static const u8 sUnlock_Forest[] = _("An ENIGMA BERRY! Such a rare\nfind inspires a natural look!");
static const u8 sUnlock_Groudon[] = _("The continent creator itself!\nThe power of the land inspires me!");
static const u8 sUnlock_Historic[] = _("The three ancient golems! Such\ntimeless power inspires me!");
static const u8 sUnlock_HoOh[] = _("The sacred rainbow bird! Its\nradiance inspires a brilliant outfit!");
static const u8 sUnlock_Kyogre[] = _("The ocean's sovereign! The deep\nblue of the seas inspires me!");
static const u8 sUnlock_Lugia[] = _("The silver guardian of the seas!\nSuch grace and elegance!");
static const u8 sUnlock_Magma[] = _("CAMERUPT! Our leader's favorite!\nThis gives me a great idea!");
static const u8 sUnlock_Master[] = _("What? You defeated him? And a\nShadow Monster afterwards?\pThat's a very inspiring story!\nI know what my new design will be!");
static const u8 sUnlock_Mudkip[] = _("A loyal champion with a loyal\nstarter! That loyalty inspires me!");
static const u8 sUnlock_Ocean[] = _("That PACIFIDLOG trade! The spirit\nof exchange inspires me!");
static const u8 sUnlock_Old[] = _("The ultimate created POKéMON!\nSuch ancient power. How classic!");
static const u8 sUnlock_Red[] = _("The three KANTO starters! A\ntrue collector's spirit!");
static const u8 sUnlock_Redmoon[] = _("All those dark, mysterious\nPOKéMON... A crimson moon rises!");
static const u8 sUnlock_Royal[] = _("A SHINY POKéMON! Such royal\nluck deserves a regal outfit!");
static const u8 sUnlock_Sakura[] = _("All those baby POKéMON!\nHow adorable! So delicate!");
static const u8 sUnlock_Aqua[] = _("SHARPEDO! Our leader's favorite!\nIts deep blue inspires me!");
static const u8 sUnlock_Silver[] = _("The three JOHTO starters!\nA true adventurer!");
static const u8 sUnlock_Torchic[] = _("A loyal champion with a loyal\nstarter! That burning passion!");
static const u8 sUnlock_Totodile[] = _("A FERALIGATR! Those powerful\njaws inspire a cool look!");
static const u8 sUnlock_Treecko[] = _("A loyal champion with a loyal\nstarter! That calm determination!");

static const u8 *const sUnlockMessages[PLAYER_STYLE_COUNT] = {
    sUnlock_Brazil, sUnlock_Chikorita, sUnlock_Cyndaquil, sUnlock_Dark,
    sUnlock_Diver, sUnlock_Enigma, sUnlock_Fabulous, sUnlock_Forest,
    sUnlock_Groudon, sUnlock_Historic, sUnlock_HoOh, sUnlock_Kyogre,
    sUnlock_Lugia, sUnlock_Magma, sUnlock_Master, sUnlock_Mudkip,
    sUnlock_Ocean, sUnlock_Old, sUnlock_Red, sUnlock_Redmoon,
    sUnlock_Royal, sUnlock_Sakura, sUnlock_Aqua, sUnlock_Silver,
    sUnlock_Torchic, sUnlock_Totodile, sUnlock_Treecko,
};

// ============================================================
// Tip strings
// ============================================================
static const u8 sTip_Brazil[] = _("I hear TROPIUS has a wonderful\ntropical look...");
static const u8 sTip_Chikorita[] = _("I would love to see a trainer with\nthe JOHTO starters at their full\lpotential one day...");
static const u8 sTip_Cyndaquil[] = _("I would love to see a trainer with\nthe JOHTO starters at their full\lpotential one day...");
static const u8 sTip_Dark[] = _("Even losing a LINK BATTLE can\nteach you something about style...");
static const u8 sTip_Diver[] = _("Deep-sea POKéMON like those found\nwhile diving are fascinating...");
static const u8 sTip_Enigma[] = _("Those mysterious ruin symbols\nmight hold fashion secrets...");
static const u8 sTip_Fabulous[] = _("Winning a POKéMON CONTEST\ncould spark new ideas...");
static const u8 sTip_Forest[] = _("The rare ENIGMA BERRY is said\nto inspire creativity...");
static const u8 sTip_Groudon[] = _("MAXIE said a creature that created\nthe continents sleeps underground...");
static const u8 sTip_Historic[] = _("They say three ancient golems\nsleep in hidden chambers...");
static const u8 sTip_HoOh[] = _("A bird said to create rainbows\nwherever it flies...");
static const u8 sTip_Kyogre[] = _("ARCHIE said a creature that created\nthe oceans sleeps in a cave...");
static const u8 sTip_Lugia[] = _("A silver guardian said to dwell\nin the depths of the sea...");
static const u8 sTip_Magma[] = _("Our leader's favorite POKéMON\nis so inspiring!");
static const u8 sTip_Master[] = _("I hear there's a very strong\ntrainer on SOUTHERN ISLAND.\pOur leader told us to stay far\nfrom him. He must be inspiring!");
static const u8 sTip_Mudkip[] = _("Becoming CHAMPION with their\nloyal starter would be inspiring...");
static const u8 sTip_Ocean[] = _("They say PACIFIDLOG TOWN has\na special trade opportunity...");
static const u8 sTip_Old[] = _("They say someone created the\nultimate POKéMON long ago...");
static const u8 sTip_Red[] = _("The KANTO starters CHARMANDER,\nBULBASAUR, SQUIRTLE are classic!");
static const u8 sTip_Redmoon[] = _("Dark POKéMON like LUNATONE,\nMURKROW, and HOUNDOOM have\la certain allure...");
static const u8 sTip_Royal[] = _("Catching a SHINY POKéMON\nwould be truly royal...");
static const u8 sTip_Sakura[] = _("Baby POKéMON like AZURILL and\nWYNAUT are so adorable...");
static const u8 sTip_Aqua[] = _("Our leader's favorite POKéMON\nis so inspiring!");
static const u8 sTip_Silver[] = _("The JOHTO starters TOTODILE,\nCYNDAQUIL, CHIKORITA are cool!");
static const u8 sTip_Torchic[] = _("Becoming CHAMPION with their\nloyal starter would be inspiring...");
static const u8 sTip_Totodile[] = _("I would love to see a trainer with\nthe JOHTO starters at their full\lpotential one day...");
static const u8 sTip_Treecko[] = _("Becoming CHAMPION with their\nloyal starter would be inspiring...");
static const u8 sText_AllUnlocked[] = _("You've unlocked every outfit!\nYou're the ultimate fashionista!");
static const u8 sText_PartialMulti[] = _("Oh, that's a great POKéMON!\nBut I need to see more like it.\pBring me more POKéMON like\nthat and I'll be truly inspired!");

static const u8 *const sTipMessages[PLAYER_STYLE_COUNT] = {
    sTip_Brazil, sTip_Chikorita, sTip_Cyndaquil, sTip_Dark,
    sTip_Diver, sTip_Enigma, sTip_Fabulous, sTip_Forest,
    sTip_Groudon, sTip_Historic, sTip_HoOh, sTip_Kyogre,
    sTip_Lugia, sTip_Magma, sTip_Master, sTip_Mudkip,
    sTip_Ocean, sTip_Old, sTip_Red, sTip_Redmoon,
    sTip_Royal, sTip_Sakura, sTip_Aqua, sTip_Silver,
    sTip_Torchic, sTip_Totodile, sTip_Treecko,
};

// ============================================================
// Dynamic menu items
// ============================================================
static EWRAM_DATA struct ListMenuItem sStyleMenuItemsBuf[PLAYER_STYLE_COUNT + 2] = {0};
static EWRAM_DATA u8 sStyleMenuCount = 0;
static EWRAM_DATA u8 sFashionistaNextCheck = 0;

static void StyleMenu_ProcessInput(u8 taskId);

// ============================================================
// Palette helpers
// ============================================================

static u16 GrayscaleColor555(u16 color)
{
    u8 r = color & 0x1F;
    u8 g = (color >> 5) & 0x1F;
    u8 b = (color >> 10) & 0x1F;
    u8 gray = (r + g + b) / 3;
    return gray | (gray << 5) | (gray << 10);
}

static void ApplyStyleOverlay(u16 *pal, const struct PlayerStyleData *style, bool8 isFemale, bool8 isOW)
{
    u8 i;
    if (isOW) pal[4] = style->clothesOutline;
    pal[5] = style->clothesLight;
    pal[6] = style->clothesMain;
    if (!isFemale) { pal[7] = style->clothesDark; pal[8] = style->clothesOutline; }
    else if (style->hairLight != 0xFFFF) { pal[7] = style->hairLight; pal[8] = style->hairDark; }
    pal[9] = style->whiteParts; pal[10] = style->bagMain; pal[11] = style->bagShadow;
    pal[12] = style->accentMain; pal[13] = style->accentDark; pal[14] = style->hatWhite;
    if (style->grayscaleSkin)
    {
        for (i = 1; i <= 4; i++) pal[i] = GrayscaleColor555(pal[i]);
        if (isFemale && style->hairLight == 0xFFFF) { pal[7] = GrayscaleColor555(pal[7]); pal[8] = GrayscaleColor555(pal[8]); }
    }
}

// ============================================================
// Public palette functions
// ============================================================

u16 GetPlayerStyle(void) { return VarGet(VAR_PLAYER_STYLE); }

void ApplyPlayerStyleToOWPalette(u8 paletteSlot, bool8 isFemale)
{
    u16 si = GetPlayerStyle();
    if (si == 0 || si > PLAYER_STYLE_COUNT) return;
    ApplyStyleOverlay(&gPlttBufferUnfaded[OBJ_PLTT_ID(paletteSlot)], &sPlayerStyles[si-1], isFemale, TRUE);
    CpuCopy16(&gPlttBufferUnfaded[OBJ_PLTT_ID(paletteSlot)], &gPlttBufferFaded[OBJ_PLTT_ID(paletteSlot)], PLTT_SIZE_4BPP);
}

void ApplyPlayerStyleToTrainerPalette(u16 off, bool8 isFemale)
{
    u16 si = GetPlayerStyle();
    if (si == 0 || si > PLAYER_STYLE_COUNT) return;
    ApplyStyleOverlay(&gPlttBufferUnfaded[off], &sPlayerStyles[si-1], isFemale, FALSE);
    CpuCopy16(&gPlttBufferUnfaded[off], &gPlttBufferFaded[off], PLTT_SIZE_4BPP);
}

void RefreshPlayerOWPalette(void)
{
    struct ObjectEvent *obj;
    u16 palTag;
    if (gPlayerAvatar.objectEventId == 0 && gObjectEvents[0].active == 0) return;
    obj = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (!obj->active) return;
    if (FlagGet(FLAG_PLAYER_STYLE_RS))
        palTag = (gSaveBlock2Ptr->playerGender != MALE) ? PAL_TAG_RS_MAY : PAL_TAG_RS_BRENDAN;
    else
        palTag = (gSaveBlock2Ptr->playerGender != MALE) ? PAL_TAG_MAY : PAL_TAG_BRENDAN;
    PatchObjectPalette(palTag, gSprites[obj->spriteId].oam.paletteNum);
}

void PlayerStyleOverworldUpdate(void)
{
    struct ObjectEvent *obj;
    u8 palSlot;
    u16 palBase;
    u16 si = GetPlayerStyle();

    if (si == 0 || si > PLAYER_STYLE_COUNT) return;
    if (gPlayerAvatar.objectEventId == 0 && gObjectEvents[0].active == 0) return;
    obj = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (!obj->active) return;

    palSlot = gSprites[obj->spriteId].oam.paletteNum;
    palBase = OBJ_PLTT_ID(palSlot);

    // Always keep unfaded styled (fade target)
    ApplyStyleOverlay(&gPlttBufferUnfaded[palBase],
        &sPlayerStyles[si-1], gSaveBlock2Ptr->playerGender != MALE, TRUE);

    // Also patch the displayed buffer when no fade is running
    // and the screen isn't blacked out (skin color != 0 means visible)
    if (!gPaletteFade.active && gPlttBufferFaded[palBase + 1] != 0)
        ApplyStyleOverlay(&gPlttBufferFaded[palBase],
            &sPlayerStyles[si-1], gSaveBlock2Ptr->playerGender != MALE, TRUE);
}

// ============================================================
// Pokedex helper (species -> caught check)
// ============================================================
static bool8 IsCaught(u16 species)
{
    return GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT);
}

// ============================================================
// Passive unlock conditions (no Pokemon showing needed)
// ============================================================
static bool8 IsPassiveConditionMet(u8 idx)
{
    u16 c = VarGet(VAR_OUTFIT_CONDITIONS);
    switch (idx)
    {
    case STYLE_DARK:     return GetGameStat(GAME_STAT_LINK_BATTLE_LOSSES) > 0;
    case STYLE_FABULOUS: return (c & COND_CONTEST_WON) != 0;
    case STYLE_FOREST:   return CheckBagHasItem(ITEM_ENIGMA_BERRY, 1);
    case STYLE_MASTER:   return FlagGet(FLAG_DEFEATED_BOSS_LUGIA);
    case STYLE_MUDKIP:   return (c & COND_HOF_MUDKIP) != 0;
    case STYLE_OCEAN:    return FlagGet(FLAG_PACIFIDLOG_NPC_TRADE_COMPLETED);
    case STYLE_TORCHIC:  return (c & COND_HOF_TORCHIC) != 0;
    case STYLE_TREECKO:  return (c & COND_HOF_TREECKO) != 0;
    default: return FALSE;
    }
}

static bool8 IsPassiveStyle(u8 idx)
{
    return idx == STYLE_DARK || idx == STYLE_FABULOUS || idx == STYLE_FOREST
        || idx == STYLE_MASTER || idx == STYLE_MUDKIP || idx == STYLE_OCEAN
        || idx == STYLE_TORCHIC || idx == STYLE_TREECKO;
}

// ============================================================
// Show-Pokemon condition checking
// ============================================================

// Check if a species is relevant to any locked show-based outfit
static bool8 IsSpeciesRelevantForOutfit(u16 species, bool8 isShiny, u8 npcTeam)
{
    bool8 isMagma = (npcTeam <= 1);
    u8 i;

    // Single-species outfits
    if (!FlagGet(sStyleUnlockFlags[STYLE_BRAZIL]) && species == SPECIES_TROPIUS) return TRUE;
    if (!FlagGet(sStyleUnlockFlags[STYLE_CHIKORITA]) && species == SPECIES_MEGANIUM) return TRUE;
    if (!FlagGet(sStyleUnlockFlags[STYLE_CYNDAQUIL]) && species == SPECIES_TYPHLOSION) return TRUE;
    if (!FlagGet(sStyleUnlockFlags[STYLE_ENIGMA]) && species == SPECIES_UNOWN) return TRUE;
    if (!FlagGet(sStyleUnlockFlags[STYLE_GROUDON]) && isMagma && species == SPECIES_GROUDON) return TRUE;
    if (!FlagGet(sStyleUnlockFlags[STYLE_HOOH]) && species == SPECIES_HO_OH) return TRUE;
    if (!FlagGet(sStyleUnlockFlags[STYLE_KYOGRE]) && !isMagma && species == SPECIES_KYOGRE) return TRUE;
    if (!FlagGet(sStyleUnlockFlags[STYLE_LUGIA]) && species == SPECIES_LUGIA) return TRUE;
    if (!FlagGet(sStyleUnlockFlags[STYLE_OLD]) && species == SPECIES_MEWTWO) return TRUE;
    if (!FlagGet(sStyleUnlockFlags[STYLE_TOTODILE]) && species == SPECIES_FERALIGATR) return TRUE;
    // Team-restricted
    if (!FlagGet(sStyleUnlockFlags[STYLE_MAGMA]) && isMagma && species == SPECIES_CAMERUPT) return TRUE;
    if (!FlagGet(sStyleUnlockFlags[STYLE_AQUA]) && !isMagma && species == SPECIES_SHARPEDO) return TRUE;
    // Multi-species
    if (!FlagGet(sStyleUnlockFlags[STYLE_DIVER]))
    { if (species == SPECIES_RELICANTH || species == SPECIES_CLAMPERL || species == SPECIES_CHINCHOU || species == SPECIES_WAILORD || species == SPECIES_KYOGRE) return TRUE; }
    if (!FlagGet(sStyleUnlockFlags[STYLE_RED]))
    { if (species == SPECIES_CHARMANDER || species == SPECIES_BULBASAUR || species == SPECIES_SQUIRTLE) return TRUE; }
    if (!FlagGet(sStyleUnlockFlags[STYLE_SILVER]))
    { if (species == SPECIES_TOTODILE || species == SPECIES_CYNDAQUIL || species == SPECIES_CHIKORITA) return TRUE; }
    if (!FlagGet(sStyleUnlockFlags[STYLE_REDMOON]))
    { if (species == SPECIES_LUNATONE || species == SPECIES_SNEASEL || species == SPECIES_MURKROW || species == SPECIES_HOUNDOOM) return TRUE; }
    if (!FlagGet(sStyleUnlockFlags[STYLE_SAKURA]))
    { if (species == SPECIES_AZURILL || species == SPECIES_WYNAUT || species == SPECIES_PICHU || species == SPECIES_IGGLYBUFF) return TRUE; }
    if (!FlagGet(sStyleUnlockFlags[STYLE_HISTORIC]))
    { if (species == SPECIES_REGIROCK || species == SPECIES_REGICE || species == SPECIES_REGISTEEL) return TRUE; }
    // Royal (shiny)
    if (!FlagGet(sStyleUnlockFlags[STYLE_ROYAL]) && isShiny) return TRUE;
    return FALSE;
}

// ============================================================
// Fashionista NPC specials
// ============================================================

void Fashionista_CountPassiveUnlocks(void)
{
    u8 i;
    u8 count = 0;
    for (i = 0; i < PLAYER_STYLE_COUNT; i++)
    {
        if (IsPassiveStyle(i) && !FlagGet(sStyleUnlockFlags[i]) && IsPassiveConditionMet(i))
            count++;
    }
    sFashionistaNextCheck = 0;
    gSpecialVar_Result = count;
}

void Fashionista_GetNextPassiveUnlock(void)
{
    u8 i;
    for (i = sFashionistaNextCheck; i < PLAYER_STYLE_COUNT; i++)
    {
        if (IsPassiveStyle(i) && !FlagGet(sStyleUnlockFlags[i]) && IsPassiveConditionMet(i))
        {
            sFashionistaNextCheck = i + 1;
            gSpecialVar_Result = i;
            StringCopy(gStringVar1, sStyleNames[i]);
            StringCopy(gStringVar2, sUnlockMessages[i]);
            return;
        }
    }
    gSpecialVar_Result = 0xFF;
}

void Fashionista_HasShowablePokemon(void)
{
    u8 i;
    u16 species;
    bool8 isShiny;
    u8 npcTeam = gSpecialVar_0x8006;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (!GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES))
            continue;
        species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
        isShiny = IsMonShiny(&gPlayerParty[i]);
        if (IsSpeciesRelevantForOutfit(species, isShiny, npcTeam))
        {
            gSpecialVar_Result = TRUE;
            return;
        }
    }
    gSpecialVar_Result = FALSE;
}

void Fashionista_CheckShownPokemon(void)
{
    u16 species;
    bool8 isShiny;
    u8 npcTeam = gSpecialVar_0x8006;
    bool8 isMagma = (npcTeam <= 1);

    species = GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_SPECIES, NULL);
    isShiny = IsMonShiny(&gPlayerParty[gSpecialVar_0x8004]);

    // Single species outfits
    #define TRY_SINGLE(style, sp) \
        if (!FlagGet(sStyleUnlockFlags[style]) && species == (sp)) { \
            gSpecialVar_Result = (style); \
            StringCopy(gStringVar1, sStyleNames[style]); \
            StringCopy(gStringVar2, sUnlockMessages[style]); \
            return; }

    TRY_SINGLE(STYLE_BRAZIL, SPECIES_TROPIUS)
    TRY_SINGLE(STYLE_CHIKORITA, SPECIES_MEGANIUM)
    TRY_SINGLE(STYLE_CYNDAQUIL, SPECIES_TYPHLOSION)
    TRY_SINGLE(STYLE_ENIGMA, SPECIES_UNOWN)
    TRY_SINGLE(STYLE_HOOH, SPECIES_HO_OH)
    TRY_SINGLE(STYLE_LUGIA, SPECIES_LUGIA)
    TRY_SINGLE(STYLE_OLD, SPECIES_MEWTWO)
    TRY_SINGLE(STYLE_TOTODILE, SPECIES_FERALIGATR)
    #undef TRY_SINGLE

    // Team-restricted (Magma NPC only)
    if (!FlagGet(sStyleUnlockFlags[STYLE_MAGMA]) && isMagma && species == SPECIES_CAMERUPT)
    { gSpecialVar_Result = STYLE_MAGMA; StringCopy(gStringVar1, sStyleNames[STYLE_MAGMA]); StringCopy(gStringVar2, sUnlockMessages[STYLE_MAGMA]); return; }
    if (!FlagGet(sStyleUnlockFlags[STYLE_GROUDON]) && isMagma && species == SPECIES_GROUDON)
    { gSpecialVar_Result = STYLE_GROUDON; StringCopy(gStringVar1, sStyleNames[STYLE_GROUDON]); StringCopy(gStringVar2, sUnlockMessages[STYLE_GROUDON]); return; }
    // Team-restricted (Aqua NPC only)
    if (!FlagGet(sStyleUnlockFlags[STYLE_AQUA]) && !isMagma && species == SPECIES_SHARPEDO)
    { gSpecialVar_Result = STYLE_AQUA; StringCopy(gStringVar1, sStyleNames[STYLE_AQUA]); StringCopy(gStringVar2, sUnlockMessages[STYLE_AQUA]); return; }
    if (!FlagGet(sStyleUnlockFlags[STYLE_KYOGRE]) && !isMagma && species == SPECIES_KYOGRE)
    { gSpecialVar_Result = STYLE_KYOGRE; StringCopy(gStringVar1, sStyleNames[STYLE_KYOGRE]); StringCopy(gStringVar2, sUnlockMessages[STYLE_KYOGRE]); return; }

    // Multi-species outfits (need all caught + show one)
    if (!FlagGet(sStyleUnlockFlags[STYLE_DIVER])
        && (species == SPECIES_RELICANTH || species == SPECIES_CLAMPERL || species == SPECIES_CHINCHOU || species == SPECIES_WAILORD || species == SPECIES_KYOGRE))
    {
        if (IsCaught(SPECIES_RELICANTH) && IsCaught(SPECIES_CLAMPERL) && IsCaught(SPECIES_CHINCHOU) && IsCaught(SPECIES_WAILORD) && IsCaught(SPECIES_KYOGRE))
        { gSpecialVar_Result = STYLE_DIVER; StringCopy(gStringVar1, sStyleNames[STYLE_DIVER]); StringCopy(gStringVar2, sUnlockMessages[STYLE_DIVER]); return; }
        else
        { gSpecialVar_Result = 0xFE; StringCopy(gStringVar2, sText_PartialMulti); return; }
    }

    if (!FlagGet(sStyleUnlockFlags[STYLE_RED])
        && (species == SPECIES_CHARMANDER || species == SPECIES_BULBASAUR || species == SPECIES_SQUIRTLE))
    {
        if (IsCaught(SPECIES_CHARMANDER) && IsCaught(SPECIES_BULBASAUR) && IsCaught(SPECIES_SQUIRTLE))
        { gSpecialVar_Result = STYLE_RED; StringCopy(gStringVar1, sStyleNames[STYLE_RED]); StringCopy(gStringVar2, sUnlockMessages[STYLE_RED]); return; }
        else
        { gSpecialVar_Result = 0xFE; StringCopy(gStringVar2, sText_PartialMulti); return; }
    }

    if (!FlagGet(sStyleUnlockFlags[STYLE_SILVER])
        && (species == SPECIES_TOTODILE || species == SPECIES_CYNDAQUIL || species == SPECIES_CHIKORITA))
    {
        if (IsCaught(SPECIES_TOTODILE) && IsCaught(SPECIES_CYNDAQUIL) && IsCaught(SPECIES_CHIKORITA))
        { gSpecialVar_Result = STYLE_SILVER; StringCopy(gStringVar1, sStyleNames[STYLE_SILVER]); StringCopy(gStringVar2, sUnlockMessages[STYLE_SILVER]); return; }
        else
        { gSpecialVar_Result = 0xFE; StringCopy(gStringVar2, sText_PartialMulti); return; }
    }

    if (!FlagGet(sStyleUnlockFlags[STYLE_REDMOON])
        && (species == SPECIES_LUNATONE || species == SPECIES_SNEASEL || species == SPECIES_MURKROW || species == SPECIES_HOUNDOOM))
    {
        if (IsCaught(SPECIES_LUNATONE) && IsCaught(SPECIES_SNEASEL) && IsCaught(SPECIES_MURKROW) && IsCaught(SPECIES_HOUNDOOM))
        { gSpecialVar_Result = STYLE_REDMOON; StringCopy(gStringVar1, sStyleNames[STYLE_REDMOON]); StringCopy(gStringVar2, sUnlockMessages[STYLE_REDMOON]); return; }
        else
        { gSpecialVar_Result = 0xFE; StringCopy(gStringVar2, sText_PartialMulti); return; }
    }

    if (!FlagGet(sStyleUnlockFlags[STYLE_SAKURA])
        && (species == SPECIES_AZURILL || species == SPECIES_WYNAUT || species == SPECIES_PICHU || species == SPECIES_IGGLYBUFF))
    {
        if (IsCaught(SPECIES_AZURILL) && IsCaught(SPECIES_WYNAUT) && IsCaught(SPECIES_PICHU) && IsCaught(SPECIES_IGGLYBUFF))
        { gSpecialVar_Result = STYLE_SAKURA; StringCopy(gStringVar1, sStyleNames[STYLE_SAKURA]); StringCopy(gStringVar2, sUnlockMessages[STYLE_SAKURA]); return; }
        else
        { gSpecialVar_Result = 0xFE; StringCopy(gStringVar2, sText_PartialMulti); return; }
    }

    if (!FlagGet(sStyleUnlockFlags[STYLE_HISTORIC])
        && (species == SPECIES_REGIROCK || species == SPECIES_REGICE || species == SPECIES_REGISTEEL))
    {
        if (IsCaught(SPECIES_REGIROCK) && IsCaught(SPECIES_REGICE) && IsCaught(SPECIES_REGISTEEL))
        { gSpecialVar_Result = STYLE_HISTORIC; StringCopy(gStringVar1, sStyleNames[STYLE_HISTORIC]); StringCopy(gStringVar2, sUnlockMessages[STYLE_HISTORIC]); return; }
        else
        { gSpecialVar_Result = 0xFE; StringCopy(gStringVar2, sText_PartialMulti); return; }
    }

    // Royal: any shiny
    if (!FlagGet(sStyleUnlockFlags[STYLE_ROYAL]) && isShiny)
    { gSpecialVar_Result = STYLE_ROYAL; StringCopy(gStringVar1, sStyleNames[STYLE_ROYAL]); StringCopy(gStringVar2, sUnlockMessages[STYLE_ROYAL]); return; }

    gSpecialVar_Result = 0xFF;
}

void Fashionista_UnlockOutfit(void)
{
    u8 idx = gSpecialVar_0x8005;
    if (idx < PLAYER_STYLE_COUNT)
        FlagSet(sStyleUnlockFlags[idx]);
}

void Fashionista_GetTip(void)
{
    u8 locked[PLAYER_STYLE_COUNT];
    u8 count = 0;
    u8 i;
    u8 npcTeam = gSpecialVar_0x8006;
    bool8 isMagma = (npcTeam <= 1);
    u16 dayPick;

    for (i = 0; i < PLAYER_STYLE_COUNT; i++)
    {
        if (FlagGet(sStyleUnlockFlags[i])) continue;
        // Skip team-restricted tips for wrong team
        if (i == STYLE_MAGMA && !isMagma) continue;
        if (i == STYLE_GROUDON && !isMagma) continue;
        if (i == STYLE_AQUA && isMagma) continue;
        if (i == STYLE_KYOGRE && isMagma) continue;
        locked[count++] = i;
    }
    if (count == 0) { StringCopy(gStringVar1, sText_AllUnlocked); return; }
    // One tip per day, each NPC offset by team ID so they differ
    dayPick = ((u16)gLocalTime.days + npcTeam) % count;
    StringCopy(gStringVar1, sTipMessages[locked[dayPick]]);
}

// ============================================================
// Hook functions (from other code)
// ============================================================

void MarkContestWonForOutfit(void)
{
    VarSet(VAR_OUTFIT_CONDITIONS, VarGet(VAR_OUTFIT_CONDITIONS) | COND_CONTEST_WON);
}

void MarkShinyCaughtForOutfit(void)
{
    VarSet(VAR_OUTFIT_CONDITIONS, VarGet(VAR_OUTFIT_CONDITIONS) | COND_SHINY_CAUGHT);
}

void CheckPartyForStarterUnlocks(void)
{
    u8 i;
    u16 species;
    u16 conds = VarGet(VAR_OUTFIT_CONDITIONS);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (!GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES)) continue;
        species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);
        if (species == SPECIES_TORCHIC || species == SPECIES_COMBUSKEN || species == SPECIES_BLAZIKEN) conds |= COND_HOF_TORCHIC;
        if (species == SPECIES_MUDKIP || species == SPECIES_MARSHTOMP || species == SPECIES_SWAMPERT) conds |= COND_HOF_MUDKIP;
        if (species == SPECIES_TREECKO || species == SPECIES_GROVYLE || species == SPECIES_SCEPTILE) conds |= COND_HOF_TREECKO;
    }
    VarSet(VAR_OUTFIT_CONDITIONS, conds);
}

// ============================================================
// Style selection menu (PC, only unlocked styles)
// ============================================================

#define tStyleWindowId data[4]
#define tStyleListId   data[5]

void ShowStyleMenu(u8 taskId)
{
    u32 pw;
    u8 w, ms, i;
    struct ListMenuTemplate mt;

    sStyleMenuCount = 0;
    sStyleMenuItemsBuf[sStyleMenuCount].name = sStyleName_Default;
    sStyleMenuItemsBuf[sStyleMenuCount].id = 0;
    sStyleMenuCount++;
    for (i = 0; i < PLAYER_STYLE_COUNT; i++)
    {
        if (FlagGet(sStyleUnlockFlags[i]))
        {
            sStyleMenuItemsBuf[sStyleMenuCount].name = sStyleNames[i];
            sStyleMenuItemsBuf[sStyleMenuCount].id = i + 1;
            sStyleMenuCount++;
        }
    }
    sStyleMenuItemsBuf[sStyleMenuCount].name = sStyleName_Cancel;
    sStyleMenuItemsBuf[sStyleMenuCount].id = STYLE_MENU_ID_CANCEL;
    sStyleMenuCount++;

    pw = 0;
    for (i = 0; i < sStyleMenuCount; i++)
        pw = DisplayTextAndGetWidth(sStyleMenuItemsBuf[i].name, pw);
    ms = (sStyleMenuCount < STYLE_MENU_MAX_VISIBLE) ? sStyleMenuCount : STYLE_MENU_MAX_VISIBLE;
    w = ConvertPixelWidthToTileWidth(pw);

    gTasks[taskId].tStyleWindowId = CreateWindowFromRect(0, 0, w, ms * 2);
    SetStandardWindowBorderStyle(gTasks[taskId].tStyleWindowId, FALSE);
    mt.items = sStyleMenuItemsBuf; mt.moveCursorFunc = ListMenuDefaultCursorMoveFunc;
    mt.itemPrintFunc = NULL; mt.totalItems = sStyleMenuCount; mt.maxShowed = ms;
    mt.windowId = gTasks[taskId].tStyleWindowId; mt.header_X = 0; mt.item_X = 8;
    mt.cursor_X = 0; mt.upText_Y = 1; mt.cursorPal = 2; mt.fillValue = 1;
    mt.cursorShadowPal = 3; mt.lettersSpacing = 0; mt.itemVerticalPadding = 0;
    mt.scrollMultiple = LIST_NO_MULTIPLE_SCROLL; mt.fontId = FONT_NORMAL;
    mt.cursorKind = CURSOR_BLACK_ARROW;

    gTasks[taskId].tStyleListId = ListMenuInit(&mt, 0, 0);
    CopyWindowToVram(gTasks[taskId].tStyleWindowId, COPYWIN_FULL);
    gTasks[taskId].func = StyleMenu_ProcessInput;
}

static void CloseStyleMenu(u8 taskId)
{
    DestroyListMenuTask(gTasks[taskId].tStyleListId, NULL, NULL);
    ClearStdWindowAndFrameToTransparent(gTasks[taskId].tStyleWindowId, FALSE);
    ClearWindowTilemap(gTasks[taskId].tStyleWindowId);
    RemoveWindow(gTasks[taskId].tStyleWindowId);
    ScheduleBgCopyTilemapToVram(0);
}

static void StyleMenu_ProcessInput(u8 taskId)
{
    s32 input;
    if (gPaletteFade.active) return;
    input = ListMenu_ProcessInput(gTasks[taskId].tStyleListId);
    if (input == LIST_NOTHING_CHOSEN) return;
    PlaySE(SE_SELECT);
    if (input == LIST_CANCEL || input == (s32)STYLE_MENU_ID_CANCEL)
    { CloseStyleMenu(taskId); ReshowPlayerPC(taskId); }
    else
    { VarSet(VAR_PLAYER_STYLE, input); CloseStyleMenu(taskId); RefreshPlayerOWPalette(); ReshowPlayerPC(taskId); }
}

#undef tStyleWindowId
#undef tStyleListId
