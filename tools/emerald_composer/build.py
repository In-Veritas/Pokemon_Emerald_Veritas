#!/usr/bin/env python3
"""Build the Emerald Composer — a single-file, offline piano-roll sequencer
that plays the actual Pokemon Emerald (Veritas) soundfont.

Reads straight from the repo:
    sound/direct_sound_data.inc      label -> sample file mapping
    sound/direct_sound_samples/*.aif 8-bit PCM + loop points (AIFF chunks)
    sound/voicegroups/*.inc          the 193 instrument banks
    sound/keysplit_tables.inc        key -> sub-voice tables
    sound/programmable_wave_data.inc GB wave channel tables (16-byte .pcm)
    songs.mk                         which songs use which voicegroup (-G)

and injects everything as JSON into template.html, producing
    tools/emerald_composer/emerald_composer.html
Open that file in any browser (no server needed). Compose, then export a
mid2agb-ready .mid and register it with tools/add_song.py.

Usage:
    python tools/emerald_composer/build.py            build the app
    python tools/emerald_composer/build.py --selftest also emit + convert a
                                                      test .mid via mid2agb
                                                      to prove the exporter
                                                      format (needs the exe)
"""
import base64
import json
import os
import re
import struct
import sys
from collections import OrderedDict, defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
OUT_HTML = os.path.join(HERE, "emerald_composer.html")
TEMPLATE = os.path.join(HERE, "template.html")


def repo(*parts):
    return os.path.join(ROOT, *parts)


# --------------------------------------------------------------------------
# AIFF parsing (mirrors what tools/aif2pcm does: 8/16-bit mono AIFF,
# loop = MARK "START"/"END" markers referenced by the INST sustainLoop;
# the END marker also truncates the sample, matching aif2pcm's num_samples).
# --------------------------------------------------------------------------
def read_extended80(b):
    """80-bit IEEE-754 extended float -> float (the AIFF sampleRate field)."""
    sign = -1 if b[0] & 0x80 else 1
    exp = ((b[0] & 0x7F) << 8) | b[1]
    mant = int.from_bytes(b[2:10], "big")
    if exp == 0 and mant == 0:
        return 0.0
    return sign * mant * 2.0 ** (exp - 16383 - 63)


