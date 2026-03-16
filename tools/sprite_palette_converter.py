#!/usr/bin/env python3
"""
Sprite Palette Converter

Converts a sprite PNG from one palette to another by matching shiny color intent.
Designed for converting back/anim sprites to use the front sprite's palette.

Usage:
    python tools/sprite_palette_converter.py <input_png> <input_shiny_pal> <species>

Example:
    python tools/sprite_palette_converter.py tools/output/Lugia/regular_back.png tools/output/Lugia/shiny_back_pal.act lugia

The tool:
1. Reads the input PNG (indexed or RGBA)
2. Reads the input's shiny palette (.act) to understand intended shiny colors
3. Reads the species' front palette (normal.pal + shiny.pal)
4. Maps each input index to the front index that produces the closest shiny color
5. Saves the converted PNG and renders previews
"""

import os
import sys

try:
    from PIL import Image
except ImportError:
    print("Error: Pillow required. pip install Pillow")
    sys.exit(1)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
GRAPHICS_DIR = os.path.join(REPO_ROOT, "graphics", "pokemon")
OUTPUT_DIR = os.path.join(SCRIPT_DIR, "output")


def read_act(path):
    with open(path, 'rb') as f:
        data = f.read()
    return [(data[i*3], data[i*3+1], data[i*3+2]) for i in range(16)]


def read_jasc_pal(path):
    with open(path) as f:
        lines = f.read().strip().splitlines()
    colors = []
    for line in lines[3:]:
        parts = line.strip().split()
        if len(parts) == 3:
            colors.append((int(parts[0]), int(parts[1]), int(parts[2])))
    while len(colors) < 16:
        colors.append((0, 0, 0))
    return colors[:16]


def color_dist(c1, c2):
    return sum((a - b) ** 2 for a, b in zip(c1, c2))


def apply_palette_rgba(indexed_img, palette):
    width, height = indexed_img.size
    pixels = list(indexed_img.getdata())
    out = Image.new("RGBA", (width, height))
    out_pixels = []
    for idx in pixels:
        if idx == 0:
            out_pixels.append((0, 0, 0, 0))
        elif idx < len(palette):
            r, g, b = palette[idx]
            out_pixels.append((r, g, b, 255))
        else:
            out_pixels.append((0, 0, 0, 0))
    out.putdata(out_pixels)
    return out


def main():
    if len(sys.argv) < 4:
        print("Usage: python {} <input_png> <input_shiny_pal.act> <species>".format(sys.argv[0]))
        print("Example: python {} tools/output/Lugia/regular_back.png tools/output/Lugia/shiny_back_pal.act lugia".format(sys.argv[0]))
        sys.exit(1)

    input_png_path = sys.argv[1]
    input_shiny_pal_path = sys.argv[2]
    species = sys.argv[3].lower()

    species_dir = os.path.join(GRAPHICS_DIR, species)
    front_normal_pal_path = os.path.join(species_dir, "normal.pal")
    front_shiny_pal_path = os.path.join(species_dir, "shiny.pal")

    if not os.path.exists(input_png_path):
        print("Error: {} not found".format(input_png_path))
        sys.exit(1)

    # Read palettes
    input_shiny = read_act(input_shiny_pal_path)
    front_normal = read_jasc_pal(front_normal_pal_path)
    front_shiny = read_jasc_pal(front_shiny_pal_path)

    # Read input PNG
    img = Image.open(input_png_path)
    w, h = img.size
    print("Input: {} (mode={}, size={}x{})".format(input_png_path, img.mode, w, h))

    # Get input palette if indexed, or convert from RGBA
    if img.mode == 'P':
        input_pal = img.getpalette()
        input_colors = [(input_pal[i*3], input_pal[i*3+1], input_pal[i*3+2]) for i in range(16)]
        pixels = list(img.getdata())
        print("Indexed PNG with palette")
    else:
        # RGBA - need to match to input normal palette
        print("RGBA PNG - will match to input shiny palette for index detection")
        print("Warning: RGBA conversion may have fuzzy matches")
        # For RGBA, we don't have the input normal palette easily
        # Just use color matching to the shiny palette
        img_rgba = img.convert('RGBA')
        px = img_rgba.load()
        pixels = []
        input_colors = input_shiny  # Use shiny colors for matching
        for y in range(h):
            for x in range(w):
                r, g, b, a = px[x, y]
                if a < 128:
                    pixels.append(0)
                    continue
                best_i = 0
                best_d = float('inf')
                for i in range(1, 16):
                    d = color_dist((r, g, b), input_colors[i])
                    if d < best_d:
                        best_d = d
                        best_i = i
                pixels.append(best_i)

    # Build mapping: input_index -> front_index by matching shiny colors
    print("\nMapping (by shiny color match):")
    remap = {}
    for ii in range(16):
        target_shiny = input_shiny[ii]

        # Find front index with closest shiny color
        best_fi = 0
        best_d = float('inf')
        for fi in range(16):
            d = color_dist(target_shiny, front_shiny[fi])
            if d < best_d:
                best_d = d
                best_fi = fi

        remap[ii] = best_fi
        exact = " (exact)" if best_d == 0 else " (dist={})".format(best_d)
        print("  input {:2d} shiny {} -> front {:2d} shiny {}{}".format(
            ii, target_shiny, best_fi, front_shiny[best_fi], exact))

    # Apply remapping
    new_pixels = [remap.get(p, 0) for p in pixels]

    # Save converted PNG
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    basename = os.path.splitext(os.path.basename(input_png_path))[0]

    new_img = Image.new('P', (w, h))
    flat_pal = []
    for r, g, b in front_normal:
        flat_pal.extend([r, g, b])
    flat_pal.extend([0, 0, 0] * (256 - 16))
    new_img.putpalette(flat_pal)
    new_img.putdata(new_pixels)

    # Save game-ready PNG
    game_path = os.path.join(OUTPUT_DIR, "{}_converted.png".format(basename))
    new_img.save(game_path, transparency=0)
    print("\nSaved: {}".format(game_path))

    # Render normal preview
    normal_preview = apply_palette_rgba(new_img, front_normal)
    normal_path = os.path.join(OUTPUT_DIR, "{}_normal_preview.png".format(basename))
    normal_preview.save(normal_path)
    print("Saved: {}".format(normal_path))

    # Render shiny preview
    shiny_preview = apply_palette_rgba(new_img, front_shiny)
    shiny_path = os.path.join(OUTPUT_DIR, "{}_shiny_preview.png".format(basename))
    shiny_preview.save(shiny_path)
    print("Saved: {}".format(shiny_path))

    print("\nDone! Check the previews. If they look good, copy the _converted.png to graphics/pokemon/{}/".format(species))


if __name__ == "__main__":
    main()
