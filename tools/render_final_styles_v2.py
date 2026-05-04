#!/usr/bin/env python3
"""Generate refined player style variants for both EM and RS, each in its own folder."""
from PIL import Image
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
OUT = os.path.join(SCRIPT_DIR, "output", "styles")
os.makedirs(OUT, exist_ok=True)

def read_jasc(path):
    with open(path) as f:
        lines = f.read().strip().split("\n")
    c = []
    for line in lines[3:]:
        p = line.strip().split()
        if len(p) >= 3:
            c.append((int(p[0]), int(p[1]), int(p[2])))
    return c[:16]

def apply_pal(img, pal):
    w, h = img.size
    px = list(img.getdata())
    out = Image.new("RGBA", (w, h))
    op = []
    for idx in px:
        if idx == 0: op.append((0, 0, 0, 0))
        elif idx < len(pal): op.append(pal[idx] + (255,))
        else: op.append((0, 0, 0, 0))
    out.putdata(op)
    return out

def darken(c, f=0.6):
    return (int(c[0]*f), int(c[1]*f), int(c[2]*f))

def lighten(c, f=1.3):
    return (min(255,int(c[0]*f)), min(255,int(c[1]*f)), min(255,int(c[2]*f)))

# Load ALL base palettes
em_m = read_jasc(os.path.join(REPO_ROOT, "graphics/trainers/palettes/brendan.pal"))
em_f = read_jasc(os.path.join(REPO_ROOT, "graphics/trainers/palettes/may.pal"))
rs_m = read_jasc(os.path.join(REPO_ROOT, "graphics/trainers/palettes/brendan_rs.pal"))
rs_f = read_jasc(os.path.join(REPO_ROOT, "graphics/trainers/palettes/may_rs.pal"))

# Load ALL sprites
S = {}
for prefix, char in [("em_m", "brendan"), ("em_f", "may"), ("rs_m", "brendan_rs"), ("rs_f", "may_rs")]:
    S[prefix + "_front"] = Image.open(os.path.join(REPO_ROOT, "graphics/trainers/front_pics/{}.png".format(char)))
    S[prefix + "_back"] = Image.open(os.path.join(REPO_ROOT, "graphics/trainers/back_pics/{}.png".format(char)))

# Palette index roles:
# EM Brendan: 5-8 clothes, 9 white parts, 10-11 bag/bandana, 12-13 accents, 14 hat white
# EM May: 5-6 clothes, 7-8 HAIR, 9 light clothes, 10-11 bag/bandana, 12-13 accents, 14 hat white
# RS Brendan: same as EM Brendan
# RS May: 5-6 clothes, 7-8 HAIR, same rest

def make_design(clothes_main, clothes_light=None, clothes_outline=None,
                bag_main=None, bag_shadow=None, accent_main=None, accent_dark=None,
                white_parts=None, hat_white=None, hair=None, grayscale_skin=False):
    """Returns dict of 4 palettes: em_m, em_f, rs_m, rs_f."""
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

    result = {}
    for style, base_m, base_f in [("em", em_m, em_f), ("rs", rs_m, rs_f)]:
        male = list(base_m)
        male[5] = cl; male[6] = cm; male[7] = cu; male[8] = co
        male[9] = wp; male[10] = bm; male[11] = bs
        male[12] = am; male[13] = ad; male[14] = hw

        female = list(base_f)
        female[5] = cl; female[6] = cm
        if hair:
            female[7] = hair[0]; female[8] = hair[1]
        female[9] = wp; female[10] = bm; female[11] = bs
        female[12] = am; female[13] = ad; female[14] = hw

        if grayscale_skin:
            for p in [male, female]:
                for i in [1,2,3,4]:
                    r,g,b = p[i]
                    gray = (r+g+b)//3
                    p[i] = (gray, gray, gray)
            if not hair:
                # Also grayscale May's hair
                for i in [7,8]:
                    r,g,b = female[i]
                    gray = (r+g+b)//3
                    female[i] = (gray, gray, gray)

        result[style + "_m"] = male
        result[style + "_f"] = female

    return result

# ============================================================
# DESIGNS - applying all user feedback
# ============================================================
designs = {}

