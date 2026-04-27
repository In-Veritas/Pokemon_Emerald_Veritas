# Wireless Minigames — Technical Report

> **Scope.** This report documents the three wireless minigames made directly accessible from the Mossdeep Game Corner (Direct Corner) in the Veritas fork: **Pokémon Jump**, **Berry Crush**, and **Dodrio Berry Picking**. For each game it covers the player-facing mechanics, the in-engine code flow / state machine, and the reward tables. The Veritas-specific overhauls — fixed Dodrio hang, scoring rewrites, scrollable reward UI, Mystery Egg shiny prizes, Shiny Charm gating, top-score menu, exit-door fix, Berry Crush dust bump — are documented inline.

---

## 0. At-a-glance summary

| Game | Players | Entry | Source |
|---|---|---|---|
| Pokémon Jump | 2–5 | `StartPokemonJump` | `src/pokemon_jump.c` |
| Berry Crush | 2–5 | `StartBerryCrush` | `src/berry_crush.c` |
| Dodrio Berry Picking | 3–5 | `StartDodrioBerryPicking` | `src/dodrio_berry_picking.c` |

All three are launched from `data/scripts/cable_club.inc:1244-1296` and dispatched at `src/union_room.c:1739-1750`.

```
CableClub_EventScript_WirelessMinigames           // cable_club.inc:1244
 ├─ Pokemon Jump       → LINK_GROUP_POKEMON_JUMP   (capacity 2-5)
 ├─ Dodrio Berry Pick  → LINK_GROUP_BERRY_PICKING  (capacity 3-5)
 └─ Berry Crush        → reached via the older "Use Berry Crush" branch
```

The Pokemon Center 2F **scoreboard NPC** (`CableClub_EventScript_RecordCorner` at `cable_club.inc:606`) now also offers a **Minigame Records** submenu — see §4.

---

## 1. Pokémon Jump

### 1.1 Premise

A cooperative skipping-rope game. 2–5 players each pick a small Pokémon from their party; everyone shares the same vine and must press **A** to jump in time as the rope swings underneath. Score accumulates per clean jump, with bonuses when multiple players peak together.

### 1.2 Eligibility

* **Pokémon must be ≤ 28 inches** and not exclusively swim/burrow/fly.
* 94 species are whitelisted in `sPokeJumpSpecies[]` (`src/pokemon_jump.c:427-529`). The Pokemon Center entry script gates on this via the `IsPokemonJumpSpeciesInParty` special.

### 1.3 Mechanics

Each Pokémon is assigned one of three jump curves based on species:

| Jump type | Total frames | Hang at apex |
|---|---|---|
| `JUMP_TYPE_FAST`   | 27 | 2 frames |
| `JUMP_TYPE_NORMAL` | 32 | 3 frames |
| `JUMP_TYPE_SLOW`   | 38 | 4 frames |

The vine cycles through 11 states. Speed escalates: 8 base speeds × 4 acceleration stages; after stage 8 the game enters a chaotic max-speed mode.

Player input is one button — **A** triggers the jump. Collision is checked while the vine transits `VINE_UPSWING_LOWER → VINE_UPSWING_LOW`: any player still grounded gets hit.

### 1.4 Scoring (Veritas-modified)

* **+20 per clean jump** (was +10 in vanilla).
* **Synchronized-peak bonus** when multiple players hit the apex on the same frame:

```c
// pokemon_jump.c
static const int sScoreBonuses[MAX_RFU_PLAYERS + 1] = { 0, 0, 100, 200, 400, 1000 };
```

| Players synced at peak | Bonus |
|---|---|
| 2 | +100 |
| 3 | +200 |
| 4 | +400 |
| 5 | +1000 |

* Score caps at **99 990**; jump-streak and excellents-streak cap at 9 999.

### 1.5 State machine

The leader runs a function table:

```
GAME_INTRO → WAIT_ROUND → GAME_ROUND → GAME_OVER
                                          │
                            ┌─────────────┼─────────────┐
                       GIVE_PRIZE        SAVE       ASK_PLAY_AGAIN
                            │             │             │
                            └─────────────┴────► RESET_GAME / EXIT
```

