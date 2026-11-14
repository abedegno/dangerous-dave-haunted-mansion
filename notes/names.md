# Where the names come from

Names compile to nothing here, except the main file's uninitialised globals, whose order
follows a hash of the name (DD2DEF.H explains the `g_<address>_v` symbols and their
readable aliases). So every name below is chosen for meaning, in the style of id
Software's own source releases of the same period: Keen Dreams (the closest, a 1991
side-scroller with the same actor model), Catacomb 3-D and Hovertank 3-D. That means
state records `s_<actor><action><n>`, routines `<Actor>Think`, `<Actor>React` and
`<Actor>Contact`, and spawners `Spawn<Actor>`.

Kinds of evidence:
- **original**: the id library files keep Hovertank 3-D's names where the code is
  Hovertank's (IDLIBC.C, IDASM.ASM, IDASME.ASM, MEMMGR.C).
- **the game's data**: the sound names are in the linked sound file itself
  (src/include/SNDSDD2.H).
- **sprites**: which creature an actor is comes from its sprite set. ScanInfoPlane marks
  the set each spawner needs (`needsprites`), and the sets were drawn from the sprite
  files (S_DAVE, S_CHUNK1, S_CHUNK2, S_FRANK, S_MASTER) to see what they show.
- **descriptive**: what the code does, where nothing names it.

## The actors

| Info plane | Spawner | Sprite set | What it shows | Other evidence |
|---|---|---|---|---|
| 255 | SpawnPlayer | 0 (S_DAVE) | Dave | the help screen's "Dangerous Dave Commands" |
| 1 | SpawnZombie | 1 | a shambling zombie, arms out | |
| 2 | SpawnThrower | 2 | a hunched figure throwing | ThrowerThink plays KNIFETHROWSND; its missile is `s_knife` |
| 3 | SpawnSlime | 4 | a slime on floors and ceilings | SlimeFlipReact plays SLIMESLOPSND |
| 4, 5 | SpawnGhost, SpawnHiddenGhost | 3 | a ghost's face forming | GhostWaitThink plays GHOSTFADESND, GhostAppearThink GHOSTREALSND |
| 6 | SpawnSpider | 5 | a spider on its thread | drops and rises (SpiderDropReact, SpiderRiseReact) |
| 7 | SpawnSkull | 6 | a skull | flies |
| 9 | SpawnWerewolf | 7 | a snouted, furred creature that leaps | "werewolf" is our reading of the sprite |
| 11 | SpawnFrank | 8 (S_FRANK) | Frankenstein's monster | its own sprite file; throws fireballs |
| 12 | SpawnMaster | 9 (S_MASTER) | a horned demon's face, its spit, green hands | MasterThink plays MASTFIRESND |
| 13 | SpawnDelbert | 9 (S_MASTER) | a small figure in a glass sphere that breaks | the level texts name Dave's brother Delbert |
| 14-19 | SpawnBonus | 0 | treasures and an extra Dave | TREASURE1-5SND, EXTRADAVESND |

Dave's states follow his routines and sprites: standing, eight walking frames, jumping and
falling, shooting straight, up and down (UpDownThink takes `ydir < 0` as up), reloading
(FidgetThink plays RELOADSND) and going through doors (OpenDoorThink plays OPENDOORSND).
The three muzzle flashes are the ones ShootThink spawns for each direction, and `s_shothit`
is the spark where a shot hits a monster.

## File names

The original file names are unknown. The game's own files take id's habit of prefixing a
game's files with its name (Hovertank 3-D: HOVERDEF.H, HOVMAIN.C, HOVDRAW.C, HOVACTS.C;
Keen Dreams: KD_DEF.H, KD_MAIN.C, KD_ACT1.C), with the game's DD2 abbreviation (its data
files are *.DD2): DD2DEF.H, DD2MAIN.C, DD2DRAW.C, DD2ACTS.C, DD2INTRO.C and DD2ASM.ASM.
The id library files keep Hovertank's names (IDLIBC.C, IDASM.ASM, IDASME.ASM, MEMMGR.C).
Module names never reach the EXE, so the names are free to choose.
