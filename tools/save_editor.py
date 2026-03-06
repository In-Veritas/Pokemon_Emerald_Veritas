#!/usr/bin/env python3
"""
Pokemon Emerald Veritas - Save File Editor
A command-line interactive tool for viewing and editing .sav files.

Save format:
  32 sectors x 4096 bytes = 131072 bytes (128KB)
  Dual save slots: sectors 0-13 (slot 1), sectors 14-27 (slot 2)
  Each sector: 3968 bytes data + 128 bytes footer
  Footer: id(u16) @ 0xFF4, checksum(u16) @ 0xFF6, signature(u32) @ 0xFF8, counter(u32) @ 0xFFC

Sector ID mapping per slot:
  0: SaveBlock2, 1-4: SaveBlock1, 5-13: PokemonStorage
"""

import struct
import sys
import os

# =============================================================================
# Constants
# =============================================================================

SAVE_FILE_SIZE = 131072  # 128KB
SECTOR_SIZE = 4096
SECTOR_DATA_SIZE = 3968
SECTOR_FOOTER_SIZE = 128
NUM_SECTORS_PER_SLOT = 14
NUM_SAVE_SLOTS = 2
SECTORS_COUNT = 32

SECTOR_SIGNATURE = 0x08012025

# Sector footer offsets
OFF_ID = 0xFF4
OFF_CHECKSUM = 0xFF6
OFF_SIGNATURE = 0xFF8
OFF_COUNTER = 0xFFC

# Sector IDs
SECTOR_ID_SAVEBLOCK2 = 0
SECTOR_ID_SAVEBLOCK1_START = 1
SECTOR_ID_SAVEBLOCK1_END = 4
SECTOR_ID_PKMN_STORAGE_START = 5
SECTOR_ID_PKMN_STORAGE_END = 13

# =============================================================================
# Veritas SaveBlock field offsets (empirically verified from agbcc builds)
# NOTE: Host GCC gives DIFFERENT offsets due to struct packing differences.
#       These offsets were verified against real save files.
# =============================================================================

# SaveBlock2 offsets
SB2_OFF_ENCRYPTION_KEY = 0x00B4

# SaveBlock1 offsets
SB1_OFF_MONEY = 0x0498
SB1_OFF_COINS = 0x049C
SB1_OFF_PC_ITEMS = 0x04A0
SB1_OFF_FLAGS = 0x1270  # from save.v0.h struct comment (unchanged between versions)
SB1_OFF_SECRET_BASES = 0x1C14
SB1_OFF_TV_SHOWS = 0x260C
SB1_OFF_POKE_NEWS = 0x2990  # tvShows + 25*36
SB1_OFF_OLD_MAN = 0x2E60
SB1_OFF_DEWFORD_TRENDS = 0x2EA0  # oldMan + 64
SB1_OFF_CONTEST_WINNERS = 0x2EC8  # dewfordTrends + 5*8
SB1_OFF_LINK_BATTLE_RECORDS = 0x3198
# These offsets are approximate (not fully verified with agbcc)
SB1_OFF_RECORD_MIXING_GIFT = 0x3B14  # approximate
SB1_OFF_LILYCOVE_LADY = 0x3B58  # approximate
SB1_OFF_TRAINER_NAME_RECORDS = 0x3BA0  # approximate

# Struct sizes
SIZEOF_TV_SHOW = 36
TV_SHOWS_COUNT = 25
SIZEOF_POKE_NEWS = 4
POKE_NEWS_COUNT = 16
SIZEOF_LINK_BATTLE_RECORD = 16
LINK_BATTLE_RECORDS_COUNT = 5
SIZEOF_SECRET_BASE = 192
SECRET_BASES_COUNT = 15
SIZEOF_OLD_MAN = 64
SIZEOF_DEWFORD_TREND = 8
DEWFORD_TRENDS_COUNT = 5
SIZEOF_CONTEST_WINNER = 32
CONTEST_WINNERS_COUNT = 13

SIZEOF_TRAINER_NAME_RECORD = 12  # trainerId(u32) + trainerName(8)
TRAINER_NAME_RECORDS_COUNT = 20

PLAYER_NAME_LENGTH = 7

# Game stats
SB1_OFF_GAME_STATS = 0x159C  # u32 gameStats[64]
GAME_STAT_FIRST_HOF_PLAY_TIME = 1
GAME_STAT_ENTERED_HOF = 10

# Flags
FLAG_SYS_GAME_CLEAR = 0x864

# Known player names for fuzzy repair (GBA encoded)
KNOWN_NAMES = [
    [0xCA, 0xCF, 0xCC, 0xCA, 0xC6, 0xBF, 0xFF],  # PURPLE
    [0xBB, 0xC3, 0xC6, 0xBF, 0xCD, 0xFF],          # AILES
    [0xBB, 0xE0, 0xDC, 0xED, 0xD5, 0xFF],          # Alhya
    [0xC1, 0xD5, 0xD6, 0xE7, 0xFF],                # Gabs
    [0xC8, 0xDD, 0xDE, 0xDD, 0xFF],                # Niji
]

def try_fuzzy_name_repair(name_data, max_len):
    """Try to fix a name where only the first 1-2 bytes are corrupted.
    Returns the fixed name bytes if matched, or None."""
    for known in KNOWN_NAMES:
        known_len = len(known) - 1  # exclude EOS
        for start_pos in range(1, min(3, known_len)):
            match = True
            for m in range(start_pos, known_len):
                if m >= max_len:
                    match = False
                    break
                if name_data[m] != known[m]:
                    match = False
                    break
            if match:
                fixed = bytearray(name_data)
                for n in range(start_pos):
                    fixed[n] = known[n]
                return bytes(fixed)
    return None

# =============================================================================
# GBA Pokemon text encoding
# =============================================================================

GBA_CHAR_TABLE = {}
GBA_CHAR_REVERSE = {}

def _build_char_tables():
    GBA_CHAR_TABLE[0x00] = ' '
    GBA_CHAR_REVERSE[' '] = 0x00
    GBA_CHAR_TABLE[0xFF] = '\x00'
    for i, ch in enumerate(range(ord('A'), ord('Z') + 1)):
        GBA_CHAR_TABLE[0xBB + i] = chr(ch)
        GBA_CHAR_REVERSE[chr(ch)] = 0xBB + i
    for i, ch in enumerate(range(ord('a'), ord('z') + 1)):
        GBA_CHAR_TABLE[0xD5 + i] = chr(ch)
        GBA_CHAR_REVERSE[chr(ch)] = 0xD5 + i
    for i, ch in enumerate(range(ord('0'), ord('9') + 1)):
        GBA_CHAR_TABLE[0xA1 + i] = chr(ch)
        GBA_CHAR_REVERSE[chr(ch)] = 0xA1 + i
    extras = {
        0xAB: '!', 0xAC: '?', 0xAD: '.', 0xAE: '-',
        0xB0: '\u2026', 0xB1: '\u201C', 0xB2: '\u201D',
        0xB3: '\u2018', 0xB4: '\u2019',
        0xB5: '\u2642', 0xB6: '\u2640',
        0xB7: ',', 0xB8: '/',
    }
    for code, ch in extras.items():
        GBA_CHAR_TABLE[code] = ch
        if ch not in GBA_CHAR_REVERSE:
            GBA_CHAR_REVERSE[ch] = code

_build_char_tables()

# Valid byte ranges for player/trainer names (matching IsValidNameByte in field_specials.c)
def is_valid_name_byte(byte):
    """Check if a byte is a valid character for a player/trainer name."""
    if byte == 0x00:  # space
        return True
    if 0x01 <= byte <= 0x2E:  # accented Latin characters
        return True
    if byte in (0x5A, 0x68, 0x6F):  # scattered accented chars (i acute, a circumflex, i acute)
        return True
    if 0xA1 <= byte <= 0xF6:  # digits, punctuation, letters, diacriticals
        return True
    return False


