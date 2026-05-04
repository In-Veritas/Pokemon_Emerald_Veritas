#!/usr/bin/env python3
"""Render player style palette variants."""
from PIL import Image
import os

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

def mk(base, c=None, bag=None, acc=None, lt=None, hat=None):
    p = list(base)
    if c:
        p[5], p[6], p[7], p[8] = c
    if bag:
        p[10], p[11] = bag
    if acc:
        p[12], p[13] = acc
    if lt:
        p[9] = lt
    if hat:
        p[14] = hat
    return p

# Load palettes
rs = read_jasc(os.path.join(REPO_ROOT, "graphics/trainers/palettes/brendan_rs.pal"))
em = read_jasc(os.path.join(REPO_ROOT, "graphics/trainers/palettes/brendan.pal"))

# Load sprites
rs_f = Image.open(os.path.join(REPO_ROOT, "graphics/trainers/front_pics/brendan_rs.png"))
rs_b = Image.open(os.path.join(REPO_ROOT, "graphics/trainers/back_pics/brendan_rs.png"))
em_f = Image.open(os.path.join(REPO_ROOT, "graphics/trainers/front_pics/brendan.png"))
em_b = Image.open(os.path.join(REPO_ROOT, "graphics/trainers/back_pics/brendan.png"))

# All variants: (name, base_type, palette)
V = []

# === ORIGINALS ===
V.append(("01_Original_RS", "rs", rs))
V.append(("02_Original_EM", "em", em))

# === PREVIOUSLY MADE ===
V.append(("03_BlueTeal", "rs", mk(rs, acc=((90,98,255),(65,65,197)), bag=((90,210,190),(60,156,140)))))
V.append(("04_NavyGold", "rs", mk(rs, c=((60,70,120),(40,50,100),(25,35,80),(15,22,60)), bag=((220,180,60),(170,130,40)), acc=((180,40,50),(130,25,35)))))
V.append(("05_Forest", "rs", mk(rs, c=((70,120,80),(50,95,60),(35,70,45),(20,48,30)), bag=((180,140,80),(130,100,55)), acc=((190,60,50),(140,40,35)))))
V.append(("06_Rocket", "rs", mk(rs, c=((80,80,90),(55,55,65),(35,35,45),(18,18,28)), bag=((180,180,190),(120,120,130)), acc=((220,50,50),(160,30,30)), lt=(200,200,210))))

# === TEAM THEMED ===
V.append(("07_Magma", "rs", mk(rs, c=((160,50,50),(130,35,35),(100,25,25),(70,15,15)), bag=((200,160,60),(150,120,40)), acc=((200,50,40),(150,30,25)), lt=(220,200,190), hat=(240,220,200))))
V.append(("08_Aqua", "rs", mk(rs, c=((50,100,160),(35,75,135),(25,55,110),(15,35,85)), bag=((80,180,200),(50,130,160)), acc=((60,140,200),(40,100,160)), lt=(200,220,240), hat=(220,235,250))))

