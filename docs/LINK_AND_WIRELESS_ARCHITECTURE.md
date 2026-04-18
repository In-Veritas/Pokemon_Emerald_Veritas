# Link and Wireless Architecture — Pokemon Emerald Veritas

This document is a reference for how Pokemon Emerald handles multi-Game Boy
Advance communication, both via the wired Game Link Cable (serial /
multi-player SIO) and via the Wireless Adapter (RFU). It also covers Record
Mixing, a Generation 3 feature that Veritas tried to automate after link
battles — that attempt was reverted, and this document describes how the
vanilla manual flow works today, alongside notes about why the auto-mix
experiment failed.

Everything below describes the code that currently ships on `main`. When a
line references something removed as part of the auto-mix revert, it is
marked explicitly.

---

## File Map

| Area | File | Role |
| --- | --- | --- |
| Low-level wired link | `src/link.c` | SIO multi-player driver, handshake, block send/recv, `gLinkPlayers`, `gRecvCmds`, `gSendCmd`, error screen, `CloseLink`, `SetCloseLinkCallback`, `IsLinkMaster`. Vanilla file. |
| Wired link header | `include/link.h` | `struct LinkPlayer`, `struct Link`, `LINKCMD_*`, `LINKTYPE_*`, `MAX_LINK_PLAYERS`, `CMD_LENGTH`. Vanilla file. |
| Cable Club glue | `src/cable_club.c` | The `Task_Linkup*` state machine, `TryBattleLinkup`, `TryTradeLinkup`, `TryRecordMixLinkup`, `CB2_ReturnFromCableClubBattle`, `Task_EnterCableClubSeat`. Bridges field scripts and `link.c`. |
| Cable Club scripts | `data/scripts/cable_club.inc` | `CableClub_OnFrame` dispatch for `VAR_CABLE_CLUB_STATE`, `CableClub_EventScript_ExitLinkRoom`, NPC scripts for Trade Center / Colosseum / Record Corner seats. |
| Cable Club text | `data/text/cable_club.inc` | Attendant dialog, record mix messages. |
| Cable Club constants | `include/constants/cable_club.h` | `USING_SINGLE_BATTLE`, `USING_TRADE_CENTER`, `USING_RECORD_CORNER`, `USING_UNION_ROOM`, etc. |
| Battle-initiated link | `src/battle_main.c` | `BattleMainCB2` runs `EndLinkBattleInSteps` (states 0-9) that unwinds a link battle, fades to black, reconnects for Frontier Pass recording, then closes the link. |
| Wireless driver | `src/link_rfu_2.c` | `InitRFUAPI`, `LinkRfu_*` creation (parent/child), `Rfu_SendPacket`, `Rfu_InitBlockSend`, `Rfu_SetCloseLinkCallback`, `Rfu_GetMultiplayerId`. Mirrors `link.c` API for wireless. |
| Wireless helpers | `src/link_rfu_3.c` | `RfuRecvQueue_*`, `RfuSendQueue_*`, `RfuBackupQueue_*`, host game data helpers, wireless status indicator. |
| Librfu (stock) | `src/librfu_rfu.c`, `src/librfu_intr.c`, `src/librfu_sio32id.c`, `src/librfu_stwi.c` | Vendor-supplied Nintendo RFU API. Don't edit. |
| Union Room | `src/union_room.c`, `src/union_room_battle.c`, `src/union_room_chat.c`, `src/union_room_player_avatar.c` | Wireless-only lobby where players meet. Uses `link_rfu_2.c` underneath. |
| Record mixing core | `src/record_mixing.c` | `Task_RecordMixing_Main` state machine, `Task_MixingRecordsRecv`, `Task_SendPacket`, `Task_CopyReceiveBuffer`, `Task_DoRecordMixing`, `union PlayerRecord` exchange layout. |
| Record mixing header | `include/record_mixing.h` | Exposes `RecordMixingPlayerSpotTriggered`, `GetPlayerHallRecords`, and `struct PlayerHallRecords`. |
| TV / News / Secret Base receive | `src/tv.c`, `src/secret_base.c`, `src/mauville_old_man.c`, `src/dewford_trend.c`, `src/lilycove_lady.c`, `src/daycare.c`, `src/apprentice.c`, `src/battle_tower.c` | Called from `record_mixing.c` to merge received data into save. |
| Record mix text | `src/strings.c` / `include/strings.h` | `gText_MixingRecords`, `gText_RecordMixingComplete` (defined as C strings, not in a .inc). |