def decode_gba_string(data, max_len=None):
    result = []
    length = max_len if max_len else len(data)
    for i in range(length):
        byte = data[i]
        if byte == 0xFF:
            break
        ch = GBA_CHAR_TABLE.get(byte, '?')
        result.append(ch)
    return ''.join(result)


def encode_gba_string(text, max_len):
    result = bytearray()
    for ch in text[:max_len]:
        code = GBA_CHAR_REVERSE.get(ch)
        if code is None:
            print(f"  Warning: Character '{ch}' not in encoding table, skipping.")
            continue
        result.append(code)
    while len(result) < max_len:
        result.append(0xFF)
    return bytes(result)


# =============================================================================
# TV Show types (from tv.h)
# =============================================================================

TV_SHOW_NAMES = {
    0: "(empty)",
    1: "Fan Club Letter",
    2: "Recent Happenings",
    3: "Pkmn Fan Club Opinions",
    4: "Dummy (unused)",
    5: "Mass Outbreak",
    6: "Pokmon Today/Caught",
    7: "Smart Shopper",
    8: "Bravo Trainer (pokemon)",
    9: "Bravo Trainer (battle tower)",
    10: "Contest Live Updates",
    11: "Battle Update",
    12: "3 Cheers for Pokeblocks",
    13: "Todays Rival Trainer",
    14: "Trend Watcher",
    15: "Treasure Investigators",
    16: "Pokemon Lotto Winners",
    17: "Today's Fishing",
    18: "Hoenn News",
    19: "Dewford Trend",
    20: "Hoenn Pokmon",
    21: "Number One (Pokemon Contest)",
    22: "Secret Base Secrets",
    23: "Safari Fan Club",
    24: "Pokemon Contest",
    25: "Pokemon Battler",
    26: "Pokemon Ranger",
    27: "Pokemon Nickname",
    28: "World of Masters",
    29: "Secret Base Visit",
    30: "Lilycove Contest Lady",
    31: "Pokemon Center Bard",
}

# =============================================================================
# Checksum
# =============================================================================

def calculate_checksum(data):
    checksum = 0
    padded = data
    if len(padded) % 4 != 0:
        padded = padded + b'\x00' * (4 - len(padded) % 4)
    for i in range(0, len(padded), 4):
        checksum += struct.unpack_from('<I', padded, i)[0]
        checksum &= 0xFFFFFFFF
    return ((checksum >> 16) + (checksum & 0xFFFF)) & 0xFFFF


# =============================================================================
# Save File Class
# =============================================================================