# === 20 NEW VARIANTS ===
V.append(("09_Sunset", "rs", mk(rs, c=((180,100,50),(150,75,35),(120,55,25),(85,35,15)), bag=((220,180,80),(170,130,50)), acc=((230,80,40),(180,50,25)))))
V.append(("10_Arctic", "rs", mk(rs, c=((140,180,210),(110,150,185),(80,120,160),(50,85,130)), bag=((200,220,240),(160,185,210)), acc=((100,170,220),(70,130,185)), lt=(230,240,250))))
V.append(("11_Shadow", "rs", mk(rs, c=((60,50,70),(45,35,55),(30,22,40),(18,12,28)), bag=((100,80,110),(70,55,80)), acc=((140,50,120),(100,30,85)), lt=(160,150,170))))
V.append(("12_Desert", "rs", mk(rs, c=((170,150,110),(140,120,85),(110,90,60),(80,65,40)), bag=((200,170,100),(160,130,70)), acc=((190,120,50),(150,85,30)))))
V.append(("13_Royal", "rs", mk(rs, c=((100,50,150),(75,35,120),(55,22,95),(35,12,70)), bag=((200,180,60),(160,140,40)), acc=((180,50,180),(130,30,130)), lt=(220,210,230))))
V.append(("14_Military", "rs", mk(rs, c=((90,100,70),(70,80,50),(50,60,35),(32,42,22)), bag=((140,120,80),(100,85,55)), acc=((60,60,50),(40,40,32)), lt=(180,185,170))))
V.append(("15_Neon", "rs", mk(rs, c=((40,40,50),(25,25,35),(15,15,25),(8,8,15)), bag=((0,230,180),(0,170,130)), acc=((255,80,200),(200,40,150)), lt=(50,50,60))))
V.append(("16_Volcanic", "rs", mk(rs, c=((50,40,40),(35,25,25),(22,15,15),(12,8,8)), bag=((230,140,30),(180,100,20)), acc=((255,100,30),(200,65,15)), lt=(80,60,55))))
V.append(("17_Ocean", "rs", mk(rs, c=((40,100,130),(28,75,108),(18,55,85),(10,35,62)), bag=((80,190,170),(50,145,130)), acc=((40,160,200),(25,120,160)), lt=(190,220,230))))
V.append(("18_Sakura", "rs", mk(rs, c=((180,130,150),(150,100,120),(120,75,95),(90,50,70)), bag=((220,180,190),(180,140,150)), acc=((230,100,130),(185,60,90)), lt=(240,220,230), hat=(250,235,240))))
V.append(("19_Stealth", "rs", mk(rs, c=((55,60,55),(40,45,40),(28,32,28),(16,20,16)), bag=((70,75,65),(48,52,42)), acc=((45,50,42),(30,34,28)), lt=(100,105,95))))
V.append(("20_Champion", "rs", mk(rs, c=((120,80,30),(95,60,20),(70,42,12),(48,28,8)), bag=((200,170,50),(160,130,35)), acc=((200,40,40),(155,25,25)), lt=(230,215,180), hat=(245,235,210))))
V.append(("21_Cyber", "rs", mk(rs, c=((40,50,60),(28,35,45),(18,24,32),(10,14,22)), bag=((0,200,255),(0,150,200)), acc=((0,255,180),(0,190,130)), lt=(60,70,80))))
V.append(("22_Crimson", "rs", mk(rs, c=((150,40,40),(120,28,28),(90,18,18),(60,10,10)), bag=((180,160,140),(140,120,100)), acc=((200,60,60),(155,35,35)), lt=(200,180,175))))
V.append(("23_Ivory", "rs", mk(rs, c=((200,195,180),(175,168,155),(148,140,128),(118,110,100)), bag=((180,160,120),(145,125,85)), acc=((170,80,60),(130,55,38)), lt=(228,225,218), hat=(240,238,232))))
V.append(("24_Denim", "rs", mk(rs, c=((80,95,140),(58,72,118),(40,52,95),(24,34,70)), bag=((190,150,80),(150,110,55)), acc=((200,60,50),(155,38,30)))))
V.append(("25_Jade", "rs", mk(rs, c=((60,130,100),(42,105,78),(28,80,58),(16,55,38)), bag=((180,170,100),(140,130,70)), acc=((50,150,110),(32,110,78)), lt=(190,215,200))))
V.append(("26_Burgundy", "rs", mk(rs, c=((120,40,60),(95,28,45),(70,18,32),(48,10,22)), bag=((180,150,100),(140,110,70)), acc=((160,50,50),(120,30,30)), lt=(200,185,185))))
V.append(("27_Storm", "rs", mk(rs, c=((80,85,100),(58,62,80),(40,42,60),(24,26,42)), bag=((160,165,180),(120,125,140)), acc=((120,140,180),(85,100,145)), lt=(195,200,210))))
V.append(("28_Ember", "rs", mk(rs, c=((140,70,30),(115,50,18),(88,35,10),(60,22,5)), bag=((210,150,50),(170,110,30)), acc=((230,90,30),(185,60,15)), lt=(220,195,170))))

