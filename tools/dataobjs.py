"""The data the game links instead of compiling: take each piece out of the user's own copy
of the game (exhume.toml [binary] exe, never committed) into build/DATA, and make its object
with tools/makeobj.py, as the original build did with MAKEOBJ.

    python3 tools/dataobjs.py          writes build/DATA/<NAME>.DD2 and build/DATA/<NAME>.OBJ

The pieces, in the order the link takes them (map/layout.md, notes/fingerprint.md):

  MAPHEAD   DS:12D0-2729  TED5's map header with the tile info planes (NORTH SOUTH EAST WEST
                          ANIM INTILE); near data, between DD2ASM.ASM's _DATA and DD2INTRO.C's
  FONT      segment 106B  the proportional font (fontseg)
  ENDSCRN   segment 112F  the 80x25 text screen shown on quitting (4000 bytes)
  EGAHEAD   segment 1229  the graphics header
  EGAGRAPH  segment 188E  the four planes of pictures
  SOUNDS    segment 257A  the PC speaker sounds ("SND")

The original files' names are unknown; these are descriptive, with the game's .DD2 extension.
A far segment's length is the distance to the next segment: TLINK pads segments to a
paragraph with zeros, so the original file could have been up to 15 bytes shorter and link
to the same bytes."""
import os, sys
here = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, here)
import makeobj

ROOT = os.path.dirname(here)
DGROUP = 0x2711

# name, mode, symbol, segment (far) or DGROUP offset (near), length
PIECES = [
    ('MAPHEAD',  'c', '_maphead',   0x12D0, 0x272A - 0x12D0),
    ('FONT',     'f', '_font',      0x106B, (0x112F - 0x106B) * 16),
    ('ENDSCRN',  'f', '_endscreen', 0x112F, (0x1229 - 0x112F) * 16),
    ('EGAHEAD',  'f', '_egahead',   0x1229, (0x188E - 0x1229) * 16),
    ('EGAGRAPH', 'f', '_egagraph',  0x188E, (0x257A - 0x188E) * 16),
    ('SOUNDS',   'f', '_sounds',    0x257A, (DGROUP - 0x257A) * 16),
]


def main():
    exe = open(os.path.join(ROOT, 'original', 'HAUNTED.UNP.EXE'), 'rb').read()
    image = int.from_bytes(exe[8:10], 'little') * 16
    out = os.path.join(ROOT, 'build', 'DATA'); os.makedirs(out, exist_ok=True)
    for stem, mode, symbol, where, size in PIECES:
        at = image + (DGROUP * 16 + where if mode == 'c' else where * 16)
        data = exe[at:at + size]
        if len(data) != size: sys.exit(f'{stem}: the EXE ends early')
        with open(os.path.join(out, stem + '.DD2'), 'wb') as f: f.write(data)
        with open(os.path.join(out, stem + '.OBJ'), 'wb') as f:
            f.write(makeobj.make(mode, data, symbol, stem[:1] + stem[1:].lower() + 'Seg', stem + '.DD2'))
        print(f'{stem:9} {mode} {symbol:11} {size:6} bytes')


if __name__ == '__main__':
    main()
