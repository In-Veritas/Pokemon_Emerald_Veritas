#!/usr/bin/env python3
"""Generate all player style palette variants with front, back, and labels."""
from PIL import Image, ImageDraw
import os, sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
OUT = os.path.join(SCRIPT_DIR, "output", "styles")
os.makedirs(OUT, exist_ok=True)

def read_jasc(path):
    with open(path) as f:
        lines = f.read().strip().split("\n")
    colors = []
    for line in lines[3:]:
        p = line.strip().split()
        if len(p) >= 3:
            colors.append((int(p[0]), int(p[1]), int(p[2])))
    return colors[:16]

def apply_pal(img, pal):
    w, h = img.size
    px = list(img.getdata())
    out = Image.new("RGBA", (w, h))
    op = []
    for idx in px:
        if idx == 0:
            op.append((0, 0, 0, 0))
        elif idx < len(pal):
            op.append(pal[idx] + (255,))
        else:
            op.append((0, 0, 0, 0))
    out.putdata(op)
    return out

def mk(base, c5=None, c6=None, c7=None, c8=None, bag10=None, bag11=None,
        acc12=None, acc13=None, lt9=None, hat14=None):
    """Make variant. c5-c8 = clothing gradient, bag10-11, acc12-13 = accents."""
    p = list(base)
    if c5: p[5] = c5
    if c6: p[6] = c6
    if c7: p[7] = c7
    if c8: p[8] = c8
    if bag10: p[10] = bag10
    if bag11: p[11] = bag11
    if acc12: p[12] = acc12
    if acc13: p[13] = acc13
    if lt9: p[9] = lt9
    if hat14: p[14] = hat14
    return p

def darken(c, factor=0.6):
    return (int(c[0]*factor), int(c[1]*factor), int(c[2]*factor))

def lighten(c, factor=1.3):
    return (min(255,int(c[0]*factor)), min(255,int(c[1]*factor)), min(255,int(c[2]*factor)))

# Emerald Brendan indices:
# 0: transparent, 1-4: skin (keep), 5: hat/pants/jacket lighter, 6: hat/pants/jacket main
# 7: unused, 8: bandana/jacket/shoes/wristband outline, 9: white hat parts/pokeball/shoe soles
# 10: bandana left/wristband/bag sash, 11: bandana right/bag/socks/wristband
# 12: neck/pokeball/hands, 13: tshirt/shoes/pokeball/hands, 14: white hat, 15: black outline

# Load base palettes
em_m = read_jasc(os.path.join(REPO_ROOT, "graphics/trainers/palettes/brendan.pal"))
em_f = read_jasc(os.path.join(REPO_ROOT, "graphics/trainers/palettes/may.pal"))

# Load sprites
sprites = {
    "em_m_front": Image.open(os.path.join(REPO_ROOT, "graphics/trainers/front_pics/brendan.png")),
    "em_m_back": Image.open(os.path.join(REPO_ROOT, "graphics/trainers/back_pics/brendan.png")),
    "em_f_front": Image.open(os.path.join(REPO_ROOT, "graphics/trainers/front_pics/may.png")),
    "em_f_back": Image.open(os.path.join(REPO_ROOT, "graphics/trainers/back_pics/may.png")),
}

# Helper: make a full design with proper outline (index 8 = dark version of main clothing)
def design(base, clothes_main, clothes_light=None, clothes_outline=None,
           bag_main=None, bag_dark=None, accent_main=None, accent_dark=None,
           white_parts=None, hat_white=None):
    """Create a coherent outfit. clothes_main = (r,g,b) for index 6."""
    cm = clothes_main
    cl = clothes_light or lighten(cm, 1.35)
    co = clothes_outline or darken(cm, 0.45)
    cu = darken(cm, 0.7)  # index 7 unused but set anyway
    bm = bag_main or lighten(cm, 1.5)
    bd = bag_dark or darken(bm, 0.7)
    am = accent_main or cm
    ad = accent_dark or darken(am, 0.7)
    wp = white_parts or (222, 230, 238)
    hw = hat_white or (255, 255, 255)
    return mk(base, c5=cl, c6=cm, c7=cu, c8=co, bag10=bm, bag11=bd,
              acc12=am, acc13=ad, lt9=wp, hat14=hw)

