# PokéNav Match Calls — Veritas changes

Breadcrumb trail for review. Everything below is in four files:

| File | What changed |
|---|---|
| `src/match_call.c` | trigger fixes, two new call families, Veritas cry playback |
| `data/text/match_call.inc` | 63 new `MatchCall_*` text labels, in two waves at the end of the file |
| `include/strings.h` | 63 matching `extern const u8 …[]` declarations |
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

### Ability tips - one per ability with a real overworld effect

**Corrected 2026-07-21.** The first pass claimed Flame Body / Magma Armor and
Pickup were battle-only in this repo. That was wrong. Both work, and two more
abilities have overworld effects that were missed entirely. All thirteen tips
below were verified against the implementation, not against vanilla.

| Label | Abilities named | Verified at |
|---|---|---|
| `MatchCall_AbilityTip_LightningRod` | Lightning Rod - doubles match-call chance (25% to 50%) | `match_call.c` `CheckMatchCallChance` |
| `MatchCall_AbilityTip_Synchronize` | Synchronize - wild nature match, **100%**, not "often" | `wild_encounter.c:714` |
| `MatchCall_AbilityTip_CuteCharm` | Cute Charm - opposite gender, 2/3 chance | `wild_encounter.c:741` |
| `MatchCall_AbilityTip_MagnetPull` | Magnet Pull (Steel), Static (Electric) - 50%, land; Static also surf | `wild_encounter.c:799-807` |
| `MatchCall_AbilityTip_EncounterRate` | Stench, White Smoke, Sand Veil (half) / Illuminate, Arena Trap (double) | `wild_encounter.c:925-936` |
| `MatchCall_AbilityTip_KeenEye` | Keen Eye, Intimidate - 50% to skip encounters 5+ levels below the lead | `wild_encounter.c:1309` |
| `MatchCall_AbilityTip_Hustle` | Hustle, Vital Spirit, Pressure - 50% max-level spawn | `wild_encounter.c:335` |
| `MatchCall_AbilityTip_SuctionCups` | Suction Cups, Sticky Hold - fishing bite 50% to 85% | `field_player_avatar.c:1942` |
| `MatchCall_AbilityTip_CompoundEyes` | Compound Eyes - wild held item odds | `pokemon.c:7289` |
| `MatchCall_AbilityTip_FlameBody` | **Flame Body, Magma Armor** - egg cycles drop 2 per 256 steps instead of 1. Works from **any party slot**, not just the lead, and the tip says so | `egg_hatch.c:930` |
| `MatchCall_AbilityTip_Pickup` | **Pickup** - 10% per battle per holder with an empty item slot; better tiers at higher level | `battle_script_commands.c:9955` |
| `MatchCall_AbilityTip_Swarm` | **Swarm** - halves the ambient wild-cry delay | `overworld.c:1396` |
| `MatchCall_AbilityTip_HyperCutter` | **Hyper Cutter** - Cut clears 5x5 instead of 3x3 | `fldeff_cut.c:157` |

Deliberately no tip: the Battle Pike status-block abilities (Water Veil,
Limber, Insomnia, Immunity) are frontier-only, and the repo's custom Baby
Charm / Quick Learner / Mystic Tempo are battle-only.

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
| Frontier Brain | `MatchCall_Frontier_Anabel` … `_Brandon` (7) | **the Brain of that facility** | **Silver Symbol held** for that facility AND streak > 1 |
| Mystery call | `MatchCall_Veritas` | **Veritas** | `FLAG_DEFEATED_VERITAS`, `FLAG_DAILY_VERITAS_MATCH_CALL` unset |

### Wave 2: quest hints, legendary rumours, record mixing

Every hint below stops arriving once its quest is finished, so the PokeNav
never nags about something already done.