# --- APPROVED (1 attempt, just need RS version) ---
designs["BlueTeal"] = [make_design((74,90,131), (98,123,156), (24,38,62), (90,210,190), (48,135,122), (90,98,255), (65,65,197))]
designs["Forest"] = [make_design((50,95,60), (70,120,80), (20,42,25), (180,140,80), (110,85,45), (190,60,50), (140,40,35))]
designs["Rocket"] = [make_design((55,55,65), (80,80,90), (18,18,28), (180,180,190), (110,110,118), (220,50,50), (160,30,30), (200,200,210))]
designs["Shadow"] = [make_design((45,35,55), (60,50,70), (18,12,28), (100,80,110), (58,44,65), (140,50,120), (100,30,85), (160,150,170))]
designs["Royal"] = [make_design((75,35,120), (100,50,150), (30,12,55), (200,180,60), (125,112,32), (180,50,180), (130,30,130), (220,210,230))]
designs["Ocean"] = [make_design((28,75,108), (40,100,130), (10,30,50), (80,190,170), (42,118,108), (40,160,200), (25,120,160), (190,220,230))]
designs["Mewtwo"] = [make_design((110,90,135), (140,120,160), (48,35,68), (180,160,200), (112,98,128), (160,80,180), (120,50,140), (210,200,220))]
designs["Kyogre"] = [make_design((20,42,115), (30,60,140), (6,16,52), (80,160,220), (42,98,140), (220,40,50), (170,25,32), (180,200,230))]
designs["Groudon"] = [make_design((110,28,22), (140,40,35), (48,10,8), (200,160,50), (125,98,28), (220,60,40), (170,35,22), (200,180,165))]
designs["Lugia"] = [make_design((40,42,78), (55,58,100), (16,18,38), (150,180,210), (92,112,132), (200,30,50), (150,18,32), (210,215,225))]
designs["Crystal"] = [make_design((50,90,150), (82,122,182), (18,35,65), (175,58,78), (105,30,42), (215,75,95), (165,48,65))]
designs["HoOh"] = [make_design((185,125,45), (225,160,65), (85,52,18), (205,65,45), (125,35,20), (235,85,55), (185,52,30))]
designs["Cyndaquil"] = [make_design((45,55,85), (72,82,118), (16,20,38), (230,140,40), (142,85,18), (200,80,40), (150,48,22), (180,185,200))]
designs["Brazil"] = [make_design((0,100,60), (20,140,80), (0,42,22), (220,200,40), (138,125,20), (0,80,160), (0,50,120), (200,225,200))]
designs["Treecko"] = [make_design((50,120,60), (80,160,85), (18,52,22), (100,180,80), (58,110,48), (200,60,50), (150,35,28))]
designs["Ethan"] = [make_design((180,50,50), (220,80,70), (80,20,20), (220,190,60), (140,120,32), (200,60,55), (150,35,32), (230,220,200))]
designs["Lyra"] = [make_design((140,80,60), (180,110,85), (60,32,22), (200,50,50), (120,28,28), (220,70,60), (170,42,35), (235,225,210), (245,238,225))]

# --- NEEDS REFINEMENT (5 new attempts with feedback) ---

# CHIKORITA: more cream yellow
designs["Chikorita"] = [
    make_design((95,150,70), (130,188,100), (38,65,28), (200,190,90), (128,120,52), (115,165,80), (72,115,48), (225,235,210)),
    make_design((90,145,65), (125,182,95), (35,62,25), (210,198,100), (135,125,58), (110,160,75), (68,110,44)),
    make_design((100,155,75), (135,192,105), (40,68,30), (195,185,85), (122,115,48), (120,170,85), (75,118,50)),
    make_design((88,142,62), (122,178,92), (32,58,22), (215,205,108), (140,130,62), (108,155,72), (65,108,42)),
    make_design((98,148,68), (132,185,98), (38,64,26), (205,195,95), (130,122,55), (118,168,82), (70,112,46)),
]