# ============================================================
# ALL DESIGNS - 5 attempts each for requested themes
# ============================================================
designs = {}

# --- TEAM MAGMA (black, dark red, red) ---
designs["Magma"] = [
    design(em_m, (130,30,30), (170,50,50), (50,10,10), (200,50,40), (140,30,25), (220,60,50), (160,35,30), (180,160,155), (200,190,185)),
    design(em_m, (110,20,25), (155,45,45), (40,8,8), (180,40,30), (120,25,18), (200,50,40), (145,28,25), (170,155,150)),
    design(em_m, (100,25,25), (145,50,40), (35,8,8), (210,60,40), (150,35,22), (190,45,35), (130,25,20), (190,170,165)),
    design(em_m, (120,15,20), (165,40,38), (45,5,5), (190,50,35), (135,30,20), (210,55,45), (155,32,28), (175,160,158)),
    design(em_m, (140,35,35), (180,60,55), (55,12,12), (220,70,50), (160,42,30), (230,65,55), (170,38,32), (185,168,162)),
]

# --- TEAM AQUA (light blue, dark blue) ---
designs["Aqua"] = [
    design(em_m, (40,80,150), (75,115,185), (15,30,70), (70,170,200), (40,120,155), (50,130,190), (30,85,145), (200,220,240), (225,238,250)),
    design(em_m, (35,70,140), (65,105,175), (12,25,62), (80,180,210), (50,130,165), (55,140,200), (32,90,150), (195,215,235)),
    design(em_m, (45,85,155), (80,120,190), (18,32,72), (60,160,195), (35,115,150), (45,125,185), (28,80,140), (205,225,245)),
    design(em_m, (30,65,135), (58,98,170), (10,22,58), (90,190,215), (55,140,170), (60,145,205), (35,95,155), (190,210,232)),
    design(em_m, (50,90,160), (85,125,195), (20,35,75), (75,175,205), (45,128,158), (48,135,192), (30,88,148), (208,228,248)),
]

# --- ETHAN (Gold/Silver protagonist - red jacket, shorts, yellow bag) ---
designs["Ethan"] = [
    design(em_m, (180,50,50), (220,80,70), (80,20,20), (220,190,60), (170,145,40), (200,60,55), (150,35,32), (230,220,200)),
    design(em_m, (170,45,45), (210,72,65), (75,18,18), (210,180,50), (160,135,35), (190,55,48), (140,30,28)),
    design(em_m, (190,55,55), (230,85,75), (85,22,22), (225,195,65), (175,150,42), (205,62,58), (155,38,34)),
    design(em_m, (175,48,48), (215,76,68), (78,19,19), (215,185,55), (165,140,38), (195,58,52), (145,33,30)),
    design(em_m, (185,52,52), (225,82,72), (82,21,21), (230,200,70), (180,155,45), (210,65,60), (158,40,36)),
]

# --- LYRA (HGSS female - overalls, big hat, red shirt) ---
designs["Lyra"] = [
    design(em_m, (140,80,60), (180,110,85), (60,32,22), (200,50,50), (150,30,30), (220,70,60), (170,42,35), (235,225,210), (245,238,225)),
    design(em_m, (130,75,55), (170,105,80), (55,28,18), (190,45,45), (140,28,28), (210,65,55), (160,38,30)),
    design(em_m, (145,85,65), (185,115,90), (62,35,25), (205,55,55), (155,32,32), (225,72,62), (175,45,38)),
    design(em_m, (135,78,58), (175,108,82), (58,30,20), (195,48,48), (145,30,30), (215,68,58), (165,40,32)),
    design(em_m, (150,88,68), (190,118,92), (65,38,28), (210,58,58), (160,35,35), (230,75,65), (180,48,40)),
]

# --- CRYSTAL (Crystal female - blue outfit) ---
designs["Crystal"] = [
    design(em_m, (60,100,160), (95,135,195), (22,42,75), (200,60,80), (150,35,55), (220,80,100), (170,50,68), (210,225,240)),
    design(em_m, (55,95,155), (88,128,188), (20,38,70), (190,55,75), (140,32,50), (210,72,92), (160,45,62)),
    design(em_m, (65,105,165), (100,140,200), (25,45,78), (205,65,85), (155,38,58), (225,85,105), (175,55,72)),
    design(em_m, (50,90,150), (82,122,182), (18,35,65), (195,58,78), (145,33,52), (215,75,95), (165,48,65)),
    design(em_m, (70,108,168), (105,142,202), (28,48,80), (210,68,88), (160,40,60), (230,88,108), (180,58,75)),
]