| Category | Label(s) | Caller | Shown while | Stops when |
|---|---|---|---|---|
| Weather Institute research | `MatchCall_Quest_WeatherResearch` | **Brendan / May** | `FLAG_SYS_GAME_CLEAR` and a beast unresolved | all of Suicune/Raikou/Entei caught or beaten |
| Museum boat ticket | `MatchCall_Quest_MuseumTicket` | registered trainer | `FLAG_IS_CHAMPION` | `FLAG_RECEIVED_MYSTIC_TICKET` |
| Fashionista outfits | `MatchCall_Quest_Fashionista` | registered trainer | any outfit still locked | all 27 `FLAG_STYLE_UNLOCKED_*` set |
| Suicune / Raikou / Entei | `MatchCall_Legend_Suicune` etc. | registered trainer | `FLAG_SYS_GAME_CLEAR` | that beast caught or beaten |
| Legendary birds | `MatchCall_Legend_Birds` | registered trainer | `FLAG_BEAT_ALL_3RD_REMATCHES` | **all three** resolved |
| Mewtwo | `MatchCall_Legend_Mewtwo` | registered trainer | `FLAG_COLLECTED_ALL_SILVER_SYMBOLS` | caught or beaten |
| Jirachi | `MatchCall_Legend_Jirachi` | registered trainer | `VAR_MOSSDEEP_CITY_STATE == 4` | `FLAG_BATTLED_JIRACHI` |
| Celebi | `MatchCall_Legend_Celebi` | registered trainer | `FLAG_IS_CHAMPION` | `FLAG_BATTLED_CELEBI` |
| Deoxys | `MatchCall_Legend_Deoxys` | registered trainer | `FLAG_IS_CHAMPION` | Aurora Ticket received, or Deoxys battled |
| Ho-Oh / Lugia | `MatchCall_Legend_NavelRock` | registered trainer | `FLAG_IS_CHAMPION` | Mystic Ticket received, or Lugia beaten |
| Mew | `MatchCall_Legend_Mew` | registered trainer | `FLAG_IS_CHAMPION` | Old Sea Map received, or Mew caught |
| Trainer sighting | `MatchCall_Mixed_Sighting` | registered trainer | a mixed-in `TVSHOW_TODAYS_RIVAL_TRAINER` or `TVSHOW_BREAKING_NEWS` exists | (data-driven) |
| Someone's catch | `MatchCall_Mixed_RareCatch` | registered trainer | a mixed-in `TVSHOW_POKEMON_TODAY_CAUGHT` exists | (data-driven) |
| Friend's base | `MatchCall_Mixed_SecretBaseM/F` | registered trainer | an occupied `secretBases[1..14]` slot | (data-driven) |
| Rival met someone | `MatchCall_Mixed_RivalMet` | **Brendan / May** | any mixed-in trainer exists | (data-driven) |
| Rival's crush | `MatchCall_Mixed_RivalCrush` | **Brendan / May** | met trainer's gender is known and opposite the rival's, **1/3 of the time** | (data-driven) |
| Norman | `MatchCall_Norman_*` (8) | **Norman** | always (he installs Match Call himself) | never |

#### Notes on the record-mixing calls

- **Gender.** Only `struct SecretBase` records the owner's gender; no TV show
  does (`trendWatcher` aside, which carries nothing else useful). So the
  crush line and the gendered secret-base line both source from secret bases,
  and the TV-show-sourced calls stay gender-neutral ("they").
- **No shiny claim.** Nothing in the mixed data records whether a caught
  Pokemon was shiny -- there is no TV show for it. `MatchCall_Mixed_RareCatch`
  therefore only says *what* was caught, never that it was shiny. The player's
  own shinies are still covered by `MatchCall_RareCatch_Shiny`.
- **Base locations needed a new table.** `secretBaseId` is all the save
  stores, and every base interior reports `MAPSEC_SECRET_BASE`, so `GetMapName`
  cannot name a friend's base route. `sSecretBaseLocations[]` in `match_call.c`
  is a generated 75-entry `secretBaseId -> MAPSEC` table, extracted from the
  `secret_base` bg_events in `data/maps/Route*/map.json`. **If base entrances
  are ever moved or added, regenerate it.**
- **Unterminated names.** `SecretBase.trainerName` is `PLAYER_NAME_LENGTH`
  bytes with *no* terminator slot, unlike TV show names which are
  `PLAYER_NAME_LENGTH + 1`. `CopyUnterminatedPlayerName()` exists for exactly
  this; a plain `StringCopy` runs off into `trainerId`.

#### The Weather Institute call is the POSTGAME quest

It is deliberately **not** about the Team Aqua takeover during the story. It
is the research quest the Institute workers give out after the Hall of Fame,
which sends the player after Suicune, Raikou and Entei. `ShouldOfferWeatherInstituteCall()`
gates on `FLAG_SYS_GAME_CLEAR`, matching the quest-givers themselves
(`Route119_WeatherInstitute_1F/2F` scripts both `goto_if_unset FLAG_SYS_GAME_CLEAR`),
and stops once all three beasts are caught or knocked out.

#### Frontier Brains: Silver Symbol gated, one voice each

A Brain only rings about a facility whose **Silver Symbol the player already
holds** -- i.e. they have beaten that Brain at least once -- and where the
streak is worth mentioning (`> 1`). `IsFrontierBrainCallable()` is the test.

