#!/usr/bin/env python3
"""Generate C header with player style palette data."""
import os

def darken(c, f=0.6):
    return (int(c[0]*f), int(c[1]*f), int(c[2]*f))

def lighten(c, f=1.3):
    return (min(255,int(c[0]*f)), min(255,int(c[1]*f)), min(255,int(c[2]*f)))

def rgb555(r, g, b):
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)

def fmt_rgb(c):
    val = (c[0] >> 3) | ((c[1] >> 3) << 5) | ((c[2] >> 3) << 10)
    return "0x{:04X}".format(val)

def make_style(clothes_main, clothes_light=None, clothes_outline=None,
               bag_main=None, bag_shadow=None, accent_main=None, accent_dark=None,
               white_parts=None, hat_white=None, hair=None, grayscale_skin=False):
    cm = clothes_main
    cl = clothes_light or lighten(cm, 1.35)
    co = clothes_outline or darken(cm, 0.45)
    cu = darken(cm, 0.7)
    bm = bag_main or lighten(cm, 1.5)
    bs = bag_shadow or darken(bm, 0.65)
    am = accent_main or (255, 98, 90)
    ad = accent_dark or darken(am, 0.7)
    wp = white_parts or (222, 230, 238)
    hw = hat_white or (255, 255, 255)
    return {
        'cl': cl, 'cm': cm, 'cu': cu, 'co': co,
        'wp': wp, 'bm': bm, 'bs': bs,
        'am': am, 'ad': ad, 'hw': hw,
        'hl': hair[0] if hair else None,
        'hd': hair[1] if hair else None,
        'gray': grayscale_skin,
    }

# ============================================================
# 27 styles in alphabetical order by DISPLAY name
# ============================================================
styles = []

