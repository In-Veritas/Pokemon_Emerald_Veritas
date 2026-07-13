#!/usr/bin/env python3
"""Generate a Tournament Prize code for Pokemon Emerald Veritas.

Usage:
    python tools/make_prize_code.py <trainer_id> <prize_id>

    trainer_id  The 5-digit ID shown on the winner's TRAINER CARD (0-65535).
    prize_id    1-15 — indexes sTournamentPrizes in src/field_specials.c.
                Currently defined: 1 = GOLDEN CAP, 2 = MASTER BALL,
                3 = RARE CANDY x5. IDs 4-15 are open.

The winner redeems the code at the Cable Club attendant (first desk in any
Pokemon Center 2F) via the TOURNAMENT PRIZE menu option. Codes are bound to
the trainer ID, so they only work on the winner's save, and each prize ID
can be redeemed once per save.

The algorithm and constants below MUST stay in sync with
RedeemTournamentCode() in src/field_specials.c. Changing the salts (in both
places) and rebuilding invalidates every code generated before the change.
"""
import sys

ALPHABET = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789"  # no I, O, 0, 1
SALT_1 = 0x56455249  # "VERI" — must match TOURNEY_SALT_1 in field_specials.c
SALT_2 = 0x54415321  # "TAS!" — must match TOURNEY_SALT_2
MASK = 0xFFFFFFFF


def mix(x):
    x &= MASK
    x ^= x >> 16
    x = (x * 0x7FEB352D) & MASK
    x ^= x >> 15
    x = (x * 0x846CA68B) & MASK
    x ^= x >> 16
    return x


def make_code(trainer_id, prize_id):
    if not 0 <= trainer_id <= 0xFFFF:
        raise ValueError("trainer_id must be 0-65535 (the 5-digit card ID)")
    if not 1 <= prize_id <= 15:
        raise ValueError("prize_id must be 1-15")
    h1 = mix(SALT_1 ^ ((trainer_id & 0xFFFF) << 8) ^ prize_id)
    h2 = mix(h1 ^ SALT_2)
    code = ALPHABET[prize_id]
    for i in range(6):
        code += ALPHABET[(h1 >> (5 * i)) & 31]
    code += ALPHABET[((h1 >> 30) ^ h2) & 31]
    return code


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    trainer_id = int(sys.argv[1])
    prize_id = int(sys.argv[2])
    code = make_code(trainer_id, prize_id)
    print("Trainer ID : {:05d}".format(trainer_id))
    print("Prize ID   : {}".format(prize_id))
    print("Prize code : {}".format(code))


if __name__ == "__main__":
    main()
