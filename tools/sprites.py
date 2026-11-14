"""Draw the game's sprite sets from your own copy, to see which creature each actor is.

    python3 tools/sprites.py          build/sprites/set0.png ... set9.png

Reads original/game/S_*.DD2 (id's Huffman format: "HUFF", the expanded length, 255 nodes,
the data from byte 1028; IDLIBC.C's HuffExpand) and the sprite table in the graphics header
(build/DATA/EGAHEAD.DD2, which tools/dataobjs.py takes out of the EXE). Each expanded file
starts with its plane size, then five planes (the mask first, then the four colour planes),
each holding the file's sprites one after another (DD2MAIN.C's LoadSpriteChunk). The sets
are DD2MAIN.C's chunkfirst/chunklast ranges; DD2ACTS.C's ScanInfoPlane says which spawner
needs which set (notes/names.md). Transparent pixels are drawn grey."""
import os, struct, sys, zlib
here = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(here)
GAME = os.path.join(ROOT, 'original', 'game')
OUT = os.path.join(ROOT, 'build', 'sprites')

FILES = ['S_DAVE', 'S_CHUNK1', 'S_CHUNK2', 'S_FRANK', 'S_MASTER']
FILE_FIRST = [0, 85, 121, 174, 187]          # DD2MAIN.C: chunkfirst2/chunklast2
FILE_LAST = [84, 120, 173, 186, 198]
SETS = [(0, 84), (85, 102), (103, 120), (121, 129), (130, 147), (148, 155), (156, 161),
        (162, 173), (174, 186), (187, 198)]   # chunkfirst/chunklast
EGA = [(0, 0, 0), (0, 0, 170), (0, 170, 0), (0, 170, 170), (170, 0, 0), (170, 0, 170), (170, 85, 0),
       (170, 170, 170), (85, 85, 85), (85, 85, 255), (85, 255, 85), (85, 255, 255), (255, 85, 85),
       (255, 85, 255), (255, 255, 85), (255, 255, 255)]
SCALE, GREY, GAP = 3, (40, 40, 60), 4


def huff_expand(data):
    if data[:4] != b'HUFF': sys.exit('not a HUFF file')
    length = struct.unpack_from('<I', data, 4)[0]
    nodes = [struct.unpack_from('<HH', data, 8 + 4 * i) for i in range(255)]
    out, node, at = bytearray(), 254, 1028          # the head node is always 254
    while len(out) < length:
        byte = data[at]; at += 1
        for bit in range(8):
            code = nodes[node][(byte >> bit) & 1]
            if code < 256:
                out.append(code); node = 254
                if len(out) == length: break
            else:
                node = code - 256
    return bytes(out)


def write_png(path, w, h, pixels):
    rows = b''.join(b'\0' + bytes(c for p in pixels[y * w:(y + 1) * w] for c in p) for y in range(h))
    chunk = lambda t, d: struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d))
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
                + chunk(b'IDAT', zlib.compress(rows)) + chunk(b'IEND', b''))


def main():
    head = open(os.path.join(ROOT, 'build', 'DATA', 'EGAHEAD.DD2'), 'rb').read()
    table = struct.unpack_from('<H', head, 0xC)[0]           # grheadtype.spritetableofs
    size = lambda shape: struct.unpack_from('<2h', head, table + shape * 32)   # width (bytes), height
    sprites = {}
    for f, first, last in zip(FILES, FILE_FIRST, FILE_LAST):
        buf = huff_expand(open(os.path.join(GAME, f + '.DD2'), 'rb').read())
        plane = struct.unpack_from('<H', buf, 0)[0]
        at = 0
        for s in range(first, last + 1):
            w, h = size(s * 4)
            sprites[s] = (w * 8, h, [buf[2 + plane * k + at:2 + plane * k + at + w * h] for k in range(5)])
            at += w * h
    os.makedirs(OUT, exist_ok=True)
    for n, (first, last) in enumerate(SETS):
        shapes = range(first, last + 1)
        W = sum(sprites[s][0] + GAP for s in shapes); H = max(sprites[s][1] for s in shapes)
        px = [GREY] * (W * SCALE * H * SCALE); x0 = 0
        for s in shapes:
            w, h, planes = sprites[s]
            for y in range(h):
                for x in range(w):
                    i, bit = y * (w // 8) + x // 8, 0x80 >> (x % 8)
                    if planes[0][i] & bit: continue                    # mask: transparent
                    colour = EGA[sum(((planes[1 + j][i] & bit) != 0) << j for j in range(4))]
                    for dy in range(SCALE):
                        row = (y * SCALE + dy) * W * SCALE + (x0 + x) * SCALE
                        px[row:row + SCALE] = [colour] * SCALE
            x0 += w + GAP
        write_png(os.path.join(OUT, f'set{n}.png'), W * SCALE, H * SCALE, px)
        print(os.path.join(OUT, f'set{n}.png'), f'sprites {first}-{last}')


if __name__ == '__main__':
    main()
