# Dangerous Dave in the Haunted Mansion: the source, recovered

This is the C and assembly source of **Dangerous Dave in the Haunted Mansion** (Softdisk,
1991), recovered from the game's own executable. It rebuilds the game **byte for byte**:
compiled with Borland C++ 2.0, linked with TLINK and packed with LZEXE 0.91, the sources
give a `HAUNTED.EXE` identical to the shipped one (SHA-256 `07635c6d...a41ff0`).

The game was made by id Software for Softdisk's Gamer's Edge, and its code is built on the
id library that id later released with Hovertank 3-D. So the recovered source follows id's
own style from their GPL releases of the same period (Keen Dreams, Catacomb 3-D,
Hovertank 3-D): `objtype` actors driven by `statetype` records, `s_zombiewalk1`-style state
names, `ZombieThink` and `SpawnZombie` routines.

The game itself is not here: you build and check against your own copy.

## Status

| | |
|---|---|
| Sources | 9 files: 6 C (Borland C++ 2.0) and 3 hand-written TASM modules, all matching the original byte for byte |
| Link | TLINK 4.0: the load image equals the original's, all 215,024 bytes |
| Packed | LZEXE 0.91: the shipped `HAUNTED.EXE` exactly, all 84,304 bytes |
| Check | `make check`: compiles, matches and verifies every source, rebuilds the symbol table and the EXE, and compares with the shipped file |
| Runs | the rebuilt game boots into level 1, screen for screen the same as the original |

## The source

| File | What it is |
|---|---|
| [`src/DD2MAIN.C`](src/DD2MAIN.C) | main: start-up, loading, help and status windows, the object list and movement code, the play loop, the game loop |
| [`src/DD2ACTS.C`](src/DD2ACTS.C) | the actors: Dave, zombies, knife throwers, slimes, ghosts, spiders, skulls, werewolves, Frankenstein, the Master, Delbert, bonuses |
| [`src/DD2DRAW.C`](src/DD2DRAW.C) | the playfield refresh: two pages and a master screen, scrolling, sprite update lists |
| [`src/DD2INTRO.C`](src/DD2INTRO.C) | the title sequence and its scaled picture |
| [`src/DD2ASM.ASM`](src/DD2ASM.ASM) | tile and screen copying in assembly |
| [`src/IDLIBC.C`](src/IDLIBC.C), [`src/IDASM.ASM`](src/IDASM.ASM), [`src/IDASME.ASM`](src/IDASME.ASM), [`src/MEMMGR.C`](src/MEMMGR.C) | the id library: input, files, Huffman loading, sound, EGA drawing, the memory manager (Hovertank 3-D's, adapted by the game) |
| [`src/include`](src/include) | `DD2DEF.H` (actor types, classes, play states, the main file's globals), `IDLIB.H`, `SNDSDD2.H` (the sounds, named from the game's own sound file) |

The game's data (map header, graphics, font, sounds, end screen) was linked into the EXE
from data files with id's MAKEOBJ tool; the build takes it out of your copy and links it
the same way ([`tools/dataobjs.py`](tools/dataobjs.py), [`tools/makeobj.py`](tools/makeobj.py)).

## Notes

- [`notes/names.md`](notes/names.md): where each name comes from: id's sources, the
  game's sound file, the sprites (drawn by [`tools/sprites.py`](tools/sprites.py)).
- [`notes/link.md`](notes/link.md): the exact link and the packing, and what the link found
  that checking each file alone could not.
- [`notes/fingerprint.md`](notes/fingerprint.md): how the toolchain and each file's compiler
  switches were identified.
- [`map/layout.md`](map/layout.md): where every file's code and data sit in the EXE.

Comments tagged `match:` explain a code shape kept because the original compiled that way
(the one dummy statement, `pic = pic` in `GameLoop`, is explained there).

## Building

See [docs/BUILD.md](docs/BUILD.md). In short: Borland C++ 2.0 and LZEXE 0.91 in
`toolchain/`, your copy of the game in `original/game/`, then `make check-all`.

The tools are [Exhume](https://github.com/abedegno/Exhume) (a git submodule), which drives
Borland's compiler, assembler and linker in a DOS emulator and checks every byte.

## Licence

The recovered source is GPL-2.0, as Hovertank 3-D's code it builds on was released; the build
tools in `tools/` are MIT. The game is Softdisk's and id Software's: no rights in it are
claimed, and none of its files are included. See [NOTICE.md](NOTICE.md).
