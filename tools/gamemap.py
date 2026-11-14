"""Map the game's own code (_TEXT 0239-6BA0): function starts and the DGROUP strings each uses.

    python3 tools/gamemap.py > map/game.tsv

Starts: map/functions.tsv's rows, every `push bp; mov bp,sp`, and every word of DGROUP's
initialised data that points at one of those or at a 55 8B EC (think and state tables).
Each function is decoded by recursive descent from its start (ends at ret/jmp with no
pending branch target). String references: 16-bit immediates and displacements that point
at the start of a NUL-terminated printable string in DGROUP's game data (DS:00AA-2828)."""
import struct, sys
import iced_x86 as ix
from pathlib import Path
P = Path(__file__).resolve().parents[1]
exe = (P / 'original/HAUNTED.UNP.EXE').read_bytes()
hdr = struct.unpack_from('<H', exe, 8)[0] * 16
code = exe[hdr:hdr + 0xD970]
ds = exe[hdr + 0x27110 - 0x0000:][:0x10000] if False else exe[hdr + 0x2711 * 16:hdr + 0x2711 * 16 + 0x10000]
LO, HI = 0x0239, 0x6BA0
rows = [l.rstrip('\n').split('\t') for l in open(P / 'map/functions.tsv') if not l.startswith('#')]
starts = {int(r[0], 16) for r in rows if LO <= int(r[0], 16) < HI}
starts |= {i for i in range(LO, HI - 3) if code[i:i + 3] == b'\x55\x8b\xec'}
ptrs = {}
for o in range(0x00AA, 0x2828, 2):
    w = struct.unpack_from('<H', ds, o)[0]
    if w in starts or (LO <= w < HI and code[w:w + 3] == b'\x55\x8b\xec'):
        ptrs.setdefault(w, []).append(o)
starts |= set(ptrs)

def string_at(o):
    if not (0xAA <= o < 0x2828) or (o > 0xAA and ds[o - 1] != 0): return None
    e = ds.find(b'\0', o)
    s = ds[o:e]
    if 2 <= len(s) < 200 and all(32 <= c < 127 or c in (9, 10, 13) for c in s): return s.decode()
    return None

def walk(start):
    seen = set(); todo = [start]; end = start; calls = set(); strs = []; tables = []; lastcmp = None; mem = set()
    while todo:
        a = todo.pop()
        while LO <= a < HI and a not in seen:
            if a != start and a in starts: break
            ins = ix.Decoder(16, code[a:a + 16], ip=a).decode()
            seen.add(a); end = max(end, a + ins.len)
            if ins.memory_base == ix.Register.NONE and ins.memory_segment in (ix.Register.DS, ix.Register.SS) and ins.memory_displ_size:
                mem.add(ins.memory_displacement & 0xFFFF)
            elif ins.memory_displ_size == 2 and ins.memory_segment == ix.Register.DS:
                mem.add(ins.memory_displacement & 0xFFFF)
            for k in range(ins.op_count):
                if ins.op_kind(k) in (ix.OpKind.IMMEDIATE16, ix.OpKind.IMMEDIATE8TO16):
                    s = string_at(ins.immediate(k) & 0xFFFF)
                    if s is not None: strs.append((ins.immediate(k) & 0xFFFF, s))
            fc = ins.flow_control
            if ins.mnemonic == ix.Mnemonic.CMP and ins.op0_register == ix.Register.BX and ins.op1_kind in (ix.OpKind.IMMEDIATE16, ix.OpKind.IMMEDIATE8TO16):
                lastcmp = ins.immediate(1) & 0xFFFF
            if fc == ix.FlowControl.INDIRECT_BRANCH and ins.memory_segment == ix.Register.CS and lastcmp is not None and lastcmp < 64:
                t0 = ins.memory_displacement & 0xFFFF; n = lastcmp + 1
                tables.append((t0, n)); end = max(end, t0 + 2 * n)
                for k in range(n): todo.append(struct.unpack_from('<H', code, t0 + 2 * k)[0])
                seen.update(range(t0, t0 + 2 * n))
                break
            if fc == ix.FlowControl.CALL and ins.op0_kind == ix.OpKind.NEAR_BRANCH16:
                calls.add(ins.near_branch16)
            if fc in (ix.FlowControl.CONDITIONAL_BRANCH,) : todo.append(ins.near_branch16)
            if fc == ix.FlowControl.UNCONDITIONAL_BRANCH and ins.op0_kind == ix.OpKind.NEAR_BRANCH16:
                a = ins.near_branch16; continue
            if fc in (ix.FlowControl.RETURN, ix.FlowControl.UNCONDITIONAL_BRANCH, ix.FlowControl.INTERRUPT) and ins.mnemonic != ix.Mnemonic.INT:
                break
            a += ins.len
    walk.mem = mem
    return end, calls, strs, tables

# frameless functions: a gap after a function's end starts another (skipping pad bytes)
while True:
    ends = {s: walk(s)[0] for s in starts}
    new = set()
    srt = sorted(starts)
    for i, s in enumerate(srt):
        e = ends[s]; n = srt[i + 1] if i + 1 < len(srt) else HI
        while e < n and code[e] in (0x00, 0x90): e += 1
        if e < n: new.add(e)
    if not new: break
    starts |= new

print('# start\tend\tcode bytes\tpointer in DS\tcalls\tstrings (DS offset: text)\tgame data (DS:00AA-2828) read or written\tuninitialised data (DS:4380-A720)')
for s in sorted(starts):
    end, calls, strs, tables = walk(s)
    print(f"{s:04X}\t{end:04X}\t{end - s}\t{' '.join('%04X' % o for o in ptrs.get(s, []))}\t"
          f"{' '.join('%04X' % c for c in sorted(calls))}\t{' | '.join('%04X:%s' % (o, t[:30].replace(chr(10), ' ')) for o, t in strs)}\t"
          f"{' '.join('%04X' % m for m in sorted(walk.mem) if 0xAA <= m < 0x2828)}\t"
          f"{' '.join('%04X' % m for m in sorted(walk.mem) if 0x4380 <= m < 0xA720)}")
