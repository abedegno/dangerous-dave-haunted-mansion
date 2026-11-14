# Toolchain fingerprint

HAUNTED.EXE (84,304 bytes, LZEXE 0.91) unpacks to a 215,152-byte MZ image with 22
relocations, entry 0000:0000, stack 3477:0080.

## Borland C++ 2.0, small model

- Runtime banner "Borland C++ - Copyright 1991 Borland Intl" at DS:0004, and the C
  runtime's messages ("Null pointer assignment", "Abnormal program termination").
- The start-up code at the entry point equals Borland C++ 2.0's C0S.OBJ, 569 of 569 bytes
  with fixups masked; C0T/C0M/C0C/C0L/C0H match 18-57%.
- 75 module code blocks of Borland C++ 2.0's CS.LIB appear verbatim (fixups as wildcards):
  exit, fseek, ctime, bioskey, the far heap, the long arithmetic helpers and others.
- Three game functions compile to the original's bytes (fixups masked) with the bcc20
  port (Borland C++ 2.0) at -ms -O -Z:
  - _TEXT:49D2, four structure stores under a test: matches under any of -O/-G/-Z.
  - _TEXT:87F4, a string's width from a font reached through a `_seg` pointer: needs -O
    (without it a jump to the epilogue, EB 00, remains).
  - _TEXT:44CC, a spawn call and four stores through a global object pointer: needs -Z
    (without it the pointer is reloaded before each store), and an unsigned tile
    coordinate (`sub ax,80h`, where a signed one gives `add ax,-80h`).
  -G is undetermined so far.

Not yet excluded: Borland C++ 3.0 (late 1991) compiles simple code the same way and its
start-up and runtime library are close; no 3.0 libraries were at hand. The 1991 Softdisk
date and the sister games (Hovertank, Keen Dreams, Catacomb 3-D, rebuilt byte for byte
with Borland C++ 2.0 from their released source) point to 2.0.

## Layout

| Segment | Size | Contents |
|---|---|---|
| 0000 | 55,664 | _TEXT: C0S, the game's C and assembly, the runtime library |
| 0D97 | 10,096 | tables then far procedures: an assembly module |
| 100E | 1,488 | Borland's floating-point helpers (cs:-relative constants, fprem) |
| 106B | 3,136 | far data |
| 112F | 4,000 | an 80x25 text screen |
| 1229 | 26,192 | far data (graphics headers?) |
| 188E | 52,928 | far data (graphics) |
| 257A | 6,512 | far data starting "SND" |
| 2711 | 55,008 in the file | DGROUP (C0's 4 zero bytes and copyright first) |

## Switches per file (found by matching)

The game code proven above uses -O -Z. The id library files differ, as in UW2 where switches
varied per file; Hovertank 3-D's repository ships the TASM intermediates of these two files
(IDLIBC.ASM, MEMMGR.ASM), which is what -B leaves:

| File | Switches | Evidence |
|---|---|---|
| MEMMGR.C | -ms -G -r- -k- -B | locals on the stack (no register variables), `inc sp; inc sp` argument pops (-G), frameless MMShutdown and a lone `ret` for the empty MMMapEMS (-k-), and `mov dx,OFFSET local` assembled as a load (TASM reads it so; the built-in assembler gives an immediate) |
| DD2ACTS.C | -ms -a -O -G -Z -k- -y | -y (line numbers): the line records stop BC's tail merging (cross-jumping) at statement boundaries, so `ChangeState` calls that end separate lines stay apart as in the game; -a: a switch table at an even offset after a pad byte; frameless functions without locals (-k-). Its one inline-asm routine, DrawTileAt, turned out to be DD2ASM.ASM's (notes/link.md): BC's built-in assembler gives `call near ptr` a segment-relative (M=1) fixup, which TLINK links as an absolute offset, not as the game's self-relative call |
| DD2INTRO.C | -ms -O -G -Z | C with inline asm through the built-in assembler, not TASM: `xor ah,ah` is 30 E4 (TASM writes 32 E4); `add sp,4` pops (-G); floating point through the emulator (INT 34h-3Dh) |
| IDLIBC.C | -ms -O -G -Z -k- -K -B | with exactly these 38 of Hovertank's functions compile unchanged; dropping any one loses matches (-K, plain char unsigned: the game zero-extends where Hovertank's source sign-extends) |

Building with -B needs TASM, so those files build in DOS (EXHUME_DOS=emu2, EXHUME_EMU2 set to
an emu2 binary); the rest compile in-process (EXHUME_NATIVE=bcc20).

## _BSS order

Borland C++ 2.0 places a file's uninitialised variables by the same hash of the name as Turbo
C++ 1.01 (profiles/borland-tc101/bssorder.py: ascending key), but starts the walk partway
round, at key 960 (probe compiles with the bcc20 port: a name of key 959 comes after `a`
(161), one of key 960 before it): the order is ascending (key - 960) mod 1024. So: in IDLIBC.C `ch` (key 1019, the largest) comes first and the rest follow in ascending
key order. With that, the game's own references placed every variable of IDLIBC's _BSS:
fontseg (78) and image (481) are defined there, needgr, grsegs, pictable and bestlevel are
not, and the slots of five variables Hovertank does not have were filled with provisional
names chosen for their keys (unk_9356la and the like, in IDLIBC.C).