---

## 1. Wired Link Flow (Game Link Cable)

### 1.1 Entry Points

The cable club is the second floor of any Pokemon Center. Three attendants
handle three activities:

- **Colosseum attendant** (single / double / multi / Lv50 battles)
- **Trade Center attendant** (trades)
- **Record Corner attendant** (record mix)

When the player talks to an attendant and is accepted:

1. The script sets `VAR_CABLE_CLUB_STATE` to one of the `USING_*` values
   (`constants/cable_club.h`).
2. The script calls `special TryBattleLinkup` (or `TryTradeLinkup` /
   `TryRecordMixLinkup`) in `src/cable_club.c`, which sets `gLinkType`
   (`LINKTYPE_SINGLE_BATTLE`, `LINKTYPE_TRADE_SETUP`,
   `LINKTYPE_RECORD_MIX_BEFORE`, ...) and spawns `Task_LinkupStart`.
3. The player walks to their assigned chair. The field script calls
   `special ColosseumPlayerSpotTriggered`, `PlayerEnteredTradeSeat`, or
   `RecordMixingPlayerSpotTriggered` with `VAR_0x8005` = seat id
   (0-3 for multi, 0-1 for single). See `data/scripts/cable_club.inc`
   around `EventScript_Colosseum_Wireless*` (line 800+), `EventScript_TradeCenter_*`,
   `EventScript_RecordCenter_*`.
4. Each of those three specials dispatches into
   `CreateTask_EnterCableClubSeat(followup)` in `cable_club.c`.
   `Task_EnterCableClubSeat` waits until
   `GetCableClubPartnersReady()` returns `CABLE_SEAT_SUCCESS`, then hands
   control to the followup (`Task_StartWiredCableClubBattle`,
   `Task_StartWiredTrade`, or `Task_RecordMixing_Main`).

### 1.2 Link Initialization

`src/link.c` owns the low-level wired link:

- `OpenLink()` — called by `InitLink()` paths when `!gWirelessCommType`. Resets
  serial, installs the `LinkCB_RequestPlayerDataExchange` callback, spawns
  `Task_TriggerHandshake`, zeros send/recv buffers.
- `SetLocalLinkPlayerId(u8 playerId)` — stores `gLocalLinkPlayer.id`. Used both
  by `Task_EnterCableClubSeat` (passes seat id from `gSpecialVar_0x8005`) and by
  the record mixing state machine.
- `InitLocalLinkPlayer()` — fills `gLocalLinkPlayer` with trainer id, name,
  gender, `gLinkType`, language, version, pokedex progress flag, and (Veritas
  addition) the player's outfit style packed into `.neverRead` via
  `VAR_PLAYER_STYLE | (FLAG_PLAYER_STYLE_RS ? 0x40 : 0)`.
- `Task_TriggerHandshake` waits 5 ticks and flips `gShouldAdvanceLinkState = 1`,
  kicking the main state machine into `LINK_STATE_HANDSHAKE`.

The main loop in `LinkMain1` / `LinkMain2` runs every frame:
- Collects the local command (`gSendCmd[CMD_LENGTH]`) and enqueues it.
- Dequeues received commands from all slots into
  `gRecvCmds[MAX_RFU_PLAYERS][CMD_LENGTH]` and calls `ProcessRecvCmds`.
- Handles block send (`LinkCB_BlockSendBegin`, `LinkCB_BlockSend`,
  `LinkCB_BlockSendEnd`) and block receive notifications
  (`SetBlockReceivedFlag`).