`SAVE` runs `Task_LinkFullSave` so all bag/flag changes from `GIVE_PRIZE` (incl. Shiny Charm flag, Master Ball, party egg adds) persist even if the link drops afterwards.

### 1.6 Records

```c
struct PokemonJumpRecords {
    u16 jumpsInRow;
    u16 unused1;
    u16 excellentsInRow;
    u16 gamesWithMaxPlayers;
    u32 unused2;
    u32 bestJumpScore;
};
```

Updated only on strict improvement; viewable from the Pokémon Center 2F scoreboard menu (§4).

### 1.7 Reward table (Veritas — distinct multi-roll)

Each game produces N independent rolls from a unified pool that grows with score. The leader picks a 16-bit seed, broadcasts it via `comm.data`, and every player runs the same LCG so all players see (and receive) the same item list.

**Quantity per score tier** — past 20 000 each additional 1 000 score adds one more roll:

| Score | Items granted |
|---|---|
|  1 000+ |  1 |
|  3 000+ |  2 |
|  5 000+ |  3 |
|  8 000+ |  5 |
| 10 000+ |  8 |
| 12 000+ | 10 |
| 16 000+ | 15 |
| 20 000+ | 20 |
| +1 000  | +1 |

**Pool growth** (each tier *adds* to all lower tiers; Master Ball stacks across high tiers to bias rolls but is hard-capped at 1 per game):

| Score | Tier added |
|---|---|
|  1 000+ | Pomeg, Kelpsy, Qualot, Hondew, Grepa, Tamato Berry, Nugget, Moo Moo Milk, Berry Juice, Lava Cookie |
|  3 000+ | Big Pearl, Fire/Water/Thunder/Leaf/Moon/Sun Stone, Star Piece, Retro Mail |
|  5 000+ | HP Up, Protein, Iron, Carbos, Calcium, Zinc, Rare Candy, Dream Mail |
|  8 000+ | Lucky Egg, Leftovers, Focus Band, Rare Candy, PP Up, Sacred Ash, **Master Ball** (×1), Bead Mail |
| 10 000+ | Choice Band, Soul Dew, **Master Ball** (×2 total), **Mystery Egg** (a guaranteed-shiny random-species egg, can be legendary) |
| 12 000+ | **Master Ball** (×3 total), Scope Lens, King's Rock, Up-Grade, PP Max, Metal Coat |
| 16 000+ | **Master Ball** (×4 total), Mystic Water, Never-Melt Ice, Starf, Enigma, Lansat Berry, Fab Mail, Amulet Coin |
| 20 000+ | **Master Ball** (×5 total) |

**Master Ball cap.** When a roll lands on `ITEM_MASTER_BALL` and a Master Ball was already awarded this game, the entry is replaced by a random Common-tier item. The display list and bag award stay in sync.

**Shiny Charm bonus.** At 20 000+, the player additionally receives **one Shiny Charm**, prepended to the prize list. This is one-shot per save — gated by `FLAG_RECEIVED_JUMP_SHINY_CHARM` (slot `0x054`, formerly `FLAG_UNUSED_0x054`) so the player can't farm it by tossing the existing charm and replaying. The Dodrio Berry Picking minigame has its own independent one-shot via `FLAG_RECEIVED_DODRIO_SHINY_CHARM` (slot `0x4E0`), so a player can earn **two** minigame-sourced Shiny Charms total — one from each game.

**Mystery Egg.** A new placeholder item `ITEM_MYSTERY_EGG` (slot `0x034`, formerly `ITEM_034`) enters the prize pool at the **10 000+** Elite tier and never enters the bag. When rolled, the player receives a guaranteed-shiny egg of a random species (helper `GiveMinigameShinyEgg` in `src/script_pokemon_util.c`) added to the party — or sent to the PC if the party is full. The "Mystery Egg" name still appears on the prize list.

### 1.8 Scrollable, session-cumulative rewards window

The prize window shows roughly 3 lines of items per page. When more rolls remain, the line `...and more!` is appended; pressing **A** clears the page and opens the next. The fanfare plays only on the first page (`PrintReceivedPrizesPage` / `BuildPrizePageString` in `pokemon_jump.c`). All items are silently added to the bag/party up front before the first page is shown, so the player can't mis-press into losing rewards.