class SaveFile:
    def __init__(self):
        self.raw_data = None
        self.filepath = None
        self.sectors = []
        self.active_slot = 0
        self.slot_counters = [0, 0]
        self.is_veritas = False
        self.sb2_base_offset = 0

    def load(self, filepath):
        self.filepath = filepath
        with open(filepath, 'rb') as f:
            self.raw_data = bytearray(f.read())

        self.header = b''
        self.trailer = b''
        if len(self.raw_data) > SAVE_FILE_SIZE:
            sig_no_header = struct.unpack_from('<I', self.raw_data, 0xFF8)[0]
            sig_with_header = struct.unpack_from('<I', self.raw_data, 0xFF8 + 16)[0] if len(self.raw_data) >= 0xFF8 + 20 else 0
            if sig_no_header == SECTOR_SIGNATURE:
                extra = len(self.raw_data) - SAVE_FILE_SIZE
                print(f"Save file has {extra} extra trailing bytes, ignoring them.")
                self.trailer = bytes(self.raw_data[SAVE_FILE_SIZE:])
                self.raw_data = self.raw_data[:SAVE_FILE_SIZE]
            elif sig_with_header == SECTOR_SIGNATURE:
                extra = len(self.raw_data) - SAVE_FILE_SIZE
                print(f"Detected {extra}-byte header, stripping it.")
                self.header = bytes(self.raw_data[:extra])
                self.raw_data = self.raw_data[extra:]
        if len(self.raw_data) != SAVE_FILE_SIZE:
            raise ValueError(f"Invalid save file size: {len(self.raw_data)} bytes (expected {SAVE_FILE_SIZE})")

        self.sectors = []
        for i in range(SECTORS_COUNT):
            offset = i * SECTOR_SIZE
            self.sectors.append(self.raw_data[offset:offset + SECTOR_SIZE])

        self._find_active_slot()

        sb2_data = self._get_saveblock2_data()
        if sb2_data[0] == 0xFF:
            self.is_veritas = True
            self.sb2_base_offset = 4
        else:
            self.is_veritas = False
            self.sb2_base_offset = 0

        print(f"Loaded: {filepath}")
        print(f"Active save slot: {self.active_slot + 1}")
        print(f"Save counters: Slot 1 = {self.slot_counters[0]}, Slot 2 = {self.slot_counters[1]}")
        if self.is_veritas:
            version = struct.unpack_from('<H', sb2_data, 2)[0]
            print(f"Veritas save format detected (save version: {version})")
        else:
            print("Standard Emerald save format detected")

    def _find_active_slot(self):
        slot_complete = [False, False]
        for slot in range(NUM_SAVE_SLOTS):
            max_counter = 0
            valid = False
            found_ids = set()
            for sector_idx in range(NUM_SECTORS_PER_SLOT):
                disk_sector = slot * NUM_SECTORS_PER_SLOT + sector_idx
                sector = self.sectors[disk_sector]
                sig = struct.unpack_from('<I', sector, OFF_SIGNATURE)[0]
                if sig == SECTOR_SIGNATURE:
                    counter = struct.unpack_from('<I', sector, OFF_COUNTER)[0]
                    sid = struct.unpack_from('<H', sector, OFF_ID)[0]
                    found_ids.add(sid)
                    if counter > max_counter or not valid:
                        max_counter = counter
                    valid = True
            self.slot_counters[slot] = max_counter if valid else 0
            # A slot is complete only if all 14 sector IDs (0-13) are present
            slot_complete[slot] = (found_ids == set(range(NUM_SECTORS_PER_SLOT)))

        # Prefer the complete slot with the higher counter
        if slot_complete[1] and slot_complete[0]:
            self.active_slot = 1 if self.slot_counters[1] > self.slot_counters[0] else 0
        elif slot_complete[1]:
            self.active_slot = 1
        elif slot_complete[0]:
            self.active_slot = 0
        else:
            # Neither slot is complete — fall back to higher counter
            self.active_slot = 1 if self.slot_counters[1] > self.slot_counters[0] else 0
            print("WARNING: Neither save slot has all 14 sectors! Save may be corrupted.")

    def _get_sector_by_id(self, sector_id, slot=None):
        if slot is None:
            slot = self.active_slot
        base = slot * NUM_SECTORS_PER_SLOT
        for disk_idx in range(base, base + NUM_SECTORS_PER_SLOT):
            sector = self.sectors[disk_idx]
            sid = struct.unpack_from('<H', sector, OFF_ID)[0]
            if sid == sector_id:
                return disk_idx
        raise ValueError(f"Sector ID {sector_id} not found in slot {slot + 1}")

    def _get_saveblock2_data(self, slot=None):
        disk_idx = self._get_sector_by_id(SECTOR_ID_SAVEBLOCK2, slot)
        return self.sectors[disk_idx][:SECTOR_DATA_SIZE]

    def _get_saveblock1_data(self, slot=None):
        result = bytearray()
        for sid in range(SECTOR_ID_SAVEBLOCK1_START, SECTOR_ID_SAVEBLOCK1_END + 1):
            disk_idx = self._get_sector_by_id(sid, slot)
            result.extend(self.sectors[disk_idx][:SECTOR_DATA_SIZE])
        return result

    def _write_saveblock2_data(self, data, slot=None):
        disk_idx = self._get_sector_by_id(SECTOR_ID_SAVEBLOCK2, slot)
        self.sectors[disk_idx][:SECTOR_DATA_SIZE] = data[:SECTOR_DATA_SIZE]
        self._recalculate_sector_checksum(disk_idx)

    def _write_saveblock1_data(self, data, slot=None):
        offset = 0
        for sid in range(SECTOR_ID_SAVEBLOCK1_START, SECTOR_ID_SAVEBLOCK1_END + 1):
            disk_idx = self._get_sector_by_id(sid, slot)
            chunk = data[offset:offset + SECTOR_DATA_SIZE]
            self.sectors[disk_idx][:len(chunk)] = chunk
            self._recalculate_sector_checksum(disk_idx)
            offset += SECTOR_DATA_SIZE

    def _recalculate_sector_checksum(self, disk_idx):
        sector = self.sectors[disk_idx]
        data = sector[:SECTOR_DATA_SIZE]
        checksum = calculate_checksum(data)
        struct.pack_into('<H', sector, OFF_CHECKSUM, checksum)

    def recalculate_all_checksums(self, slot=None):
        if slot is None:
            slot = self.active_slot
        base = slot * NUM_SECTORS_PER_SLOT
        count = 0
        for disk_idx in range(base, base + NUM_SECTORS_PER_SLOT):
            sector = self.sectors[disk_idx]
            sig = struct.unpack_from('<I', sector, OFF_SIGNATURE)[0]
            if sig == SECTOR_SIGNATURE:
                self._recalculate_sector_checksum(disk_idx)
                count += 1
        print(f"Recalculated checksums for {count} sectors in slot {slot + 1}.")

    def verify_checksums(self, slot=None):
        if slot is None:
            slot = self.active_slot
        base = slot * NUM_SECTORS_PER_SLOT
        bad = []
        for disk_idx in range(base, base + NUM_SECTORS_PER_SLOT):
            sector = self.sectors[disk_idx]
            sig = struct.unpack_from('<I', sector, OFF_SIGNATURE)[0]
            if sig != SECTOR_SIGNATURE:
                bad.append((disk_idx, "bad signature"))
                continue
            stored_checksum = struct.unpack_from('<H', sector, OFF_CHECKSUM)[0]
            computed = calculate_checksum(sector[:SECTOR_DATA_SIZE])
            if stored_checksum != computed:
                bad.append((disk_idx, f"checksum mismatch: stored=0x{stored_checksum:04X}, computed=0x{computed:04X}"))
        return bad

    def save(self, filepath=None):
        if filepath is None:
            filepath = self.filepath
        output = bytearray()
        for sector in self.sectors:
            output.extend(sector)
        if len(output) < SAVE_FILE_SIZE:
            output.extend(b'\x00' * (SAVE_FILE_SIZE - len(output)))
        with open(filepath, 'wb') as f:
            if self.header:
                f.write(self.header)
            f.write(output[:SAVE_FILE_SIZE])
            if self.trailer:
                f.write(self.trailer)
        print(f"Saved to: {filepath}")

    # =========================================================================
    # Player Info
    # =========================================================================

    def get_player_name(self):
        sb2 = self._get_saveblock2_data()
        off = self.sb2_base_offset
        return decode_gba_string(sb2[off:off + 8])

    def get_player_gender(self):
        sb2 = self._get_saveblock2_data()
        off = self.sb2_base_offset + 8
        return sb2[off]

    def get_trainer_id(self):
        sb2 = self._get_saveblock2_data()
        off = self.sb2_base_offset + 8 + 1 + 1 + 1  # name[8] + gender + lookStyle + warpFlags
        tid_bytes = sb2[off:off + 4]
        public_id = struct.unpack_from('<H', tid_bytes, 0)[0]
        secret_id = struct.unpack_from('<H', tid_bytes, 2)[0]
        return public_id, secret_id

    def get_play_time(self):
        sb2 = self._get_saveblock2_data()
        off = self.sb2_base_offset + 8 + 1 + 1 + 1 + 4  # after trainerId[4]
        if off % 2 != 0:
            off += 1
        hours = struct.unpack_from('<H', sb2, off)[0]
        minutes = sb2[off + 2]
        seconds = sb2[off + 3]
        return hours, minutes, seconds

    def get_flag(self, flag_id):
        """Read a flag from the SaveBlock1 flags array."""
        sb1 = self._get_saveblock1_data()
        byte_idx = flag_id // 8
        bit_idx = flag_id % 8
        return bool(sb1[SB1_OFF_FLAGS + byte_idx] & (1 << bit_idx))

    def get_game_stat(self, stat_index):
        """Read a u32 game stat from SaveBlock1."""
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_GAME_STATS + stat_index * 4
        return struct.unpack_from('<I', sb1, off)[0]

    def set_game_stat(self, stat_index, value):
        """Write a u32 game stat to SaveBlock1 (both slots)."""
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_GAME_STATS + stat_index * 4
        struct.pack_into('<I', sb1, off, value)
        self._write_saveblock1_data(sb1)
        # Also write to other slot
        other_slot = 1 - self.active_slot
        try:
            sb1_other = self._get_saveblock1_data(other_slot)
            struct.pack_into('<I', sb1_other, off, value)
            self._write_saveblock1_data(sb1_other, other_slot)
        except (ValueError, IndexError):
            pass

    def get_encryption_key(self):
        sb2 = self._get_saveblock2_data()
        key = struct.unpack_from('<I', sb2, SB2_OFF_ENCRYPTION_KEY)[0]
        return key

    def get_money(self):
        sb1 = self._get_saveblock1_data()
        key = self.get_encryption_key()
        encrypted = struct.unpack_from('<I', sb1, SB1_OFF_MONEY)[0]
        return encrypted ^ key

    def set_money(self, amount):
        if amount < 0 or amount > 999999:
            print("Money must be between 0 and 999999.")
            return
        sb1 = self._get_saveblock1_data()
        key = self.get_encryption_key()
        encrypted = amount ^ key
        struct.pack_into('<I', sb1, SB1_OFF_MONEY, encrypted)
        self._write_saveblock1_data(sb1)
        print(f"Money set to {amount}.")

    def get_coins(self):
        sb1 = self._get_saveblock1_data()
        key = self.get_encryption_key()
        encrypted = struct.unpack_from('<H', sb1, SB1_OFF_COINS)[0]
        return encrypted ^ (key & 0xFFFF)

    # =========================================================================
    # Game Stats (encrypted with encryptionKey)
    # =========================================================================

    def _find_game_stats_offset(self):
        """Auto-detect gameStats offset by searching for encrypted values."""
        if hasattr(self, '_game_stats_offset'):
            return self._game_stats_offset

        sb1 = self._get_saveblock1_data()
        key = self.get_encryption_key()

        # Search for a block where decrypted stat[0] (saved count) is reasonable
        # and stat[5] (steps) is reasonable
        for off in range(0x0800, SB1_OFF_SECRET_BASES, 4):
            if off + 64 * 4 > len(sb1):
                break
            saved = struct.unpack_from('<I', sb1, off)[0] ^ key
            steps = struct.unpack_from('<I', sb1, off + 20)[0] ^ key
            battles = struct.unpack_from('<I', sb1, off + 28)[0] ^ key
            captures = struct.unpack_from('<I', sb1, off + 44)[0] ^ key
            if (1 <= saved <= 100000 and 0 <= steps <= 10000000 and
                0 <= battles <= 1000000 and 0 <= captures <= 100000):
                self._game_stats_offset = off
                return off

        self._game_stats_offset = None
        return None

    def get_game_stat(self, stat_id):
        off = self._find_game_stats_offset()
        if off is None:
            return None
        sb1 = self._get_saveblock1_data()
        key = self.get_encryption_key()
        stat_offset = off + stat_id * 4
        if stat_offset + 4 > len(sb1):
            return None
        return struct.unpack_from('<I', sb1, stat_offset)[0] ^ key

    # =========================================================================
    # Bard / Old Man
    # =========================================================================

    def view_bard_lyrics(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_OLD_MAN
        man_id = sb1[off]

        print("\n--- Bard / Old Man ---")
        print(f"  Old Man type: {man_id} (0=Bard(default), 1=Bard, 2=Hipster, 3=Trader, 4=Storyteller)")

        if man_id not in (0, 1):
            print("  Old Man is not currently the Bard.")
            return

        print("  Song lyrics (Easy Chat word IDs):")
        for i in range(6):
            word = struct.unpack_from('<H', sb1, off + 2 + i * 2)[0]
            print(f"    Word {i + 1}: 0x{word:04X}")

        print("  Temporary lyrics:")
        for i in range(6):
            word = struct.unpack_from('<H', sb1, off + 14 + i * 2)[0]
            print(f"    Word {i + 1}: 0x{word:04X}")

        has_changed = sb1[off + 0x29]
        print(f"  Has changed song: {'Yes' if has_changed else 'No'}")

    def reset_bard_lyrics(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_OLD_MAN

        sb1[off] = 1  # MAUVILLE_MAN_BARD
        default_lyrics = [0x2811, 0x1029, 0x1018, 0x0E0D, 0x1A1A, 0x1A1D]

        for i, word in enumerate(default_lyrics):
            struct.pack_into('<H', sb1, off + 2 + i * 2, word)
        for i, word in enumerate(default_lyrics):
            struct.pack_into('<H', sb1, off + 14 + i * 2, word)
        sb1[off + 0x29] = 0

        self._write_saveblock1_data(sb1)
        print("Bard lyrics reset to defaults (SHAKE IT DO THE DIET DANCE).")

    # =========================================================================
    # Link Battle Records
    # =========================================================================

    def view_link_battle_records(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_LINK_BATTLE_RECORDS

        print("\n--- Link Battle Records ---")
        has_records = False
        for i in range(LINK_BATTLE_RECORDS_COUNT):
            entry_off = off + i * SIZEOF_LINK_BATTLE_RECORD
            tid = struct.unpack_from('<H', sb1, entry_off + 8)[0]
            name_bytes = sb1[entry_off:entry_off + 8]
            # Skip empty slots (all zero/0xFF bytes, or trainer ID is 0)
            if tid == 0 and all(b == 0 or b == 0xFF for b in name_bytes):
                continue
            name = decode_gba_string(name_bytes)
            if not name.strip() and tid == 0:
                continue
            has_records = True
            wins = struct.unpack_from('<H', sb1, entry_off + 10)[0]
            losses = struct.unpack_from('<H', sb1, entry_off + 12)[0]
            draws = struct.unpack_from('<H', sb1, entry_off + 14)[0]
            print(f"  {i + 1}. {name} (ID: {tid:05d}) - W:{wins} L:{losses} D:{draws}")

        if not has_records:
            print("  No link battle records found.")

        lang_off = off + LINK_BATTLE_RECORDS_COUNT * SIZEOF_LINK_BATTLE_RECORD
        print(f"  Languages: ", end="")
        for i in range(LINK_BATTLE_RECORDS_COUNT):
            lang = sb1[lang_off + i]
            print(f"{lang} ", end="")
        print()

    def clear_link_battle_records(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_LINK_BATTLE_RECORDS
        total_size = LINK_BATTLE_RECORDS_COUNT * SIZEOF_LINK_BATTLE_RECORD + LINK_BATTLE_RECORDS_COUNT + 1
        for i in range(total_size):
            sb1[off + i] = 0
        self._write_saveblock1_data(sb1)
        print("Link battle records cleared.")

    # =========================================================================
    # TV Shows
    # =========================================================================

    def view_tv_shows(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_TV_SHOWS

        print("\n--- TV Shows ---")
        print(f"  (Located at SB1 offset 0x{off:04X}, {TV_SHOWS_COUNT} slots x {SIZEOF_TV_SHOW} bytes)")
        has_shows = False
        for i in range(TV_SHOWS_COUNT):
            show_off = off + i * SIZEOF_TV_SHOW
            show_kind = sb1[show_off]
            is_active = sb1[show_off + 1]

            if show_kind == 0:
                continue

            has_shows = True
            show_name = TV_SHOW_NAMES.get(show_kind, f"Unknown({show_kind})")
            active_str = "active" if is_active else "inactive"

            # Try to read player name from the show (most show types have it at offset 2)
            player_name = decode_gba_string(sb1[show_off + 2:show_off + 10])

            print(f"  Slot {i:2d}: [{show_kind:2d}] {show_name} ({active_str})")
            if player_name.strip():
                print(f"          Player: {player_name}")

            # Show raw hex for debugging
            raw = sb1[show_off:show_off + SIZEOF_TV_SHOW]
            hex_str = ' '.join(f'{b:02X}' for b in raw[:16])
            hex_str2 = ' '.join(f'{b:02X}' for b in raw[16:])
            print(f"          Raw: {hex_str}")
            print(f"               {hex_str2}")

        if not has_shows:
            print("  No TV shows recorded.")

    def clear_tv_shows(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_TV_SHOWS
        total = TV_SHOWS_COUNT * SIZEOF_TV_SHOW
        for i in range(total):
            sb1[off + i] = 0
        self._write_saveblock1_data(sb1)
        print(f"Cleared all {TV_SHOWS_COUNT} TV show slots.")

    # =========================================================================
    # PokeNews
    # =========================================================================

    def view_poke_news(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_POKE_NEWS

        print("\n--- PokeNews ---")
        print(f"  (Located at SB1 offset 0x{off:04X}, {POKE_NEWS_COUNT} slots x {SIZEOF_POKE_NEWS} bytes)")
        has_news = False
        for i in range(POKE_NEWS_COUNT):
            news_off = off + i * SIZEOF_POKE_NEWS
            kind = sb1[news_off]
            state = sb1[news_off + 1]
            days_left = struct.unpack_from('<H', sb1, news_off + 2)[0]

            if kind == 0:
                continue

            has_news = True
            print(f"  Slot {i:2d}: kind={kind}, state={state}, daysLeft={days_left}")

        if not has_news:
            print("  No PokeNews entries.")

    def clear_poke_news(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_POKE_NEWS
        total = POKE_NEWS_COUNT * SIZEOF_POKE_NEWS
        for i in range(total):
            sb1[off + i] = 0
        self._write_saveblock1_data(sb1)
        print(f"Cleared all {POKE_NEWS_COUNT} PokeNews entries.")

    # =========================================================================
    # Dewford Trends
    # =========================================================================

    def view_dewford_trends(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_DEWFORD_TRENDS

        print("\n--- Dewford Trends ---")
        for i in range(DEWFORD_TRENDS_COUNT):
            trend_off = off + i * SIZEOF_DEWFORD_TREND
            word1 = struct.unpack_from('<H', sb1, trend_off)[0]
            word2 = struct.unpack_from('<H', sb1, trend_off + 2)[0]
            max_trendiness = struct.unpack_from('<H', sb1, trend_off + 4)[0]
            cur_trendiness = struct.unpack_from('<H', sb1, trend_off + 6)[0]
            print(f"  Trend {i + 1}: words=0x{word1:04X}/0x{word2:04X}, trendiness={cur_trendiness}/{max_trendiness}")

    # =========================================================================
    # Secret Bases (other players)
    # =========================================================================

    def view_secret_bases(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_SECRET_BASES

        print("\n--- Secret Bases ---")
        print(f"  (Located at SB1 offset 0x{off:04X}, {SECRET_BASES_COUNT} slots x {SIZEOF_SECRET_BASE} bytes)")

        for i in range(SECRET_BASES_COUNT):
            base_off = off + i * SIZEOF_SECRET_BASE
            secret_base_id = sb1[base_off]

            if secret_base_id == 0:
                continue

            # Owner name is at offset 2 in SecretBase struct
            owner_name = decode_gba_string(sb1[base_off + 2:base_off + 2 + 8])
            owner_label = "PLAYER" if i == 0 else f"Other #{i}"

            print(f"  Slot {i:2d} ({owner_label}): baseId={secret_base_id}, owner={owner_name}")

    def clear_other_secret_bases(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_SECRET_BASES

        for i in range(1, SECRET_BASES_COUNT):
            base_off = off + i * SIZEOF_SECRET_BASE
            for j in range(SIZEOF_SECRET_BASE):
                sb1[base_off + j] = 0

        self._write_saveblock1_data(sb1)
        print(f"Cleared {SECRET_BASES_COUNT - 1} other players' secret bases (kept player's own).")

    # =========================================================================
    # Record Mixing Gift
    # =========================================================================

    def view_record_mixing_gift(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_RECORD_MIXING_GIFT

        print("\n--- Record Mixing Gift ---")
        raw = sb1[off:off + 16]
        hex_str = ' '.join(f'{b:02X}' for b in raw)
        all_zero = all(b == 0 for b in raw)
        print(f"  Raw (16 bytes): {hex_str}")
        print(f"  Status: {'Empty/cleared' if all_zero else 'Has data'}")

    def clear_record_mixing_gift(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_RECORD_MIXING_GIFT
        for i in range(16):
            sb1[off + i] = 0
        self._write_saveblock1_data(sb1)
        print("Record mixing gift cleared.")

    # =========================================================================
    # Contest Winners
    # =========================================================================

    def view_contest_winners(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_CONTEST_WINNERS

        print("\n--- Contest Winners ---")
        for i in range(CONTEST_WINNERS_COUNT):
            cw_off = off + i * SIZEOF_CONTEST_WINNER
            # ContestWinner struct: nickname[11], trainerName[8], ...
            nickname = decode_gba_string(sb1[cw_off:cw_off + 11])
            trainer_name = decode_gba_string(sb1[cw_off + 11:cw_off + 19])

            if not nickname.strip() and not trainer_name.strip():
                continue

            print(f"  Slot {i:2d}: Pokemon={nickname}, Trainer={trainer_name}")

    # =========================================================================
    # Lilycove Lady
    # =========================================================================

    def view_lilycove_lady(self):
        sb1 = self._get_saveblock1_data()
        off = SB1_OFF_LILYCOVE_LADY

        print("\n--- Lilycove Lady ---")
        lady_id = sb1[off]
        lady_names = {0: "Favor Lady", 1: "Contest Lady", 2: "Quiz Lady"}
        print(f"  Type: {lady_names.get(lady_id, f'Unknown({lady_id})')}")

        raw = sb1[off:off + 64]
        hex_str = ' '.join(f'{b:02X}' for b in raw[:32])
        hex_str2 = ' '.join(f'{b:02X}' for b in raw[32:])
        print(f"  Raw: {hex_str}")
        print(f"       {hex_str2}")

    # =========================================================================
    # Trainer Card Info
    # =========================================================================

    def view_trainer_card_info(self):
        print("\n--- Trainer Card Info ---")

        name = self.get_player_name()
        gender = self.get_player_gender()
        public_id, secret_id = self.get_trainer_id()
        hours, minutes, seconds = self.get_play_time()
        money = self.get_money()
        coins = self.get_coins()

        print(f"  Player Name: {name}")
        print(f"  Gender: {'Female' if gender else 'Male'}")
        print(f"  Trainer ID: {public_id:05d}")
        print(f"  Secret ID: {secret_id:05d}")
        print(f"  Play Time: {hours}:{minutes:02d}:{seconds:02d}")
        print(f"  Money: ${money:,}")
        print(f"  Coins: {coins}")

        print("\n  --- Star Requirements ---")
        hof_time = self.get_game_stat(1)
        hof_entries = self.get_game_stat(10)

        if hof_time is not None:
            total_seconds = hof_time // 60 if hof_time > 0 else 0
            hof_hours = total_seconds // 3600
            hof_mins = (total_seconds % 3600) // 60
            hof_secs = total_seconds % 60
            print(f"  First HOF Play Time: {hof_hours}:{hof_mins:02d}:{hof_secs:02d} (raw: {hof_time})")
        else:
            print("  First HOF Play Time: Could not read")

        if hof_entries is not None:
            print(f"  HOF Entries: {hof_entries}")
            print(f"  Star 1 (Beat E4): {'Yes' if hof_entries > 0 else 'No'}")
        else:
            print("  HOF Entries: Could not read")

        for stat_name, stat_id in [
            ("Pokemon Captures", 11),
            ("Total Battles", 7),
            ("Steps", 5),
            ("Saved Game Count", 0),
            ("Link Battle Wins", 23),
            ("Link Battle Losses", 24),
            ("Link Battle Draws", 25),
        ]:
            val = self.get_game_stat(stat_id)
            if val is not None:
                print(f"  {stat_name}: {val}")

    # =========================================================================
    # Display Info
    # =========================================================================

    def view_info(self):
        print("\n--- Player Info ---")
        name = self.get_player_name()
        gender = self.get_player_gender()
        public_id, secret_id = self.get_trainer_id()
        hours, minutes, seconds = self.get_play_time()
        money = self.get_money()
        coins = self.get_coins()

        print(f"  Name: {name}")
        print(f"  Gender: {'Female' if gender else 'Male'}")
        print(f"  Trainer ID: {public_id:05d} / Secret ID: {secret_id:05d}")
        print(f"  Play Time: {hours}:{minutes:02d}:{seconds:02d}")
        print(f"  Money: ${money:,}")
        print(f"  Coins: {coins}")

        if self.is_veritas:
            sb2 = self._get_saveblock2_data()
            version = struct.unpack_from('<H', sb2, 2)[0]
            # Check both: old playerLookStyle field (pre-v5) and FLAG_PLAYER_STYLE_RS (v5+)
            old_look_style = sb2[self.sb2_base_offset + 8 + 1]  # after playerName[8] + gender
            rs_flag = self.get_flag(0x296)
            rs_style = old_look_style != 0 or rs_flag
            print(f"  Save Version: {version}")
            print(f"  Player Look Style: {'RS' if rs_style else 'Emerald'}")
            # Debug: dump SB2 header bytes
            print(f"  [DEBUG] SB2 bytes 0-20: {' '.join(f'{sb2[i]:02X}' for i in range(21))}")
            print(f"  [DEBUG] playerLookStyle byte (off {self.sb2_base_offset + 9}): 0x{old_look_style:02X}")
            sb1 = self._get_saveblock1_data()
            flag_byte_idx = 0x296 // 8  # = 82
            flag_bit = 0x296 % 8  # = 6
            print(f"  [DEBUG] flags[{flag_byte_idx}] (SB1 off 0x{SB1_OFF_FLAGS + flag_byte_idx:04X}): 0x{sb1[SB1_OFF_FLAGS + flag_byte_idx]:02X} (bit {flag_bit} = {rs_flag})")

    # =========================================================================
    # Raw Sector Viewer
    # =========================================================================

    def view_sector_info(self, slot=None):
        if slot is None:
            slot = self.active_slot
        base = slot * NUM_SECTORS_PER_SLOT

        print(f"\n--- Sector Layout (Slot {slot + 1}) ---")
        print(f"  {'Disk':>4}  {'ID':>4}  {'Checksum':>8}  {'Signature':>10}  {'Counter':>10}  Status")
        print(f"  {'----':>4}  {'--':>4}  {'--------':>8}  {'----------':>10}  {'----------':>10}  ------")

        for disk_idx in range(base, base + NUM_SECTORS_PER_SLOT):
            sector = self.sectors[disk_idx]
            sid = struct.unpack_from('<H', sector, OFF_ID)[0]
            checksum = struct.unpack_from('<H', sector, OFF_CHECKSUM)[0]
            sig = struct.unpack_from('<I', sector, OFF_SIGNATURE)[0]
            counter = struct.unpack_from('<I', sector, OFF_COUNTER)[0]

            computed = calculate_checksum(sector[:SECTOR_DATA_SIZE])
            status = "OK" if (sig == SECTOR_SIGNATURE and checksum == computed) else "BAD"

            sector_type = ""
            if sid == 0:
                sector_type = " (SaveBlock2)"
            elif 1 <= sid <= 4:
                sector_type = f" (SaveBlock1 chunk {sid})"
            elif 5 <= sid <= 13:
                sector_type = f" (PkmnStorage chunk {sid - 5})"

            print(f"  {disk_idx:4d}  {sid:4d}  0x{checksum:04X}    0x{sig:08X}  {counter:10d}  {status}{sector_type}")

    # =========================================================================
    # Mixed Records Overview
    # =========================================================================

    def view_mixed_records(self):
        print("\n========================================")
        print("  Mixed Records Overview")
        print("========================================")
        print("  Data received from other players via Record Mixing:\n")
        self.view_tv_shows()
        self.view_poke_news()
        self.view_secret_bases()
        self.view_dewford_trends()
        self.view_contest_winners()
        self.view_record_mixing_gift()
        self.view_lilycove_lady()

    # =========================================================================
    # Corruption Scanner
    # =========================================================================

    def _check_name_corruption(self, data, max_len):
        """Check a name field for corrupted bytes. Returns list of (index, byte) tuples."""
        issues = []
        for i in range(max_len):
            byte = data[i]
            if byte == 0xFF:
                break
            if not is_valid_name_byte(byte):
                issues.append((i, byte))
        return issues

    def _format_name_hex(self, data, max_len):
        """Format name bytes as hex with corruption markers."""
        parts = []
        for i in range(max_len):
            byte = data[i]
            if byte == 0xFF:
                parts.append('FF')
                break
            if not is_valid_name_byte(byte):
                parts.append(f'*{byte:02X}*')
            else:
                parts.append(f'{byte:02X}')
        return ' '.join(parts)

    def scan_corrupted_text(self):
        """Scan all text fields for corrupted characters and allow manual correction."""
        sb1 = self._get_saveblock1_data()
        found_any = False

        # Check for spurious Hall of Fame stats without FLAG_SYS_GAME_CLEAR
        game_clear = self.get_flag(FLAG_SYS_GAME_CLEAR)
        hof_time = self.get_game_stat(GAME_STAT_FIRST_HOF_PLAY_TIME)
        hof_count = self.get_game_stat(GAME_STAT_ENTERED_HOF)
        if not game_clear and (hof_time != 0 or hof_count != 0):
            hours = (hof_time >> 16) & 0xFFFF
            minutes = (hof_time >> 8) & 0xFF
            seconds = hof_time & 0xFF
            print(f"\n  WARNING: Hall of Fame data found but FLAG_SYS_GAME_CLEAR is not set!")
            print(f"    HOF debut time: {hours}h:{minutes:02d}m:{seconds:02d}s")
            print(f"    HOF entries: {hof_count}")
            confirm = input("  Clear spurious HOF stats? (y/n): ").strip().lower()
            if confirm == 'y':
                self.set_game_stat(GAME_STAT_FIRST_HOF_PLAY_TIME, 0)
                self.set_game_stat(GAME_STAT_ENTERED_HOF, 0)
                print("  Cleared HOF stats from both save slots.")
                # Re-read sb1 since we wrote to it
                sb1 = self._get_saveblock1_data()

        print("\n========================================")
        print("  Corruption Scanner")
        print("========================================")
        print("  Scanning all name/text fields...\n")

        corrupt_entries = []

        # 1. Link battle records — also collect valid names for auto-repair
        off = SB1_OFF_LINK_BATTLE_RECORDS
        link_records = []
        for i in range(LINK_BATTLE_RECORDS_COUNT):
            entry_off = off + i * SIZEOF_LINK_BATTLE_RECORD
            name_data = sb1[entry_off:entry_off + 8]
            tid = struct.unpack_from('<H', sb1, entry_off + 8)[0]
            link_records.append({'name_data': bytes(name_data), 'tid': tid, 'entry_off': entry_off})

        fuzzy_fixes = []
        for i in range(LINK_BATTLE_RECORDS_COUNT):
            rec = link_records[i]
            if all(b == 0 or b == 0xFF for b in rec['name_data'][:8]):
                continue
            # Try fuzzy name repair first
            fixed = try_fuzzy_name_repair(rec['name_data'], 8)
            if fixed and fixed != rec['name_data']:
                old_name = decode_gba_string(rec['name_data'], 8)
                new_name = decode_gba_string(fixed, 8)
                for b_idx in range(8):
                    sb1[rec['entry_off'] + b_idx] = fixed[b_idx]
                rec['name_data'] = fixed
                link_records[i]['name_data'] = fixed
                fuzzy_fixes.append(f"    Link record {i}: \"{old_name}\" -> \"{new_name}\"")
            issues = self._check_name_corruption(rec['name_data'], 8)
            if issues:
                # Try auto-repair: find another record with same trainer ID and valid name
                auto_fix_name = None
                auto_fix_j = None
                for j in range(LINK_BATTLE_RECORDS_COUNT):
                    if j == i:
                        continue
                    other = link_records[j]
                    if other['tid'] == rec['tid'] and other['tid'] != 0:
                        other_issues = self._check_name_corruption(other['name_data'], 8)
                        if not other_issues and not all(b == 0 or b == 0xFF for b in other['name_data'][:8]):
                            auto_fix_name = other['name_data']
                            auto_fix_j = j
                            break

                decoded = decode_gba_string(rec['name_data'], 8)
                hex_str = self._format_name_hex(rec['name_data'], 8)
                entry = {
                    'type': 'Link Battle Record',
                    'index': i,
                    'offset': rec['entry_off'],
                    'name_offset': rec['entry_off'],
                    'max_len': 8,
                    'decoded': decoded,
                    'hex': hex_str,
                    'issues': issues,
                    'auto_fix': auto_fix_name,
                    'auto_fix_source': decode_gba_string(auto_fix_name, 8) if auto_fix_name else None,
                    'auto_fix_j': auto_fix_j,
                }
                corrupt_entries.append(entry)

        # 2. Secret bases (skip index 0 = player's own)
        off = SB1_OFF_SECRET_BASES
        for i in range(SECRET_BASES_COUNT):
            base_off = off + i * SIZEOF_SECRET_BASE
            if sb1[base_off] == 0:
                continue
            # Owner name at offset 2 in SecretBase struct
            name_off = base_off + 2
            name_data = sb1[name_off:name_off + 8]
            issues = self._check_name_corruption(name_data, 8)
            if issues:
                label = "Player's base" if i == 0 else f"Other #{i}"
                decoded = decode_gba_string(name_data, 8)
                hex_str = self._format_name_hex(name_data, 8)
                entry = {
                    'type': f'Secret Base ({label})',
                    'index': i,
                    'offset': base_off,
                    'name_offset': name_off,
                    'max_len': 8,
                    'decoded': decoded,
                    'hex': hex_str,
                    'issues': issues,
                }
                corrupt_entries.append(entry)

        # 3. Contest winners
        off = SB1_OFF_CONTEST_WINNERS
        for i in range(CONTEST_WINNERS_COUNT):
            cw_off = off + i * SIZEOF_CONTEST_WINNER
            # nickname at offset 0 (11 bytes), trainerName at offset 11 (8 bytes)
            for field_name, field_off, field_len in [
                ('Nickname', cw_off, 11),
                ('Trainer', cw_off + 11, 8),
            ]:
                name_data = sb1[field_off:field_off + field_len]
                if all(b == 0 or b == 0xFF for b in name_data[:field_len]):
                    continue
                issues = self._check_name_corruption(name_data, field_len)
                if issues:
                    decoded = decode_gba_string(name_data, field_len)
                    hex_str = self._format_name_hex(name_data, field_len)
                    entry = {
                        'type': f'Contest Winner #{i} {field_name}',
                        'index': i,
                        'offset': cw_off,
                        'name_offset': field_off,
                        'max_len': field_len,
                        'decoded': decoded,
                        'hex': hex_str,
                        'issues': issues,
                    }
                    corrupt_entries.append(entry)

        if fuzzy_fixes:
            print(f"  Fuzzy name repair applied to {len(fuzzy_fixes)} record(s):")
            for msg in fuzzy_fixes:
                print(msg)
            self._write_saveblock1_data(sb1)
            # Also write to other slot
            other_slot = 1 - self.active_slot
            try:
                sb1_other = self._get_saveblock1_data(other_slot)
                for rec_fix in link_records:
                    for b_idx in range(8):
                        sb1_other[rec_fix['entry_off'] + b_idx] = sb1[rec_fix['entry_off'] + b_idx]
                self._write_saveblock1_data(sb1_other, other_slot)
            except (ValueError, IndexError):
                pass
            print()

        if not corrupt_entries:
            print("  No corrupted text found! All scanned fields are clean.")
            return

        print(f"  Found {len(corrupt_entries)} corrupted text field(s):\n")
        for idx, entry in enumerate(corrupt_entries):
            print(f"  [{idx + 1}] {entry['type']}")
            print(f"      Decoded: \"{entry['decoded']}\"")
            print(f"      Hex:     {entry['hex']}")
            print(f"      Bad bytes: {', '.join(f'pos {p}: 0x{b:02X}' for p, b in entry['issues'])}")
            if entry.get('auto_fix_source'):
                print(f"      AUTO-FIX available: \"{entry['auto_fix_source']}\" (same trainer ID)")
            print()

        # Auto-fix all entries that have a match
        auto_fixable = [e for e in corrupt_entries if e.get('auto_fix')]
        if auto_fixable:
            confirm = input(f"  Auto-fix {len(auto_fixable)} entry/entries from matching trainer IDs? (y/n): ").strip().lower()
            if confirm == 'y':
                for entry in auto_fixable:
                    if entry['type'] == 'Link Battle Record' and entry.get('auto_fix_j') is not None:
                        # Merge W/L/D into the valid record, then wipe the corrupted one
                        src_off = entry['offset']
                        dst_off = SB1_OFF_LINK_BATTLE_RECORDS + entry['auto_fix_j'] * SIZEOF_LINK_BATTLE_RECORD
                        # Read W/L/D from corrupted record (offsets +10, +12, +14)
                        src_wins = struct.unpack_from('<H', sb1, src_off + 10)[0]
                        src_losses = struct.unpack_from('<H', sb1, src_off + 12)[0]
                        src_draws = struct.unpack_from('<H', sb1, src_off + 14)[0]
                        # Read W/L/D from valid record
                        dst_wins = struct.unpack_from('<H', sb1, dst_off + 10)[0]
                        dst_losses = struct.unpack_from('<H', sb1, dst_off + 12)[0]
                        dst_draws = struct.unpack_from('<H', sb1, dst_off + 14)[0]
                        # Sum into valid record (cap at 0xFFFF)
                        struct.pack_into('<H', sb1, dst_off + 10, min(dst_wins + src_wins, 0xFFFF))
                        struct.pack_into('<H', sb1, dst_off + 12, min(dst_losses + src_losses, 0xFFFF))
                        struct.pack_into('<H', sb1, dst_off + 14, min(dst_draws + src_draws, 0xFFFF))
                        # Wipe corrupted record entirely
                        for b_idx in range(SIZEOF_LINK_BATTLE_RECORD):
                            sb1[src_off + b_idx] = 0
                        # Also wipe the language byte for this entry
                        lang_off = SB1_OFF_LINK_BATTLE_RECORDS + LINK_BATTLE_RECORDS_COUNT * SIZEOF_LINK_BATTLE_RECORD + entry['index']
                        sb1[lang_off] = 0
                        print(f"    Merged: {entry['type']} W:{src_wins}/L:{src_losses}/D:{src_draws} -> \"{entry['auto_fix_source']}\" (W:{dst_wins + src_wins}/L:{dst_losses + src_losses}/D:{dst_draws + src_draws})")
                    else:
                        # Non-link records: just copy name
                        name_off = entry['name_offset']
                        for j_idx in range(entry['max_len']):
                            sb1[name_off + j_idx] = entry['auto_fix'][j_idx]
                        print(f"    Fixed: {entry['type']} -> \"{entry['auto_fix_source']}\"")
                    entry['issues'] = []
                self._write_saveblock1_data(sb1)
                # Also write to other slot
                other_slot = 1 - self.active_slot
                try:
                    sb1_other = self._get_saveblock1_data(other_slot)
                    for entry in auto_fixable:
                        if entry['type'] == 'Link Battle Record' and entry.get('auto_fix_j') is not None:
                            src_off = entry['offset']
                            dst_off = SB1_OFF_LINK_BATTLE_RECORDS + entry['auto_fix_j'] * SIZEOF_LINK_BATTLE_RECORD
                            # Copy merged W/L/D from active slot
                            for b_idx in range(SIZEOF_LINK_BATTLE_RECORD):
                                sb1_other[dst_off + b_idx] = sb1[dst_off + b_idx]
                            # Wipe corrupted record
                            for b_idx in range(SIZEOF_LINK_BATTLE_RECORD):
                                sb1_other[src_off + b_idx] = 0
                            lang_off = SB1_OFF_LINK_BATTLE_RECORDS + LINK_BATTLE_RECORDS_COUNT * SIZEOF_LINK_BATTLE_RECORD + entry['index']
                            sb1_other[lang_off] = 0
                        else:
                            name_off = entry['name_offset']
                            for j_idx in range(entry['max_len']):
                                sb1_other[name_off + j_idx] = entry['auto_fix'][j_idx]
                    self._write_saveblock1_data(sb1_other, other_slot)
                    print("  Applied to both save slots.")
                except (ValueError, IndexError):
                    print("  Applied to active slot (other slot incomplete).")

        # Allow manual correction for remaining
        remaining = [e for e in corrupt_entries if e.get('issues')]
        if not remaining:
            if corrupt_entries:
                print("\n  All corruptions fixed!")
                print("  Use option 16/17 to save to disk!")
            return

        print(f"\n  {len(remaining)} entry/entries still need manual correction:")
        for idx, entry in enumerate(remaining):
            print(f"  [{idx + 1}] {entry['type']}: \"{entry['decoded']}\"")

        while True:
            choice = input("  Enter number to fix (or 'q' to quit): ").strip().lower()
            if choice == 'q' or choice == '':
                break
            try:
                num = int(choice) - 1
                if num < 0 or num >= len(remaining):
                    print("  Invalid number.")
                    continue
            except ValueError:
                print("  Invalid input.")
                continue

            entry = remaining[num]
            print(f"\n  Fixing: {entry['type']}")
            print(f"  Current: \"{entry['decoded']}\" ({entry['hex']})")
            print(f"  Available characters: A-Z, a-z, 0-9, space, ! ? . - , /")
            new_name = input("  Enter corrected name: ").strip()
            if not new_name:
                print("  Cancelled.")
                continue

            encoded = encode_gba_string(new_name, entry['max_len'])
            print(f"  New bytes: {' '.join(f'{b:02X}' for b in encoded)}")
            confirm = input("  Apply? (y/n): ").strip().lower()
            if confirm == 'y':
                name_off = entry['name_offset']
                for j in range(entry['max_len']):
                    sb1[name_off + j] = encoded[j]
                # Write to active slot
                self._write_saveblock1_data(sb1)
                # Also write to the other slot if it has the needed sectors
                other_slot = 1 - self.active_slot
                try:
                    sb1_other = self._get_saveblock1_data(other_slot)
                    for j in range(entry['max_len']):
                        sb1_other[name_off + j] = encoded[j]
                    self._write_saveblock1_data(sb1_other, other_slot)
                    print(f"  Fixed in both save slots!")
                except (ValueError, IndexError):
                    print(f"  Fixed in active slot (other slot incomplete).")
                # Re-decode to verify
                new_decoded = decode_gba_string(encoded, entry['max_len'])
                print(f"  New value: \"{new_decoded}\"")
                # Update the entry for display
                entry['decoded'] = new_decoded
                entry['hex'] = ' '.join(f'{b:02X}' for b in encoded)
                entry['issues'] = []
            else:
                print("  Cancelled.")

        if any(not e['issues'] for e in corrupt_entries):
            print("\n  Corrections applied in memory. Use option 16/17 to save to disk!")

    def wipe_all_mixed_records(self):
        """Wipe all record mixing data from other players."""
        sb1 = self._get_saveblock1_data()

        # TV Shows
        off = SB1_OFF_TV_SHOWS
        for i in range(TV_SHOWS_COUNT * SIZEOF_TV_SHOW):
            sb1[off + i] = 0
        print("  Wiped TV shows.")

        # PokeNews
        off = SB1_OFF_POKE_NEWS
        for i in range(POKE_NEWS_COUNT * SIZEOF_POKE_NEWS):
            sb1[off + i] = 0
        print("  Wiped PokeNews.")

        # Other players' secret bases (keep index 0 = player's own)
        off = SB1_OFF_SECRET_BASES
        for i in range(1, SECRET_BASES_COUNT):
            base_off = off + i * SIZEOF_SECRET_BASE
            for j in range(SIZEOF_SECRET_BASE):
                sb1[base_off + j] = 0
        print("  Wiped other players' secret bases.")

        # Record mixing gift
        off = SB1_OFF_RECORD_MIXING_GIFT
        for i in range(16):
            sb1[off + i] = 0
        print("  Wiped record mixing gift.")

        self._write_saveblock1_data(sb1)
        print("\nAll mixed records wiped (player data preserved).")


# =============================================================================
# Interactive Menu
# =============================================================================

def print_menu():
    print("\n========================================")
    print("  Pokemon Emerald Veritas Save Editor")
    print("========================================")
    print("  1. Load save file")
    print("  2. View player info")
    print("  3. View trainer card data")
    print("  4. View/Edit money")
    print("  5. View Bard song lyrics")
    print("  6. Reset Bard lyrics to defaults")
    print("  7. View link battle records")
    print("  8. Clear link battle records")
    print("  9. View TV shows")
    print(" 10. View PokeNews")
    print(" 11. View/Edit mixed records")
    print(" 12. View sector layout")
    print(" 13. Verify checksums")
    print(" 14. Recalculate all checksums")
    print(" 15. Scan for corrupted text")
    print(" 16. Save file")
    print(" 17. Save file as (new path)")
    print("  0. Exit")
    print("========================================")


def main():
    save = SaveFile()
    loaded = False

    print("\nPokemon Emerald Veritas Save Editor")
    print("===================================\n")

    if len(sys.argv) > 1:
        filepath = sys.argv[1]
        if os.path.isfile(filepath):
            try:
                save.load(filepath)
                loaded = True
            except Exception as e:
                print(f"Error loading file: {e}")

    while True:
        print_menu()
        try:
            choice = input("\nSelect option: ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\nExiting.")
            break

        if choice == '0':
            print("Exiting.")
            break

        elif choice == '1':
            filepath = input("Enter save file path: ").strip().strip('"')
            if not os.path.isfile(filepath):
                print(f"File not found: {filepath}")
                continue
            try:
                save = SaveFile()
                save.load(filepath)
                loaded = True
            except Exception as e:
                print(f"Error loading file: {e}")
                loaded = False

        elif choice == '2':
            if not loaded:
                print("No save file loaded.")
                continue
            save.view_info()

        elif choice == '3':
            if not loaded:
                print("No save file loaded.")
                continue
            save.view_trainer_card_info()

        elif choice == '4':
            if not loaded:
                print("No save file loaded.")
                continue
            money = save.get_money()
            print(f"\nCurrent money: ${money:,}")
            edit = input("Edit money? (y/n): ").strip().lower()
            if edit == 'y':
                try:
                    new_amount = int(input("Enter new amount (0-999999): ").strip())
                    confirm = input(f"Set money to ${new_amount:,}? (y/n): ").strip().lower()
                    if confirm == 'y':
                        save.set_money(new_amount)
                    else:
                        print("Cancelled.")
                except ValueError:
                    print("Invalid number.")

        elif choice == '5':
            if not loaded:
                print("No save file loaded.")
                continue
            save.view_bard_lyrics()

        elif choice == '6':
            if not loaded:
                print("No save file loaded.")
                continue
            confirm = input("Reset Bard lyrics to defaults? (y/n): ").strip().lower()
            if confirm == 'y':
                save.reset_bard_lyrics()

        elif choice == '7':
            if not loaded:
                print("No save file loaded.")
                continue
            save.view_link_battle_records()

        elif choice == '8':
            if not loaded:
                print("No save file loaded.")
                continue
            confirm = input("Clear ALL link battle records? (y/n): ").strip().lower()
            if confirm == 'y':
                save.clear_link_battle_records()

        elif choice == '9':
            if not loaded:
                print("No save file loaded.")
                continue
            save.view_tv_shows()

        elif choice == '10':
            if not loaded:
                print("No save file loaded.")
                continue
            save.view_poke_news()

        elif choice == '11':
            if not loaded:
                print("No save file loaded.")
                continue
            print("\n--- Mixed Records ---")
            print("  a. View all mixed records")
            print("  b. Clear TV shows")
            print("  c. Clear PokeNews")
            print("  d. Clear other secret bases")
            print("  e. Clear record mixing gift")
            print("  f. Wipe ALL mixed records")
            print("  g. View Dewford trends")
            print("  h. View contest winners")
            print("  i. View Lilycove Lady")
            sub = input("Select: ").strip().lower()
            if sub == 'a':
                save.view_mixed_records()
            elif sub == 'b':
                confirm = input("Clear all TV shows? (y/n): ").strip().lower()
                if confirm == 'y':
                    save.clear_tv_shows()
            elif sub == 'c':
                confirm = input("Clear all PokeNews? (y/n): ").strip().lower()
                if confirm == 'y':
                    save.clear_poke_news()
            elif sub == 'd':
                confirm = input("Clear other players' secret bases? (y/n): ").strip().lower()
                if confirm == 'y':
                    save.clear_other_secret_bases()
            elif sub == 'e':
                confirm = input("Clear record mixing gift? (y/n): ").strip().lower()
                if confirm == 'y':
                    save.clear_record_mixing_gift()
            elif sub == 'f':
                confirm = input("Wipe ALL mixed records (TV, news, bases, gift)? (y/n): ").strip().lower()
                if confirm == 'y':
                    save.wipe_all_mixed_records()
            elif sub == 'g':
                save.view_dewford_trends()
            elif sub == 'h':
                save.view_contest_winners()
            elif sub == 'i':
                save.view_lilycove_lady()
            else:
                print("Invalid option.")

        elif choice == '12':
            if not loaded:
                print("No save file loaded.")
                continue
            print("View slot 1, 2, or both? (1/2/b): ", end="")
            slot_choice = input().strip().lower()
            if slot_choice == '1':
                save.view_sector_info(0)
            elif slot_choice == '2':
                save.view_sector_info(1)
            else:
                save.view_sector_info(0)
                save.view_sector_info(1)

        elif choice == '13':
            if not loaded:
                print("No save file loaded.")
                continue
            for slot in range(2):
                bad = save.verify_checksums(slot)
                if bad:
                    print(f"Slot {slot + 1}: {len(bad)} bad sector(s):")
                    for disk_idx, reason in bad:
                        print(f"  Sector {disk_idx}: {reason}")
                else:
                    print(f"Slot {slot + 1}: All checksums valid.")

        elif choice == '14':
            if not loaded:
                print("No save file loaded.")
                continue
            confirm = input("Recalculate checksums for active slot? (y/n): ").strip().lower()
            if confirm == 'y':
                save.recalculate_all_checksums()

        elif choice == '15':
            if not loaded:
                print("No save file loaded.")
                continue
            save.scan_corrupted_text()

        elif choice == '16':
            if not loaded:
                print("No save file loaded.")
                continue
            confirm = input(f"Overwrite {save.filepath}? (y/n): ").strip().lower()
            if confirm == 'y':
                save.recalculate_all_checksums()
                save.save()

        elif choice == '17':
            if not loaded:
                print("No save file loaded.")
                continue
            filepath = input("Enter output file path: ").strip().strip('"')
            confirm = input(f"Save to {filepath}? (y/n): ").strip().lower()
            if confirm == 'y':
                save.recalculate_all_checksums()
                save.save(filepath)

        else:
            print("Invalid option.")


if __name__ == '__main__':
    main()
