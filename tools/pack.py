"""Pack the linked HAUNTED.EXE as the shipped one was: LZEXE 0.91, then a 15-byte trailer.

    python3 tools/pack.py        build/PACK/HAUNTED.EXE, compared with original/game/HAUNTED.EXE

Needs LZEXE 0.91 (Fabrice Bellard, MIT licence; https://bellard.org/lzexe/lzexe91.zip, the
plain 0.91, not 0.91e) unpacked at toolchain/LZEXE91/LZEXE.EXE, or LZEXE=path. It runs in emu2.

What the comparison showed (2026-11-14): LZEXE 0.91 packs the linked EXE into an exact prefix
of the shipped file: header, decompressor, compressed image and relocation table. 0.91e
differs in its decompressor (one more `push ax`), so the game used 0.91. The shipped file
then has 15 more bytes, e9 40 10 80 10 c0 bc e9 40 11 80 11 7e bf c0, which make it a whole
number of paragraphs, and its header's length counts them. LZEXE's source writes nothing
after the relocation table's end marker (00 01 00), and the decompressor stops there, so
some later step added them (a stamp, or padding written from leftover memory: unknown).
They are taken from the user's own copy here, like the other linked data, and the header's
length is set to include them."""
import os, struct, subprocess, sys
here = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(here)
EXHUME = os.environ.get('EXHUME_HOME', os.path.join(ROOT, 'exhume'))
LZEXE = os.environ.get('LZEXE', os.path.join(ROOT, 'toolchain', 'LZEXE91', 'LZEXE.EXE'))
SHIPPED = os.path.join(ROOT, 'original', 'game', 'HAUNTED.EXE')
TRAILER = 15


def main():
    linked = os.path.join(ROOT, 'build', 'LINK', 'HAUNTED.EXE')
    if not os.path.exists(LZEXE): sys.exit(f'{LZEXE}: missing (see the docstring)')
    if not os.path.exists(linked): sys.exit(f'{linked}: missing (run tools/link.py)')
    out = os.path.join(ROOT, 'build', 'PACK'); os.makedirs(out, exist_ok=True)
    for f in ('HAUNTED.EXE', 'HAUNTED.OLD'):
        if os.path.exists(os.path.join(out, f)): os.remove(os.path.join(out, f))
    r = subprocess.run(['node', os.path.join(EXHUME, 'tools', 'dosrun.mjs'), out, '-t', '120',
                        '-f', LZEXE + '=LZEXE.EXE', '-f', linked + '=HAUNTED.EXE',
                        '-c', 'LZEXE HAUNTED.EXE', '-o', 'HAUNTED.EXE'], capture_output=True, text=True)
    path = os.path.join(out, 'HAUNTED.EXE')
    if r.returncode or not os.path.exists(path): sys.exit('LZEXE failed: ' + (r.stdout + r.stderr).strip())
    packed = bytearray(open(path, 'rb').read())
    shipped = open(SHIPPED, 'rb').read()
    packed += shipped[-TRAILER:]
    size = len(packed)
    struct.pack_into('<HH', packed, 2, size % 512, (size + 511) // 512)
    open(path, 'wb').write(packed)
    if bytes(packed) == shipped:
        print(f'{path}: IDENTICAL to the shipped HAUNTED.EXE ({size} bytes)')
        return 0
    bad = [i for i in range(min(len(packed), len(shipped))) if packed[i] != shipped[i]]
    print(f'{path}: DIFFERENT ({size} vs {len(shipped)} bytes, {len(bad)} differing, first at {bad[0]:#x})'
          if bad else f'{path}: DIFFERENT length ({size} vs {len(shipped)})')
    return 1


if __name__ == '__main__':
    sys.exit(main())