- Detects errors and fills `sLinkErrorBuffer`; if fatal, `CB2_LinkError` is
  scheduled.

### 1.3 Multiplayer State Machine — Who Talks, Who Listens

- `gLinkStatus` packs role information. `IsLinkMaster()` returns
  `EXTRACT_MASTER(gLinkStatus)` on wired, or `Rfu_IsMaster()` on wireless.
- `gLinkPlayers[i]` (EWRAM, `MAX_RFU_PLAYERS` slots) is the roster received
  during handshake. `gLocalLinkPlayer` mirrors our own slot.
- `gReceivedRemoteLinkPlayers` becomes `TRUE` after the handshake completes
  and every remote's `LinkPlayer` has been transmitted. Most tasks wait on
  this flag before proceeding.
- `GetMultiplayerId()` returns our own slot index (0..3) from `gLink.localId`.
- `gSendCmd[8]` is the outgoing command buffer; tasks call `BuildSendCmd` or
  write to it directly during their tick, and `LinkVSync` ships it.
- `gRecvCmds[player][8]` is filled by `DequeueRecvCmds`.
- Block transfers (for payloads larger than 8 u16 words) are staged via
  `SendBlock(unused, ptr, size)` / `SendBlockRequest(BLOCK_REQ_SIZE_*)` and
  land in `gBlockRecvBuffer[player][]`. `GetBlockReceivedStatus()` returns a
  bitmask.

### 1.4 Disconnection

- `SetCloseLinkCallback()` installs `LinkCB_ReadyCloseLink`, which sends
  `LINKCMD_READY_CLOSE_LINK` until every partner acknowledges
  (`gReadyToCloseLink[i]`). Then `LinkCB_WaitCloseLink` calls `CloseLink()`
  which clears `gReceivedRemoteLinkPlayers`, calls `LinkRfu_Shutdown` if
  wireless, sets `sLinkOpen = FALSE`, and runs `DisableSerial()`.
- The battle teardown path in `EndLinkBattleInSteps` (see below) invokes
  `SetCloseLinkCallback()` in state 8 on wired link only.
- The cable club exit path in `CableClub_EventScript_ExitLinkRoom` calls
  `special CloseLink` directly (via `CableClub_EventScript_CloseLinkAndExitLinkRoom`).

---

## 2. Wireless Link Flow (RFU Adapter)

### 2.1 Adapter Detection

`IsWirelessAdapterConnected()` at `src/link.c:237`:
1. Calls `SetWirelessCommType1()` to flip `gWirelessCommType = 1`.
2. Calls `InitRFUAPI()` in `link_rfu_2.c` to prime the RFU state manager.
3. Calls `rfu_LMAN_REQBN_softReset_and_checkID()` — if the adapter replies
   with its serial ID, we return `TRUE`.
4. If not, switches back to `gWirelessCommType = 0` and calls `CloseLink()`.

`gWirelessCommType` is the global switch: almost every helper
(`SetLinkStandbyCallback`, `SetCloseLinkCallback`, `GetMultiplayerId`, block
send, etc.) branches on it:

```c
if (gWirelessCommType)
    Rfu_SetCloseLinkCallback();
else
    // wired path
```

### 2.2 Union Room and Direct Corner Entry

- The Wireless Adapter attendant on Pokemon Center 2F opens the **Union Room**
  (wireless lobby) — set via `USING_UNION_ROOM`. `src/union_room.c` then
  drives player discovery, chat, battles, trades, card greetings. It uses
  `link_rfu_2.c` for transport, not `link.c`.
- The **Direct Corner** hosts the wired cable-club variants when the wireless
  adapter is plugged in. The attendant opens the same door but signals the
  cable-club system; cable scripts in `cable_club.inc` honor
  `CableClub_EventScript_OpenDirectCornerBarrier` on the way in.

### 2.3 RFU Session Lifecycle

Wireless replaces the serial IRQ handshake with a higher-level state machine
in `link_rfu_2.c`:

1. `RFUSTATE_INIT` / `INIT_END` — set up the API.
2. `RFUSTATE_PARENT_CONNECT` / `CHILD_CONNECT` — the parent creates a group,
   children join. `LmanAcceptSlotFlagIsNotZero()` reports peer activity.
3. `RFUSTATE_CONNECTED` — players exchange `struct SioInfo` (player count,
   per-player `LinkPlayer` roster), mirroring `gLinkPlayers` on every game.
4. `Rfu_InitBlockSend` / `Rfu_SetBlockReceivedFlag` — RFU-flavored versions of
   the wired block transport. They read/write the same `gBlockSendBuffer` /
   `gBlockRecvBuffer` so the consumer code (record mixing, trade, battle) is
   transport-agnostic.
5. `Rfu_SetCloseLinkCallback` — orderly shutdown; equivalent to
   `SetCloseLinkCallback`.
6. `LinkRfu_Shutdown()` — called from `CloseLink` when the session ends.

### 2.4 Differences from Wired Link

| Concern | Wired (`link.c`) | Wireless (`link_rfu_2.c`) |
| --- | --- | --- |
| Player count | 2-4 (SIO multi) | 2-5 (`MAX_RFU_PLAYERS`) |
| Handshake | SIO hardware, `Task_TriggerHandshake`, `DoHandshake` | Higher-level parent/child join, `RFUSTATE_*` |
| Master detection | `EXTRACT_MASTER(gLinkStatus)` | `Rfu_IsMaster()` |
| Block recv flags | `gBlockReceivedStatus[]` / `GetBlockReceivedStatus` | `Rfu_GetBlockReceivedStatus` (shares same fields after translation) |
| Close | `LinkCB_ReadyCloseLink` / `CloseLink` | `Rfu_SetCloseLinkCallback` / `LinkRfu_Shutdown` |
| Reestablish | `CreateTask_ReestablishCableClubLink` (see `cable_club.c`) | Session stays alive until explicit shutdown; no reestablish needed |

The `Task_RecordMixing_Main` state-5 wireless fix in Veritas was added
because the vanilla code referenced the reestablish task id even on wireless,
where that task never got created — see `src/record_mixing.c` state 3/5 below.

---

## 3. Record Mixing

Record mixing is a Gen 3 feature where two to four nearby players exchange
non-trade data: secret bases, TV shows, PokeNews broadcasts, Dewford trends,
Mauville Old Man, Daycare Mail, Battle Tower records, Lilycove Lady, Apprentices,
Ranking Hall records, and a random gift item from player 0.

### 3.1 Vanilla Flow

Entry: Pokemon Center 2F → Record Corner attendant. After the linkup task
completes, the scripts `EventScript_RecordCenter_Spot0..3`
(`data/scripts/cable_club.inc:887+`) each call
`special RecordMixingPlayerSpotTriggered` with the seat id in `VAR_0x8005`.

`RecordMixingPlayerSpotTriggered()` in `src/record_mixing.c:166`:

```c
void RecordMixingPlayerSpotTriggered(void)
{
    CreateTask_EnterCableClubSeat(Task_RecordMixing_Main);
}
```

That sets up a `Task_EnterCableClubSeat` that, once partners are ready,
switches the task function to `Task_RecordMixing_Main`.

`Task_RecordMixing_Main` state machine (`src/record_mixing.c:311`):

| State | Action |
| --- | --- |
| 0 | `Alloc` send/recv records, call `SetLocalLinkPlayerId`, set `VAR_TEMP_MIXED_RECORDS=1`, `PrepareExchangePacket()`, spawn `Task_MixingRecordsRecv` and `Task_RecordMixing_SoundEffect`, transition to 1. |
| 1 | Wait for `Task_MixingRecordsRecv` to end (both sides are in sync). `FlagSet(FLAG_SYS_MIX_RECORD)`. Kill the lights and sound helper. Transition to 2. |
| 2 | Spawn `Task_DoRecordMixing` (which iterates over receive buffers and merges into save blocks). Play `SE_M_BATON_PASS`. Transition to 3. |
| 3 | On `Task_DoRecordMixing` end: if wired, create `CreateTask_ReestablishCableClubLink` and capture its task id in `tLinkTaskId`; if wireless, set `tLinkTaskId = taskId` (self). Print `gText_RecordMixingComplete`. |
| 4 | Wait 60 frames, then go to 5. |
| 5 | Wait for reestablish task to finish (wired) or skip for wireless. `Free` the record buffers, call `SetLinkWaitingForScript`, spawn `Task_ReturnToFieldRecordMixing` for wireless, clear the dialog box, destroy self, re-enable script context. |

