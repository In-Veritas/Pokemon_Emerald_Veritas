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

This fork includes all features from Pokémon Emerald Legacy Enhanced (see [Base Features](#base-features-from-emerald-legacy-enhanced) below), plus the following additions and changes:

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

#### Default Options for New Games

* **Pokémon Follower**: Now enabled by default
* **Surfing Pokémon Sprites**: Now enabled by default
* Both options remain fully toggleable in the Options Plus menu

### Technical Improvements

* Changed flag initialization order to prevent follower/surfer defaults from being cleared
* Improved intro scene transitions and sprite management

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

National Dex Mode is an alternate play mode for new games on Emerald Legacy Enhanced allowing to select from all nine starters from the start of the game as well as having all of Base Legacy's post-game route encounters available from the start of the game (i.e Rattata as a possible rare spawn on Route 101). The game **has not** been rebalanced for National Dex Pokémon, as such I do not recommend using this game mode for your first play through of Emerald Legacy Enhanced (unless you have finished Emerald Legacy already).

**Key Notes:**

* Able to select National Dex as an option when starting a new game.
* National Dex wild encounters unlocked
  * National Dex encounters are still rarer encounters than Hoenn counterparts through the game.
* Able to choose from all 9 available Gen 1-3 starters to play
* If you select a non-Hoenn starter:
  * The rival will still have the Hoenn starter of the type super effective against yours (i.e. if you select Charmander, your rival will start with Mudkip)
  * The Hoenn starters will be available in the post-game from your rival by trading the base form of your selected starter.
  * The relevant quest or reward matching the player's starter will be shiny. (i.e. If you chose Charmander, your rival will gift you a shiny Charmander for the relevant quest.)
* No in-game trainers or battles have been updated, the game is still Hoenn leaning.
  * As a result, the game is "unbalanced" in favour of the player.
* Some post-game gift National Dex encounters remain in the post-game (such as Aerodactyl for example)
* Safari Zone is fully available to explore (meaning the Johto encounters are available in the main game)
* One Snorlax is available somewhere in the overworld before post-game. (Respawns after each E4 clear)
* Eevee is available as a gift before post-game from Lanette.
* Porygon available from Game Corner.
* First form pseudo-legendaries all available in the wild.
  * Dratini remains available in Sootopolis city with original Base Legacy post-game location.
  * As noted above, Larvitar moved to Route 111 (Desert) as 1% encounter.
    * No Larvitar encounter in Victory Road **at all**.
* All post-game legendaries from Base Legacy **remain** post-game for National Dex mode, no changes to legendary availability.
* Starter learnsets have been updated

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

**Shiny Charm Rates:**

* 0 Charms - 1/8192 (No Shiny Charms in bag) - 0.01221% - Same as original Emerald
1. 1/1024 (Available from Start of Game) - 0.09766%
2. 1/512  - 0.1953%
3. 3/1024 - 0.2930% (I suspect most people will end up here by end of their main playthrough)
4. 1/256  - 0.3906%
5. 5/1024 - 0.4883%
6. 3/512  - 0.5859%
7. 7/1024 - 0.6836%
8. 1/128  - 0.7812%

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