def parse_aiff(path):
    with open(path, "rb") as f:
        data = f.read()
    if data[0:4] != b"FORM" or data[8:12] != b"AIFF":
        raise ValueError("%s: not an AIFF file" % path)
    pos = 12
    frames = None
    rate = None
    bits = 8
    markers = {}
    loop_begin_id = loop_end_id = None
    play_mode = 0
    ssnd = None
    while pos + 8 <= len(data):
        ck = data[pos:pos + 4]
        size = int.from_bytes(data[pos + 4:pos + 8], "big")
        body = data[pos + 8:pos + 8 + size]
        if ck == b"COMM":
            channels, nframes, bits = struct.unpack(">hIh", body[0:8])
            if channels != 1:
                raise ValueError("%s: %d channels (must be mono)" % (path, channels))
            rate = read_extended80(body[8:18])
            frames = nframes
        elif ck == b"MARK":
            (n,) = struct.unpack(">H", body[0:2])
            p = 2
            for _ in range(n):
                mid, mpos = struct.unpack(">HI", body[p:p + 6])
                namelen = body[p + 6]
                p += 7 + namelen
                if (namelen + 1) % 2:
                    p += 1
                markers[mid] = mpos
        elif ck == b"INST":
            # baseNote, detune, lowNote, highNote, lowVel, highVel, gain,
            # sustainLoop(playMode, beginId, endId), releaseLoop(...)
            play_mode, loop_begin_id, loop_end_id = struct.unpack(">HHH", body[8:14])
        elif ck == b"SSND":
            offset, _block = struct.unpack(">II", body[0:8])
            ssnd = body[8 + offset:]
        pos += 8 + size
        if size % 2:
            pos += 1
    if ssnd is None or rate is None:
        raise ValueError("%s: missing SSND/COMM" % path)
    if bits == 8:
        pcm = bytearray(ssnd[:frames])
    elif bits == 16:
        # down-convert like aif2pcm: take the high byte
        pcm = bytearray(frames)
        for i in range(min(frames, len(ssnd) // 2)):
            pcm[i] = ssnd[2 * i]  # big-endian high byte, already signed
    else:
        raise ValueError("%s: %d-bit samples unsupported" % (path, bits))

    loop = False
    loop_start = 0
    if play_mode != 0 and loop_begin_id in markers and loop_end_id in markers:
        loop_start = markers[loop_begin_id]
        loop_end = markers[loop_end_id]
        if 0 < loop_end <= len(pcm) and loop_start < loop_end:
            pcm = pcm[:loop_end]  # aif2pcm: END marker truncates the sample
            loop = True
    return {
        "rate": round(rate, 3),
        "n": len(pcm),
        "loop": 1 if loop else 0,
        "loopStart": loop_start,
        "loopEnd": len(pcm),
        "b64": base64.b64encode(bytes(pcm)).decode("ascii"),
    }


# --------------------------------------------------------------------------
# Repo data parsers
# --------------------------------------------------------------------------
def parse_direct_sound_data():
    """direct_sound_data.inc: 'Label::' + '.incbin \"sound/....bin\"'."""
    out = OrderedDict()
    label = None
    path = repo("sound", "direct_sound_data.inc")
    for line in open(path, encoding="utf-8", errors="replace"):
        m = re.match(r"^(\w+)::", line)
        if m:
            label = m.group(1)
            continue
        m = re.search(r'\.incbin\s+"([^"]+\.bin)"', line)
        if m and label:
            out[label] = m.group(1)[:-4] + ".aif"
            label = None
    return out


def parse_wave_data():
    """programmable_wave_data.inc: 'Label::' + .incbin 16-byte .pcm.
    Each byte = two 4-bit samples, high nibble first."""
    out = OrderedDict()
    label = None
    path = repo("sound", "programmable_wave_data.inc")
    for line in open(path, encoding="utf-8", errors="replace"):
        m = re.match(r"^(\w+)::", line)
        if m:
            label = m.group(1)
            continue
        m = re.search(r'\.incbin\s+"([^"]+)"', line)
        if m and label:
            raw = open(repo(*m.group(1).split("/")), "rb").read()[:16]
            vals = []
            for b in raw:
                vals.append((b >> 4) & 0xF)
                vals.append(b & 0xF)
            out[label] = vals
            label = None
    return out


def parse_keysplit_tables():
    """keysplit_tables.inc uses a negative-offset trick:
        .set KeySplitTableN, . - <firstNote>
        .byte X  @ <note>
    The label points firstNote bytes BEFORE its data, so stream[setPos +
    key - firstNote] is the lookup. Tables share one byte stream."""
    stream = []
    tables = {}  # name -> (streamPosAtSet, firstNote)
    path = repo("sound", "keysplit_tables.inc")
    for line in open(path, encoding="utf-8", errors="replace"):
        m = re.match(r"\s*\.set\s+(\w+)\s*,\s*\.\s*-\s*(\d+)", line)
        if m:
            tables[m.group(1)] = (len(stream), int(m.group(2)))
            continue
        m = re.match(r"\s*\.byte\s+(\d+)", line)
        if m:
            stream.append(int(m.group(1)))
    out = {}
    for name, (pos, first) in tables.items():
        arr = []
        for key in range(128):
            idx = pos + key - first
            arr.append(stream[idx] if 0 <= idx < len(stream) else 0)
        out[name] = arr
    return out


VOICE_RES = [
    ("dsn", re.compile(r"^\s*voice_directsound_no_resample\s+(.+)$")),
    ("dsa", re.compile(r"^\s*voice_directsound_alt\s+(.+)$")),
    ("ds", re.compile(r"^\s*voice_directsound\s+(.+)$")),
    ("sq1a", re.compile(r"^\s*voice_square_1_alt\s+(.+)$")),
    ("sq1", re.compile(r"^\s*voice_square_1\s+(.+)$")),
    ("sq2a", re.compile(r"^\s*voice_square_2_alt\s+(.+)$")),
    ("sq2", re.compile(r"^\s*voice_square_2\s+(.+)$")),
    ("wavea", re.compile(r"^\s*voice_programmable_wave_alt\s+(.+)$")),
    ("wave", re.compile(r"^\s*voice_programmable_wave\s+(.+)$")),
    ("noisea", re.compile(r"^\s*voice_noise_alt\s+(.+)$")),
    ("noise", re.compile(r"^\s*voice_noise\s+(.+)$")),
    ("ks", re.compile(r"^\s*voice_keysplit\s+(.+)$")),
    ("ksall", re.compile(r"^\s*voice_keysplit_all\s+(.+)$")),
]


def parse_voicegroup(path):
    voices = []
    for line in open(path, encoding="utf-8", errors="replace"):
        line = line.split("@")[0].rstrip()
        if not line.strip():
            continue
        matched = None
        for kind, rx in VOICE_RES:
            m = rx.match(line)
            if m:
                matched = (kind, [a.strip() for a in m.group(1).split(",")])
                break
        if not matched:
            if re.match(r"^\s*(cry|cry_reverse)\s", line):
                voices.append({"t": "none"})
            continue
        kind, args = matched
        try:
            if kind in ("ds", "dsn", "dsa"):
                voices.append({
                    "t": "dsn" if kind == "dsn" else "ds",
                    "base": int(args[0]), "pan": int(args[1]), "smp": args[2],
                    "a": int(args[3]), "d": int(args[4]),
                    "s": int(args[5]), "r": int(args[6]),
                })
            elif kind in ("sq1", "sq1a"):
                voices.append({
                    "t": "sq1", "base": int(args[0]), "pan": int(args[1]),
                    "sweep": int(args[2]), "duty": int(args[3], 0),
                    "a": int(args[4]), "d": int(args[5]),
                    "s": int(args[6]), "r": int(args[7]),
                })
            elif kind in ("sq2", "sq2a"):
                voices.append({
                    "t": "sq2", "base": int(args[0]), "pan": int(args[1]),
                    "duty": int(args[2], 0),
                    "a": int(args[3]), "d": int(args[4]),
                    "s": int(args[5]), "r": int(args[6]),
                })
            elif kind in ("wave", "wavea"):
                voices.append({
                    "t": "wave", "base": int(args[0]), "pan": int(args[1]),
                    "wav": args[2],
                    "a": int(args[3]), "d": int(args[4]),
                    "s": int(args[5]), "r": int(args[6]),
                })
            elif kind in ("noise", "noisea"):
                voices.append({
                    "t": "noise", "base": int(args[0]), "pan": int(args[1]),
                    "period": int(args[2]),
                    "a": int(args[3]), "d": int(args[4]),
                    "s": int(args[5]), "r": int(args[6]),
                })
            elif kind == "ks":
                voices.append({"t": "ks", "grp": args[0], "tbl": args[1]})
            elif kind == "ksall":
                voices.append({"t": "ksall", "grp": args[0]})
        except (ValueError, IndexError) as e:
            print("  warning: %s: bad voice line %r (%s)" % (path, line, e))
            voices.append({"t": "none"})
    # pad/trim to 128 slots
    while len(voices) < 128:
        voices.append({"t": "none"})
    return voices[:128]


def parse_songs_mk():
    """songs.mk: per-song mid2agb rules -> {voicegroup int: [song names]}."""
    usage = defaultdict(list)
    text = open(repo("songs.mk"), encoding="utf-8", errors="replace").read()
    for m in re.finditer(
            r"\$\(MID_SUBDIR\)/(\w+)\.s:.*?\n\t\$\(MID\)[^\n]*?-G(\d+)",
            text):
        usage[int(m.group(2))].append(m.group(1))
    return usage


# --------------------------------------------------------------------------
# Self-test: emit a .mid with the exact byte layout the app's JS exporter
# uses, then run tools/mid2agb on it to prove the format is accepted.
# --------------------------------------------------------------------------
def vlq(n):
    out = [n & 0x7F]
    n >>= 7
    while n:
        out.insert(0, (n & 0x7F) | 0x80)
        n >>= 7
    return out


def midi_track(events):
    events.sort(key=lambda e: (e[0], e[1]))
    out = []
    last = 0
    for t, _ord, byts in events:
        out += vlq(t - last) + list(byts)
        last = t
    out += vlq(0) + [0xFF, 0x2F, 0x00]
    return [0x4D, 0x54, 0x72, 0x6B,
            (len(out) >> 24) & 255, (len(out) >> 16) & 255,
            (len(out) >> 8) & 255, len(out) & 255] + out


def build_test_midi(path):
    PPQN = 24
    meta = []
    name = b"emerald-composer"
    meta.append((0, 0, [0xFF, 0x03] + vlq(len(name)) + list(name)))
    mpqn = round(60000000 / 120)
    meta.append((0, 1, [0xFF, 0x51, 0x03,
                        (mpqn >> 16) & 255, (mpqn >> 8) & 255, mpqn & 255]))
    meta.append((0, 2, [0xFF, 0x58, 0x04, 4, 2, 24, 8]))
    meta.append((0, 3, [0xFF, 0x06, 1, ord("[")]))
    meta.append((PPQN * 8, 3, [0xFF, 0x06, 1, ord("]")]))
    trk = []
    trk.append((0, 1, [0xC0, 17]))          # program change -> voice 17
    trk.append((0, 2, [0xB0, 7, 100]))      # CC7 volume
    trk.append((0, 3, [0xB0, 10, 64]))      # CC10 pan center
    for i in range(8):                       # one note per beat, two bars
        t = i * PPQN
        key = [60, 64, 67, 72][i % 4]
        trk.append((t, 10, [0x90, key, 100]))
        trk.append((t + PPQN - 2, 5, [0x80, key, 0]))
    data = ([0x4D, 0x54, 0x68, 0x64, 0, 0, 0, 6, 0, 1, 0, 2, 0, PPQN]
            + midi_track(meta) + midi_track(trk))
    with open(path, "wb") as f:
        f.write(bytes(data))


def selftest():
    import subprocess
    mid = os.path.join(HERE, "selftest.mid")
    out_s = os.path.join(HERE, "selftest.s")
    build_test_midi(mid)
    exe = repo("tools", "mid2agb", "mid2agb.exe")
    if not os.path.exists(exe):
        exe = repo("tools", "mid2agb", "mid2agb")
    if not os.path.exists(exe):
        print("selftest: mid2agb binary not found (build tools first); "
              "wrote %s only" % mid)
        return
    r = subprocess.run([exe, mid, out_s, "-E", "-R50", "-G051", "-V100"],
                       capture_output=True, text=True)
    if r.returncode != 0:
        print("selftest FAILED: mid2agb rejected the exporter format:")
        print(r.stdout, r.stderr)
        sys.exit(1)
    body = open(out_s, encoding="utf-8", errors="replace").read()
    checks = ["voicegroup051", "VOICE ", "TEMPO", "GOTO"]
    missing = [c for c in checks if c not in body]
    print("selftest: mid2agb accepted the exporter's MIDI (%s)"
          % ("all markers present" if not missing
             else "MISSING: %s" % missing))


# --------------------------------------------------------------------------
def main():
    print("Emerald Composer build")
    print("  repo: %s" % ROOT)

    labels = parse_direct_sound_data()
    print("  %d sample labels in direct_sound_data.inc" % len(labels))

    samples = OrderedDict()
    missing = []
    for label, aif in labels.items():
        p = repo(*aif.split("/"))
        if not os.path.exists(p):
            missing.append(aif)
            continue
        try:
            samples[label] = parse_aiff(p)
        except ValueError as e:
            print("  warning: %s" % e)
    if missing:
        print("  note: %d referenced .aif not found (cries/unused), skipped"
              % len(missing))

    waves = parse_wave_data()
    print("  %d programmable waves" % len(waves))
    keysplits = parse_keysplit_tables()
    print("  %d keysplit tables" % len(keysplits))

    vg_dir = repo("sound", "voicegroups")
    voicegroups = OrderedDict()
    for fn in sorted(os.listdir(vg_dir)):
        m = re.match(r"(voicegroup\d+)\.inc$", fn)
        if m:
            voicegroups[m.group(1)] = parse_voicegroup(os.path.join(vg_dir, fn))
    print("  %d voicegroups" % len(voicegroups))

    usage = parse_songs_mk()
    vg_songs = {}
    for num, songs in usage.items():
        name = "voicegroup%03d" % num
        mus = [s for s in songs if s.startswith("mus_")]
        if mus:
            vg_songs[name] = mus[:3] + (["+%d" % (len(mus) - 3)] if len(mus) > 3 else [])
    # default: the BGM voicegroup used by the most mus_ tracks
    best = max(
        ((n, len([s for s in ss if s.startswith("mus_")])) for n, ss in usage.items()),
        key=lambda kv: kv[1], default=(0, 0))
    default_vg = "voicegroup%03d" % best[0]
    if default_vg not in voicegroups:
        default_vg = next(iter(voicegroups))
    print("  default voicegroup: %s" % default_vg)

    # prune: only keep samples/waves actually referenced by some voicegroup
    used_smp, used_wav = set(), set()
    for g in voicegroups.values():
        for v in g:
            if v.get("smp"):
                used_smp.add(v["smp"])
            if v.get("wav"):
                used_wav.add(v["wav"])
    dead = [k for k in samples if k not in used_smp]
    for k in dead:
        del samples[k]
    unresolved = sorted(used_smp - set(samples))
    if unresolved:
        print("  warning: %d referenced samples have no data: %s"
              % (len(unresolved), ", ".join(unresolved[:6])))

    data = {
        "meta": {
            "generated": "build.py",
            "defaultVoicegroup": default_vg,
            "vgSongs": vg_songs,
        },
        "samples": samples,
        "waves": waves,
        "keysplits": keysplits,
        "voicegroups": voicegroups,
    }
    blob = json.dumps(data, separators=(",", ":")).replace("</", "<\\/")
    tag = ('<script id="emerald-data" type="application/json">%s</script>'
           % blob)

    template = open(TEMPLATE, encoding="utf-8").read()
    if "<!--__EMERALD_DATA__-->" not in template:
        print("error: template placeholder missing")
        sys.exit(1)
    html = template.replace("<!--__EMERALD_DATA__-->", tag)
    with open(OUT_HTML, "w", encoding="utf-8") as f:
        f.write(html)
    print("  wrote %s (%.1f MB)"
          % (OUT_HTML, os.path.getsize(OUT_HTML) / 1048576.0))

    if "--selftest" in sys.argv:
        selftest()


if __name__ == "__main__":
    main()