The window is **session-cumulative**: every round the player plays in the same lobby has its rolls appended to `sPokemonJump->prizeSession[256]`, and the prize display paginates over the running total. Round 5 of a long session still shows the items earned in rounds 1-4. The session list resets in `InitGame` on a fresh entry to the lobby; `AppendRoundPrizesToSession` runs in state 0 of `TryGivePrize` after the round's items are awarded.

### 1.9 Exit-door warp fix

Vanilla `WarpForWirelessMinigame` (`src/union_room.c:1591`) didn't call `SetCableClubWarp()`, so the post-minigame return tile was whatever stale dynamic-warp slot existed from a prior link session. This made the player exit through what looked like the Union Room door and the metatile under their feet rendered incorrectly. Veritas now calls `SetCableClubWarp()` inside `WarpForWirelessMinigame` so the entry tile is recorded as the return tile.

---

## 2. Berry Crush

### 2.1 Premise

A button-masher. 2–5 players each contribute one berry from their bag; the berries fall into a giant crusher; players all press **A** as fast (and as in-sync) as possible to push the lid down. Reward is **Berry Powder**, spendable at the Slateport powder vendor.

### 2.2 Mechanics

Each berry has a `difficulty` (A-presses needed, 50–200) and a base `powder` value (40–1500). The cumulative target = sum of selected berries' difficulties.

**Sync detection.** Two or more players pressing A on the same RFU frame triggers a "big impact" sprite and a press multiplier:

| # players synced | Press bonus |
|---|---|
| 2 | +1 |
| 3 | +2 |
| 4 | +3 |
| 5 | +5 |

**Time limit.** 10 minutes. Reaching the press target before that triggers `Cmd_FinishGame`; timing out triggers `Cmd_HandleTimeUp`.

### 2.3 Powder formula (Veritas-bumped)

```
silkiness = (numBigSparkles / numBigSparkleChecks) * 50 + 50;     // range 50-100
powder    = sumOfBerryPowders * playerCount * (silkiness / 100) * 1.5;
```

The trailing **×1.5** is new in Veritas — the vanilla yield felt too punishing, so a flat 50 % boost is applied to the final powder value (`berry_crush.c:3030` area).

### 2.4 Sparkle-tier bug fix

A vanilla typo (`#define field sparkleAmount` then writing `game->field = 2` / `= 3`) made tiers 2 and 3 unreachable. Veritas removes the macro hack so all five sparkle amounts (0 – 4) work correctly (`berry_crush.c:2741-2756`).

### 2.5 State machine

```
INIT → PICK_BERRY → DROP_BERRIES → DROP_LID → COUNTDOWN
        → PLAY_GAME (LEADER vs MEMBER variant)
        → FINISH_GAME or TIMES_UP
        → CALC_RESULTS → SHOW_RESULTS (×3 pages) → SAVE
        → ASK_PLAY_AGAIN → CLOSE_LINK → QUIT
```

`SAVE` runs `Task_LinkFullSave` after powder is awarded.

### 2.6 Records

```c
struct BerryCrush {
    u16 pressingSpeeds[4];   // best presses/sec, indexed by playerCount-2
    u32 berryPowderAmount;
    u32 unk;
};
```

Viewable from the Pokémon Center 2F scoreboard menu (§4).

---

## 3. Dodrio Berry Picking

### 3.1 Premise

3–5 players each control a Dodrio. Each Dodrio has three heads (left/up/right); players press **DPAD-LEFT / UP / RIGHT** to extend a head and pick a falling berry from one of 11 columns. Berry colors: Blue (10 pts), Green (30 pts), Gold (50 pts); a missed berry costs 50 pts and turns one of the 10 status squares grey. The round ends when all 10 squares are grey.

In Veritas the player no longer needs to carry a Dodrio (the in-game Dodrio sprite uses the player's Dodrio shiny status only if they happen to have one).

### 3.2 Hang fix

Vanilla had a one-frame race in `FUNC_WAIT_START`: members sent `PACKET_READY_START` exactly once and immediately set `startGame = TRUE` locally; the leader had a single-frame window to receive each member's packet via `AllPlayersReadyToStart()`. Direct entry from the Mossdeep Game Corner (which lacks the Union Room's longer pre-link sync window) made this race trigger reliably and the game would freeze right after the countdown.

