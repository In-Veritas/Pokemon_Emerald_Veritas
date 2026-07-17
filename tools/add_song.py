#!/usr/bin/env python3
"""Register a composed .mid as a real song in Pokemon Emerald Veritas.

Takes a MIDI exported from tools/emerald_composer/emerald_composer.html
(or any mid2agb-compatible MIDI) and wires it into the build:

    1. copies it to sound/songs/midi/<label>.mid
    2. appends the mid2agb rule to songs.mk   (voicegroup + volume)
    3. appends `song <label>, 0, 0` to sound/song_table.inc
    4. adds `#define <CONST> <index>` to include/constants/songs.h

After that, build the ROM as usual and play it with e.g. a script command
`playbgm MUS_MYSONG` (or hook it to a map's music in map.json).

Usage:
    python tools/add_song.py path/to/my_song.mid MUS_MYSONG --voicegroup 51
    python tools/add_song.py song.mid MUS_MYSONG -G 51 -V 100 --dry-run

The song label is derived from the constant: MUS_MYSONG -> mus_mysong.
--dry-run prints every planned change without touching anything.
"""
import argparse
import os
import re
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, ".."))


def repo(*parts):
    return os.path.join(ROOT, *parts)


def die(msg):
    print("error: " + msg)
    sys.exit(1)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("midi", help="path to the .mid to register")
    ap.add_argument("const", help="song constant, e.g. MUS_MYSONG")
    ap.add_argument("-G", "--voicegroup", type=int, required=True,
                    help="voicegroup number the song was composed for (0-192)")
    ap.add_argument("-V", "--volume", type=int, default=100,
                    help="master volume for mid2agb (default 100)")
    ap.add_argument("-P", "--priority", type=int, default=None,
                    help="optional player priority (vanilla BGM omits it)")
    ap.add_argument("--dry-run", action="store_true",
                    help="print planned changes without writing")
    args = ap.parse_args()

    if not re.match(r"^MUS_[A-Z0-9_]+$", args.const):
        die("constant must look like MUS_MYSONG (got %r)" % args.const)
    if not os.path.isfile(args.midi):
        die("no such file: %s" % args.midi)
    if not 0 <= args.voicegroup <= 192:
        die("voicegroup must be 0-192")
    label = args.const.lower()
    dest_mid = repo("sound", "songs", "midi", label + ".mid")

    songs_h = repo("include", "constants", "songs.h")
    table = repo("sound", "song_table.inc")
    mk = repo("songs.mk")
    for p in (songs_h, table, mk):
        if not os.path.isfile(p):
            die("repo file missing: %s (run from the repo)" % p)

    h_text = open(songs_h, encoding="utf-8").read()
    t_text = open(table, encoding="utf-8").read()
    mk_text = open(mk, encoding="utf-8").read()

    if re.search(r"#define\s+%s\b" % args.const, h_text):
        die("%s already exists in songs.h" % args.const)
    if re.search(r"^\s*song\s+%s\b" % label, t_text, re.M):
        die("song %s already in song_table.inc" % label)
    if label + ".s" in mk_text:
        die("%s already has a rule in songs.mk" % label)
    if os.path.exists(dest_mid):
        die("%s already exists" % dest_mid)

    # The new song's table index == current number of `song` rows (0-based).
    rows = re.findall(r"^\s*song\s+\w+", t_text, re.M)
    new_index = len(rows)

    # -- songs.mk rule -----------------------------------------------------
    prio = " -P%d" % args.priority if args.priority is not None else ""
    rule = ("\n$(MID_SUBDIR)/%s.s: %%.s: %%.mid\n"
            "\t$(MID) $< $@ -E -R$(STD_REVERB) -G%03d -V%03d%s\n"
            % (label, args.voicegroup, args.volume, prio))

    # -- song_table.inc row (before the trailing dummy_song_header) --------
    anchor = "\n\t.align 2\ndummy_song_header:"
    if anchor not in t_text:
        die("song_table.inc layout unexpected (dummy_song_header anchor missing)")
    t_new = t_text.replace(anchor, "\n\tsong %s, 0, 0%s" % (label, anchor), 1)

    # -- songs.h define ----------------------------------------------------
    define = "#define %-27s %d\n" % (args.const, new_index)
    h_anchor = "\n#define MUS_ROUTE118"
    if h_anchor in h_text:
        h_new = h_text.replace(h_anchor, "\n" + define + h_anchor, 1)
    elif "#endif" in h_text:
        h_new = h_text.replace("#endif", define + "\n#endif", 1)
    else:
        die("songs.h layout unexpected")

    print("Registering %s" % args.const)
    print("  label      : %s" % label)
    print("  table index: %d  (song table currently has %d rows)" % (new_index, len(rows)))
    print("  midi       : %s -> %s" % (args.midi, os.path.relpath(dest_mid, ROOT)))
    print("  songs.mk   : -G%03d -V%03d%s" % (args.voicegroup, args.volume, prio))
    print("  songs.h    : %s" % define.strip())
    if args.dry_run:
        print("dry run — nothing written.")
        return

    shutil.copyfile(args.midi, dest_mid)
    with open(mk, "a", encoding="utf-8", newline="\n") as f:
        f.write(rule)
    open(table, "w", encoding="utf-8", newline="\n").write(t_new)
    open(songs_h, "w", encoding="utf-8", newline="\n").write(h_new)
    print("done. Build the ROM as usual; play it with `playbgm %s` "
          "or set it as a map's music." % args.const)


if __name__ == "__main__":
    main()
