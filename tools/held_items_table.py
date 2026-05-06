#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Render a PNG table of every held item in Veritas with its in-game icon
and a one-line summary of its effect (Veritas-modified or vanilla).

Usage:
    python tools/held_items_table.py

Reads:
    src/data/item_icon_table.h    — item -> icon constant mapping
    graphics/items/icons/*.png    — the 24x24 icon sprites
Output:
    tools/output/held_items_table_en.png   (English)
    tools/output/held_items_table_fr.png   (French / Français)
    tools/output/held_items_table.png      (alias of EN, for back-compat)

Held items hand-curated below. Veritas-changed effects flagged in the
last column. Add new entries to ITEMS as the project gains/loses
hold-item content.
"""

import os
import re
import sys
from pathlib import Path

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    sys.exit("Pillow not installed. Run: pip install Pillow")

REPO = Path(__file__).resolve().parent.parent
ICONS_DIR = REPO / "graphics" / "items" / "icons"
ICON_TABLE_FILE = REPO / "src" / "data" / "item_icon_table.h"

# (item_id, name_en, effect_en, name_fr, effect_fr, veritas_changed)
# Effects are summarized in <=70 chars to fit in one row.
# French names use canonical translations from the official Pokémon
# series (Poképédia / French Bulbapedia).
# Items grouped roughly by category for readability.
ITEMS = [
    # Type-power boosters (all bumped 10% -> 25% in Veritas)
    ("ITEM_SILVER_POWDER",  "Silver Powder",  "Bug moves +25% damage.",
                            "Poudre Argentée","Dégâts Insecte +25%.",                                  True),
    ("ITEM_METAL_COAT",     "Metal Coat",     "Steel moves +25%. Trade-evos Onix/Scyther.",
                            "Peau Métal",     "Acier +25%. Évol. Onix/Insécateur par échange.",       True),
    ("ITEM_SOFT_SAND",      "Soft Sand",      "Ground moves +25% damage.",
                            "Sable Doux",     "Dégâts Sol +25%.",                                      True),
    ("ITEM_HARD_STONE",     "Hard Stone",     "Rock moves +25% damage.",
                            "Pierre Dure",    "Dégâts Roche +25%.",                                    True),
    ("ITEM_MIRACLE_SEED",   "Miracle Seed",   "Grass moves +25% damage.",
                            "Graine Miracle", "Dégâts Plante +25%.",                                   True),
    ("ITEM_BLACK_GLASSES",  "BlackGlasses",   "Dark moves +25% damage.",
                            "Lunettes Noires","Dégâts Ténèbres +25%.",                                 True),
    ("ITEM_BLACK_BELT",     "Black Belt",     "Fighting moves +25% damage.",
                            "Ceinture Noire", "Dégâts Combat +25%.",                                   True),
    ("ITEM_MAGNET",         "Magnet",         "Electric moves +25% damage.",
                            "Aimant",         "Dégâts Électrik +25%.",                                 True),
    ("ITEM_MYSTIC_WATER",   "Mystic Water",   "Water moves +25% damage.",
                            "Eau Mystique",   "Dégâts Eau +25%.",                                      True),
    ("ITEM_SHARP_BEAK",     "Sharp Beak",     "Flying moves +25% damage.",
                            "Bec Pointu",     "Dégâts Vol +25%.",                                      True),
    ("ITEM_POISON_BARB",    "Poison Barb",    "Poison moves +15%. Doubles secondary poison odds.",
                            "Pic Venin",      "Poison +15%. Double les chances d'empoisonner.",       True),
    ("ITEM_NEVER_MELT_ICE", "NeverMeltIce",   "Ice moves +25% damage.",
                            "Glace Éternelle","Dégâts Glace +25%.",                                    True),
    ("ITEM_SPELL_TAG",      "Spell Tag",      "Ghost moves +25% damage.",
                            "Rune Sort",      "Dégâts Spectre +25%.",                                  True),
    ("ITEM_TWISTED_SPOON",  "TwistedSpoon",   "Psychic moves +25% damage.",
                            "Cuiller Tordue", "Dégâts Psy +25%.",                                      True),
    ("ITEM_CHARCOAL",       "Charcoal",       "Fire moves +25% damage.",
                            "Charbon",        "Dégâts Feu +25%.",                                      True),
    ("ITEM_DRAGON_FANG",    "Dragon Fang",    "Dragon moves +25% damage.",
                            "Croc Dragon",    "Dégâts Dragon +25%.",                                   True),
    ("ITEM_SILK_SCARF",     "Silk Scarf",     "Normal moves +25% damage.",
                            "Mouchoir Soie",  "Dégâts Normal +25%.",                                   True),
    ("ITEM_SEA_INCENSE",    "Sea Incense",    "Water moves +5%. Marill/Azurill egg item.",
                            "Encens Mer",     "Eau +5%. Objet œuf Marill/Azurill.",                   False),

    # Veritas-special species/type items
    ("ITEM_DRAGON_SCALE",   "Dragon Scale",   "Water + Dragon moves +15%. Trade-evo Seadra.",
                            "Écaille Draco",  "Eau + Dragon +15%. Évol. Hypocéan par échange.",       True),
    ("ITEM_DEEP_SEA_TOOTH", "DeepSeaTooth",   "SpA x2 for Clamperl/Huntail/Gorebyss.",
                            "Dent Océan",     "Att.Spé x2 pour Coquiperl/Serpang/Rosabyss.",          True),
    ("ITEM_DEEP_SEA_SCALE", "DeepSeaScale",   "SpD x2 for Clamperl/Huntail/Gorebyss.",
                            "Écaille Océan",  "Déf.Spé x2 pour Coquiperl/Serpang/Rosabyss.",          True),
    ("ITEM_LIGHT_BALL",     "Light Ball",     "SpA x2 Pikachu/Pichu/Plusle/Minun. Mareep Def/SpD x2.5. Flaaffy/Ampharos Spd x2.",
                            "Boule Lumière",  "Att.Spé x2 Pikachu/Pichu/Posipi/Negapi. Wattouat Déf/Déf.Spé x2.5. Lainergie/Pharamp Vit. x2.", True),
    ("ITEM_THICK_CLUB",     "Thick Club",     "Atk x2 for Cubone and Marowak.",
                            "Os Épais",       "Att. x2 pour Osselait et Ossatueur.",                  False),
    ("ITEM_METAL_POWDER",   "Metal Powder",   "Def x2 + Speed x2 for Ditto.",
                            "Poudre Métal",   "Déf. x2 + Vit. x2 pour Métamorph.",                    True),
    ("ITEM_LUCKY_PUNCH",    "Lucky Punch",    "Atk x2 + crit-rate for Chansey.",
                            "Boxe Punch",     "Att. x2 + crit pour Leveinard.",                       True),
    ("ITEM_UP_GRADE",       "Up-Grade",       "Def x2 for Porygon. Trade-evo Porygon.",
                            "Améliorator",    "Déf. x2 Porygon. Évol. Porygon par échange.",         True),
    ("ITEM_KINGS_ROCK",     "King's Rock",    "10% flinch on damaging moves.",
                            "Roche Royale",   "10% d'apeurer sur attaque offensive.",                 False),
    ("ITEM_QUICK_CLAW",     "Quick Claw",     "20% chance to move first regardless of speed.",
                            "Vive Griffe",    "20% d'attaquer en premier malgré la vitesse.",         False),
    ("ITEM_SCOPE_LENS",     "Scope Lens",     "+1 critical-hit stage.",
                            "Lentilscope",    "+1 niveau de coup critique.",                          False),
    ("ITEM_FOCUS_BAND",     "Focus Band",     "10% chance to survive a KO at 1 HP.",
                            "Bandeau Mental", "10% de survivre à 1 PV à un KO.",                      False),
    ("ITEM_SOUL_DEW",       "Soul Dew",       "SpA + SpD +50% for Latios and Latias.",
                            "Rosée Âme",      "Att.Spé + Déf.Spé +50% pour Latios et Latias.",        False),
    ("ITEM_SOOTHE_BELL",    "Soothe Bell",    "Friendship gain +50% for the holder.",
                            "Grelot Zen",     "Gain d'amitié +50% pour le porteur.",                  False),
    ("ITEM_CLEANSE_TAG",    "Cleanse Tag",    "Lower wild encounter rate for the lead Pokemon.",
                            "Rune Purif",     "Réduit la fréquence de rencontres sauvages.",          False),

    # Healing / status (held while in battle)
    ("ITEM_LEFTOVERS",      "Leftovers",      "Heals 1/16 max HP each turn.",
                            "Restes",         "Soigne 1/16 PV max chaque tour.",                      False),
    ("ITEM_SHELL_BELL",     "Shell Bell",     "Heals 1/6 of damage dealt (was 1/8).",
                            "Grelot Coque",   "Soigne 1/6 des dégâts infligés (était 1/8).",          True),
    ("ITEM_BERRY_JUICE",    "Berry Juice",    "Heals 20 HP at low HP, then consumed.",
                            "Jus de Baie",    "Soigne 20 PV en danger, puis consommé.",               False),
    ("ITEM_AMULET_COIN",    "Amulet Coin",    "Doubles money won when held by a battler.",
                            "Pièce Rune",     "Double l'argent gagné si tenu par un combattant.",     False),
    ("ITEM_LAX_INCENSE",    "Lax Incense",    "Boosts the holder's evasion slightly.",
                            "Encens Doux",    "Augmente légèrement l'esquive du porteur.",            False),
    ("ITEM_BRIGHT_POWDER",  "Bright Powder",  "Lowers opponent's accuracy slightly.",
                            "Poudre Claire",  "Baisse légèrement la précision adverse.",              False),
    ("ITEM_FOCUS_BAND",     None,             None,
                            None,             None,                                                   False),  # dup-skip; see filter

    # Choice / power
    ("ITEM_CHOICE_BAND",    "Choice Band",    "Atk +50%, locks in first move chosen.",
                            "Bandeau Choix",  "Att. +50%, verrouille la première capacité.",          False),
    ("ITEM_WHITE_HERB",     "White Herb",     "Restores any lowered stat. Consumed.",
                            "Herbe Blanche",  "Restaure toute stat baissée. Consommée.",              False),
    ("ITEM_MENTAL_HERB",    "Mental Herb",    "Cures attraction + confusion. Reusable.",
                            "Herbe Mental",   "Soigne attirance + confusion. Réutilisable.",          True),

    # Status-curing berries (held)
    ("ITEM_CHERI_BERRY",    "Cheri Berry",    "Cures paralysis when held.",
                            "Baie Ceriz",     "Soigne la paralysie si tenue.",                        False),
    ("ITEM_CHESTO_BERRY",   "Chesto Berry",   "Cures sleep when held.",
                            "Baie Maron",     "Soigne le sommeil si tenue.",                          False),
    ("ITEM_PECHA_BERRY",    "Pecha Berry",    "Cures poison when held.",
                            "Baie Pecha",     "Soigne le poison si tenue.",                           False),
    ("ITEM_RAWST_BERRY",    "Rawst Berry",    "Cures burn when held.",
                            "Baie Fraive",    "Soigne la brûlure si tenue.",                          False),
    ("ITEM_ASPEAR_BERRY",   "Aspear Berry",   "Cures freeze when held.",
                            "Baie Willia",    "Soigne le gel si tenue.",                              False),
    ("ITEM_LEPPA_BERRY",    "Leppa Berry",    "Restores 10 PP to a depleted move.",
                            "Baie Mepo",      "Restaure 10 PP à une capacité épuisée.",               False),
    ("ITEM_ORAN_BERRY",     "Oran Berry",     "Restores 10 HP at half HP.",
                            "Baie Oran",      "Restaure 10 PV à la moitié des PV.",                   False),
    ("ITEM_PERSIM_BERRY",   "Persim Berry",   "Cures confusion when held.",
                            "Baie Kika",      "Soigne la confusion si tenue.",                        False),
    ("ITEM_LUM_BERRY",      "Lum Berry",      "Cures any status when held.",
                            "Baie Prine",     "Soigne tout statut si tenue.",                         False),
    ("ITEM_SITRUS_BERRY",   "Sitrus Berry",   "Restores 30 HP at half HP.",
                            "Baie Sitrus",    "Restaure 30 PV à la moitié des PV.",                   False),

    # Pinch berries (HP threshold tightened in Veritas: 1/4 -> 1/2)
    ("ITEM_FIGY_BERRY",     "Figy Berry",     "Heal 1/8 HP at 1/2 HP. Confuses if Naughty disliked.",
                            "Baie Figuy",     "Soigne 1/8 PV à 1/2 PV. Confuse si nature détestée.",  True),
    ("ITEM_WIKI_BERRY",     "Wiki Berry",     "Heal 1/8 HP at 1/2 HP. Confuses if disliked.",
                            "Baie Wiki",      "Soigne 1/8 PV à 1/2 PV. Confuse si détestée.",         True),
    ("ITEM_MAGO_BERRY",     "Mago Berry",     "Heal 1/8 HP at 1/2 HP. Confuses if disliked.",
                            "Baie Mago",      "Soigne 1/8 PV à 1/2 PV. Confuse si détestée.",         True),
    ("ITEM_AGUAV_BERRY",    "Aguav Berry",    "Heal 1/8 HP at 1/2 HP. Confuses if disliked.",
                            "Baie Gowav",     "Soigne 1/8 PV à 1/2 PV. Confuse si détestée.",         True),
    ("ITEM_IAPAPA_BERRY",   "Iapapa Berry",   "Heal 1/8 HP at 1/2 HP. Confuses if disliked.",
                            "Baie Papaya",    "Soigne 1/8 PV à 1/2 PV. Confuse si détestée.",         True),
    ("ITEM_LIECHI_BERRY",   "Liechi Berry",   "+1 Atk at 1/4 HP.",
                            "Baie Lichii",    "+1 Att. à 1/4 PV.",                                    False),
    ("ITEM_GANLON_BERRY",   "Ganlon Berry",   "+1 Def and 1/8 HP heal at 1/3 HP.",
                            "Baie Lonme",     "+1 Déf. et 1/8 PV à 1/3 PV.",                          True),
    ("ITEM_SALAC_BERRY",    "Salac Berry",    "+1 Speed at 1/4 HP.",
                            "Baie Sailak",    "+1 Vit. à 1/4 PV.",                                    False),
    ("ITEM_PETAYA_BERRY",   "Petaya Berry",   "+1 SpA at 1/4 HP.",
                            "Baie Pommo",     "+1 Att.Spé à 1/4 PV.",                                 False),
    ("ITEM_APICOT_BERRY",   "Apicot Berry",   "+1 SpD and 1/8 HP heal at 1/3 HP.",
                            "Baie Abriko",    "+1 Déf.Spé et 1/8 PV à 1/3 PV.",                       True),
    ("ITEM_LANSAT_BERRY",   "Lansat Berry",   "+2 crit-hit stages at 1/3 HP.",
                            "Baie Linka",     "+2 niveaux de crit à 1/3 PV.",                         True),
    ("ITEM_STARF_BERRY",    "Starf Berry",    "+2 random stat and 1/8 HP heal at 1/3 HP.",
                            "Baie Stars",     "+2 stat aléatoire et 1/8 PV à 1/3 PV.",                True),
    ("ITEM_ENIGMA_BERRY",   "Enigma Berry",   "Counters foe's stat-drop or confusion with a boost.",
                            "Baie Enigma",    "Contre baisse de stat / confusion par un boost.",      True),
]


# Localized header / legend strings, keyed by language code.
LOCALE = {
    "en": {
        "title":  "Pokémon Emerald Veritas — Held Items Reference",
        "legend": "Rows highlighted in orange have Veritas-specific changes.",
    },
    "fr": {
        "title":  "Pokémon Emerald Veritas — Référence des Objets Tenus",
        "legend": "Les lignes orange indiquent les changements spécifiques à Veritas.",
    },
}


def parse_icon_table():
    """Return dict {item_id: png_path_or_None}."""
    text = ICON_TABLE_FILE.read_text(encoding="utf-8")
    # Match: [ITEM_X] = {gItemIcon_Y, gItemIconPalette_Z},
    pat = re.compile(r"\[(ITEM_[A-Z0-9_]+)\]\s*=\s*\{gItemIcon_([A-Za-z0-9]+)\s*,")
    out = {}
    for m in pat.finditer(text):
        item_id = m.group(1)
        icon_camel = m.group(2)
        # CamelCase -> snake_case
        snake = re.sub(r"(?<!^)(?=[A-Z])", "_", icon_camel).lower()
        png = ICONS_DIR / f"{snake}.png"
        out[item_id] = png if png.exists() else None
    return out


def load_icon(path, target_size=48):
    """Load a 24x24 4bpp icon and scale up nearest-neighbor to target size.
    Falls back to a magenta placeholder if the file is missing."""
    if path is None or not path.exists():
        img = Image.new("RGBA", (target_size, target_size), (200, 50, 200, 255))
        return img
    im = Image.open(path).convert("RGBA")
    # Item icon PNGs typically have a transparent palette index — Pillow
    # picks it up via the P mode 'transparency' field on convert.
    return im.resize((target_size, target_size), Image.NEAREST)


def render(items_list, output_path, language="en"):
    """Render the held-items table to ``output_path`` using strings for
    ``language`` ("en" or "fr").

    ``items_list`` entries are 6-tuples:
        (item_id, name_en, effect_en, name_fr, effect_fr, veritas_changed)
    """
    icon_map = parse_icon_table()

    if language == "fr":
        name_idx, effect_idx = 3, 4
    else:
        name_idx, effect_idx = 1, 2

    # Drop the empty/dup placeholder rows the manual list intentionally
    # has (so the table stays tidy without fussy data structures).
    cleaned = [it for it in items_list if it[name_idx] is not None]

    # Layout
    cols = 2
    rows = (len(cleaned) + cols - 1) // cols
    icon_size = 48
    pad = 8
    name_w = 130
    effect_w = 460
    cell_w = icon_size + pad + name_w + pad + effect_w + pad * 2
    cell_h = icon_size + pad
    header_h = 70
    width = cell_w * cols + pad
    height = header_h + cell_h * rows + pad

    bg = (24, 24, 32, 255)
    fg = (240, 240, 240, 255)
    veritas_color = (255, 200, 80, 255)
    name_color = (200, 220, 255, 255)
    grid_color = (60, 60, 80, 255)

    img = Image.new("RGB", (width, height), bg[:3])
    draw = ImageDraw.Draw(img)

    # Try a TTF for nicer text; fall back to PIL default if absent.
    # Arial supports the accented Latin characters we need for French
    # (é, è, à, ô, î, etc.).
    try:
        title_font = ImageFont.truetype("arialbd.ttf", 22)
        name_font = ImageFont.truetype("arialbd.ttf", 13)
        effect_font = ImageFont.truetype("arial.ttf", 12)
        legend_font = ImageFont.truetype("arial.ttf", 11)
    except OSError:
        title_font = ImageFont.load_default()
        name_font = ImageFont.load_default()
        effect_font = ImageFont.load_default()
        legend_font = ImageFont.load_default()

    strings = LOCALE.get(language, LOCALE["en"])

    # Header
    draw.text((pad * 2, pad * 2), strings["title"], fg, font=title_font)
    draw.text((pad * 2, pad * 2 + 30),
              strings["legend"],
              veritas_color, font=legend_font)

    # Cells
    for idx, entry in enumerate(cleaned):
        item_id = entry[0]
        name = entry[name_idx]
        effect = entry[effect_idx]
        changed = entry[5]

        col = idx % cols
        row = idx // cols
        x0 = pad + col * cell_w
        y0 = header_h + row * cell_h

        # Light separator at row top
        draw.line([(x0, y0), (x0 + cell_w - pad, y0)], fill=grid_color)

        # Highlight row background if veritas-changed
        if changed:
            draw.rectangle(
                [(x0, y0), (x0 + cell_w - pad, y0 + cell_h)],
                fill=(60, 40, 20),
            )

        # Icon
        png = icon_map.get(item_id)
        icon = load_icon(png, icon_size)
        # Composite RGBA icon over the row background
        crop_box = (x0 + pad, y0 + (cell_h - icon_size) // 2)
        if icon.mode == "RGBA":
            img.paste(icon, crop_box, icon)
        else:
            img.paste(icon, crop_box)

        # Name
        text_x = x0 + pad + icon_size + pad
        text_y = y0 + 6
        draw.text((text_x, text_y), name, name_color, font=name_font)
        if changed:
            draw.text((text_x + name_w - 18, text_y), "*", veritas_color, font=name_font)

        # Effect
        eff_x = text_x
        eff_y = y0 + 6 + 18
        draw.text((eff_x, eff_y), effect, fg, font=effect_font)

    img.save(output_path, "PNG")
    print(f"Wrote {output_path}  ({width}x{height})")


def main():
    out_dir = REPO / "tools" / "output"
    out_dir.mkdir(parents=True, exist_ok=True)

    en_path = out_dir / "held_items_table_en.png"
    fr_path = out_dir / "held_items_table_fr.png"

    render(ITEMS, en_path, language="en")
    render(ITEMS, fr_path, language="fr")

    # Backwards-compatible alias: the old un-suffixed file mirrors EN.
    legacy = out_dir / "held_items_table.png"
    render(ITEMS, legacy, language="en")


if __name__ == "__main__":
    main()
