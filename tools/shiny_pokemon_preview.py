#!/usr/bin/env python3
"""
Shiny Pokemon Preview Tool

Renders a Pokemon's front battle sprite and overworld follower sprite
with the shiny palette applied, without needing to build the ROM.

Usage:
    python tools/shiny_pokemon_preview.py <pokemon_name_or_dex_number>

Examples:
    python tools/shiny_pokemon_preview.py lugia
    python tools/shiny_pokemon_preview.py 249
    python tools/shiny_pokemon_preview.py gardevoir
"""

import os
import sys
import re

try:
    from PIL import Image
except ImportError:
    print("Error: Pillow is required. Install it with: pip install Pillow")
    sys.exit(1)

# Resolve paths relative to the repo root (parent of tools/)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
GRAPHICS_DIR = os.path.join(REPO_ROOT, "graphics", "pokemon")
SPECIES_H = os.path.join(REPO_ROOT, "include", "constants", "species.h")
OUTPUT_DIR = os.path.join(SCRIPT_DIR, "output")


def parse_species_header():
    """Parse include/constants/species.h to build name->number and number->name mappings."""
    name_to_num = {}
    num_to_name = {}

    with open(SPECIES_H, "r") as f:
        for line in f:
            # Match lines like: #define SPECIES_BULBASAUR 1
            m = re.match(r"#define\s+SPECIES_(\w+)\s+(\d+)", line)
            if m:
                name = m.group(1)
                num = int(m.group(2))
                # Skip EGG and meta-defines
                if name in ("NONE", "EGG", "SHINY_TAG", "OVERWORLD_TAG",
                            "OVERWORLD_SHINY_TAG"):
                    continue
                name_to_num[name] = num
                num_to_name[num] = name

    return name_to_num, num_to_name


def species_name_to_folder(species_name):
    """Convert a SPECIES_XXX name (e.g. 'BULBASAUR') to the folder name (e.g. 'bulbasaur')."""
    return species_name.lower()


def parse_jasc_pal(filepath):
    """Parse a JASC-PAL file and return a list of 16 (R, G, B) tuples."""
    colors = []
    with open(filepath, "r") as f:
        lines = f.read().strip().splitlines()

    # Validate header
    if len(lines) < 3 or lines[0].strip() != "JASC-PAL" or lines[1].strip() != "0100":
        raise ValueError("Not a valid JASC-PAL file: {}".format(filepath))

    num_colors = int(lines[2].strip())
    for i in range(3, 3 + num_colors):
        parts = lines[i].strip().split()
        r, g, b = int(parts[0]), int(parts[1]), int(parts[2])
        colors.append((r, g, b))

    # Pad to 16 if fewer colors
    while len(colors) < 16:
        colors.append((0, 0, 0))

    return colors[:16]


def apply_palette_rgba(indexed_img, palette, transparent_index=0):
    """
    Given an indexed (mode 'P') PIL Image and a list of (R,G,B) colors,
    create an RGBA image with the new palette applied.
    Index 0 is treated as transparent.
    """
    width, height = indexed_img.size
    # Get raw pixel index data
    pixels = list(indexed_img.getdata())

    out = Image.new("RGBA", (width, height))
    out_pixels = []
    for idx in pixels:
        if idx == transparent_index:
            out_pixels.append((0, 0, 0, 0))
        elif idx < len(palette):
            r, g, b = palette[idx]
            out_pixels.append((r, g, b, 255))
        else:
            out_pixels.append((0, 0, 0, 0))

    out.putdata(out_pixels)
    return out


def apply_palette_indexed(indexed_img, palette):
    """
    Given an indexed (mode 'P') PIL Image and a list of (R,G,B) colors,
    create a new indexed PNG with the same pixel indices but the new palette.
    This produces a game-ready 4bpp-compatible indexed PNG.
    """
    width, height = indexed_img.size
    # Get raw pixel index data
    pixels = list(indexed_img.getdata())

    out = Image.new("P", (width, height))
    out.putdata(pixels)

    # Build a flat palette list [R0, G0, B0, R1, G1, B1, ...]
    flat_pal = []
    for r, g, b in palette:
        flat_pal.extend([r, g, b])
    # Pad to 256 colors (PIL requires 768 bytes for a full palette)
    while len(flat_pal) < 768:
        flat_pal.extend([0, 0, 0])

    out.putpalette(flat_pal)

    # Preserve transparency: set index 0 as the transparent color
    out.info["transparency"] = 0

    return out


