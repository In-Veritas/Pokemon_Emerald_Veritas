# Dead-clock (frozen RTC) compensation

On a cartridge with a dead RTC battery — or a bootleg whose clock chip never
ticks — the real-time clock is frozen. Every system that advances on wall-clock
time silently stops. This doc tracks what has been made to run on **steps**
instead, and what could be next.

## Detection

`bool8 RtcIsClockNotRunning(void)` (`src/rtc.c`) is the shared predicate. It
returns TRUE when:
- the RTC never answered the boot probe (`RTC_INIT_ERROR`) — the common bootleg
  case, which vanilla stays silent about because that bit is **not** in
  `RTC_ERR_FLAG_MASK` (0x0FF0); or
- the chip returns valid-looking data that has **not advanced** since the boot
  snapshot (dead oscillator / fake constant RTC).

Call it only well after boot (from the overworld), so a healthy clock has had
time to tick. On a working cart it returns FALSE and every compensation below
is a complete no-op.

## Implemented

### PokéNav match calls (earlier work)
`UpdateMatchCallMinutesCounter()` in `src/match_call.c` counts steps with zero
clock movement; after ~1200 steps of no movement it declares the clock frozen
and paces call opportunities by steps (one gate pass per ~600 steps) instead of
the 10-minute wall-clock gate.

### Berry growth — `src/clock.c`
When `RtcIsClockNotRunning()`, `UpdatePerMinute()` diverts to
`UpdateBerriesByStep()`, which grows berries from `GAME_STAT_STEPS` instead of
the RTC:

- A synthetic clock ticks **one minute per `STEPS_PER_BERRY_MINUTE` (67) steps**.
- `StepsToSyntheticBerryTime()` turns the lifetime step count into a
  `struct Time`; the existing `BerryTreeTimeUpdate(minutes)` path then does all
  the real growth / yield / regrowth / watering work unchanged.
- Calibration: the most common berry stage is **3 hours** (13 of the berries),
  and `180 min × 67 ≈ 12,060 steps` per stage — the ~12,000-steps-per-phase
  target. Faster/slower berries scale in proportion (1h ≈ 4,000 steps/stage,
  24h ≈ 96,000), so berry balance is preserved rather than flattened.
- Change `STEPS_PER_BERRY_MINUTE` to retune (higher = slower berries).

**No save-structure change.** All state lives in the existing
`gSaveBlock2Ptr->lastBerryTreeUpdate` field (which the RTC path leaves untouched
while the clock is frozen) plus one reclaimed flag,
`FLAG_BERRY_STEP_BASELINE_SET` (was `FLAG_UNUSED_0x4EB`). The flag latches the
first frozen update so an existing high-step save just captures a baseline
instead of dumping one huge growth (which would trip `BerryTreeTimeUpdate`'s
death threshold and kill every tree). The healthy-clock path clears the latch
and re-baselines, so a save moving between a dead-battery cart and a working RTC
transitions cleanly in both directions.

Why not other storage: `struct BerryTree` is fully packed (no spare bit), every
EventVar 0x40DF–0x40FF is allocated, and unused game stats (52–63) are
write-blocked by `SetGameStat`'s `index < NUM_USED_GAME_STATS` guard. The
existing berry timestamp was the only clean persistent home.

## Ideas not yet implemented

These are the other RTC-dependent systems that die on a frozen clock. Listed
roughly by value/effort. None are started.

### A. Daily events (biggest win) — step-derived day counter
`UpdatePerDay()` in `src/clock.c` drives, via `daysSince`, all of:
`ClearDailyFlags()` (lottery pick, berry-lady gifts, secret-base daily, Shoal
daily, contest lobby, etc.), `UpdateDewfordTrendPerDay`, `UpdateTVShowsPerDay`,
`UpdateWeatherPerDay` (abnormal-weather Kyogre/Groudon reroll), `UpdateMirageRnd`
(Mirage Island), `UpdateFrontierManiac`, `UpdateFrontierGambler`,
`SetShoalItemFlag`, `SetRandomLotteryNumber`, `UpdatePartyPokerusTime`. All
frozen today. A step-synthesized day counter (mirror of the berry approach,
e.g. one synthetic day per 20–40k steps) fed to `UpdatePerDay` would revive the
whole daily-event economy at once. **Risk:** wide blast radius; needs a large
steps-per-day so a single walk doesn't jump many days and thrash TV shows /
trends. Same no-save-change storage trick (reuse `VAR_DAYS` + a latch flag).

### B. Shoal Cave tides — time-of-day
Low/high tide is chosen from the RTC hour, so a frozen clock locks the cave to
one tide forever (blocking the Shell Bell / Shoal-salt loop). Could rotate tide
on a step cycle when frozen.

### C. Lottery Corner winnable
Even with daily flags revived (idea A), confirm `SetRandomLotteryNumber`
produces a varying number under the synthetic day counter, or the lottery is
unwinnable.

### D. Celebi / wall-clock event
The Celebi Mirage-Island-clock trigger keys off specific real times; on a frozen
clock it can never fire. Would need a step- or interaction-based alternative.

### E. Player-facing notice
On a bootleg the vanilla "clock has stopped" screen never appears (silent probe
failure). A one-time in-game notice that "your cartridge clock isn't running —
time-based events now run on steps" would tell the player why things changed.
Low effort, purely informational.

### F. Pokérus spread timing
`UpdatePartyPokerusTime` uses day deltas; minor, would come free with idea A.

### G. Day/night tint & time encounters
Removed previously for performance, but if ever restored they'd also need a
step-driven clock on frozen carts.
