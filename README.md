# Pokémon Emerald Veritas

Hey thanks for checking this out! Emerald Veritas is a rom hack I made based on Exclsior's Emerald Enhanced. I made this mostly to play with my friends but feel free to try it if you want to.

## About This Project

I believe that Legacy and Enhanced both do an excellent job in capturing Emerald's essence, but since I wanted something more abstract as in "Hoenn's" essence, I made some slight changes. In Veritas, title screens and intros both share cutscenes and elements from Ruby and Sapphire, and the player may choose to get a Ruby/Sapphire look for their character.

After creating a save game, the player also unlocks Groudon and Kyogre for the title screen. 

This version also removes some things from enhanced, such as the gift shiny Pokemon, Stat editor, and Dive speed. The general idea is to give a bit less control to the player and make the experience more noob friendly for casuals whilst also giving enough options so that experienced players can get a kick out of it. 

I also removed the gift Milotic from Legacy in order to make feebas more unique.

Pokémon Emerald Veritas is a fork of [Pokémon Emerald Legacy Enhanced](https://github.com/Exclsior/Pokemon_Emerald_Legacy_Enhanced) by Exclsior, which itself is based on [Pokémon Emerald Legacy](https://github.com/cRz-Shadows/Pokemon_Emerald_Legacy) by TheSmithPlays and the Legacy team. This ROM hack builds upon the excellent foundation of both projects while adding its own unique flavor and modifications.

This is my first ROM Hack, so any suggestions and feedback are highly appreciated!

**Base Version**: Emerald Legacy Enhanced v1.1.4

## Download and Play

* To set up the repository, see [INSTALL.md](INSTALL.md).
* For patching instructions, see the original [Emerald Legacy Enhanced README](https://github.com/Exclsior/Pokemon_Emerald_Legacy_Enhanced#download-and-play).

## What's New in Veritas

This fork includes (almost) all features from Pokémon Emerald Legacy Enhanced (see [Base Features](#base-features-from-emerald-legacy-enhanced) below), plus the following additions and changes:

### Added Features

#### Randomized Intro Sequences

* Three different intro styles with equal probability (33% each):
  * **Emerald Style**: Features Flygon flying through forest scenery
  * **Ruby Style**: Features Latios flying through ocean/cloud scenery
  * **Sapphire Style**: Features Latias flying through ocean/cloud scenery
* Player sprite adapts to match intro style:
  * Flygon intro: 50% Emerald Brendan, 50% Emerald May
  * Lati intros: 50% Classic RS Brendan, 50% Classic RS May
* Dynamic background scenery and animations match selected intro style
* Fixed palette colors for all legendary Pokémon in intro sequences

#### Player Character Style Customization

* Choose between **Emerald** or **Classic** (Ruby/Sapphire) player sprite styles during new game setup
* Style selection persists through naming screen and into gameplay
* Style affects overworld sprites, battle back sprites, and **region map/fly menu icons**
* Style option unlocked after defeating the Elite 4 (correctly gated behind FLAG_SYS_GAME_CLEAR)

#### Legendary Title Screen Animation

* Added animated title screen featuring Groudon and Kyogre(Random chance after creating a Save. Without a save the Title Screen Legendary is always Rayquaza)

#### Dynamic Credits Avatars

* Credits sequence now dynamically displays player and rival sprites based on:
  * **Player's chosen gender** (male/female)
  * **Player's chosen style** (Emerald or Classic RS)
  * **Rival always displays opposite gender** with Emerald style
* Four possible combinations:
  * Emerald Brendan + Emerald May (rival)
  * RS Brendan + Emerald May (rival)
  * Emerald May + Emerald Brendan (rival)
  * RS May + Emerald Brendan (rival)
* Credits timing optimized to match 2:55 soundtrack length

#### Unlockable PC Box Wallpapers

* Added 29 new custom wallpapers organized into 7 themed categories
* Wallpapers unlock progressively as you complete in-game milestones:
  * **Other**: Always available (Block, Pokecenter, Circles)
  * **Pokemon 1**: Catch 100 Pokémon (Zigzagoon, Luvdisc, Togepi, Azumarill, Pikachu, Dusclops)
  * **Pokemon 2**: Catch 200 Pokémon (Ludicolo, Whiscash, Minun, Plusle, Diglett, Pichu)
  * **Team**: Complete Team Aqua Hideout (Aqua 1, Aqua 2, Magma 1, Magma 2)
  * **Contest**: Receive the Pokéblock Case (Cute, Smart, Cool, Tough, Beauty)
  * **Legends**: Catch Latias or Latios (Legendary, Latias, Latios)
  * **Secret**: Defeat Pokémon Trainer Veritas on Southern Island (Exclsior, Veritas)
* Access special wallpapers via PC Box > Wallpaper > Special menu
* For developers: See `docs/CUSTOM_WALLPAPERS.md` for adding new wallpapers

#### Default Options for New Games

* **Pokémon Follower**: Now enabled by default
* **Surfing Pokémon Sprites**: Now enabled by default
* Both options remain fully toggleable in the Options Plus menu

### Technical Improvements

* **RNG Seeding Fix**: Enabled Real-Time Clock (RTC) based RNG seeding at boot
  * Vanilla Emerald only enables this with BUGFIX flag defined
  * Now always active, providing truly random wild encounters, shiny rolls, and other RNG-dependent events
  * Prevents RNG manipulation exploits that rely on predictable seed values
* Changed flag initialization order to prevent follower/surfer defaults from being cleared
* Improved intro scene transitions and sprite management
* **RS/Classic Player Style Animation Fix**: Fixed running animation for Classic (Ruby/Sapphire) player sprites
  * Previously, Classic style players would show a static sprite when running instead of proper animation
  * Added missing running frames to the RS sprite tables and corrected animation table assignment
* **Shadow Palette Fix**: Fixed shadow rendering for both player and follower Pokémon
  * Player shadows now correctly use the player's palette (supports RS/Emerald style switching)
  * Follower shadows explicitly load and use the Brendan palette which contains proper shadow colors
  * Fixes white/incorrect shadow colors when jumping ledges or during other animations
* **Player Style Storage Refactor**: Migrated player style storage from SaveBlock2 field to flag system
  * Now uses `FLAG_PLAYER_STYLE_RS` (flag 0x296) instead of `gSaveBlock2Ptr->playerLookStyle`
  * Avoids modifying the save structure, improving compatibility
  * Flag is properly preserved through new game initialization
* **Save Compatibility**: Automatic migration for older saves
  * Old saves using the SaveBlock2 `playerLookStyle` field are automatically migrated to the new flag system
  * Migration happens transparently when loading a save file
  * No player action required - style preference is preserved
* **Shiny Mechanics Overhaul**: Reworked shiny odds system
  * Base shiny odds: **1/8192** (vanilla rate)
  * Each Shiny Charm adds +1 re-roll attempt
  * Example: 1 charm = 1/4096, 2 charms = 1/2731, 8 charms = 1/910
* **Fast Surf Speed Reduction**: Reduced fast surfing speed by approximately 30%
  * Now uses 6-frame movement cycles instead of 4-frame cycles
  * Provides smoother surfing experience while still being faster than normal
* **Pokérus Rate Increase**: Increased Pokérus encounter rate from ~1/21845 to **1/2048**
  * Makes this beneficial mechanic more accessible to players

### New Options Menu Items

* **Short POKéCENTER**: Added option to World menu
  * When enabled (ON): Skips the "Would you like to heal?" yes/no question at Pokémon Centers
  * When disabled (OFF): Original vanilla behavior with the yes/no prompt
  * Default is OFF (vanilla experience)

* **Devon Scope Toggle**: Added option to World menu (only visible after receiving Devon Scope)
  * When enabled (ON): Shows sparkling Feebas tiles on Route 119, 5% encounter rate on those tiles
  * When disabled (OFF): Vanilla Feebas hunting mechanics with hidden tiles, 50% encounter rate
  * Toggling this option also remixes the Feebas tile locations
  * Allows players to choose between the QoL visible tiles or the classic blind fishing experience

### Removed Features

#### Debug System

- Debug menu access (R+Start trigger removed)
- Debug menu option from Start menu
- All debug functionality disabled in production builds

#### Game Modes

- Removed Nuzlocke(Hardcore) mode

- Removed National Dex mode

#### Options Menu Simplifications

* **Dive Speed option** removed from Surf submenu
* **Stat Editor visibility toggle** removed (Stat Editor itself remains functional)
* **Button Mode simplified** - Removed "LR" mode option, now only offers "NORMAL" and "L=A" modes

#### Gift Pokémon Changes

The following gift Pokémon are **no longer guaranteed shiny** (changed from Enhanced base):

* **Eevee from Lanette** (Route114) - changed to regular encounter
* **Eevee from Trick House** (completion reward) - changed to regular encounter
* **Snorlax from Trainer Hill** (completion reward) - changed to regular encounter

Still shiny gift Pokémon (unchanged from Enhanced):

* **Beldum from Scott** (Battle Frontier Gold Symbols reward) - remains shiny
* **Latios/Latias** (Southern Island) - remains shiny

#### Contest Reward Changes

Contest painting rewards have been modified:

* **Master Beauty Contest reward** - Changed from Milotic to Misdreavus (Level 50)
* Other contest rewards remain unchanged:
  * Cool Contest: Slaking (Level 50)
  * Cute Contest: Delcatty (Level 50)
  * Smart Contest: Gardevoir (Level 50)
  * Tough Contest: Aggron (Level 50)

## Base Features from Emerald Legacy Enhanced

This ROM hack includes **all features** from Pokémon Emerald Legacy Enhanced v1.1.4, including:

* **All Pokémon Emerald Legacy features** (improved story, better trainer battles, expanded post-game)
* **Pokémon Followers** with shiny support
* **Unique Surfing Sprites** per Pokémon
* **Optional National Dex Mode** (all 9 starters available from start)
* **HM Improvements** (no need to teach HMs, just have them in bag)
* **Shiny Charms** (up to 8 available through in-game milestones)
* **Stat Editor** (IV/EV editing after National Dex)
* **Nature Mints** & **Ability Capsules**
* **Egg Move Tutor** (post-game)
* **EXP. All** (Gen 3 style party-wide exp share)
* **Expanded Options Menu** with tons of customization
* And many more quality of life improvements!

For a complete list of Enhanced features, see the [full Emerald Legacy Enhanced README](https://github.com/Exclsior/Pokemon_Emerald_Legacy_Enhanced#base-patch---v114).

For details on base Emerald Legacy changes, see the [Emerald Legacy Main Doc](https://docs.google.com/document/d/1rBSuhFmiiehghr3AQ37JwBzbLCD21TXo_SWpUUXsz9k/copy).

## Credits

### Pokémon Emerald Veritas

* **Developer**: In-Veritas

### Based On

* **Pokémon Emerald Legacy Enhanced** by Exclsior and team
  * [GitHub Repository](https://github.com/Exclsior/Pokemon_Emerald_Legacy_Enhanced)
* **Pokémon Emerald Legacy** by TheSmithPlays, cRz Shadows, and the Legacy team
  * [GitHub Repository](https://github.com/cRz-Shadows/Pokemon_Emerald_Legacy)
* **pokeemerald Disassembly** by the Pret team
  * [Pret Projects](https://pret.github.io/)

### Special Thanks

* Exclsior for creating Emerald Legacy Enhanced and providing an excellent foundation
* TheSmithPlays and the entire Legacy team for Pokémon Emerald Legacy
* The Pret team for the pokeemerald disassembly
* All feature creators credited in the original Enhanced README

## Community Links

* **Pokémon Legacy Discord**: [Join Server](https://discord.gg/Wupx8tHRVS)
* **Pokémon Legacy Reddit**: [r/PokemonLegacy](https://www.reddit.com/r/PokemonLegacy)
* **Emerald Legacy Enhanced Thread**: [Discord Channel](https://discord.com/channels/1111380675837308948/1328484761148198973)

---

# Full Feature Documentation

For complete documentation of all Emerald Legacy Enhanced features, please refer to the sections below or visit the [original Enhanced README](https://github.com/Exclsior/Pokemon_Emerald_Legacy_Enhanced).

<details>
<summary><b>Click to expand: National Dex Mode Details</b></summary>

</details>

<details>
<summary><b>Click to expand: Shiny Charm System</b></summary>

Completely optional to use Shiny Charms have been added to the game. These are a "secret" feature, meaning no in-game NPCs or dialogue refer to their existence.

In-game milestones for Shiny Charm acquisition (all added to Player Item PC, max 8 available per game.):

* One from start of the game
* Beating the Game (E4 and Champion)
* Getting all Contest Artworks in Lilycove Museum
* Defeating Steven in Meteor Falls
* Completing Hoenn Pokédex (And speak to Prof. Birch)
* Completing National Pokédex (And speak to game developers in Lilycove City)
* Getting all Silver Symbols in Battle Frontier
* Getting all Gold Symbols in Battle Frontier

**Key Notes:**

* Shiny Charms will be silently awarded to the player's item PC when in-game milestones have been met.
  * do not work if left or stored in the player's item PC.
* Shiny Charms only take effect if they are in the player's bag.
* Shiny Charms cannot be tossed.
* Breeding Eggs from Daycare are impacted by Shiny Charm(s) as long as they're in the bag when you collect the egg.
* Shiny Charms can be held by Pokémon and therefore traded between different Emerald Legacy Enhanced games.
  * do not change Shiny changes if held by a Pokémon
* If migrating a save and already completed a relevant milestone, the Shiny Charm will be added to your PC, except for the Pokédex Shiny Charms which require the above noted in-game tasks to be added to the PC.

**Shiny Charm Rates (Veritas System):**

Veritas uses a re-roll system where each Shiny Charm adds +1 re-roll attempt. Base odds are 1/8192 (vanilla rate):

| Charms | Total Rolls | Odds | Percentage |
|--------|-------------|------|------------|
| 0 | 1 | 1/8192 | 0.012% |
| 1 | 2 | 1/4096 | 0.024% |
| 2 | 3 | 1/2731 | 0.037% |
| 3 | 4 | 1/2048 | 0.049% |
| 4 | 5 | 1/1638 | 0.061% |
| 5 | 6 | 1/1365 | 0.073% |
| 6 | 7 | 1/1170 | 0.085% |
| 7 | 8 | 1/1024 | 0.098% |
| 8 | 9 | 1/910 | 0.110% |

*Note: Most players will have 3 charms by end of main story (1/2048 odds).*

</details>

<details>
<summary><b>Click to expand: Options Menu Features</b></summary>

* Ability to enable to disable Pokémon followers.
* Ability to toggle between unique per-Pokémon surfing overworld and original "Surf blob"
* Ability to increase player and Non-player character movement speed in the world.
* Ability to enable or disable Auto Run.
  * Run without holding the B Button.
  * Hold B to walk.
* Ability to enable to disable Fast Surf.
  * Fast Surfing without holding the B button.
  * Hold B to Surf at normal speed.
* Ability to enable or disable Improved Fishing: Does not allow hooked fish to escape (disabled by default).
* Ability to change diving movement speed.
* Ability to enable or disable Bike Music.
* Ability to enable or disable Surf Music.
* Ability to swap game battle mode between Normal, Hard and Hardcore (Nuzlocke) after defeating Steven in Meteor Falls.
* Ability to change speed of HP Bar draining in battle.
* Ability to change speed of EXP Bar filling in battle.
* Ability to reduce or turn off in-battle item use animation.
* Ability to toggle Type Effectiveness colour coding within battle (Off by default).
  * Green: Super effective, Red: Not very effective, Grey: No effect.
* Ability to change in-game font from Hoenn (original Emerald font) to Kanto (FireRed/LeafGreen font).
* Ability to hide post-game Stat Editor from Party Menu.
* Ability to hide Nickname option from Party Menu.
* Ability to change Battle Mode (Normal/Hard Mode/Hardcore(Nuzlocke)) after beating Steven in Meteor Falls.
* Ability to increase player and NPC in-game speed (World Speed) by 2x, 4x, or 8x (Music speed stays the same.)
  * Holding "R" button will slow back to standard 1x speed.
    * May conflict with changing Bike type. Recommend to use Button Mode: "L = Settings" to move Bike swap.
  * **Note:** 8x Speed may have some minor visual bugs. This is due some some frames being skipped at that speed.
* Added extra "Button Modes" for the "L" button as a shortcut in the overworld when pressing the "L" button:
  * L = Settings: Will change the relevant contextual setting based on what the player is doing:
    * If walking or running, toggle Auto Run.
    * If surfing, toggle Fast Surf.
    * If diving, will step through the different diving speeds on each press.
    * If on bike and unlocked dual swapping bike in post-game, swap bikes.
      * This is as an alternate for players using the above World Speed options to be able to change bike type and use the manual slow down of holding "R". Pressing "R" in this mode does not change the bike type.
  * L = Speed: Will either step through, or toggle on/off the relevant chosen World Speed option
  * L = Fast Mode: Either toggles Auto Run, Fast Surf, World Speed and Diving speed to On or Max (as appropriate), or turns them all to Off or Minimum as appropriate.
  * L = Follower: Toggles Follower On or Off:
    * When turning off, follower will return to Pokeball immediately.
    * When toggling off, player needs to take a step for follower to spawn (if able)
  * All new above Button Modes have LR Button Mode enabled.

</details>

---

## License and Distribution

This project is based on the pokeemerald disassembly, which is a work derived from Pokémon Emerald. All rights to Pokémon and Pokémon Emerald are held by Nintendo, Game Freak, and The Pokémon Company.

This is a non-commercial fan project created for educational and entertainment purposes.