def resolve_pokemon(query, name_to_num, num_to_name):
    """
    Resolve a user query (name or number) to (species_name, dex_number).
    Returns (species_name_upper, dex_number) or (None, None) on failure.
    """
    # Try as a number first
    try:
        num = int(query)
        if num in num_to_name:
            return num_to_name[num], num
        else:
            print("Error: No Pokemon with dex number {}.".format(num))
            return None, None
    except ValueError:
        pass

    # Try as a name (case-insensitive)
    query_upper = query.upper().replace(" ", "_").replace("-", "_")

    # Direct match
    if query_upper in name_to_num:
        return query_upper, name_to_num[query_upper]

    # Try partial/fuzzy: check if any species name contains the query
    matches = []
    for name in name_to_num:
        if name == query_upper:
            return name, name_to_num[name]
        if query_upper in name:
            matches.append(name)

    if len(matches) == 1:
        return matches[0], name_to_num[matches[0]]
    elif len(matches) > 1:
        print("Ambiguous name '{}'. Did you mean one of:".format(query))
        for m in sorted(matches):
            print("  - {} (#{})".format(m.lower(), name_to_num[m]))
        return None, None

    print("Error: Could not find Pokemon '{}'.".format(query))
    return None, None


def main():
    if len(sys.argv) < 2:
        print("Usage: python {} <pokemon_name_or_dex_number>".format(sys.argv[0]))
        print("Examples:")
        print("  python {} lugia".format(sys.argv[0]))
        print("  python {} 249".format(sys.argv[0]))
        print("  python {} gardevoir".format(sys.argv[0]))
        sys.exit(1)

    query = " ".join(sys.argv[1:])

    # Parse species header
    name_to_num, num_to_name = parse_species_header()

    # Resolve the query
    species_name, dex_num = resolve_pokemon(query, name_to_num, num_to_name)
    if species_name is None:
        sys.exit(1)

    folder_name = species_name_to_folder(species_name)
    species_dir = os.path.join(GRAPHICS_DIR, folder_name)

    if not os.path.isdir(species_dir):
        print("Error: Graphics directory not found: {}".format(species_dir))
        sys.exit(1)

    print("Pokemon: {} (#{})".format(species_name, dex_num))
    print("Graphics dir: {}".format(species_dir))
    print()

    # Create output directory
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    generated = []

    # --- Front battle sprite with shiny palette ---
    front_png = os.path.join(species_dir, "front.png")
    normal_pal = os.path.join(species_dir, "normal.pal")
    shiny_pal = os.path.join(species_dir, "shiny.pal")

    if os.path.exists(front_png) and os.path.exists(shiny_pal):
        try:
            front_img = Image.open(front_png)
            if front_img.mode != "P":
                print("Warning: front.png is not indexed color (mode={}), "
                      "converting.".format(front_img.mode))
                front_img = front_img.convert("P", colors=16)

            palette = parse_jasc_pal(shiny_pal)
            out_path = os.path.join(OUTPUT_DIR,
                                    "{}_shiny_front.png".format(folder_name))
            rgba_img = apply_palette_rgba(front_img, palette)
            rgba_img.save(out_path)
            generated.append(out_path)
            print("Battle sprite (shiny preview): {}".format(out_path))
        except Exception as e:
            print("Error processing front sprite: {}".format(e))
    else:
        if not os.path.exists(front_png):
            print("Skipping battle sprite: front.png not found")
        if not os.path.exists(shiny_pal):
            print("Skipping battle sprite: shiny.pal not found")

    # --- Normal front sprite for comparison ---
    if os.path.exists(front_png) and os.path.exists(normal_pal):
        try:
            front_img = Image.open(front_png)
            if front_img.mode != "P":
                front_img = front_img.convert("P", colors=16)
            palette = parse_jasc_pal(normal_pal)
            out_path = os.path.join(OUTPUT_DIR,
                                    "{}_normal_front.png".format(folder_name))
            rgba_img = apply_palette_rgba(front_img, palette)
            rgba_img.save(out_path)
            generated.append(out_path)
            print("Battle sprite (normal preview):  {}".format(out_path))
        except Exception as e:
            print("Error processing normal front sprite: {}".format(e))

    # --- Anim front sprite with shiny palette ---
    anim_png = os.path.join(species_dir, "anim_front.png")

    if os.path.exists(anim_png) and os.path.exists(shiny_pal):
        try:
            anim_img = Image.open(anim_png)
            if anim_img.mode != "P":
                anim_img = anim_img.convert("P", colors=16)

            palette = parse_jasc_pal(shiny_pal)
            out_path = os.path.join(OUTPUT_DIR,
                                    "{}_shiny_anim_front.png".format(folder_name))
            rgba_img = apply_palette_rgba(anim_img, palette)
            rgba_img.save(out_path)
            generated.append(out_path)
            print("Anim sprite (shiny preview):   {}".format(out_path))

            if os.path.exists(normal_pal):
                palette = parse_jasc_pal(normal_pal)
                out_path = os.path.join(OUTPUT_DIR,
                                        "{}_normal_anim_front.png".format(folder_name))
                rgba_img = apply_palette_rgba(anim_img, palette)
                rgba_img.save(out_path)
                generated.append(out_path)
                print("Anim sprite (normal preview):  {}".format(out_path))
        except Exception as e:
            print("Error processing anim sprite: {}".format(e))

    # --- Back battle sprite with shiny palette ---
    back_png = os.path.join(species_dir, "back.png")

    if os.path.exists(back_png) and os.path.exists(shiny_pal):
        try:
            back_img = Image.open(back_png)
            if back_img.mode != "P":
                print("Warning: back.png is not indexed color (mode={}), "
                      "converting.".format(back_img.mode))
                back_img = back_img.convert("P", colors=16)

            palette = parse_jasc_pal(shiny_pal)
            out_path = os.path.join(OUTPUT_DIR,
                                    "{}_shiny_back.png".format(folder_name))
            rgba_img = apply_palette_rgba(back_img, palette)
            rgba_img.save(out_path)
            generated.append(out_path)
            print("Back sprite (shiny preview):   {}".format(out_path))
        except Exception as e:
            print("Error processing back sprite: {}".format(e))

        # Also render normal back for comparison
        if os.path.exists(normal_pal):
            try:
                palette = parse_jasc_pal(normal_pal)
                out_path = os.path.join(OUTPUT_DIR,
                                        "{}_normal_back.png".format(folder_name))
                rgba_img = apply_palette_rgba(back_img, palette)
                rgba_img.save(out_path)
                generated.append(out_path)
                print("Back sprite (normal preview):  {}".format(out_path))
            except Exception as e:
                print("Error processing normal back sprite: {}".format(e))
    else:
        if not os.path.exists(back_png):
            print("Skipping back sprite: back.png not found")

    # --- Overworld follower sprite with shiny palette ---
    ow_png = os.path.join(species_dir, "overworld.png")
    ow_shiny_pal = os.path.join(species_dir, "overworld_shiny.pal")

    if os.path.exists(ow_png) and os.path.exists(ow_shiny_pal):
        try:
            ow_img = Image.open(ow_png)
            if ow_img.mode != "P":
                print("Warning: overworld.png is not indexed color (mode={}), "
                      "converting.".format(ow_img.mode))
                ow_img = ow_img.convert("P", colors=16)

            ow_palette = parse_jasc_pal(ow_shiny_pal)

            # RGBA preview for human viewing
            preview_path = os.path.join(
                OUTPUT_DIR,
                "{}_shiny_overworld_preview.png".format(folder_name))
            rgba_ow = apply_palette_rgba(ow_img, ow_palette)
            rgba_ow.save(preview_path)
            generated.append(preview_path)
            print("Overworld sprite (shiny preview): {}".format(preview_path))

            # Game-ready indexed PNG
            gameready_path = os.path.join(
                OUTPUT_DIR,
                "{}_overworld_shiny.png".format(folder_name))
            indexed_ow = apply_palette_indexed(ow_img, ow_palette)
            indexed_ow.save(gameready_path)
            generated.append(gameready_path)
            print("Overworld sprite (game-ready):    {}".format(gameready_path))
        except Exception as e:
            print("Error processing overworld sprite: {}".format(e))
    else:
        if not os.path.exists(ow_png):
            print("Skipping overworld sprite: overworld.png not found")
        if not os.path.exists(ow_shiny_pal):
            print("Skipping overworld sprite: overworld_shiny.pal not found")

    # Summary
    print()
    if generated:
        print("Generated {} file(s) in {}".format(len(generated), OUTPUT_DIR))
        for path in generated:
            print("  -> {}".format(os.path.basename(path)))
    else:
        print("No files were generated.")


if __name__ == "__main__":
    main()
