"""Turn a data file into an OMF object, the way id Software's MAKEOBJ (John Romero) did for
the games built from the id library: Keen Dreams' static/make.bat links its map, graphics and
audio headers this way, and this game links its map header, graphics, sounds, font and end
screen the same way (map/layout.md).

    python3 tools/makeobj.py c INPUT OUTPUT.OBJ SYMBOL            near data in DGROUP's _DATA
    python3 tools/makeobj.py f INPUT OUTPUT.OBJ SYMBOL [SEGNAME]  a far segment of its own

Mode c: the data is a word-aligned public _DATA segment (class DATA) in DGROUP, so TLINK
places it in DGROUP's data in link order, between the _DATA of the objects around it.
Mode f: a paragraph-aligned private segment of class FAR_DATA, named SEGNAME (MAKEOBJ's
default: the file name capitalised with "Seg" after it). TLINK places FAR_DATA segments
after the code, in the order it meets them.
The object is ours, written from the OMF format, not MAKEOBJ's bytes: only what it puts in
the linked program matters. Records: THEADR, LNAMES, SEGDEF, (GRPDEF), PUBDEF, LEDATA in
1 KB pieces, MODEND."""
import os, struct, sys


def record(kind, body):
    data = bytes([kind]) + struct.pack('<H', len(body) + 1) + body
    return data + bytes([(-sum(data)) & 0xFF])


def name(s):
    s = s.encode('ascii')
    return bytes([len(s)]) + s


def make(mode, data, symbol, segname=None, module='DATA'):
    out = record(0x80, name(module.upper()[:12]))
    if mode == 'c':
        out += record(0x96, name('DGROUP') + name('_DATA') + name('DATA') + name(''))
        # SEGDEF: word aligned (A=2), public (C=2): 010 010 0 0
        out += record(0x98, bytes([0x48]) + struct.pack('<H', len(data)) + bytes([2, 3, 4]))
        out += record(0x9A, bytes([1, 0xFF, 1]))           # GRPDEF DGROUP: segment 1
        group = 1
    else:
        out += record(0x96, name(segname) + name('FAR_DATA') + name(''))
        # SEGDEF: paragraph aligned (A=3), private (C=0): 011 000 0 0
        if len(data) > 0xFFFF: raise ValueError('a far segment holds at most 64 KB')
        out += record(0x98, bytes([0x60]) + struct.pack('<H', len(data)) + bytes([1, 2, 3]))
        group = 0
    out += record(0x90, bytes([group, 1]) + name(symbol) + struct.pack('<H', 0) + bytes([0]))
    for at in range(0, len(data), 1024):
        out += record(0xA0, bytes([1]) + struct.pack('<H', at) + data[at:at + 1024])
    return out + record(0x8A, bytes([0]))


def main(argv):
    if len(argv) < 4 or argv[0] not in ('c', 'f'): sys.exit(__doc__)
    mode, src, dst, symbol = argv[:4]
    stem = os.path.splitext(os.path.basename(src))[0]
    segname = argv[4] if len(argv) > 4 else stem[:1].upper() + stem[1:].lower() + 'Seg'
    with open(src, 'rb') as f: data = f.read()
    with open(dst, 'wb') as f: f.write(make(mode, data, symbol, segname, os.path.basename(src)))


if __name__ == '__main__':
    main(sys.argv[1:])