# --- HO-OH (gold/rainbow, warm colors) ---
designs["HoOh"] = [
    design(em_m, (180,120,40), (220,155,60), (80,50,15), (200,60,40), (150,35,22), (230,80,50), (180,50,28), (240,230,200), (250,242,220)),
    design(em_m, (170,115,35), (210,148,55), (75,45,12), (190,55,35), (140,30,18), (220,72,45), (170,45,24)),
    design(em_m, (185,125,45), (225,160,65), (85,52,18), (205,65,45), (155,38,25), (235,85,55), (185,52,30)),
    design(em_m, (175,118,38), (215,152,58), (78,48,14), (195,58,38), (145,32,20), (225,78,48), (175,48,26)),
    design(em_m, (190,128,48), (230,162,68), (88,55,20), (210,68,48), (160,40,28), (240,88,58), (190,55,32)),
]

# --- JIRACHI (yellow/white, star theme) ---
designs["Jirachi"] = [
    design(em_m, (190,180,120), (225,215,155), (85,80,50), (220,200,80), (175,155,55), (200,190,100), (155,145,65), (240,238,225), (252,250,240)),
    design(em_m, (185,175,115), (218,208,148), (82,78,48), (215,195,75), (170,150,50), (195,185,95), (150,140,60)),
    design(em_m, (195,185,125), (228,218,158), (88,82,52), (225,205,85), (180,160,58), (205,195,105), (160,148,68)),
    design(em_m, (188,178,118), (222,212,152), (84,79,49), (218,198,78), (172,152,52), (198,188,98), (152,142,62)),
    design(em_m, (198,188,128), (232,222,162), (90,84,54), (228,208,88), (182,162,60), (208,198,108), (162,150,70)),
]

# --- GRAYSCALE ---
designs["Grayscale"] = [
    design(em_m, (120,120,120), (160,160,160), (48,48,48), (180,180,180), (130,130,130), (140,140,140), (95,95,95), (210,210,210)),
    design(em_m, (110,110,110), (150,150,150), (42,42,42), (170,170,170), (120,120,120), (130,130,130), (88,88,88), (200,200,200)),
    design(em_m, (130,130,130), (170,170,170), (52,52,52), (190,190,190), (140,140,140), (150,150,150), (102,102,102), (220,220,220)),
    design(em_m, (100,100,100), (140,140,140), (38,38,38), (165,165,165), (115,115,115), (125,125,125), (82,82,82), (195,195,195)),
    design(em_m, (115,115,115), (155,155,155), (45,45,45), (175,175,175), (125,125,125), (135,135,135), (90,90,90), (205,205,205)),
]

# --- TORCHIC (orange/red, warm fire starter) ---
designs["Torchic"] = [
    design(em_m, (210,120,40), (240,155,65), (95,50,15), (230,170,50), (180,125,30), (240,100,40), (190,65,20)),
    design(em_m, (200,115,35), (232,148,58), (90,46,12), (225,165,45), (175,120,25), (235,95,35), (185,60,18)),
    design(em_m, (215,125,45), (245,158,68), (98,52,18), (235,175,55), (185,128,32), (245,105,45), (195,68,22)),
    design(em_m, (205,118,38), (238,152,62), (92,48,14), (228,168,48), (178,122,28), (238,98,38), (188,62,18)),
    design(em_m, (220,128,48), (248,162,72), (100,55,20), (240,178,58), (190,132,35), (248,108,48), (198,72,25)),
]

# --- TREECKO (green, forest starter) ---
designs["Treecko"] = [
    design(em_m, (50,120,60), (80,160,85), (18,52,22), (100,180,80), (65,135,55), (60,140,65), (35,95,38)),
    design(em_m, (45,115,55), (75,155,80), (15,48,18), (95,175,75), (60,130,50), (55,135,60), (30,90,35)),
    design(em_m, (55,125,65), (85,165,90), (20,55,25), (105,185,85), (70,140,58), (65,145,70), (38,98,42)),
    design(em_m, (48,118,58), (78,158,82), (16,50,20), (98,178,78), (62,132,52), (58,138,62), (32,92,36)),
    design(em_m, (58,128,68), (88,168,92), (22,58,28), (108,188,88), (72,142,60), (68,148,72), (40,100,44)),
]

