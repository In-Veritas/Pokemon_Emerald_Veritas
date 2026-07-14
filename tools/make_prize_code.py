#!/usr/bin/env python3
"""Generate a Tournament Prize code for Pokemon Emerald Veritas.

Usage:
    python tools/make_prize_code.py <trainer_id> <prize_id> [tournament_id] [placement]

    trainer_id     The 5-digit ID shown on the winner's TRAINER CARD (0-65535).
    prize_id       1-15 — indexes sTournamentPrizes in src/field_specials.c.
                   Currently defined: 1 = GOLDEN CAP, 2 = MASTER BALL,
                   3 = RARE CANDY x5, 4 = SHINY CHARM. IDs 5-15 are open.
    tournament_id  Optional, 0-7 (default 0 = generic congratulations).
                   1 = MAGMA VS AQUA, 2 = SOUTHERN ISLAND. IDs 3-7 open —
                   see sTournamentNames in src/field_specials.c.
    placement      Optional, 1 / 2 / 3 for first / second / third place
                   (default 1). Only shown when tournament_id != 0.

The winner redeems the code at the Cable Club attendant (first desk in any
Pokemon Center 2F) via the TOURNAMENT PRIZE menu option. Codes are bound to
the trainer ID, so they only work on the winner's save, and each prize ID
can be redeemed once per save. With a tournament set, the clerk
congratulates the winner on their placement in that tournament by name.

The algorithm and constants below MUST stay in sync with
RedeemTournamentCode() in src/field_specials.c. Changing the salts (in both
places) and rebuilding invalidates every code generated before the change.
"""
import sys

ALPHABET = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789"  # no I, O, 0, 1
SALT_1 = 0x56455249  # "VERI" — must match TOURNEY_SALT_1 in field_specials.c
SALT_2 = 0x54415321  # "TAS!" — must match TOURNEY_SALT_2
MASK = 0xFFFFFFFF

TOURNAMENTS = {0: "(none)", 1: "MAGMA VS AQUA", 2: "SOUTHERN ISLAND"}
PLACES = {1: "first place", 2: "second place", 3: "third place"}


def mix(x):
    x &= MASK
    x ^= x >> 16
    x = (x * 0x7FEB352D) & MASK
    x ^= x >> 15
    x = (x * 0x846CA68B) & MASK
    x ^= x >> 16
    return x


def make_code(trainer_id, prize_id, tournament_id=0, placement=1):
    if not 0 <= trainer_id <= 0xFFFF:
        raise ValueError("trainer_id must be 0-65535 (the 5-digit card ID)")
    if not 1 <= prize_id <= 15:
        raise ValueError("prize_id must be 1-15")
    if not 0 <= tournament_id <= 7:
        raise ValueError("tournament_id must be 0-7")
    if not 1 <= placement <= 3:
        raise ValueError("placement must be 1, 2 or 3")
    payload = (tournament_id << 2) | (placement - 1)
    h1 = mix(SALT_1 ^ ((trainer_id & 0xFFFF) << 12) ^ (prize_id << 7) ^ payload ^ SALT_2)
    code = ALPHABET[prize_id] + ALPHABET[payload]
    for i in range(6):
        code += ALPHABET[(h1 >> (5 * i)) & 31]
    return code


def main():
    if len(sys.argv) < 3 or len(sys.argv) > 5:
        print(__doc__)
        sys.exit(1)
    trainer_id = int(sys.argv[1])
    prize_id = int(sys.argv[2])
    tournament_id = int(sys.argv[3]) if len(sys.argv) > 3 else 0
    placement = int(sys.argv[4]) if len(sys.argv) > 4 else 1
    code = make_code(trainer_id, prize_id, tournament_id, placement)
    print("Trainer ID : {:05d}".format(trainer_id))
    print("Prize ID   : {}".format(prize_id))
    print("Tournament : {}".format(TOURNAMENTS.get(tournament_id, "#%d" % tournament_id)))
    if tournament_id:
        print("Placement  : {}".format(PLACES[placement]))
    print("Prize code : {}".format(code))


if __name__ == "__main__":
    main()
