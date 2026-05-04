#!/usr/bin/env python3
"""Update overworld shiny palettes to match modified battle shiny palettes."""

import os
import sys

def parse_pal(path):
    colors = []
    with open(path) as f:
        lines = f.readlines()
    for line in lines:
        parts = line.strip().split()
        if len(parts) == 3:
            try:
                colors.append(tuple(int(x) for x in parts))
            except ValueError:
                pass
    return colors

def closest_idx(color, palette):
    best_dist = float('inf')
    best_i = 0
    for i, c in enumerate(palette):
        d = sum((a - b) ** 2 for a, b in zip(color, c))
        if d < best_dist:
            best_dist = d
            best_i = i
    return best_i, best_dist

def write_pal(path, colors):
    with open(path, 'w', newline='\n') as f:
        f.write('JASC-PAL\n0100\n16\n')
        for c in colors:
            f.write('{} {} {}\n'.format(c[0], c[1], c[2]))

def transform_overworld(species):
    base = os.path.join('graphics', 'pokemon', species)
    battle_normal = parse_pal(os.path.join(base, 'normal.pal'))
    battle_shiny = parse_pal(os.path.join(base, 'shiny.pal'))
    ow_normal = parse_pal(os.path.join(base, 'overworld_normal.pal'))

    ow_shiny = []
    for i, ow_color in enumerate(ow_normal):
        idx, dist = closest_idx(ow_color, battle_normal)
        bn = battle_normal[idx]
        bs = battle_shiny[idx]
        new_color = tuple(
            max(0, min(255, c_ow + c_bs - c_bn))
            for c_ow, c_bn, c_bs in zip(ow_color, bn, bs)
        )
        ow_shiny.append(new_color)
        print('  [{}] OW:{} -> closest battle[{}]:{} (dist={}) -> shiny:{} -> result:{}'.format(
            i, ow_color, idx, bn, dist, bs, new_color))

    out_path = os.path.join(base, 'overworld_shiny.pal')
    write_pal(out_path, ow_shiny)
    print('  Written to {}'.format(out_path))

if __name__ == '__main__':
    species_list = sys.argv[1:] if len(sys.argv) > 1 else ['gardevoir', 'kyogre', 'groudon']
    for species in species_list:
        print('\n=== {} ==='.format(species.upper()))
        transform_overworld(species)