# --- MUDKIP (blue, water starter) ---
designs["Mudkip"] = [
    design(em_m, (60,110,170), (90,145,205), (22,48,78), (100,170,220), (65,128,175), (70,130,190), (40,85,142), (200,218,235)),
    design(em_m, (55,105,165), (85,140,200), (20,45,75), (95,165,215), (60,124,170), (65,125,185), (36,80,138)),
    design(em_m, (65,115,175), (95,148,208), (24,50,80), (105,175,225), (68,132,178), (75,135,195), (42,88,145)),
    design(em_m, (58,108,168), (88,142,202), (21,46,76), (98,168,218), (62,126,172), (68,128,188), (38,82,140)),
    design(em_m, (68,118,178), (98,152,212), (26,52,82), (108,178,228), (72,135,182), (78,138,198), (44,90,148)),
]

# --- TOTODILE (blue-green, teal croc) ---
designs["Totodile"] = [
    design(em_m, (40,110,130), (70,148,168), (14,48,58), (80,180,190), (50,135,148), (50,140,160), (28,92,108), (195,225,230)),
    design(em_m, (35,105,125), (65,142,162), (12,44,54), (75,175,185), (45,130,142), (45,135,155), (24,88,104)),
    design(em_m, (45,115,135), (75,152,172), (16,50,60), (85,185,195), (55,138,152), (55,145,165), (30,95,112)),
    design(em_m, (38,108,128), (68,145,165), (13,46,56), (78,178,188), (48,132,145), (48,138,158), (26,90,106)),
    design(em_m, (48,118,138), (78,155,175), (18,52,62), (88,188,198), (58,140,155), (58,148,168), (32,98,115)),
]

# --- CHIKORITA (light green, leaf) ---
designs["Chikorita"] = [
    design(em_m, (100,160,80), (135,195,110), (42,70,32), (150,200,100), (110,160,70), (120,175,90), (75,120,52), (215,235,205)),
    design(em_m, (95,155,75), (130,190,105), (38,66,28), (145,195,95), (105,155,65), (115,170,85), (70,115,48)),
    design(em_m, (105,165,85), (140,200,115), (45,72,35), (155,205,105), (115,165,75), (125,180,95), (78,125,55)),
    design(em_m, (98,158,78), (132,192,108), (40,68,30), (148,198,98), (108,158,68), (118,172,88), (72,118,50)),
    design(em_m, (108,168,88), (142,202,118), (48,75,38), (158,208,108), (118,168,78), (128,182,98), (80,128,58)),
]

# --- CYNDAQUIL (dark blue body, fire back) ---
designs["Cyndaquil"] = [
    design(em_m, (45,55,85), (72,82,118), (16,20,38), (230,140,40), (180,100,25), (200,80,40), (150,48,22), (180,185,200)),
    design(em_m, (40,50,80), (68,78,112), (14,18,35), (225,135,35), (175,95,20), (195,75,35), (145,44,18)),
    design(em_m, (50,60,90), (76,86,122), (18,22,40), (235,145,45), (185,105,28), (205,85,45), (155,52,25)),
    design(em_m, (42,52,82), (70,80,115), (15,19,36), (228,138,38), (178,98,22), (198,78,38), (148,46,20)),
    design(em_m, (52,62,92), (78,88,125), (20,24,42), (238,148,48), (188,108,30), (208,88,48), (158,55,28)),
]

# --- SHINY HUNTER (purple/gold, shiny sparkle theme) ---
designs["ShinyHunter"] = [
    design(em_m, (90,60,130), (125,88,168), (35,22,58), (220,200,60), (175,155,38), (180,150,50), (135,110,28), (210,205,225)),
    design(em_m, (85,55,125), (120,82,162), (32,18,54), (215,195,55), (170,150,32), (175,145,45), (130,105,24)),
    design(em_m, (95,65,135), (128,92,172), (38,25,60), (225,205,65), (180,158,40), (185,155,55), (140,115,30)),
    design(em_m, (88,58,128), (122,85,165), (34,20,56), (218,198,58), (172,152,35), (178,148,48), (132,108,26)),
    design(em_m, (98,68,138), (132,95,175), (40,28,62), (228,208,68), (182,162,42), (188,158,58), (142,118,32)),
]