### 3.2 Wireless Reestablish Fix (kept)

Commit `7b91ebf8` added the `else tLinkTaskId = taskId;` branch in state 3
and the `gWirelessCommType != 0 || ...` guard in state 5. Without it, wireless
mixing would hang indefinitely because it checked `gTasks[stale].isActive`
against a task that never existed. This fix is still present on `main`.

### 3.3 What Gets Mixed (Data Layout)

The exchange uses `union PlayerRecord` (`src/record_mixing.c:81`):

```c
union PlayerRecord
{
    struct PlayerRecordRS ruby;      // when any partner is RS
    struct PlayerRecordEmerald emerald;
};
```

`struct PlayerRecordEmerald` layout (total 0x1444 bytes, line 65):

```
0x0000  struct SecretBase      secretBases[SECRET_BASES_COUNT];
0x0C80  TVShow                 tvShows[TV_SHOWS_COUNT];
0x1004  PokeNews               pokeNews[POKE_NEWS_COUNT];
0x1044  OldMan                 oldMan;
0x1084  struct DewfordTrend    dewfordTrends[SAVED_TRENDS_COUNT];
0x10AC  struct RecordMixingDaycareMail daycareMail;
0x1124  struct EmeraldBattleTowerRecord battleTowerRecord;
0x1210  u16                    giftItem;          // only player 0 fills this
0x1214  LilycoveLady           lilycoveLady;
0x1254  struct Apprentice      apprentices[2];
0x12DC  struct PlayerHallRecords hallRecords;
0x1434  u8                     filler[16];
```

`PrepareExchangePacket()` fills `sSentRecord->emerald` (or `.ruby` for
cross-version mixing) with pointers taken from `gSaveBlock1Ptr` and
`gSaveBlock2Ptr`. `ReceiveExchangePacket(multiplayerId)` dispatches each
section to the receiver function in the corresponding subsystem
(`ReceiveSecretBasesData`, `ReceiveTvShowsData`, `ReceivePokeNewsData`, ...).

Chunks cross the wire via `Task_SendPacket` / `Task_CopyReceiveBuffer` in
200-byte payloads (`BUFFER_CHUNK_SIZE`). The sender loops until
`sRecordStructSize / 200 + 1` chunks have been sent; the receiver populates
`gBlockRecvBuffer[player][]` and copies chunks into `sReceivedRecords[player]`.

### 3.4 What Record Mixing Currently Does NOT Do (Veritas revert)

The auto-record-mix layer that tried to run record mixing automatically after
every wired link battle has been REMOVED. Specifically:

- `StartRecordMixingDirect()` / `IsRecordMixingTaskActive()` /
  `AbortRecordMixing()` are gone.
- `sCalledFromBattle` and `gRecordMixingAborted` are gone.
- `EndLinkBattleInSteps` state 10 (auto-mix wait with 30s timeout) is gone.
- `CheckRecordMixingAborted` field special is gone.
- `CableClub_EventScript_ShowRecordMixMessage` and
  `CableClub_EventScript_ShowRecordMixError` are gone; `ExitLinkRoom` no
  longer branches on `FLAG_SYS_MIX_RECORD`.
- `CableClub_Text_RecordsMixed` / `CableClub_Text_RecordMixError` are gone.

The manual flow (Cable Club → Record Corner attendant → pick a seat → all
players in their seats → task runs) is untouched. `FLAG_SYS_MIX_RECORD` is
still set inside `Task_RecordMixing_Main` state 1 for the dewford-trend /
diploma / pokenews consumers that check it in other parts of the game.

