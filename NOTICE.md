# Notice

This is a decompilation of **Dangerous Dave in the Haunted Mansion** (Softdisk, 1991, made
by id Software). The game is Softdisk's and id Software's. Its executable, data files and
graphics are not included, and no rights in the game are claimed. To build and check the
source you need your own copy (`original/`, which git ignores).

## Licences

- **The recovered source** (`src/`): GNU General Public License version 2 ([LICENSE](LICENSE)).
  Part of it starts from id Software code that Flat Rock Software released under GPL-2.0
  with Hovertank 3-D (github.com/FlatRockSoft/Hovertank3D): the id library files
  (`IDLIBC.C`, `IDASM.ASM`, `IDASME.ASM`, `MEMMGR.C`) and the headers in `src/include` taken
  from it. Those files carry that licence, and so does the rest of the source, which is built
  on them.
- **The build tools** (`tools/`): MIT ([tools/LICENSE](tools/LICENSE)).
- **Exhume** (`exhume/`, a submodule): its own licence (MIT).

## What the build uses but this repository does not include

- Borland C++ 2.0 (compiler, TASM, TLINK, libraries), which you supply in `toolchain/BC20`.
- LZEXE 0.91 by Fabrice Bellard (MIT licence), which you supply in `toolchain/LZEXE91`.
- Your copy of the game in `original/game`: the linked data (map header, graphics, font,
  sounds, end screen) and a 15-byte trailer of the shipped file are taken from it at build
  time and never committed.

The naming follows id Software's style in their GPL source releases of Keen Dreams,
Catacomb 3-D and Hovertank 3-D; no code was taken from Keen Dreams or Catacomb 3-D.