# --- SHINY GROUDON (dark red/maroon) ---
designs["ShinyGroudon"] = [
    design(em_m, (100,25,25), (140,42,38), (42,8,8), (200,100,30), (150,68,18), (160,40,30), (110,22,16), (185,170,165)),
    design(em_m, (95,22,22), (135,38,35), (38,6,6), (195,95,25), (145,64,15), (155,35,25), (105,18,12)),
    design(em_m, (105,28,28), (145,45,40), (45,10,10), (205,105,35), (155,72,20), (165,42,32), (115,25,18)),
    design(em_m, (98,24,24), (138,40,36), (40,7,7), (198,98,28), (148,66,16), (158,38,28), (108,20,14)),
    design(em_m, (108,30,30), (148,48,42), (48,12,12), (208,108,38), (158,74,22), (168,45,35), (118,28,20)),
]

# --- SHINY KYOGRE (dark navy/sapphire) ---
designs["ShinyKyogre"] = [
    design(em_m, (20,35,80), (35,55,115), (6,12,35), (40,100,180), (22,68,135), (30,70,150), (14,40,100), (170,185,210)),
    design(em_m, (18,32,75), (32,50,110), (5,10,32), (35,95,175), (20,64,130), (28,65,145), (12,36,95)),
    design(em_m, (22,38,85), (38,58,118), (7,14,38), (45,105,185), (25,72,138), (32,75,155), (16,42,105)),
    design(em_m, (19,34,78), (34,52,112), (6,11,34), (38,98,178), (21,66,132), (29,68,148), (13,38,98)),
    design(em_m, (24,40,88), (40,60,122), (8,16,40), (48,108,188), (28,75,142), (35,78,158), (18,45,108)),
]

# --- SHINY RAYQUAZA (dark emerald/black-green) ---
designs["ShinyRayquaza"] = [
    design(em_m, (20,50,25), (35,78,38), (6,18,8), (40,100,45), (22,68,28), (30,70,32), (14,40,16), (155,175,158)),
    design(em_m, (18,45,22), (32,72,35), (5,15,6), (35,95,40), (20,64,25), (28,65,28), (12,36,14)),
    design(em_m, (22,55,28), (38,82,42), (7,20,10), (45,105,48), (25,72,30), (32,75,35), (16,42,18)),
    design(em_m, (19,48,24), (34,75,36), (6,16,7), (38,98,42), (21,66,26), (29,68,30), (13,38,15)),
    design(em_m, (24,58,30), (40,85,45), (8,22,12), (48,108,52), (28,75,32), (35,78,38), (18,45,20)),
]

# --- BRAZIL FLAG (green, yellow, blue) ---
designs["Brazil"] = [
    design(em_m, (0,100,60), (20,140,80), (0,42,22), (220,200,40), (170,155,25), (0,80,160), (0,50,120), (200,225,200)),
    design(em_m, (0,95,55), (15,135,75), (0,38,18), (215,195,35), (165,150,20), (0,75,155), (0,45,115)),
    design(em_m, (0,105,65), (25,145,85), (0,45,25), (225,205,45), (175,158,28), (0,85,165), (0,55,125)),
    design(em_m, (0,98,58), (18,138,78), (0,40,20), (218,198,38), (168,152,22), (0,78,158), (0,48,118)),
    design(em_m, (0,108,68), (28,148,88), (0,48,28), (228,208,48), (178,162,30), (0,88,168), (0,58,128)),
]

# === FIXED EXISTING DESIGNS (with proper outlines) ===
# These had the "brown outline" issue - now index 8 matches clothing hue

