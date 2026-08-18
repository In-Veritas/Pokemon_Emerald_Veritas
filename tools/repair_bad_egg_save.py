#!/usr/bin/env python3
"""Repair Bad Eggs created by the OT-stamp bug, and/or re-arm Steven's BELDUM.

Background
----------
Two gift-mon bugs in Veritas corrupted Pokemon into Bad Eggs by overwriting
the plaintext OT id AFTER the mon was created: the substructs stay encrypted
with the old key (personality ^ oldOtId), so the new key decrypts garbage and
the checksum fails. The data is NOT lost -- it just needs the matching OT id
put back (or a proper re-encryption). Known victims:

  * Steven's BELDUM (created with the player's OT, then stamped with a random
    "Steven" OT id by SetLastGiftMonStevenOT). Every Beldum taken with the
    broken build is a Bad Egg. Original key = the player's own trainer id.
  * Bernard the tournament GOLDEEN (created with OT PECHE/02607, then stamped
    with the player's OT by GiveMonToPlayer). Original key = 2607.

This tool scans the save's party + PC boxes for encrypted mons whose checksum
fails, tries the known original keys, and can:

  --repair          fix repairable Bad Eggs in place. A Beldum whose data is
                    keyed to the player's own trainer id and whose OT name is
                    STEVEN is re-encrypted to the fixed Steven OT id (00374)
                    so it matches what the fixed game now gives out; other
                    victims get their original OT id written back.
  --reset-beldum    clear FLAG_RECEIVED_BELDUM and the Pokeball hide flag so
                    the ball in Steven's house reappears and a fresh BELDUM
                    can be taken (use if the player prefers a new one; do NOT
                    combine with a repaired Beldum unless you want two).
  --delete-bad-eggs remove unrepairable Bad Eggs (zero the PC slot / compact
                    the party).

With no action flag the tool only scans and reports. Actions write in place
after saving a .bak copy next to the file.

Layout note: offsets below are THIS HACK's save layout, computed from its
headers (SaveBlock1 has secretBaseWarp added, unused_9C2 removed and a
120-slot item pocket; SaveBlock2 has _saveSentinel/saveVersion/playerLookStyle
prepended). They do NOT match vanilla Emerald. Several sanity checks validate
the layout against the loaded file before anything is written.
"""

import argparse
import os
import shutil
import struct
import sys

# ---- Flash / sector layout (include/save.h) --------------------------------
SECTOR_SIZE = 4096
SECTOR_DATA_SIZE = 3968
SECTOR_SIGNATURE = 0x08012025
NUM_SECTORS_PER_SLOT = 14
SLOT_SIZE = SECTOR_SIZE * NUM_SECTORS_PER_SLOT  # 0xE000

SECTOR_ID_SAVEBLOCK2 = 0
SECTOR_ID_SAVEBLOCK1_START = 1
SECTOR_ID_SAVEBLOCK1_END = 4
SECTOR_ID_PKMN_STORAGE_START = 5
SECTOR_ID_PKMN_STORAGE_END = 13

# Bytes covered by each sector's checksum (sizeof of the chunk the game
# stores there). Sectors 0 and 4 hold partially-sized tail chunks whose
# exact sizes we don't need -- this tool never modifies them.
POKEMON_STORAGE_SIZE = 4 + 14 * 30 * 80 + 14 * 9 + 14 + 2  # 0x83D0 (padded)
SECTOR_CHECKSUM_SIZE = {}
for _sid in (1, 2, 3):
    SECTOR_CHECKSUM_SIZE[_sid] = SECTOR_DATA_SIZE
for _sid in range(5, 13):
    SECTOR_CHECKSUM_SIZE[_sid] = SECTOR_DATA_SIZE
SECTOR_CHECKSUM_SIZE[13] = POKEMON_STORAGE_SIZE - 8 * SECTOR_DATA_SIZE  # 2000

# ---- SaveBlock offsets (THIS hack; see module docstring) -------------------
SB2_SENTINEL = 0x00          # 0xFF
SB2_PLAYER_NAME = 0x04       # 8 bytes
SB2_PLAYER_GENDER = 0x0C
SB2_TRAINER_ID = 0x0F        # u32 LE

