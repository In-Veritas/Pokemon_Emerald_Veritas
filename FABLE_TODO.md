# Fable To-Do

Ideas captured for Fable to pick up in a later session (once usage credits are
available again). These are **not started** — each is a prompt/brief, not a plan.

---

## 1. 4-player double battle over link (2v2, one Pokémon each) — ✅ DONE (2026-07-27)

**Resolved: the mode already existed.** Vanilla Multi Battle is exactly this —
4 human players, two teams of two, each controlling their own Pokémon in a
double battle. Verified in Veritas: wired Cable Club + wireless Direct Corner
(`LINK_GROUP_MULTI_BATTLE`, capacity 4), all four battlers human-controlled
(`InitLinkBtlControllers`), each player picks 1–3 mons at their colosseum seat
(minimum 1 → "one Pokémon each" works out of the box). Teams = same side of
the colosseum table (seat parity).

Fixed while verifying: the 4-player VS mugshot picked the opponent by
link-slot parity instead of seat parity and could show a teammate
(`src/battle_transition.c`, `Mugshots_CreateTrainerPics`); plus two stale
comments and a fictional README claim about side-by-side dual mugshots.
Documented under "4-Player Multi Battles" in README.md.

> ❯ Would it be possible to make a 4v4 battle mode on link battles, where each
> player controls one pokemon in a double battle, in two different teams?

**What's wanted:** a link battle where **four human players** connect, split into
**two teams of two**, and fight a **double battle** — each player controlling a
single Pokémon on their side. (The "4v4" in the title means four players / four
Pokémon on the field at once, two per side, not 4 Pokémon per trainer.)

**Starting pointers (unverified — Fable should confirm before trusting):**
- Vanilla Emerald already has a **Multi Battle** (`BATTLE_TYPE_MULTI`), a
  2-players-per-side double battle, reachable through the Cable Club / Union
  Room. That is the closest existing scaffold; the question is whether it
  already supports 4 *separate* human controllers or pairs a player with an
  NPC/partner. Start there rather than building battle plumbing from scratch.
- Relevant battle-type flags to look at: `BATTLE_TYPE_MULTI`,
  `BATTLE_TYPE_LINK`, `BATTLE_TYPE_DOUBLE`, and how `gLinkPlayers[]` /
  `GetMultiplayerId()` map link slots to battler positions.
- This session already touched link-battle infrastructure that Fable can reuse
  as reference for how negotiated link state is broadcast: the **link battle
  scene/terrain picker** (broadcast via a TrainerCard filler byte + XOR
  resolve, `battle_bg.c`), the **link battle chamber transition**, and the
  **RS-style dual-sprite flag broadcast** for multi battles. See memory notes
  `project_link_battle_scene_picker` and `project_link_battle_chamber_transition`,
  and the RFU packet-gate rules in `reference_rfu_packet_gate` (leader must
  transmit to receive; `Rfu_SendPacket` no-ops when `gSendCmd` is busy).
- Open questions to resolve first: does the wireless adapter path support 4
  simultaneous battlers, or is this cable-only? How is turn input collected
  from 4 controllers? Team assignment UI (who picks sides)?

---

## 2. Champion Exclsior — ✅ DONE (2026-07-27)

Re-enabled the roll in `EverGrandeCity_ChampionsRoom/scripts.inc`: after the
player has beaten `TRAINER_WALLACE_2` once, each Champion's Room entry has a
1-in-5 chance of Exclsior replacing Wallace. Also added
`CheckPartyForStarterUnlocks` to his victory path for parity with the other
champion fights. Trainer/party/dialogue/map object were already final.

> ❯ Add champion Exclsior

Bring back the **Champion Exclsior** cameo — the post-game trainer who has a
chance to replace Wallace in the Champion's Room as a tribute to Exclsior (the
person, author of Emerald Legacy Enhanced). The scaffolding already exists but
is **disabled**:
- `TRAINER_CHAMPION_EXCLSIOR` (opponents.h, id 888), party `sParty_ChampionExclsior`
  (`src/data/trainer_parties.h`), display name `_("Exclsior")` (`src/data/trainers.h`).
- Scripts in `data/maps/EverGrandeCity_ChampionsRoom/scripts.inc`:
  `EventScript_TryExclsiorRoll` and `EventScript_ChampionExclsior` — currently
  commented out ("Exclsior champion feature disabled for now", ~line 18).
- Gate flag `FLAG_HIDE_CHAMPIONS_ROOM_EXCLSIOR` (0x4F) and its own
  `LOCALID_EXCLSIOR` (=5) in that room are already reserved for this.
- Note: this is the **person's** cameo, deliberately kept as "Exclsior" and NOT
  renamed to Veritas (the Southern Island superboss is the one that became
  `TRAINER_VERITAS`). See the reserved-space comments in flags.h / opponents.h.

**To do:** decide the trigger (the old code rolled a chance after beating
`TRAINER_WALLACE_2`), re-enable the roll script, and finalize the party/dialogue.

**Absolute constraint (applies to everything here):** do **not** change
SaveBlock1/SaveBlock2 structure — no new vars/flags past the END markers, no
struct resizes. Reuse existing unused slots. See `MEMORY.md` rule #1.
