# Layout of the game's own code (_TEXT 0239-6BA0)

From tools/gamemap.py (map/game.tsv: 143 functions tiling 0239-6BA0 with no gaps but a pad
byte at 5599) and from what each function uses in DGROUP. A C file's string literals come at
the end of its _DATA, after its initialised variables, and files' _DATA follow link order.

| code | _DATA | what | evidence |
|------|-------|------|----------|
| 0239-2572 | 00AA-0743 (strings 0132-0743) | main file: help, memory, god mode, control panel, loaders, main, game loop, game over | strings in function order from 0239 to 219E |
| 2573-2DF4 | (none; _BSS 908C-9097) | DD2DRAW.C (matched): the playfield refresh |
| 2DF5-5575 | 0744-1293 (state tables) | DD2ACTS.C (matched): the objects | think pointers in DS tables; data used from 3113 to 4ACF |
| 5576-6233 | 1294-12CF (30-word row buffer) | DD2ASM.ASM (matched): DrawTileAt, tile and screen copy routines | EVEN pad byte 90 at 5599 after DrawTileAt; no C idioms; mov ax,ss / mov ds,ax returns |
| (data) | 12D0-2729 | MAPHEAD, linked data (tools/dataobjs.py, makeobj c): map header with tile info: 8 words, then name offsets, NORTH SOUTH EAST WEST ANIM INTILE (6 x 858 bytes) | offsets relative to 12D0 end exactly at 272A; 0749 points a variable at 12D0 |
| 6234-6B9F | 272A-2827 (strings only) | DD2INTRO.C (matched): intro and title (STARPIC.DD2, INTRO.DD2, Gamer's Edge text) | strings; _BSS 90AC-920C |

The draw file's limits come from its _BSS (908C-9097, used by it and the main file only) and
from the actors' think routines, which start at 2E43; 2DF5, the actors' movement helper,
is frameless as they are.