SB1_PARTY_COUNT = 0x23C
SB1_PARTY = 0x240
SB1_FLAGS = 0x13C8           # u8 flags[0x12C]

STORAGE_BOXES = 0x04         # 14 boxes x 30 slots x 80 bytes

# ---- Flags -----------------------------------------------------------------
FLAG_RECEIVED_BELDUM = 0x12A
FLAG_HIDE_BELDUM_BALL = 0x3C8       # FLAG_HIDE_MOSSDEEP_CITY_STEVENS_HOUSE_BELDUM_POKEBALL
FLAG_SYS_POKEMON_GET = 0x860
FLAG_SYS_POKEDEX_GET = 0x861

# ---- Mon constants ---------------------------------------------------------
SPECIES_NAMES = {398: "BELDUM", 118: "GOLDEEN", 415: "EGG"}
SPECIES_BELDUM = 398
STEVEN_BELDUM_OT_ID = 374            # fixed OT id used by the fixed game
PECHE_OT_ID = 2607                   # Bernard the GOLDEEN's OT

# Substruct shuffle orders, indexed by personality % 24 (G=Growth, A=Attacks,
# E=EVs/Condition, M=Misc).
SUBSTRUCT_ORDERS = [
    "GAEM", "GAME", "GEAM", "GEMA", "GMAE", "GMEA",
    "AGEM", "AGME", "AEGM", "AEMG", "AMGE", "AMEG",
    "EGAM", "EGMA", "EAGM", "EAMG", "EMGA", "EMAG",
    "MGAE", "MGEA", "MAGE", "MAEG", "MEGA", "MEAG",
]

STEVEN_OTNAME = bytes([0xCD, 0xCE, 0xBF, 0xD0, 0xBF, 0xC8])  # "STEVEN"


def decode_gba_text(raw):
    out = []
    for b in raw:
        if b == 0xFF:
            break
        if 0xBB <= b <= 0xD4:
            out.append(chr(ord("A") + b - 0xBB))
        elif 0xD5 <= b <= 0xEE:
            out.append(chr(ord("a") + b - 0xD5))
        elif 0xA1 <= b <= 0xAA:
            out.append(chr(ord("0") + b - 0xA1))
        elif b == 0x00:
            out.append(" ")
        else:
            out.append("?")
    return "".join(out)


def sector_checksum(data, size):
    total = 0
    for i in range(0, size, 4):
        total += struct.unpack_from("<I", data, i)[0]
    return ((total >> 16) + total) & 0xFFFF


def mon_checksum(dec48):
    return sum(struct.unpack("<24H", dec48)) & 0xFFFF


def xor_crypt(data48, key):
    words = struct.unpack("<12I", data48)
    return struct.pack("<12I", *[w ^ key for w in words])


class BoxMon:
    """An 80-byte BoxPokemon inside one of the assembled logical buffers."""

    def __init__(self, buf, off, where):
        self.buf = buf
        self.off = off
        self.where = where
        self.personality = struct.unpack_from("<I", buf, off)[0]
        self.ot_id = struct.unpack_from("<I", buf, off + 4)[0]
        self.nickname = decode_gba_text(bytes(buf[off + 8:off + 18]))
        self.flags = buf[off + 19]
        self.ot_name_raw = bytes(buf[off + 20:off + 27])
        self.checksum = struct.unpack_from("<H", buf, off + 28)[0]
        self.data = bytes(buf[off + 32:off + 80])

    def is_empty(self):
        return all(b == 0 for b in self.buf[self.off:self.off + 80])

    def decrypt(self, key):
        return xor_crypt(self.data, key)

    def valid_under(self, ot_id):
        dec = self.decrypt(ot_id ^ self.personality)
        return mon_checksum(dec) == self.checksum

    def substruct(self, dec48, letter):
        i = SUBSTRUCT_ORDERS[self.personality % 24].index(letter)
        return dec48[i * 12:(i + 1) * 12]

    def species(self, dec48):
        return struct.unpack_from("<H", self.substruct(dec48, "G"))[0]

    def is_egg(self, dec48):
        ivword = struct.unpack_from("<I", self.substruct(dec48, "M"), 4)[0]
        return (ivword >> 30) & 1

    def write_ot_id(self, ot_id, dec48=None):
        """Set the plaintext OT id; if dec48 is given, re-encrypt the
        substructs under the new key (full re-key), else leave them alone."""
        struct.pack_into("<I", self.buf, self.off + 4, ot_id)
        self.ot_id = ot_id
        if dec48 is not None:
            enc = xor_crypt(dec48, ot_id ^ self.personality)
            self.buf[self.off + 32:self.off + 80] = enc
            self.data = enc

    def fix_header_flags(self, is_egg):
        b = self.buf[self.off + 19]
        b &= ~0x01  # isBadEgg = 0
        b |= 0x02   # hasSpecies = 1
        if is_egg:
            b |= 0x04
        else:
            b &= ~0x04
        self.buf[self.off + 19] = b
        self.flags = b


