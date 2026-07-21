# PokéNav Match Calls — Veritas changes

Breadcrumb trail for review. Everything below is in four files:

| File | What changed |
|---|---|
| `src/match_call.c` | trigger fixes, two new call families, Veritas cry playback |
| `data/text/match_call.inc` | 39 new `MatchCall_*` text labels (one Veritas block at the end of the file) |
| `include/strings.h` | 39 matching `extern const u8 …[]` declarations |
| `include/constants/flags.h` | two daily flags reclaimed from `FLAG_UNUSED_0x923` / `0x924` |

No save-structure change. No new vars. The three flags used are all inside the
existing `DAILY_FLAGS` block, which `ClearDailyFlags()` wipes at midnight.

---

## 1. Why players weren't getting calls

Two independent causes, both fixed.

**a) Vanilla silently drops most calls.** `TryStartMatchCall()` is a chain of
five gates. The last one, `SelectMatchCallTrainer()`, picks a random registered
trainer and then requires *either*

- that trainer is rematch-ready **and standing on the player's current map**, or
- `ShouldTrainerRequestBattle()` — which **hard-returns `FALSE` below 5 badges**

If neither holds, vanilla returns `FALSE` and the call is thrown away. So for
most of the game a call needed a rematch-ready trainer to happen to live on the
route the player was standing on. That is the drought.

Fix: a failed pick no longer always drops. One in three becomes a *casual call*
(`FORCED_CALL_CASUAL`) — the same trainer rings, but with tip/rumour/chatter
material instead of a rematch request.

**b) A frozen RTC jams the 10-minute gate forever.** `UpdateMatchCallMinutesCounter()`
compares `gLocalTime` against a stored minute. On a cart with a dead RTC battery
— or a bootleg whose clock never ticks, which is confirmed to be in this
project's player base — that delta is permanently `0`, so the gate never opens
and the PokéNav goes silent for good.

Fix: the function counts consecutive checks that saw zero clock movement. It
runs once per 10 steps, and a working clock always crosses a minute boundary
within ~120 such checks, so 120 checks with no movement means the clock is dead.
From then on the wait is emulated with steps (one pass per 600 steps). Players
with a working RTC never reach the counter.

**Pacing was NOT made faster.** The 10-minute gate is untouched at vanilla.
The per-attempt chance was in fact *lowered*: `Random() % 10 < callChance * 3`
(30%, 60% with a lead Lightning Rod) became `Random() % 20 < callChance * 5`
(25%, 50%), because a failed rematch pick can now still produce a call.

---

## 2. How the new system is structured

`SelectMatchCallTrainer()` decides *who* calls, `SelectMatchCallMessage()`
decides *what they say*. The two communicate through
`sMatchCallState.forcedCallType`:

```
FORCED_CALL_NONE      vanilla rematch/chatter selection
FORCED_CALL_BIRTHDAY  once-per-birthday wish
FORCED_CALL_CASUAL    family A — registered trainer, new material
FORCED_CALL_SPECIAL   family B — fixed caller, identified by specialCallId
```

**Family A (`FORCED_CALL_CASUAL`)** — the caller *is* a registered rematch
trainer, so `{STR_VAR_1}` is filled by the existing `PopulateTrainerName` path
and only extra vars are written by hand. Selection lives in
`SelectCasualMatchCallText()`, which builds a candidate list of categories that
pass their gates and picks one uniformly. `CASUAL_CHATTER` is inserted **twice**
so vanilla trainer voices stay the most common outcome.

**Family B (`FORCED_CALL_SPECIAL`)** — Mom, Prof. Birch, the rival, Steven,
Wallace, a Frontier Brain, Veritas. These people have **no entry in
`sMatchCallTrainers`**, so their name is either baked into the text or written
straight into `gStringVar1`.

> ⚠️ **The one real trap in this change.** `GetTrainerMatchCallId()` scans
> `sMatchCallTrainers` with `while (1)` and **no bound** — hand it a trainerId
> that isn't in the table and it loops forever. So in `SelectMatchCallMessage()`
> the `FORCED_CALL_SPECIAL` branch is placed **before** the
> `matchCallId = GetTrainerMatchCallId(trainerId)` line, not after it like the
> other two forced types. Do not reorder those.

Family B is offered *first*, before the registered-trainer pick, and before the
`numRegistered == 0` early-out — so these calls work even for a player with no
registered trainers at all. Each caller gets its own roll and falls through to
the next on a miss (rarest first).

---

## 3. Everything that was added

### Ability tips — one per ability with a real overworld effect