# 1. Brazil
styles.append(("Brazil", "BRAZIL", make_style((0,100,60), (20,140,80), (0,42,22), (220,200,40), (138,125,20), (0,80,160), (0,50,120), (200,225,200))))
# 2. Chikorita
styles.append(("Chikorita", "CHIKORITA", make_style((95,150,70), (130,188,100), (38,65,28), (200,190,90), (128,120,52), (115,165,80), (72,115,48), (225,235,210))))
# 3. Cyndaquil
styles.append(("Cyndaquil", "CYNDAQUIL", make_style((45,55,85), (72,82,118), (16,20,38), (230,140,40), (142,85,18), (200,80,40), (150,48,22), (180,185,200))))
# 4. Dark (was Shadow)
styles.append(("Dark", "DARK", make_style((45,35,55), (60,50,70), (18,12,28), (100,80,110), (58,44,65), (140,50,120), (100,30,85), (160,150,170))))
# 5. Diver (was BlueTeal)
styles.append(("Diver", "DIVER", make_style((74,90,131), (98,123,156), (24,38,62), (90,210,190), (48,135,122), (90,98,255), (65,65,197))))
# 6. Enigma (NEW - all black, white hat/eyes)
styles.append(("Enigma", "ENIGMA", make_style((10,10,12), (18,18,22), (2,2,4), (15,15,18), (8,8,10), (12,12,15), (6,6,8), (255,255,255), (255,255,255))))
# 7. Fabulous (was Mewtwo)
styles.append(("Fabulous", "FABULOUS", make_style((110,90,135), (140,120,160), (48,35,68), (180,160,200), (112,98,128), (160,80,180), (120,50,140), (210,200,220))))
# 8. Forest
styles.append(("Forest", "FOREST", make_style((50,95,60), (70,120,80), (20,42,25), (180,140,80), (110,85,45), (190,60,50), (140,40,35))))
# 9. Groudon
styles.append(("Groudon", "GROUDON", make_style((110,28,22), (140,40,35), (48,10,8), (200,160,50), (125,98,28), (220,60,40), (170,35,22), (200,180,165))))
# 10. Historic (NEW - brownish yellow)
styles.append(("Historic", "HISTORIC", make_style((165,135,70), (200,170,95), (70,55,25), (190,160,100), (125,100,58), (180,120,50), (130,80,30), (235,225,200), (245,240,225))))
# 11. Ho-Oh
styles.append(("Ho-Oh", "HOOH", make_style((185,125,45), (225,160,65), (85,52,18), (205,65,45), (125,35,20), (235,85,55), (185,52,30))))
# 12. Kyogre
styles.append(("Kyogre", "KYOGRE", make_style((20,42,115), (30,60,140), (6,16,52), (80,160,220), (42,98,140), (220,40,50), (170,25,32), (180,200,230))))
# 13. Lugia (NEW - white/silver/grey)
styles.append(("Lugia", "LUGIA", make_style((180,180,190), (210,212,220), (60,62,70), (180,195,210), (120,132,148), (100,120,180), (60,75,130), (240,242,248), (255,255,255))))
# 14. Magma
styles.append(("Magma", "MAGMA", make_style((90,15,15), (130,32,30), (30,5,5), (170,40,30), (95,18,12), (150,30,25), (100,15,10), (160,148,145), (185,178,175))))
# 15. Master (was Lugia style)
styles.append(("Master", "MASTER", make_style((40,42,78), (55,58,100), (16,18,38), (150,180,210), (92,112,132), (200,30,50), (150,18,32), (210,215,225))))
# 16. Mudkip
styles.append(("Mudkip", "MUDKIP", make_style((55,100,165), (85,135,200), (20,42,72), (95,160,210), (52,96,130), (230,130,40), (175,85,22), (195,215,232))))
# 17. Ocean
styles.append(("Ocean", "OCEAN", make_style((28,75,108), (40,100,130), (10,30,50), (80,190,170), (42,118,108), (40,160,200), (25,120,160), (190,220,230))))
# 18. Old (was Grayscale)
styles.append(("Old", "OLD", make_style((120,120,120), (160,160,160), (48,48,48), (180,180,180), (110,110,110), (140,140,140), (95,95,95), (210,210,210), None, ((130,130,130),(65,65,65)), True)))
# 19. Red (was Lyra)
styles.append(("Red", "RED", make_style((140,80,60), (180,110,85), (60,32,22), (200,50,50), (120,28,28), (220,70,60), (170,42,35), (235,225,210), (245,238,225))))
# 20. Redmoon (was Crystal)
styles.append(("Redmoon", "REDMOON", make_style((50,90,150), (82,122,182), (18,35,65), (175,58,78), (105,30,42), (215,75,95), (165,48,65))))
# 21. Royal
styles.append(("Royal", "ROYAL", make_style((75,35,120), (100,50,150), (30,12,55), (200,180,60), (125,112,32), (180,50,180), (130,30,130), (220,210,230))))
# 22. Sakura
styles.append(("Sakura", "SAKURA", make_style((158,108,128), (188,138,158), (70,48,60), (228,188,202), (162,132,145), (238,108,138), (192,68,98))))
# 23. Sapphire (was Aqua)
styles.append(("Aqua", "AQUA", make_style((25,50,120), (42,72,155), (8,18,52), (50,80,170), (25,42,105), (35,60,150), (18,32,100), (180,195,225), (210,222,242))))
# 24. Silver (was Rocket)
styles.append(("Silver", "SILVER", make_style((55,55,65), (80,80,90), (18,18,28), (180,180,190), (110,110,118), (220,50,50), (160,30,30), (200,200,210))))
# 25. Torchic
styles.append(("Torchic", "TORCHIC", make_style((190,92,22), (225,128,45), (78,34,6), (218,150,42), (132,90,20), (222,68,28), (168,40,12))))
# 26. Totodile
styles.append(("Totodile", "TOTODILE", make_style((35,85,145), (58,118,180), (12,32,62), (65,150,200), (32,90,128), (200,60,50), (150,35,28), (188,210,228))))
# 27. Treecko
styles.append(("Treecko", "TREECKO", make_style((50,120,60), (80,160,85), (18,52,22), (100,180,80), (58,110,48), (200,60,50), (150,35,28))))