def is_shiny(ot_id, personality):
    return ((ot_id >> 16) ^ (ot_id & 0xFFFF)
            ^ (personality >> 16) ^ (personality & 0xFFFF)) < 8


class SaveFile:
    def __init__(self, path):
        self.path = path
        with open(path, "rb") as f:
            self.raw = bytearray(f.read())
        if len(self.raw) < SLOT_SIZE:
            raise SystemExit("File too small to be an Emerald save (%d bytes)" % len(self.raw))

    def parse_slot(self, slot):
        base = slot * SLOT_SIZE
        if base + SLOT_SIZE > len(self.raw):
            return None
        sectors = {}
        counter = None
        for i in range(NUM_SECTORS_PER_SLOT):
            off = base + i * SECTOR_SIZE
            sid, chk = struct.unpack_from("<HH", self.raw, off + 0xFF4)
            sig, ctr = struct.unpack_from("<II", self.raw, off + 0xFF8)
            if sig != SECTOR_SIGNATURE:
                return None
            sectors[sid] = off
            counter = ctr
        if sorted(sectors.keys()) != list(range(NUM_SECTORS_PER_SLOT)):
            return None
        return {"sectors": sectors, "counter": counter}

    def pick_slot(self, forced):
        slots = {s: self.parse_slot(s) for s in (0, 1)}
        if forced in (0, 1):
            if slots[forced] is None:
                raise SystemExit("Requested slot %d is not a valid save slot" % forced)
            return forced, slots[forced]
        valid = {s: v for s, v in slots.items() if v is not None}
        if not valid:
            raise SystemExit("Neither save slot is valid -- is this an Emerald .sav?")
        best = max(valid, key=lambda s: valid[s]["counter"])
        return best, valid[best]

    def sector_data(self, slot_info, sid):
        off = slot_info["sectors"][sid]
        return self.raw[off:off + SECTOR_DATA_SIZE]

    def assemble(self, slot_info, first, last):
        out = bytearray()
        for sid in range(first, last + 1):
            out += self.sector_data(slot_info, sid)
        return out

    def verify_sector_checksums(self, slot_info, sids):
        for sid in sids:
            size = SECTOR_CHECKSUM_SIZE[sid]
            off = slot_info["sectors"][sid]
            stored = struct.unpack_from("<H", self.raw, off + 0xFF6)[0]
            calc = sector_checksum(self.raw[off:off + SECTOR_DATA_SIZE], size)
            if stored != calc:
                raise SystemExit(
                    "Sector id %d checksum mismatch (stored %04X, calc %04X) -- "
                    "layout assumptions do not hold for this save; refusing to touch it."
                    % (sid, stored, calc))

    def writeback(self, slot_info, first, last, buf):
        changed = []
        for i, sid in enumerate(range(first, last + 1)):
            chunk = buf[i * SECTOR_DATA_SIZE:(i + 1) * SECTOR_DATA_SIZE]
            off = slot_info["sectors"][sid]
            if self.raw[off:off + len(chunk)] != chunk:
                self.raw[off:off + len(chunk)] = chunk
                new_chk = sector_checksum(self.raw[off:off + SECTOR_DATA_SIZE],
                                          SECTOR_CHECKSUM_SIZE[sid])
                struct.pack_into("<H", self.raw, off + 0xFF6, new_chk)
                changed.append(sid)
        return changed

    def save(self):
        bak = self.path + ".bak"
        n = 1
        while os.path.exists(bak):
            bak = "%s.bak%d" % (self.path, n)
            n += 1
        shutil.copyfile(self.path, bak)
        with open(self.path, "wb") as f:
            f.write(self.raw)
        return bak


