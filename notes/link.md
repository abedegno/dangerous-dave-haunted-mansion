# The exact link

`python3 tools/link.py` rebuilds HAUNTED.EXE from the sources and compares it with the
unpacked original. Result (2026-11-14): the load image is identical (and packed, the whole shipped file: see
the end), all 215,024 bytes, with
the same entry point, stack and the same 22 relocation addresses. The linked EXE boots into
level 1, and screenshots there are identical to the unpacked original's.

## What it does

1. `tools/dataobjs.py` takes the linked data out of the user's own copy of the game into
   build/DATA and turns each piece into an object with `tools/makeobj.py`, as the original
   build did with id Software's MAKEOBJ (Keen Dreams' static/make.bat shows the method):
   the map header (near data, `_maphead`, DS:12D0) and five far segments: font, end screen,
   graphics header, graphics, sounds. None of it is committed.
2. TLINK 4.0 (Borland C++ 2.0) in emu2:
   `TLINK /c /s /i C0S MAIN DRAW ACTORS DD2ASM MAPHEAD INTRO IDLIBC IDASM IDASME MEMMGR
   FONT ENDSCRN EGAHEAD EGAGRAPH SOUNDS, HAUNTED, HAUNTED, EMU MATHS CS`.
   `/i` writes _BSS and the stack into the file as zeros, as the original has them.
   Segment 0D97 is EMU.LIB's emulator (EMU_PROG) and 100E is E87_PROG. TLINK 3.01 links
   the same image.

## What the link found that verify could not

verify checks each file against the original on its own, so a file whose data overlaps a
neighbour's passed when the shared bytes agreed:

- DD2ACTS.C began with an invented `int unk_0742 = 63;`. Those two bytes are really DD2MAIN.C's
  last string `" ?"` (its `?` and NUL), so ACTORS' data starts at DS:0744.
- IDLIBC.C's _BSS ended at A718, but MEMMGR's starts at A71A: a 2-byte variable nothing
  references sits between them (`unknown_a718a`, provisional, taken as IDLIBC's).
- DrawTileAt is not C at the end of DD2ACTS.C but the first routine of DD2ASM.ASM (TASM
  `PROC ... USES si,di`, then `EVEN`). The `EVEN` gives the 90 byte at 5599, and the call
  to DrawTile inside one TASM module gives the self-relative `call`. As C with inline asm,
  BC wrote a segment-relative (M=1) fixup, and TLINK linked that as an absolute offset.

## Packing: the shipped file itself

`python3 tools/pack.py` packs the linked EXE with LZEXE 0.91 (Fabrice Bellard, MIT licence,
from https://bellard.org/lzexe/; put it at toolchain/LZEXE91/LZEXE.EXE, never committed) in
emu2, and the result equals the shipped HAUNTED.EXE: 84,304 bytes, SHA-256
07635c6d...a41ff0.

- LZEXE 0.91 gives an exact prefix of the shipped file. The English 0.91e differs in its
  decompressor, so the game used 0.91.
- The shipped file has 15 bytes more (e9 40 10 80 10 c0 bc e9 40 11 80 11 7e bf c0), which
  make it a whole number of paragraphs; its header's length counts them. LZEXE writes
  nothing after the relocation table's end marker and the decompressor never reads past
  it, so a later step added them; what it was is unknown. pack.py takes them from the
  user's copy and sets the header's length.
- LZEXE's min-alloc follows the input's, and it agrees only for an input linked with
  TLINK's /i, which confirms the original link used /i.
