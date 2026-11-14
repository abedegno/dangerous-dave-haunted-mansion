"""Disassemble game code, reading Borland's floating-point emulator calls as x87 instructions.

    python3 tools/fpdis.py START END       (hex _TEXT offsets)

INT 34h-3Bh stand for the x87 opcodes D8-DF (the next bytes are the ModR/M and operands),
INT 3Ch for a segment-override prefix before one (ES:), and INT 3Dh for FWAIT. Names come
from symbols.tsv."""
import struct, sys
import iced_x86 as ix
from pathlib import Path
P = Path(__file__).resolve().parents[1]
exe = (P / 'original/HAUNTED.UNP.EXE').read_bytes()
hdr = struct.unpack_from('<H', exe, 8)[0] * 16
code = exe[hdr:hdr + 0xD970]
names = {}
for l in open(P / 'symbols.tsv'):
    if l.startswith('#'): continue
    n, a = l.split('\t')[:2]
    if a.startswith('0000:'): names[int(a[5:], 16)] = n
    elif a.startswith('DS:'): names.setdefault(('ds', int(a[3:], 16)), n)
f = ix.Formatter(ix.FormatterSyntax.MASM)
a, end = int(sys.argv[1], 16), int(sys.argv[2], 16)
while a < end:
    b = code[a:a + 16]; note = ''
    if b[0] == 0xCD and 0x34 <= b[1] <= 0x3B:
        ins = ix.Decoder(16, bytes([0xD8 + b[1] - 0x34]) + b[2:16], ip=a).decode(); n = ins.len + 1; note = '(emu)'
    elif b[0] == 0xCD and b[1] == 0x3C:
        ins = ix.Decoder(16, bytes([0x26, 0xD8 + b[2] - 0x34]) + b[3:16], ip=a).decode(); n = ins.len + 1; note = '(emu, override)'
    elif b[0] == 0xCD and b[1] == 0x3D:
        print(f'  {a:04X} fwait (emu)'); a += 2; continue
    else:
        ins = ix.Decoder(16, b, ip=a).decode(); n = ins.len
    t = f.format(ins)
    if ins.flow_control == ix.FlowControl.CALL and ins.op0_kind == ix.OpKind.NEAR_BRANCH16:
        t += '   ; ' + names.get(ins.near_branch16, '')
    if ins.memory_displ_size and ins.memory_base == ix.Register.NONE:
        nm = names.get(('ds', ins.memory_displacement & 0xFFFF))
        if nm: t += '   ; ' + nm
    print(f'  {a:04X} {t} {note}')
    a += n
