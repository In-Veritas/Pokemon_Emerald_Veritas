#!/usr/bin/env python3
"""Fix surfing sprite shiny palettes to match redesigned overworld shiny palettes."""
import os, sys
from PIL import Image

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SURFABLE_DIR = os.path.join(REPO, 'graphics', 'object_events', 'pics', 'pokemon', 'surfable')
POKEMON_DIR = os.path.join(REPO, 'graphics', 'pokemon')

def read_jasc_pal(path):
    colors = []
    for line in open(path).read().strip().splitlines()[3:]:
        parts = line.strip().split()
        if len(parts) == 3:
            colors.append((int(parts[0]), int(parts[1]), int(parts[2])))
    while len(colors) < 16:
        colors.append((0, 0, 0))
    return colors[:16]

def get_png_palette(path):
    img = Image.open(path)
    pal = img.getpalette()
    n = min(16, len(pal) // 3)
    colors = [tuple(pal[i*3:i*3+3]) for i in range(n)]
    while len(colors) < 16:
        colors.append((0, 0, 0))
    return colors

def color_dist(a, b):
    return sum((x-y)**2 for x, y in zip(a, b))

def find_closest(color, palette):
    best_i, best_d = 0, float('inf')
    for i, c in enumerate(palette):
        d = color_dist(color, c)
        if d < best_d:
            best_d, best_i = d, i
    return best_i, best_d

SPECIES = {
    'mudkip': '0258_mudkip',
    'marshtomp': '0259_marshtomp',
    'swampert': '0260_swampert',
    'zigzagoon': '0263_zigzagoon',
    'linoone': '0264_linoone',
    'wingull': '0278_wingull',
    'pelipper': '0279_pelipper',
    'wailord': '0321_wailord',
    'kyogre': '0382_kyogre',
    'rayquaza': '0384_rayquaza',
}

fixed = 0
for name, prefix in SPECIES.items():
    ow_normal_path = os.path.join(POKEMON_DIR, name, 'overworld_normal.pal')
    ow_shiny_path = os.path.join(POKEMON_DIR, name, 'overworld_shiny.pal')
    surf_normal_path = os.path.join(SURFABLE_DIR, prefix + '.png')
    surf_shiny_path = os.path.join(SURFABLE_DIR, prefix + '_shiny.png')

    if not os.path.exists(surf_shiny_path):
        print('{}: no surfing shiny sprite, skipping'.format(name))
        continue

    ow_normal = read_jasc_pal(ow_normal_path)
    ow_shiny = read_jasc_pal(ow_shiny_path)
    surf_normal_pal = get_png_palette(surf_normal_path)
    surf_shiny_pal = get_png_palette(surf_shiny_path)

    new_pal = []
    changes = 0
    for si in range(16):
        if si == 0:
            new_pal.append(ow_shiny[0])
            if surf_shiny_pal[si] != ow_shiny[0]:
                changes += 1
            continue
        ow_idx, dist = find_closest(surf_normal_pal[si], ow_normal)
        new_color = ow_shiny[ow_idx]
        if surf_shiny_pal[si] != new_color:
            changes += 1
        new_pal.append(new_color)

    print('{}: {} color changes'.format(name, changes))
    if changes == 0:
        continue

    for si in range(16):
        if surf_shiny_pal[si] != new_pal[si]:
            print('  [{:2d}] {} -> {}'.format(si, surf_shiny_pal[si], new_pal[si]))

    img = Image.open(surf_shiny_path)
    pixels = list(img.getdata())
    w, h = img.size

    new_img = Image.new('P', (w, h))
    flat = []
    for r, g, b in new_pal:
        flat.extend([r, g, b])
    flat.extend([0, 0, 0] * (256 - 16))
    new_img.putpalette(flat)
    new_img.putdata(pixels)
    new_img.save(surf_shiny_path, transparency=img.info.get('transparency', 0))
    print('  -> SAVED')
    fixed += 1

print('\nDone: {} species updated'.format(fixed))