> ⚠️ `GetPlayerSymbolCountForFacility()` in `frontier_util.c` cannot be reused
> here. It computes `FLAG_SYS_TOWER_SILVER + facility * 2` using the **real**
> `FRONTIER_FACILITY_*` ids, but this file deliberately swaps Pike and Factory
> (see the comment at the top of `match_call.c`). `sFrontierSilverSymbolFlags[]`
> is therefore written in this file's swapped index space, matching
> `sBattleFrontierFacilityNames[]`, `sFrontierBrainTrainerIds[]` and
> `sFrontierBrainTexts[]`. Lucy must land on Battle Pike, Noland on Factory.

Each Brain has their own line rather than sharing one generic message:
Anabel quiet and intuitive, Tucker playing to the crowd, Spenser laughing
about bonds over numbers, Greta spoiling for a scrap, Noland impressed you
did it with rentals, Lucy managing four short sentences, Brandon booming
about ruins.

#### Norman

A family B caller with an 8-entry pool, picked uniformly, so **half his calls
are battle advice** and the rest split between checking in and teasing the
player about the rival. His own name is baked into every line, so
`{STR_VAR_1}` carries the **rival's** name instead (harmless in the texts that
don't reference it).

The four tips were checked against this repo's battle code rather than
written from general Gen 3 knowledge:

| Tip | Claim | Verified at |
|---|---|---|
| `Tip_RapidSpin` | clears Spikes, Leech Seed and binding moves, **and leaves the user faster** | `BattleScript_RapidSpinAway` — this repo adds a `+1 SPE` stage that vanilla doesn't have |
| `Tip_RestTalk` | Rest full-heals but sleeps 2 turns; Sleep Talk attacks through it | `EFFECT_REST` / `EFFECT_SLEEP_TALK`, both stock |
| `Tip_SpikesPhaze` | Spikes stack to **3** layers, each hurting more; Roar/Whirlwind force switch-ins into them | `Cmd_trysetspikes` caps at 3; damage is `maxHP / ((5 - layers) * 2)` = 1/8, 1/6, 1/4 |
| `Tip_SubPunch` | Substitute costs 1/4 max HP, absorbs the hit and blocks status; Focus Punch behind it can't be flinched | stock Gen 3 behaviour |

No tip mentions Stealth Rock or Toxic Spikes — **neither move exists in this
repo** (`MOVE_STEALTH_ROCK` / `MOVE_TOXIC_SPIKES` are undefined), so Spikes is
the only entry hazard and the Rapid Spin tip says exactly that.

#### A real flag clash found while doing this

`FLAG_SYS_NO_COLLISION` and `FLAG_STYLE_UNLOCKED_TREECKO` are **both 0x2B5**
(`include/constants/flags.h`). The comment on the former acknowledges it
("Shares with TREECKO unlock (debug only)"). For the outfit hint this is
harmless -- with debug no-collision on, the hint can stop one outfit early --
but it is a genuine collision and worth fixing separately.

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

---

## 8. How often does the PokeNav actually ring?

Target: **about 2 calls per hour of walking.**

The binding constraint is the 10-minute gate, which can open at most **6 times
per hour**. A window that opens and then loses the chance roll is *consumed* --
`UpdateMatchCallMinutesCounter` stamps the timestamp before
`CheckMatchCallChance` runs -- so 6 attempts/hour is a hard ceiling and the
rolls after it have to be generous to hit the target.

Current tuning:
- `CheckMatchCallChance`: `Random() % 4 < callChance + 1` -- **50%** base,
  **75%** with a lead Lightning Rod. (Vanilla: 30% / 60%.)
- casual fallback when the rematch pick fails: **1/2**. (Was 1/3.)

| Stage | Windows/hr | x chance | x caller found | **Calls/hr** |
|---|---|---|---|---|
| Vanilla, under 5 badges | 6 | 0.30 | ~0.03 | **~0.05** (one per ~20 h) |
| Veritas, early game | 6 | 0.50 | ~0.59 | **~1.8** |
| Veritas, mid game | 6 | 0.50 | ~0.61 | **~1.8** |
| Veritas, postgame (all callers unlocked) | 6 | 0.50 | ~0.74 | **~2.2** |
| Veritas, postgame + Lightning Rod lead | 6 | 0.75 | ~0.74 | **~3.3** |

"Caller found" is the chance `SelectMatchCallTrainer` returns TRUE: the family
B rolls in sequence (~0.46 postgame with everything unlocked, ~0.17 early),
then the vanilla rematch check, then the 1/2 casual fallback.

Vanilla's ~0.03 is the odds that a randomly picked registered trainer happens
to be rematch-ready *on the player's current map*, which below the 5th badge
is the only thing vanilla accepts. That number is the whole reason for this
change.

Real-world figures land under the table: the step counter only advances while
walking, so time in buildings, battles, menus and cutscenes doesn't count
toward the next window.