Fix:
* New `PACKET_GAME_STARTED` ack packet (`dodrio_berry_picking.c:80-86`).
* `SendLinkData_Member` keeps sending `PACKET_READY_START` every frame in `FUNC_WAIT_START` and **no longer sets `startGame` locally**.
* `RecvLinkData_Member` listens for `PACKET_GAME_STARTED` in `FUNC_WAIT_START` and only then sets `startGame`.
* `SendLinkData_Leader` broadcasts `PACKET_GAME_STARTED` continuously while in `FUNC_WAIT_START` once `startGame` is true.
* `WaitGameStart` holds the leader for ~30 frames after `startGame` flips before transitioning, so the GO ack reaches every member before the leader stops broadcasting it.

### 3.3 Scoring

```c
sBerryScoreMultipliers[] = { BLUE=10, GREEN=30, GOLD=50, MISSED=-50 };
```

Capped at `MAX_SCORE = 999 990`. Minimum threshold for any prize is `PRIZE_SCORE = 1 000` (down from vanilla 3 000).

### 3.4 State machine

Leader/member tasks call `RecvLinkData_*`, dispatch to the funcId table, run `UpdateGame_*`, then `SendLinkData_*`.

```
FUNC_INTRO → FUNC_INIT_COUNTDOWN → FUNC_COUNTDOWN → FUNC_WAIT_START
   → FUNC_PLAY_GAME → FUNC_INIT_RESULTS → FUNC_RESULTS
   → FUNC_ASK_PLAY_AGAIN → FUNC_WAIT_END_GAME → FUNC_END_LINK → FUNC_EXIT
                                                              ↘ FUNC_RESET_GAME
```

### 3.5 Records

```c
struct BerryPickData {
    u32 bestScore;
    u32 berriesPicked;
    u16 berriesPickedInRow;
};
```

Viewable from the Pokémon Center 2F scoreboard menu (§4).

### 3.6 Reward table (Veritas)

**Quantity per score tier** — post-20k, every additional 1 000 score adds one more roll for both players and the winner:

| Score | Items per player | Winner total |
|---|---|---|
|  1 000+ |  1 |  2 |
|  3 000+ |  2 |  3 |
|  5 000+ |  3 |  4 |
|  8 000+ |  4 |  5 |
| 12 000+ |  5 |  7 |
| 16 000+ | 10 | 20 |
| 20 000+ | 10 | 20 |
| +1 000  | +1 | +1 |

**Tier roll** (driven by `sPrizeTierThresholds[playerCount-3]`):

| Tier | Items | 3 players | 4 players | 5 players |
|---|---|---|---|---|
| Common    | Pomeg, Kelpsy, Qualot, Hondew, Grepa, Tamato Berry | 60 % | 43 % | 30 % |
| Uncommon  | Liechi, Ganlon, Salac, Petaya, Apicot Berry | 25 % | 28 % | 33 % |
| Rare      | Nugget, Rare Candy, PP Up, **Pearl**, **Big Pearl**, **HP Up, Protein, Iron, Carbos, Calcium, Zinc** | 13 % | 23 % | 30 % |
| Ultra Rare | Lansat Berry, Starf Berry, PP Max | 1 % | 4 % | 4 % |
| Legendary | **Master Ball** (50 %) or **Mystery Egg → shiny Doduo** (50 %) | 1 % | 2 % | 3 % |

**Master Ball cap** identical to Pokémon Jump: at most one per game; duplicates are re-rolled.

**Shiny Charm bonus.** At 20 000+, every player additionally receives **one Shiny Charm** as a prepended bonus, gated by `FLAG_RECEIVED_MINIGAME_SHINY_CHARM` so it can never be farmed.

**Mystery Egg.** Rolling a Mystery Egg in the Legendary tier produces a guaranteed-shiny **Doduo** egg with **31 IVs in every stat**, added to the player's party (or PC if full). This is one-shot per save — gated by `FLAG_RECEIVED_DODRIO_SHINY_DODUO_EGG` (slot `0x4E1`); after the player gets one, the egg slot collapses to a Master Ball so the legendary tier still produces a meaningful prize. The "Mystery Egg" entry still appears on the prize list. The perfect-IV path is `GiveMinigameShinyEggEx(species, perfectIvs=TRUE)` in `src/script_pokemon_util.c`.