# GRAYSCALE: skin and hair also grayscale
designs["Grayscale"] = [
    make_design((120,120,120), (160,160,160), (48,48,48), (180,180,180), (110,110,110), (140,140,140), (95,95,95), (210,210,210), None, ((130,130,130),(65,65,65)), True),
    make_design((110,110,110), (150,150,150), (42,42,42), (170,170,170), (102,102,102), (130,130,130), (88,88,88), (200,200,200), None, ((125,125,125),(60,60,60)), True),
    make_design((130,130,130), (170,170,170), (52,52,52), (190,190,190), (118,118,118), (150,150,150), (102,102,102), (220,220,220), None, ((135,135,135),(70,70,70)), True),
    make_design((115,115,115), (155,155,155), (45,45,45), (175,175,175), (108,108,108), (135,135,135), (92,92,92), (208,208,208), None, ((128,128,128),(62,62,62)), True),
    make_design((125,125,125), (165,165,165), (50,50,50), (185,185,185), (115,115,115), (145,145,145), (98,98,98), (215,215,215), None, ((132,132,132),(68,68,68)), True),
]

# MUDKIP: more orange accents
designs["Mudkip"] = [
    make_design((55,100,165), (85,135,200), (20,42,72), (95,160,210), (52,96,130), (230,130,40), (175,85,22), (195,215,232)),
    make_design((50,95,160), (80,130,195), (18,38,68), (90,155,205), (48,92,125), (225,125,35), (170,80,18)),
    make_design((60,105,170), (90,140,205), (22,45,75), (100,165,215), (55,100,135), (235,135,45), (180,88,25)),
    make_design((48,92,158), (78,128,192), (16,36,66), (88,152,202), (46,88,122), (222,122,32), (168,78,16)),
    make_design((62,108,172), (92,142,208), (24,48,78), (102,168,218), (58,102,138), (238,138,48), (182,92,28)),
]

# SAKURA: fix pink bandana shadow too dark
designs["Sakura"] = [
    make_design((150,100,120), (180,130,150), (65,42,55), (220,180,195), (155,125,138), (230,100,130), (185,60,90), (240,220,230), (250,235,240)),
    make_design((145,95,115), (175,125,145), (62,38,52), (215,175,190), (150,120,132), (225,95,125), (180,55,85)),
    make_design((155,105,125), (185,135,155), (68,45,58), (225,185,200), (158,128,142), (235,105,135), (190,65,95)),
    make_design((142,92,112), (172,122,142), (60,36,50), (212,172,188), (148,118,130), (222,92,122), (178,52,82)),
    make_design((158,108,128), (188,138,158), (70,48,60), (228,188,202), (162,132,145), (238,108,138), (192,68,98)),
]

# SHINY HUNTER -> rename to GOLDEN (gold/amber luxury theme)
designs["Golden"] = [
    make_design((160,130,40), (200,168,60), (70,55,14), (220,195,70), (140,122,38), (190,150,50), (145,110,28), (235,228,200), (248,242,225)),
    make_design((155,125,35), (195,162,55), (65,50,12), (215,190,65), (135,118,34), (185,145,45), (140,105,24)),
    make_design((165,135,45), (205,172,65), (72,58,16), (225,200,75), (145,125,40), (195,155,55), (150,115,30)),
    make_design((150,120,32), (190,158,52), (62,48,10), (210,185,60), (130,115,30), (182,140,42), (138,102,22)),
    make_design((168,138,48), (208,175,68), (75,60,18), (228,205,78), (148,128,42), (198,158,58), (152,118,32)),
]

# TOTODILE: less green, more blue, keep red
designs["Totodile"] = [
    make_design((35,85,145), (58,118,180), (12,32,62), (65,150,200), (32,90,128), (200,60,50), (150,35,28), (188,210,228)),
    make_design((30,80,140), (52,112,175), (10,28,58), (60,145,195), (28,86,124), (195,55,45), (145,30,24)),
    make_design((40,90,150), (62,122,185), (14,35,65), (70,155,205), (35,94,132), (205,65,55), (155,38,32)),
    make_design((28,78,138), (50,110,172), (8,26,56), (58,142,192), (26,82,120), (192,52,42), (142,28,22)),
    make_design((42,92,152), (65,125,188), (16,38,68), (72,158,208), (38,96,135), (208,68,58), (158,40,34)),
]