def flag_get(flags_buf, flag):
    return (flags_buf[flag >> 3] >> (flag & 7)) & 1


def flag_clear(flags_buf, flag):
    flags_buf[flag >> 3] &= ~(1 << (flag & 7))


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("save", help="path to the .sav file")
    ap.add_argument("--repair", action="store_true",
                    help="repair OT-stamp Bad Eggs in place")
    ap.add_argument("--reset-beldum", action="store_true",
                    help="clear FLAG_RECEIVED_BELDUM + ball hide flag so Steven's ball reappears")
    ap.add_argument("--delete-bad-eggs", action="store_true",
                    help="delete unrepairable Bad Eggs (PC slot zeroed / party compacted)")
    ap.add_argument("--slot", choices=["auto", "0", "1"], default="auto",
                    help="save slot to use (default: the newest valid one)")
    args = ap.parse_args()

    sav = SaveFile(args.save)
    forced = None if args.slot == "auto" else int(args.slot)
    slot, slot_info = sav.pick_slot(forced)

    sb2 = sav.sector_data(slot_info, SECTOR_ID_SAVEBLOCK2)
    if sb2[SB2_SENTINEL] != 0xFF:
        raise SystemExit("SaveBlock2 sentinel byte is not 0xFF -- not a Veritas save?")
    player = decode_gba_text(bytes(sb2[SB2_PLAYER_NAME:SB2_PLAYER_NAME + 8]))
    trainer_id = struct.unpack_from("<I", sb2, SB2_TRAINER_ID)[0]
    print("Slot %d (counter %d): player %s, IDNo %05d (full OT id %08X)"
          % (slot, slot_info["counter"], player, trainer_id & 0xFFFF, trainer_id))

    sb1 = sav.assemble(slot_info, SECTOR_ID_SAVEBLOCK1_START, SECTOR_ID_SAVEBLOCK1_END)
    storage = sav.assemble(slot_info, SECTOR_ID_PKMN_STORAGE_START, SECTOR_ID_PKMN_STORAGE_END)
    flags_buf = sb1[SB1_FLAGS:SB1_FLAGS + 0x12C]

    # Layout sanity: these are set on any real save that reached Steven's house.
    if not (flag_get(flags_buf, FLAG_SYS_POKEMON_GET) and flag_get(flags_buf, FLAG_SYS_POKEDEX_GET)):
        raise SystemExit("Layout validation failed (SYS flags not set at the computed "
                         "flags offset). Refusing to continue.")

    party_count = sb1[SB1_PARTY_COUNT]
    if not (1 <= party_count <= 6):
        raise SystemExit("Party count %d is implausible; refusing to continue." % party_count)

    print("FLAG_RECEIVED_BELDUM: %s / Beldum ball hidden: %s"
          % (bool(flag_get(flags_buf, FLAG_RECEIVED_BELDUM)),
             bool(flag_get(flags_buf, FLAG_HIDE_BELDUM_BALL))))

    # ---- Scan all mons -----------------------------------------------------
    mons = []
    for i in range(party_count):
        mons.append(BoxMon(sb1, SB1_PARTY + i * 100, ("party", i)))
    for box in range(14):
        for slot_i in range(30):
            off = STORAGE_BOXES + (box * 30 + slot_i) * 80
            m = BoxMon(storage, off, ("box", box, slot_i))
            if not m.is_empty():
                mons.append(m)

    candidates = [("player", trainer_id),
                  ("PECHE (Bernard the GOLDEEN)", PECHE_OT_ID),
                  ("STEVEN (fixed Beldum id)", STEVEN_BELDUM_OT_ID),
                  ("SCOTT (frontier shiny Beldum)", 2005)]
    healthy, repairable, hopeless = 0, [], []
    for m in mons:
        if m.is_empty():
            continue
        if m.valid_under(m.ot_id):
            healthy += 1
            continue
        found = None
        for label, key in candidates:
            if m.valid_under(key):
                found = (label, key)
                break
        (repairable if found else hopeless).append((m, found))

    print("Scanned %d mons: %d healthy, %d repairable Bad Eggs, %d unrepairable"
          % (healthy + len(repairable) + len(hopeless), healthy,
             len(repairable), len(hopeless)))

    dirty = False
    for m, (label, key) in repairable:
        dec = m.decrypt(key ^ m.personality)
        species = m.species(dec)
        name = SPECIES_NAMES.get(species, "species #%d" % species)
        print("  %s @ %s: data keyed to OT id %08X (%s)"
              % (name, m.where, key, label))
        if not args.repair:
            continue
        if (species == SPECIES_BELDUM and key == trainer_id
                and m.ot_name_raw[:6] == STEVEN_OTNAME):
            # Re-key to the fixed game's Steven OT so it matches new gifts.
            was_shiny = is_shiny(key, m.personality)
            m.write_ot_id(STEVEN_BELDUM_OT_ID, dec48=dec)
            if is_shiny(STEVEN_BELDUM_OT_ID, m.personality) != was_shiny:
                print("    note: shininess changed with the OT re-key")
            print("    repaired: re-keyed to STEVEN/%05d" % STEVEN_BELDUM_OT_ID)
        else:
            m.write_ot_id(key)  # plaintext restore, data already matches
            print("    repaired: OT id restored to %08X" % key)
        m.fix_header_flags(m.is_egg(dec))
        dirty = True

    if hopeless:
        for m, _ in hopeless:
            print("  UNREPAIRABLE Bad Egg @ %s (personality %08X, OT id %08X)"
                  % (m.where, m.personality, m.ot_id))
        if args.delete_bad_eggs:
            # Delete PC ones by zeroing; party ones by compacting.
            party_dead = sorted([m.off for m, _ in hopeless if m.where[0] == "party"],
                                reverse=True)
            for m, _ in hopeless:
                if m.where[0] == "box":
                    storage[m.off:m.off + 80] = bytes(80)
            for off in party_dead:
                idx = (off - SB1_PARTY) // 100
                for j in range(idx, party_count - 1):
                    a = SB1_PARTY + j * 100
                    sb1[a:a + 100] = sb1[a + 100:a + 200]
                party_count -= 1
                sb1[SB1_PARTY + party_count * 100:SB1_PARTY + (party_count + 1) * 100] = bytes(100)
            sb1[SB1_PARTY_COUNT] = party_count
            print("Deleted %d unrepairable Bad Egg(s)" % len(hopeless))
            dirty = True

    if args.reset_beldum:
        flag_clear(flags_buf, FLAG_RECEIVED_BELDUM)
        flag_clear(flags_buf, FLAG_HIDE_BELDUM_BALL)
        sb1[SB1_FLAGS:SB1_FLAGS + 0x12C] = flags_buf
        print("Cleared FLAG_RECEIVED_BELDUM and the ball hide flag -- the Pokeball "
              "in Steven's house is back.")
        dirty = True

    if not dirty:
        print("No changes written." + ("" if (args.repair or args.reset_beldum or
                                              args.delete_bad_eggs)
                                       else " (scan only; use --repair / --reset-beldum)"))
        return

    if args.repair or args.delete_bad_eggs or args.reset_beldum:
        sb1[SB1_FLAGS:SB1_FLAGS + 0x12C] = flags_buf
        # Verify the sectors we are about to rewrite still checksum cleanly
        # under our size table before we touch anything.
        sav.verify_sector_checksums(slot_info, list(range(1, 4)) + list(range(5, 14)))
        changed = sav.writeback(slot_info, SECTOR_ID_SAVEBLOCK1_START,
                                SECTOR_ID_SAVEBLOCK1_END, sb1)
        changed += sav.writeback(slot_info, SECTOR_ID_PKMN_STORAGE_START,
                                 SECTOR_ID_PKMN_STORAGE_END, storage)
        bak = sav.save()
        print("Wrote %s (sectors %s updated; backup at %s)"
              % (sav.path, sorted(changed), bak))
        print("If a repaired mon sits in the party, deposit + withdraw it once "
              "to refresh its recalculated stats.")


if __name__ == "__main__":
    main()