Verified against *this repo*: Flame Body / Magma Armor (egg hatching) and Pickup
are battle-only here, so they deliberately have **no** tip. Nine tips cover the
abilities that do change overworld behaviour.

| Label | Abilities named |
|---|---|
| `MatchCall_AbilityTip_LightningRod` | Lightning Rod (doubles match-call chance) |
| `MatchCall_AbilityTip_Synchronize` | Synchronize (wild nature matching) |
| `MatchCall_AbilityTip_CuteCharm` | Cute Charm (opposite-gender encounters) |
| `MatchCall_AbilityTip_MagnetPull` | Magnet Pull, Static (type-biased encounters) |
| `MatchCall_AbilityTip_EncounterRate` | Stench, White Smoke, Sand Veil / Illuminate, Arena Trap |
| `MatchCall_AbilityTip_KeenEye` | Keen Eye, Intimidate (repel low levels) |
| `MatchCall_AbilityTip_Hustle` | Hustle, Vital Spirit, Pressure (attract high levels) |
| `MatchCall_AbilityTip_SuctionCups` | Suction Cups, Sticky Hold (fishing) |
| `MatchCall_AbilityTip_CompoundEyes` | Compound Eyes (wild held items) |

### Everything else

| Category | Labels | Caller | Gate |
|---|---|---|---|
| Birthday wish | `MatchCall_BirthdayText1-3` | registered trainer | `IsPlayerBirthdayToday()` + `FLAG_DAILY_BIRTHDAY_MATCH_CALL` unset |
| General tips | `MatchCall_Tip_FanClubBirthday`, `_Pokerus`, `_TradedExp`, `_SecretBase` | registered trainer | none |
| Southern Island rumour | `MatchCall_Lore_SouthernIsland` | registered trainer | none (it's a teaser hint) |
| Dragon lady | `MatchCall_Lore_DragonLady` | registered trainer | `FLAG_DEFEATED_RAYQUAZA` |
| Rare sighting | `MatchCall_Rare_Outbreak` | registered trainer | live mass outbreak, species in Hoenn dex |
| Rare sighting (foreign) | `MatchCall_Rare_OutbreakNational` | registered trainer | live outbreak, species **not** in Hoenn dex, **National Dex unlocked** |
| Late night | `MatchCall_Time_LateNight1-2` | registered trainer | local hour ≥ 22 or < 4 |
| Early morning | `MatchCall_Time_EarlyMorning1-2` | registered trainer | local hour 4–6 |
| Rare catch (legendary) | `MatchCall_RareCatch_Legendary` | registered trainer | legendary in party, `FLAG_DAILY_RARE_CATCH_MATCH_CALL` unset |
| Rare catch (shiny) | `MatchCall_RareCatch_Shiny` | registered trainer | shiny in party, same daily flag |
| Leisure — Mom | `MatchCall_Leisure_Mom1-2` | **Mom** | none |
| Leisure — Birch | `MatchCall_Leisure_Birch1-2` | **Prof. Birch** | `FLAG_SYS_POKEDEX_GET` |
| Leisure — rival | `MatchCall_Leisure_Rival1-2` | **Brendan / May** | none |
| Leisure — Steven | `MatchCall_Leisure_Steven1-2` | **Steven** | `FLAG_DEFEATED_METEOR_FALLS_STEVEN` or `FLAG_IS_CHAMPION` |
| Heart-to-heart — rival | `MatchCall_Heart_Rival` | **Brendan / May** | `FLAG_IS_CHAMPION` |
| Heart-to-heart — Steven | `MatchCall_Heart_Steven` | **Steven** | `FLAG_IS_CHAMPION` |
| Champion memory | `MatchCall_Heart_Wallace` | **Wallace** | `GAME_STAT_ENTERED_HOF > 0` |
| Frontier Brain | `MatchCall_Frontier_Brain` | **Anabel/Tucker/Spenser/Greta/Noland/Lucy/Brandon** | any facility streak > 1 |
| Mystery call | `MatchCall_Veritas` | **Veritas** | `FLAG_DEFEATED_VERITAS`, `FLAG_DAILY_VERITAS_MATCH_CALL` unset |

### Rarity rolls (family B, evaluated in this order)

| Caller | Roll |
|---|---|
| Veritas | 1/12 |
| Rival or Steven heart-to-heart | 1/24, then a coin flip between the two |
| Wallace | 1/20 |
| Frontier Brain | 1/16 |
| Leisure (Mom / Birch / rival / Steven) | 1/8, then uniform among the four |

---

## 4. Save-data reads

| Call | Data source |
|---|---|
| Outbreak | `gSaveBlock1Ptr->outbreakPokemonSpecies` / `outbreakDaysLeft` / `outbreakLocation{MapGroup,MapNum}` → `Overworld_GetMapHeaderByGroupAndId()` → `GetMapName()` |
| National-Dex check | `NationalToHoennOrder(SpeciesToNationalPokedexNum(species)) == 0` + `IsNationalPokedexEnabled()` |
| Frontier Brain | `GetFrontierStreakInfo()` over all `NUM_FRONTIER_FACILITIES`, highest streak wins; brain name from `gTrainers[…].trainerName` |
| Wallace | `GAME_STAT_ENTERED_HOF` + the player's first non-egg party mon |
| Rare catch | party scan: `IsMonShiny()` / `IsSpeciesLegendary()`, OT via `MON_DATA_OT_NAME` |
| Rival gender | `gSaveBlock2Ptr->playerGender` → `gTrainers[TRAINER_MAY_ROUTE_103_TREECKO / TRAINER_BRENDAN_ROUTE_103_TREECKO].trainerName` |

**Deliberate non-read: the Hall of Fame roster.** Wallace's call was specced
against "the last Hall of Fame team". The actual HoF lives in flash sectors
28–29 and reading it means `TryLoadSaveSector()` into `gDecompressionBuffer` —
a blocking flash read in the middle of overworld movement. That is not safe to
do from a match call, so the call uses `GAME_STAT_ENTERED_HOF` plus the player's
current lead Pokémon instead. Same emotional beat, no flash access. **If you
want the real roster, this is the line to revisit.**

---

## 5. The Veritas call

`MatchCall_Veritas` is two silent `………………\p` pages followed by
`…Thank you once again.` — then instead of the usual "press A to hang up", the
call plays Lugia's cry and drops **on its own** when the cry finishes.

Implemented in `MatchCall_PrintMessage()`, gated on
`sMatchCallState.veritasCallActive` (set in `BuildSpecialCallString`, cleared
when the call ends) so it cannot affect any other call. It uses task slot
`data[1]` as `tCryState`, which the match-call task zeroes at start and
otherwise never reads (`data[1] = 0; // Never read`).

The unlock flag is **`FLAG_DEFEATED_VERITAS`** (`0x293`) — note the trainer
constant is `TRAINER_VERITAS`, not `TRAINER_VERITAS`; "Veritas" is the in-game
name only. This is the same flag `pokemon_storage_system.c` and `script_menu.c`
already test for his unlocks.

---

## 6. Flags used

| Flag | Value | Purpose |
|---|---|---|
| `FLAG_DAILY_BIRTHDAY_MATCH_CALL` | `DAILY_FLAGS_START + 0x0` | birthday wish fires once per birthday (was `FLAG_UNUSED_0x920`) |
| `FLAG_DAILY_VERITAS_MATCH_CALL` | `DAILY_FLAGS_START + 0x3` | Veritas call fires at most once a day (was `FLAG_UNUSED_0x923`) |
| `FLAG_DAILY_RARE_CATCH_MATCH_CALL` | `DAILY_FLAGS_START + 0x4` | rare-catch congratulation fires at most once a day (was `FLAG_UNUSED_0x924`) |

All three sit inside the existing daily block and auto-clear at midnight via
`ClearDailyFlags()`. Both reclaimed flags were verified to have zero references
anywhere in the tree before being taken.

---

## 7. What to look at first when reviewing

1. `SelectMatchCallMessage()` — the `FORCED_CALL_SPECIAL` branch must stay
   above `GetTrainerMatchCallId()`. See the warning in §2.
2. `SelectCasualMatchCallText()` — `candidates[]` is sized `CASUAL_COUNT + 2`
   and the maximum number of pushes is 9. Adding a category means bumping that.
3. `sFrontierBrainTrainerIds[]` — indexed with this file's **swapped**
   `MATCH_CALL_PIKE` / `MATCH_CALL_FACTORY` convention (see the comment at the
   top of `match_call.c`), matching `sBattleFrontierFacilityNames[]`. Lucy must
   land on Battle Pike and Noland on Battle Factory.
4. `UpdateMatchCallMinutesCounter()` — the frozen-clock fallback. Confirm it
   can't fire for a player with a working RTC.
5. Charmap: the new `.inc` block uses `…` and `é` (both in the charmap) and
   contains **no** em/en-dashes. Verified with a UTF-8-aware scan — note that a
   byte-level grep for `—` false-positives on `…` since both start `E2 80`.