# === 10 THEMATIC (Pokemon-inspired) ===
V.append(("T01_Pikachu", "rs", mk(rs, c=((220,190,40),(185,155,25),(150,120,15),(110,85,8)), bag=((230,200,50),(190,160,30)), acc=((200,50,30),(150,30,18)), lt=(245,235,180), hat=(250,240,200))))
V.append(("T02_Gengar", "rs", mk(rs, c=((100,60,130),(75,42,100),(55,28,75),(35,16,52)), bag=((140,80,160),(100,55,120)), acc=((200,50,60),(150,30,40)), lt=(170,150,185))))
V.append(("T03_Charizard", "rs", mk(rs, c=((210,120,40),(175,90,25),(140,65,15),(100,42,8)), bag=((230,180,60),(185,140,35)), acc=((230,60,30),(180,35,18)), lt=(240,210,170))))
V.append(("T04_Mewtwo", "rs", mk(rs, c=((140,120,160),(110,90,135),(82,65,108),(55,40,80)), bag=((180,160,200),(140,120,165)), acc=((160,80,180),(120,50,140)), lt=(210,200,220))))
V.append(("T05_Rayquaza", "rs", mk(rs, c=((40,80,40),(28,60,28),(18,42,18),(10,28,10)), bag=((200,180,40),(155,135,25)), acc=((190,50,40),(145,30,25)), lt=(160,180,155))))
V.append(("T06_Kyogre", "rs", mk(rs, c=((30,60,140),(20,42,115),(12,28,90),(6,16,65)), bag=((80,160,220),(50,120,180)), acc=((220,40,50),(170,25,32)), lt=(180,200,230))))
V.append(("T07_Groudon", "rs", mk(rs, c=((140,40,35),(110,28,22),(82,18,14),(55,10,8)), bag=((200,160,50),(155,120,30)), acc=((220,60,40),(170,35,22)), lt=(200,180,165))))
V.append(("T08_Lugia", "rs", mk(rs, c=((55,58,100),(40,42,78),(30,32,58),(20,22,42)), bag=((150,180,210),(110,140,175)), acc=((200,30,50),(150,18,32)), lt=(210,215,225))))
V.append(("T09_Eevee", "rs", mk(rs, c=((170,140,100),(140,110,75),(110,82,52),(78,58,35)), bag=((200,175,130),(165,140,95)), acc=((190,140,80),(150,105,55)), lt=(230,220,200), hat=(240,232,215))))
V.append(("T10_Umbreon", "rs", mk(rs, c=((40,40,48),(28,28,35),(18,18,24),(10,10,15)), bag=((220,200,50),(175,155,30)), acc=((220,200,50),(175,155,30)), lt=(60,55,70))))

# === RENDER BIG GRID ===
cols = 6
rows_needed = (len(V) + cols - 1) // cols
cell_w = 140
cell_h = 80
grid = Image.new("RGBA", (cols * cell_w, rows_needed * cell_h), (235, 235, 230, 255))

for idx, (name, btype, pal) in enumerate(V):
    col = idx % cols
    row = idx // cols
    xo = col * cell_w
    yo = row * cell_h

    # Cell bg
    for py in range(cell_h):
        for px in range(cell_w):
            grid.putpixel((xo + px, yo + py), (218, 218, 212, 255))
    # Border
    for px in range(cell_w):
        grid.putpixel((xo + px, yo), (185, 185, 180, 255))
    for py in range(cell_h):
        grid.putpixel((xo, yo + py), (185, 185, 180, 255))

    f = rs_f if btype == "rs" else em_f
    b = rs_b if btype == "rs" else em_b

    fr = apply_pal(f, pal)
    br = apply_pal(b, pal)
    grid.paste(fr, (xo + 4, yo + 8), fr)
    grid.paste(br, (xo + 72, yo + 8), br)

grid.save(os.path.join(OUT, "all_variants_grid.png"))
print("Saved all_variants_grid.png ({} variants, {}x{})".format(len(V), cols * cell_w, rows_needed * cell_h))

# Save individual renders too
for name, btype, pal in V:
    f = rs_f if btype == "rs" else em_f
    b = rs_b if btype == "rs" else em_b
    apply_pal(f, pal).save(os.path.join(OUT, "{}_front.png".format(name)))
    apply_pal(b, pal).save(os.path.join(OUT, "{}_back.png".format(name)))

print("Saved {} individual front+back renders".format(len(V)))