### 3.7 Scrollable, session-cumulative rewards window

`BuildDodrioPrizePage(startIdx)` writes one page (~2 narrow-font lines, ~210 px) into `sPrizeListBuffer`; if more remain, `...and more!` is appended. State 11 of the prize-display loop reacts to **A**: if more pages remain, repaint the next page; otherwise advance.

The window is **session-cumulative**: each round's awarded items are appended to `sSessionPrizeIds[256]` via `AppendSessionPrizes` after `TryGivePrize` returns. `BuildDodrioPrizePage` paginates the session list, so a player on round 4 still sees everything they got on rounds 1-3. The session list resets in `StartDodrioBerryPicking` on fresh lobby entry.

---

## 4. Top-score scoreboard menu (Pokemon Center 2F)

The 2F **Record Corner** NPC (`CableClub_EventScript_RecordCorner`) now opens with a multichoice:

```
> Link records      ← original wireless record-mixing flow
  Minigame records  ← Veritas: opens the per-game record submenu
  Exit
```

The Minigame Records submenu calls one of three existing display specials:

| Game | Special | Source |
|---|---|---|
| Pokémon Jump | `ShowPokemonJumpRecords` | `src/pokemon_jump.c:4291` |
| Berry Crush | `ShowBerryCrushRankings` | `src/berry_crush.c:1855` (now exported in `include/berry_crush.h`) |
| Dodrio Berry Picking | `ShowDodrioBerryPickingRecords` | `src/dodrio_berry_picking.c:3101` |

Adding the entry to `CableClub_EventScript_RecordCorner` automatically gives every Pokemon Center 2F (10+ maps) the new menu — none of the per-map scripts had to be edited.

New multichoice IDs (`include/constants/script_menu.h`):
* `MULTI_RECORD_CORNER_OPTIONS` (117)
* `MULTI_MINIGAME_RECORDS` (118)

---

## 5. Cross-cutting infrastructure

### 5.1 Mystery Egg item (`ITEM_MYSTERY_EGG`)

* Repurposed from `ITEM_034` (slot 52). Stub at `src/data/items.h:694` and `src/data/item_icon_table.h:59`.
* Name: `"Mystery Egg"`. The item never enters the bag — when minigame code rolls it, `GiveMinigameShinyEgg(species)` puts a guaranteed-shiny egg in the party (or PC if the party is full) and the item id is only used as a display entry on the prize list.
* Helpers: `GiveMinigameShinyEgg(u16 species)` and `PickRandomMinigameEggSpecies(void)` in `src/script_pokemon_util.c`.

### 5.2 Shiny Charm flags (one per minigame)

There are now **two** independent one-shot Shiny Charm flags so a player can earn one from each minigame:

* `FLAG_RECEIVED_JUMP_SHINY_CHARM` — slot `0x054` (formerly `FLAG_UNUSED_0x054`). Set when Pokémon Jump awards the 20 000+ Shiny Charm.
* `FLAG_RECEIVED_DODRIO_SHINY_CHARM` — slot `0x4E0` (formerly `FLAG_UNUSED_0x4E0`). Set when Dodrio Berry Picking awards the 20 000+ Shiny Charm.

A third flag, `FLAG_RECEIVED_DODRIO_SHINY_DODUO_EGG` (slot `0x4E1`, formerly `FLAG_UNUSED_0x4E1`), gates the once-per-save Mystery Egg → 31-IV shiny Doduo prize on Dodrio.

The flags only track the minigame-sourced rewards. Players who own additional Shiny Charms from any other source are unaffected — the gate is purely "did the minigame ever award one?".

### 5.3 Save persistence on prize

