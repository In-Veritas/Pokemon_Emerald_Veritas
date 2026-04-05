#!/usr/bin/env python3
"""Generate dark Salamence shiny preview."""
from PIL import Image
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
OUT = os.path.join(SCRIPT_DIR, "output")
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

# Load sprites
front = Image.open(os.path.join(REPO_ROOT, "graphics/pokemon/salamence/front.png"))
back = Image.open(os.path.join(REPO_ROOT, "graphics/pokemon/salamence/back.png"))
anim = Image.open(os.path.join(REPO_ROOT, "graphics/pokemon/salamence/anim_front.png"))

# Current normal palette
normal = read_jasc(os.path.join(REPO_ROOT, "graphics/pokemon/salamence/normal.pal"))
# Current shiny palette
shiny_current = read_jasc(os.path.join(REPO_ROOT, "graphics/pokemon/salamence/shiny.pal"))

# New dark dragon shiny palette
# Index 0: transparent (keep)
# Index 1-4: body (blue in normal, green in shiny) -> dark grey/black
# Index 5-8: underbelly/accents (brown/tan) -> dark charcoal with subtle warmth
# Index 9-11: wing/membrane (grey) -> darker grey
# Index 12: white highlights -> pale silver
# Index 13: black outline -> keep
# Index 14: yellow eyes -> deep crimson red eyes
# Index 15: dark grey -> keep
dark_dragon = list(normal)  # start from normal as base
# ONLY change indices 1-4 (body) and 14 (eyes)
# Everything else stays EXACTLY the same as normal
dark_dragon[1] = (30, 30, 40)       # body shadow (darker)
dark_dragon[2] = (60, 60, 75)       # body mid-dark (darker)
dark_dragon[3] = (105, 105, 125)    # body mid-light (kept readable)
dark_dragon[4] = (160, 160, 180)    # body highlight (not too dark)
# Wings (indices 5-8): livid red, same brightness as normal brown
dark_dragon[5] = (130, 45, 40)      # dark wing red
dark_dragon[6] = (195, 75, 60)      # mid wing red
dark_dragon[7] = (225, 100, 75)     # light wing red
dark_dragon[8] = (255, 130, 95)     # highlight wing red
dark_dragon[14] = (200, 40, 40)     # crimson red eyes

# Generate previews
bg_color = (40, 40, 45, 255)

# Front
img_normal = apply_pal(front, normal)
img_shiny_old = apply_pal(front, shiny_current)
img_shiny_new = apply_pal(front, dark_dragon)

# Back
img_back_normal = apply_pal(back, normal)
img_back_shiny = apply_pal(back, dark_dragon)

# Anim front
img_anim_normal = apply_pal(anim, normal)
img_anim_shiny = apply_pal(anim, dark_dragon)

# Composite: normal front, old shiny front, new shiny front
comp_w = 64 * 3 + 16
comp_h = 64 + 8
comp = Image.new("RGBA", (comp_w, comp_h), bg_color)
comp.paste(img_normal, (4, 4), img_normal)
comp.paste(img_shiny_old, (72, 4), img_shiny_old)
comp.paste(img_shiny_new, (140, 4), img_shiny_new)
comp.save(os.path.join(OUT, "salamence_shiny_front_compare.png"))

# Composite: new shiny front + back + anim
comp2_w = 64 * 3 + 16
comp2_h = 128 + 8
comp2 = Image.new("RGBA", (comp2_w, comp2_h), bg_color)
comp2.paste(img_shiny_new, (4, 36), img_shiny_new)
comp2.paste(img_back_shiny, (72, 36), img_back_shiny)
comp2.paste(img_anim_shiny, (140, 4), img_anim_shiny)
comp2.save(os.path.join(OUT, "salamence_shiny_dark_all.png"))

print("Generated:")
print("  tools/output/salamence_shiny_front_compare.png (normal | old shiny | dark dragon)")
print("  tools/output/salamence_shiny_dark_all.png (front | back | anim)")