---

## 4. Battle-Initiated Link Flow

### 4.1 Battle Entry

`TryBattleLinkup()` in `src/cable_club.c:571` sets `gLinkType` based on
`gSpecialVar_0x8004` (single / double / multi / Lv50 variants / battle tower)
and starts `Task_LinkupStart`. Once the linkup succeeds, control transfers
to `Task_StartWiredCableClubBattle` or `Task_StartWirelessCableClubBattle`
which warp to the battle map and install `BattleMainCB2`.

During the battle, `battle_main.c` runs the usual battle scripts. On
`BattleMainCB2` tick after the battle ends, `gBattleMainFunc` eventually points
to `EndLinkBattleInSteps`.

### 4.2 `EndLinkBattleInSteps` State Machine (`src/battle_main.c:2337`)

| State | Action |
| --- | --- |
| 0 | Show BG0/1/2, prime fade timer (`gBattleCommunication[1] = 0xFF`). |
| 1 | Count down, kick a fade-out to black. |
| 2 | After fade: gather `anyLinkBattlerHasFrontierPass` from `RecordedBattle_GetFrontierPassFlag()`. If everyone is Emerald and frontier-pass-eligible, either launch `CB2_InitAskRecordBattle`, reconnect via `Task_ReconnectWithLinkPlayers`, or free resources and exit. |
| 3 | Clear VRAM, reload battle background, fade back in. |
| 4 | Wait for fade. |
| 5 | Wait for `Task_ReconnectWithLinkPlayers` to finish. |
| 6 | When `IsLinkTaskFinished()` is true: `SetLinkStandbyCallback()` and print `gText_LinkStandby3`. |
| 7 | When text is done and link idle: advance to state 8 (close link). |
| 8 | On wired, call `SetCloseLinkCallback()`. Advance to 9. |
| 9 | When not frontier-recording (or on wireless, or only player connected), free battle resources and return to `gMain.savedCallback` (the cable-club return CB). |

Control then reaches `CB2_ReturnFromCableClubBattle` (`src/cable_club.c:1045`),
which updates link-battle records and fans out to
`CB2_ReturnFromUnionRoomBattle` (wireless) or
`CB2_ReturnToFieldFromMultiplayer` (wired). Saving runs via
`CB2_SetUpSaveAfterLinkBattle`, then the cable-club map re-loads and
`CableClub_OnFrame` dispatches `CableClub_EventScript_ExitLinkRoom`.

### 4.3 Why the Auto-Mix Experiment Failed (Historical)

Veritas added an extra state 10 between states 7 and 8 that called
`StartRecordMixingDirect()`, spawned `Task_RecordMixing_Main`, and waited
for it to finish before closing the link. The theory was that since
`gReceivedRemoteLinkPlayers` was still true and SIO was still alive, the
record-mixing task could piggyback onto the existing session.

In practice, three problems kept surfacing:

1. **Link state bleed-through.** `Task_RecordMixing_Main` calls
   `SetLocalLinkPlayerId(gSpecialVar_0x8005)`, which is no longer the mixing
   seat id at that point — it is whatever battle sub-state last wrote to
   `gSpecialVar_0x8005`. The `Task_MixingRecordsRecv` handshake could assign
   wrong player slots, and subsequent `GetLinkPlayerCountAsBitFlags` /
   `GetSavedPlayerCount` comparisons (`Task_MixingRecordsRecv` state 101/201/301 in
   `src/record_mixing.c`) would not converge, hanging mixing there.
2. **Reestablish path irrelevance.** After a battle, the cable club is about
   to close the link, so calling `CreateTask_ReestablishCableClubLink()` was
   nonsensical. The workaround was `sCalledFromBattle` branches that skipped
   the reestablish, but those branches also short-circuited the
   `gText_RecordMixingComplete` text and the 60-frame wait, meaning the mix
   could finish before the buffers were populated by slower peers.