# TORCHIC: fix orange outline, make it darker red-brown
designs["Torchic"] = [
    make_design((200,100,30), (235,138,55), (85,40,10), (225,160,50), (140,98,25), (230,75,35), (178,48,18)),
    make_design((195,95,25), (228,132,48), (80,36,8), (220,155,45), (135,94,22), (225,70,30), (172,42,14)),
    make_design((205,105,35), (240,142,58), (88,42,12), (230,165,55), (145,102,28), (235,80,38), (182,52,20)),
    make_design((190,92,22), (225,128,45), (78,34,6), (218,150,42), (132,90,20), (222,68,28), (168,40,12)),
    make_design((208,108,38), (242,145,62), (90,45,14), (232,168,58), (148,105,30), (238,82,42), (185,55,22)),
]

# AQUA: remove teal, very dark blue throughout
designs["Aqua"] = [
    make_design((25,50,120), (42,72,155), (8,18,52), (50,80,170), (25,42,105), (35,60,150), (18,32,100), (180,195,225), (210,222,242)),
    make_design((20,45,115), (38,65,148), (6,15,48), (45,75,165), (22,38,100), (30,55,145), (15,28,95), (175,190,220)),
    make_design((28,55,125), (45,78,160), (10,20,55), (55,85,175), (28,45,108), (38,65,155), (20,35,105), (185,200,230)),
    make_design((18,42,112), (35,62,145), (5,14,45), (42,72,162), (20,36,98), (28,52,142), (14,26,92), (172,188,218)),
    make_design((30,58,128), (48,80,162), (12,22,58), (58,88,178), (30,48,112), (40,68,158), (22,38,108), (188,202,232)),
]

# MAGMA: more black/very dark red
designs["Magma"] = [
    make_design((90,15,15), (130,32,30), (30,5,5), (170,40,30), (95,18,12), (150,30,25), (100,15,10), (160,148,145), (185,178,175)),
    make_design((80,12,12), (120,28,25), (25,3,3), (160,35,25), (88,14,10), (140,25,20), (90,12,8), (155,142,140)),
    make_design((95,18,18), (135,35,32), (32,6,6), (175,45,32), (98,20,14), (155,32,28), (105,18,12), (165,152,148)),
    make_design((75,10,10), (115,25,22), (22,2,2), (155,32,22), (84,12,8), (135,22,18), (85,10,6), (150,138,135)),
    make_design((100,20,20), (140,38,35), (35,7,7), (180,48,35), (102,22,15), (160,35,30), (110,20,14), (168,155,150)),
]

# ============================================================
# RENDER
# ============================================================
for dname, attempts in designs.items():
    folder = os.path.join(OUT, dname)
    # Clear old files
    if os.path.exists(folder):
        for f in os.listdir(folder):
            os.remove(os.path.join(folder, f))
    os.makedirs(folder, exist_ok=True)

    for i, pals in enumerate(attempts):
        num = i + 1

        # Render all 8 sprites (EM + RS, M + F, front + back)
        for style in ["em", "rs"]:
            for gender in ["m", "f"]:
                key = style + "_" + gender
                pal = pals[key]
                for view in ["front", "back"]:
                    sprite_key = key + "_" + view
                    img = apply_pal(S[sprite_key], pal)
                    fname = "attempt{}_{}_{}_{}.png".format(num, style.upper(), gender.upper(), view)
                    img.save(os.path.join(folder, fname))

        # Composite preview: EM(M front, M back, F front, F back) + RS(same)
        comp = Image.new("RGBA", (560, 72), (220, 220, 215, 255))
        x = 4
        for style in ["em", "rs"]:
            for gender in ["m", "f"]:
                for view in ["front", "back"]:
                    key = style + "_" + gender
                    pal = pals[key]
                    sprite_key = key + "_" + view
                    img = apply_pal(S[sprite_key], pal)
                    comp.paste(img, (x, 4), img)
                    x += 68
            x += 8  # gap between EM and RS

        comp.save(os.path.join(folder, "attempt{}_preview.png".format(num)))

    print("  {} - {} attempts (EM + RS)".format(dname, len(attempts)))

print("\nDone! All styles in tools/output/styles/ with both EM and RS versions")
