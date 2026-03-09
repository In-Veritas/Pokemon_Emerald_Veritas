# Baby Pokemon Overhaul

Baby Pokemon have been reworked with new abilities, adjusted stats, and compressed learnsets so they can hold their own in battle without needing to evolve.

## New Abilities

### Baby Charm
**Effect:** On switch-in, infatuates all opposing Pokemon. Unlike Cute Charm, this bypasses the gender requirement — any opponent can be infatuated regardless of gender. Blocked by Oblivious and Substitute.

**Pokemon:** Pichu, Cleffa, Igglybuff, Smoochum, Azurill, Teddiursa

### Quick Learner
**Effect:** When hit by any damaging move, boosts Attack, Special Attack, and Evasion by 1 stage each. Triggers on any hit regardless of contact.

**Pokemon:** Magby, Elekid, Tyrogue

### Mystic Tempo
**Effect:** After using Metronome, sharply boosts a random stat by 3 stages. The boosted stat is chosen randomly from: Attack, Defense, Speed, Sp. Attack, Sp. Defense, Accuracy, or Evasion.

**Pokemon:** Togepi

---

## Pokemon Changes

### Pichu
| Stat | Original | New |
|------|----------|-----|
| Sp. Attack | 35 | **100** |
| Ability 1 | Static | **Baby Charm** |

**New Learnset:** Thunder Shock (1), Charm (1), Tail Whip (5), Thunder Wave (8), Sweet Kiss (10), Quick Attack (13), Double Team (16), Slam (20), Thunderbolt (24), Agility (28), Thunder (33), Light Screen (37)

### Magby
| Stat | Original | New |
|------|----------|-----|
| Sp. Attack | 70 | **110** |
| Ability 1 | Flame Body | **Quick Learner** |

**New Learnset:** Ember (1), Leer (5), Smog (9), Fire Punch (13), Smokescreen (17), Will-O-Wisp (20), Sunny Day (23), Flamethrower (27), Confuse Ray (31), Fire Blast (35)

### Azurill
| Stat | Original | New |
|------|----------|-----|
| Attack | 20 | **50** |
| Sp. Attack | 20 | **30** |
| Ability 1 | Thick Fat | **Baby Charm** |

**New Learnset:** Splash (1), Charm (1), Tail Whip (4), Bubble (7), Water Gun (10), Defense Curl (13), Rollout (16), Bubble Beam (19), Slam (22), Rain Dance (25), Take Down (28), Superpower (31), Hydro Pump (34), Belly Drum (38)

### Cleffa
| Stat | Original | New |
|------|----------|-----|
| Defense | 28 | **73** |
| Sp. Defense | 55 | **90** |
| Ability 1 | Cute Charm | **Baby Charm** |

**New Learnset:** Pound (1), Charm (1), Encore (4), Sing (7), Sweet Kiss (10), Double Slap (13), Follow Me (16), Minimize (19), Defense Curl (22), Metronome (25), Cosmic Power (28), Moonlight (31), Light Screen (34), Meteor Mash (38)

### Igglybuff
| Stat | Original | New |
|------|----------|-----|
| Defense | 15 | **20** |
| Sp. Defense | 20 | **25** |
| Ability 1 | Cute Charm | **Baby Charm** |

**New Learnset:** Sing (1), Charm (1), Defense Curl (4), Pound (7), Sweet Kiss (9), Double Slap (12), Disable (14), Rollout (17), Fake Tears (19), Body Slam (22), Rest (25), Encore (28), Soft-Boiled (30), Hyper Voice (33), Mimic (36), Double-Edge (39)

### Togepi
| Stat | Original | New |
|------|----------|-----|
| HP | 35 | **50** |
| Ability 1 | Hustle | **Mystic Tempo** |

**Learnset:** Unchanged (already has Metronome at level 6)

### Tyrogue
| Stat | Original | New |
|------|----------|-----|
| HP | 35 | **40** |
| Attack | 35 | **40** |
| Defense | 35 | **40** |
| Sp. Attack | 35 | **40** |
| Sp. Defense | 35 | **40** |
| Speed | 35 | **40** |
| Ability 1 | Guts | **Quick Learner** |

**New Learnset:** Tackle (1), Focus Energy (1), Double Kick (5), Mach Punch (8), Meditate (11), Fake Out (14), Rolling Kick (17), Pursuit (20), Brick Break (23), Rapid Spin (26), Counter (29), Agility (32), Hi Jump Kick (35), Detect (38)

### Smoochum
| Stat | Original | New |
|------|----------|-----|
| (no stat changes) | | |
| Ability 1 | Oblivious | **Baby Charm** |

**New Learnset:** Pound (1), Lick (1), Sweet Kiss (5), Powder Snow (9), Confusion (13), Sing (17), Mean Look (21), Fake Tears (25), Psychic (29), Perish Song (33), Blizzard (37)

### Elekid
| Stat | Original | New |
|------|----------|-----|
| (no stat changes) | | |
| Ability 1 | Static | **Quick Learner** |

**New Learnset:** Quick Attack (1), Leer (1), Thunder Punch (7), Light Screen (13), Swift (19), Screech (25), Thunderbolt (31), Thunder (37)

### Teddiursa
| Stat | Original | New |
|------|----------|-----|
| Attack | 80 | **95** |
| Speed | 40 | **55** |
| Ability 1 | Pickup | **Baby Charm** |

**New Learnset:** Scratch (1), Leer (1), Lick (5), Fury Swipes (9), Fake Tears (13), Faint Attack (17), Rest (21), Slash (25), Snore (29), Submission (33), Thrash (37)

---

## Debug Testing

Use **Debug Script 7** and **Debug Script 8** from the debug menu to test:

- **Script 7:** Gives party of Pichu, Cleffa, Togepi, Tyrogue, Magby, Azurill (all lv30)
- **Script 8:** Gives party of Igglybuff, Smoochum, Elekid, Teddiursa (all lv30), with option to start a wild battle vs lv50 Magikarp

### Testing Checklist
- [ ] Baby Charm infatuates opponent on switch-in
- [ ] Baby Charm is blocked by Oblivious
- [ ] Baby Charm is blocked by Substitute
- [ ] Quick Learner boosts ATK/SPATK/Evasion when hit
- [ ] Mystic Tempo boosts a random stat by 3 after Metronome
- [ ] All learnsets load correctly at appropriate levels