All three games already run `Task_LinkFullSave` after their prize step (Pokémon Jump's `SavePokeJump`, Berry Crush's `Cmd_SaveGame`, Dodrio's prize → save state). No new save logic was needed; the new prize logic (Mystery Egg, Shiny Charm flag, Master Ball cap, multi-roll rewards) automatically persists through that existing path.

---

## 6. Other gameplay fixes shipped in this batch

### 6.1 Exp Share egg leveling bug

In `Cmd_getexp` case 2 (`src/battle_script_commands.c`), the inner egg check only suppressed the message; downstream code still consumed `gBattleMoveDamage` and applied EXP. With Exp Share / Exp All active and an egg in slot 0, the egg was effectively gaining EXP and could level up internally before hatching.

Fix: skip eggs at the very top of case 2 the same way level-capped mons are skipped — set `gBattleMoveDamage = 0`, advance `getexpState = 5`, decrement `sentInPokes`, and break.

### 6.2 Latios + Latias breeding

* New compatibility tier `PARENTS_LEGENDARY_PAIR = 5` in `include/constants/daycare.h` — about a 5 % egg chance per 256 steps in the daycare.
* `GetDaycareCompatibilityScore` in `src/daycare.c` special-cases the Latios + Latias pair before the `EGG_GROUP_NO_EGGS_DISCOVERED` blocklist; pairing either with Ditto returns `PARENTS_INCOMPATIBLE`.
* `DetermineEggSpeciesAndParentSlots` overrides the offspring species: male egg → Latios, female egg → Latias (driven by the existing `EGG_GENDER_MALE` bit on `offspringPersonality`).
* `SetInitialEggData` writes `LEGENDARY_PAIR_EGG_CYCLES = 200` into the egg's `MON_DATA_FRIENDSHIP` (the hatch counter) for these eggs, capped at 255 to fit the u8 storage.
* The hatch decrement loop in `TryProduceOrHatchEgg` toggles a file-scope EWRAM parity flag (`sLegendaryHatchTickParity`) on every 256-step tick; for Latios/Latias eggs the decrement is skipped on alternate ticks, halving the rate. Combined with the cycle cap that yields ~100 000+ steps to hatch.

---

## 7. Files of interest

| Concern | Path |
|---|---|
| Pokémon Center entry scripts | `data/scripts/cable_club.inc:1244-1296` |
| Wireless minigame multichoice IDs | `include/constants/script_menu.h:91, 117-118` |
| Activity dispatch (link → game) | `src/union_room.c:1739-1750` |
| Wireless-minigame warp (exit fix) | `src/union_room.c:1591-1599` |
| Pokémon Jump core | `src/pokemon_jump.c` |
| Pokémon Jump prize tables / multi-roll | `src/pokemon_jump.c:2250-2470 area` |
| Berry Crush core | `src/berry_crush.c` |
| Berry Crush sparkle bug fix | `src/berry_crush.c:2741-2756` |
| Berry Crush powder ×1.5 | `src/berry_crush.c:3030 area` |
| Berry powder values | `src/berry.c:892-938` |
| Dodrio Berry Picking core | `src/dodrio_berry_picking.c` |
| Dodrio hang fix (`PACKET_GAME_STARTED`) | `src/dodrio_berry_picking.c:80-86, 899-916, 1581-1680, 3413-3460 area` |
| Dodrio prize tables + tier roll | `src/dodrio_berry_picking.c:596-621, 2887-2902` |
| Dodrio paginated prize display | `src/dodrio_berry_picking.c:2880 area, 5085-5125` |
| Mystery Egg item def | `include/constants/items.h:66`, `src/data/items.h:694`, `src/data/item_icon_table.h:59` |
| Mystery Egg helper | `src/script_pokemon_util.c:113-150 area`, `include/script_pokemon_util.h` |
| Shiny Charm flag | `include/constants/flags.h:101` |
| Top-score scoreboard menu | `data/scripts/cable_club.inc:606-650 area`, `src/data/script_menu.h`, `include/constants/script_menu.h:117-118` |
| Latios/Latias breeding | `src/daycare.c:862-1080 area`, `include/constants/daycare.h` |
| Exp Share egg fix | `src/battle_script_commands.c` (Cmd_getexp case 2) |
| Existing French rewards doc | `docs/MINIGAMES_REWARDS.md` |
| Save struct fields | `include/global.h:214-245` |

---

*Report current as of the second commit on the wireless-minigame branch.*