# ============================================================
# Generate C header
# ============================================================
lines = []
lines.append("#ifndef GUARD_DATA_PLAYER_STYLES_H")
lines.append("#define GUARD_DATA_PLAYER_STYLES_H")
lines.append("")
lines.append("// Auto-generated by tools/generate_style_data.py")
lines.append("// Do not edit manually")
lines.append("")
lines.append("#define PLAYER_STYLE_COUNT {}".format(len(styles)))
lines.append("")

# Style name strings
for i, (name, cname, _) in enumerate(styles):
    lines.append("static const u8 sStyleName_{}[] = _(\"{}\");".format(cname, name))

lines.append("")
lines.append("static const u8 sStyleName_Default[] = _(\"Default\");")
lines.append("static const u8 sStyleName_Cancel[] = _(\"Cancel\");")
lines.append("")

# Style data struct
lines.append("struct PlayerStyleData {")
lines.append("    u16 clothesLight;")
lines.append("    u16 clothesMain;")
lines.append("    u16 clothesDark;")
lines.append("    u16 clothesOutline;")
lines.append("    u16 whiteParts;")
lines.append("    u16 bagMain;")
lines.append("    u16 bagShadow;")
lines.append("    u16 accentMain;")
lines.append("    u16 accentDark;")
lines.append("    u16 hatWhite;")
lines.append("    u16 hairLight;")
lines.append("    u16 hairDark;")
lines.append("    bool8 grayscaleSkin;")
lines.append("};")
lines.append("")

# Style data array
lines.append("static const struct PlayerStyleData sPlayerStyles[PLAYER_STYLE_COUNT] = {")
for i, (name, cname, s) in enumerate(styles):
    hl = fmt_rgb(s['hl']) if s['hl'] else "0xFFFF"
    hd = fmt_rgb(s['hd']) if s['hd'] else "0xFFFF"
    gray = "TRUE" if s['gray'] else "FALSE"
    lines.append("    {{ // {} ({})".format(i, name))
    lines.append("        .clothesLight = {},".format(fmt_rgb(s['cl'])))
    lines.append("        .clothesMain = {},".format(fmt_rgb(s['cm'])))
    lines.append("        .clothesDark = {},".format(fmt_rgb(s['cu'])))
    lines.append("        .clothesOutline = {},".format(fmt_rgb(s['co'])))
    lines.append("        .whiteParts = {},".format(fmt_rgb(s['wp'])))
    lines.append("        .bagMain = {},".format(fmt_rgb(s['bm'])))
    lines.append("        .bagShadow = {},".format(fmt_rgb(s['bs'])))
    lines.append("        .accentMain = {},".format(fmt_rgb(s['am'])))
    lines.append("        .accentDark = {},".format(fmt_rgb(s['ad'])))
    lines.append("        .hatWhite = {},".format(fmt_rgb(s['hw'])))
    lines.append("        .hairLight = {},".format(hl))
    lines.append("        .hairDark = {},".format(hd))
    lines.append("        .grayscaleSkin = {},".format(gray))
    lines.append("    },")
lines.append("};")
lines.append("")

# Style names array (for unlock notification messages and dynamic menu)
lines.append("static const u8 *const sStyleNames[PLAYER_STYLE_COUNT] = {")
for i, (name, cname, _) in enumerate(styles):
    lines.append("    sStyleName_{},".format(cname))
lines.append("};")
lines.append("")

# Unlock flag IDs (0x29B through 0x2B5)
lines.append("// Unlock flags: one per style (0x29B-0x2B5)")
lines.append("// These must match FLAG_STYLE_UNLOCKED_* in constants/flags.h")
lines.append("#define FLAG_STYLE_UNLOCK_BASE 0x29B")
lines.append("")

# Menu constants
lines.append("#define STYLE_MENU_ID_DEFAULT 0")
lines.append("#define STYLE_MENU_ID_CANCEL  0xFFFF")
lines.append("")

lines.append("#endif // GUARD_DATA_PLAYER_STYLES_H")

out_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "src", "data", "player_styles.h")
with open(out_path, 'w') as f:
    f.write('\n'.join(lines) + '\n')

print("Generated {} with {} styles".format(out_path, len(styles)))