designs["BlueTeal_Fix"] = [
    design(em_m, (74,90,131), (98,123,156), (24,41,82), (90,210,190), (60,156,140), (90,98,255), (65,65,197)),
]
designs["Forest_Fix"] = [
    design(em_m, (50,95,60), (70,120,80), (20,48,30), (180,140,80), (130,100,55), (190,60,50), (140,40,35)),
]
designs["Rocket_Fix"] = [
    design(em_m, (55,55,65), (80,80,90), (18,18,28), (180,180,190), (120,120,130), (220,50,50), (160,30,30), (200,200,210)),
]
designs["Shadow_Fix"] = [
    design(em_m, (45,35,55), (60,50,70), (18,12,28), (100,80,110), (70,55,80), (140,50,120), (100,30,85), (160,150,170)),
]
designs["Royal_Fix"] = [
    design(em_m, (75,35,120), (100,50,150), (35,12,70), (200,180,60), (160,140,40), (180,50,180), (130,30,130), (220,210,230)),
]
designs["Ocean_Fix"] = [
    design(em_m, (28,75,108), (40,100,130), (10,35,62), (80,190,170), (50,145,130), (40,160,200), (25,120,160), (190,220,230)),
]
designs["Sakura_Fix"] = [
    design(em_m, (150,100,120), (180,130,150), (65,42,55), (220,180,190), (180,140,150), (230,100,130), (185,60,90), (240,220,230), (250,235,240)),
]
designs["Mewtwo_Fix"] = [
    design(em_m, (110,90,135), (140,120,160), (48,35,68), (180,160,200), (140,120,165), (160,80,180), (120,50,140), (210,200,220)),
]
designs["Kyogre_Fix"] = [
    design(em_m, (20,42,115), (30,60,140), (6,16,52), (80,160,220), (50,120,180), (220,40,50), (170,25,32), (180,200,230)),
]
designs["Groudon_Fix"] = [
    design(em_m, (110,28,22), (140,40,35), (48,10,8), (200,160,50), (155,120,30), (220,60,40), (170,35,22), (200,180,165)),
]
designs["Lugia_Fix"] = [
    design(em_m, (40,42,78), (55,58,100), (16,18,38), (150,180,210), (110,140,175), (200,30,50), (150,18,32), (210,215,225)),
]

# ============================================================
# RENDER
# ============================================================
print("Rendering {} design groups...".format(len(designs)))

for dname, attempts in designs.items():
    # For each design, render a row of attempts showing front+back for male
    n = len(attempts)
    cell_w = 136
    cell_h = 80
    row = Image.new("RGBA", (n * cell_w, cell_h), (225, 225, 220, 255))

    for i, pal in enumerate(attempts):
        xo = i * cell_w
        # Also make female version (same palette on female sprite)
        mf = apply_pal(sprites["em_m_front"], pal)
        mb = apply_pal(sprites["em_m_back"], pal)
        row.paste(mf, (xo + 2, xo and 8 or 8), mf)
        row.paste(mb, (xo + 68, 8), mb)

    row.save(os.path.join(OUT, "{}_attempts.png".format(dname)))

    # Save best attempt (first one) individual sprites
    best = attempts[0]
    apply_pal(sprites["em_m_front"], best).save(os.path.join(OUT, "{}_M_front.png".format(dname)))
    apply_pal(sprites["em_m_back"], best).save(os.path.join(OUT, "{}_M_back.png".format(dname)))
    apply_pal(sprites["em_f_front"], best).save(os.path.join(OUT, "{}_F_front.png".format(dname)))
    apply_pal(sprites["em_f_back"], best).save(os.path.join(OUT, "{}_F_back.png".format(dname)))

print("Done! All renders in tools/output/styles/")

# Big master grid of first attempts
all_first = [(name, attempts[0]) for name, attempts in designs.items()]
cols = 6
rows_n = (len(all_first) + cols - 1) // cols
cell_w = 140
cell_h = 85
master = Image.new("RGBA", (cols * cell_w, rows_n * cell_h), (235, 235, 230, 255))

for idx, (name, pal) in enumerate(all_first):
    col = idx % cols
    row = idx // cols
    xo = col * cell_w
    yo = row * cell_h
    for py in range(cell_h):
        for px in range(cell_w):
            master.putpixel((xo + px, yo + py), (218, 218, 212, 255))
    for px in range(cell_w):
        master.putpixel((xo + px, yo), (185, 185, 180, 255))
    for py in range(cell_h):
        master.putpixel((xo, yo + py), (185, 185, 180, 255))

    mf = apply_pal(sprites["em_m_front"], pal)
    mb = apply_pal(sprites["em_m_back"], pal)
    master.paste(mf, (xo + 4, yo + 10), mf)
    master.paste(mb, (xo + 72, yo + 10), mb)

master.save(os.path.join(OUT, "master_grid.png"))
print("Saved master_grid.png")