3. **Multi-battle (`BATTLE_TYPE_MULTI`) timing.** With four clients, the
   block-recv buffers fill at different rates. The 30-second
   `AbortRecordMixing` timeout fired frequently because `Task_CopyReceiveBuffer`
   never got the full bitmask. `AbortRecordMixing` then tore down tasks mid-send,
   frequently leaving the other side waiting on a block that never finished.

Because there was no clean way to rendezvous between a battle-ending link
and the record-mixing link state machine — different player id mapping,
different close semantics, different wireless handling — the feature was
ripped out. Manual record mixing at the Record Corner still works because it
starts from a clean cable-club linkup where all these variables are
correctly initialized by `Task_EnterCableClubSeat`.

---

## 5. Appendix — Useful Globals

| Symbol | Meaning |
| --- | --- |
| `gLinkPlayers[MAX_RFU_PLAYERS]` | Roster of all players in the link session. |
| `gLocalLinkPlayer` | Our own slot (copied into `gLinkPlayers[id]` during exchange). |
| `gRecvCmds[player][CMD_LENGTH]` | Per-player command buffer received this frame. |
| `gSendCmd[CMD_LENGTH]` | Outgoing command for this frame; written by `BuildSendCmd`. |
| `gLinkType` | `LINKTYPE_*` flag of the current session; set by `TryBattleLinkup` / `TryTradeLinkup` / `TryRecordMixLinkup`. |
| `gWirelessCommType` | 0 = SIO cable, 1 = Wireless Adapter. |
| `gReceivedRemoteLinkPlayers` | TRUE once the handshake is done and every remote slot has a `LinkPlayer` filled. Most tasks wait on this. |
| `gBlockSendBuffer[0x100]` | Staging area for `SendBlock` / `Rfu_InitBlockSend`. |
| `gBlockRecvBuffer[MAX_RFU_PLAYERS][0x80]` | Per-player block receive buffer. |
| `gBlockReceivedStatus[MAX_LINK_PLAYERS]` | Bitmask of which players' block has arrived. |
| `gBattleCommunication[MULTIUSE_STATE]` | Shared scratch byte used by `EndLinkBattleInSteps` (and many battle controllers). |
| `FLAG_SYS_MIX_RECORD` | Set by `Task_RecordMixing_Main` state 1 once records have been merged — consumed by Dewford trend display, PokeNews updates, etc. |
| `VAR_CABLE_CLUB_STATE` | Selects which cable-club activity is in progress; gates `CableClub_OnFrame` dispatch. |
| `VAR_0x8005` | Seat id when `ColosseumPlayerSpotTriggered` / `PlayerEnteredTradeSeat` / `RecordMixingPlayerSpotTriggered` is invoked. |
| `VAR_TEMP_MIXED_RECORDS` | Transient flag set when a record mix run starts; used to skip some once-per-session logic. |
| `VAR_TEMP_RECORD_MIX_GIFT_ITEM` | Gift item id passed to the `RecordCorner_EventScript_ReceivedGiftItem` branch when player 0 gave a present. |

---

## 6. Scripts — Quick Reference

- `CableClub_EventScript_ExitLinkRoom` (`data/scripts/cable_club.inc:124`) —
  shared exit path for battles / berry crush / minigames. Calls `CloseLink`,
  resets `VAR_CABLE_CLUB_STATE`, plays the exit SE, erases the text box, and
  releases.
- `CableClub_EventScript_ExitRecordCorner` — exits after a manual mix.
- `CableClub_EventScript_ExitTradeCenter` — exits after a trade.
- `EventScript_RecordCenter_Spot0..3` (line 887+) — per-seat entry script that
  calls `RecordMixingPlayerSpotTriggered` and afterwards checks
  `VAR_TEMP_RECORD_MIX_GIFT_ITEM` to show the "sent over one X" message.
- `EventScript_TradeCenter_Chair0..3` — trade entry, calls `PlayerEnteredTradeSeat`.
- `EventScript_Colosseum_Wireless*` / `EventScript_Colosseum_*` — battle seat
  entry, calls `ColosseumPlayerSpotTriggered`.
