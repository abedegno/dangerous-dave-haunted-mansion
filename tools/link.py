"""The exact link: TLINK (Borland C++ 2.0) in headless DOS over C0S, the game's objects, the
linked data and Borland's libraries, then a comparison of the load image with the unpacked
original (exhume.toml [binary] exe).

    python3 tools/link.py            build/LINK/HAUNTED.EXE and HAUNTED.MAP

Needs every source built (exhume/tools/build.py, or the gate) and runs tools/dataobjs.py
first for the linked data. Exit status 0 when the load image (everything after the MZ header)
equals the original's; the header itself differs, because the original's was rebuilt by
LZEXE's unpacker: tools/pack.py packs this EXE and compares with the shipped file.

Link order is the order of the game's code in _TEXT, with the map header's object between
DD2ASM.ASM and DD2INTRO.C because its data sits between theirs in DGROUP, and the far data
objects in the order of their segments. The libraries are what BCC passes for a small-model
program that uses floating point: EMU.LIB (the 8087 emulator, segment 0D97), MATHS.LIB
(segment 100E) and CS.LIB. /i writes _BSS and the stack into the file as zeros, as the
original has them (notes/link.md). TLINK=path chooses another TLINK (3.01 links the same)."""
import os, subprocess, sys
here = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(here)
EXHUME = os.environ.get('EXHUME_HOME', os.path.join(ROOT, 'exhume'))
sys.path.insert(0, os.path.join(EXHUME, 'tools'))
import config

CODE = ['DD2MAIN', 'DD2DRAW', 'DD2ACTS', 'DD2ASM', 'MAPHEAD', 'DD2INTRO', 'IDLIBC', 'IDASM', 'IDASME', 'MEMMGR']
FAR = ['FONT', 'ENDSCRN', 'EGAHEAD', 'EGAGRAPH', 'SOUNDS']
DATA = {'MAPHEAD'} | set(FAR)
LIBS = ['EMU', 'MATHS', 'CS']


def image(data):
    """the load image of an MZ file: from the header's end to the length it gives"""
    pages, last, paras = int.from_bytes(data[4:6], 'little'), int.from_bytes(data[2:4], 'little'), int.from_bytes(data[8:10], 'little')
    return data[paras * 16:(pages - 1) * 512 + last if last else pages * 512]


def main():
    cfg = config.load(os.path.join(ROOT, 'exhume.toml'))
    bc = cfg.home
    obj = lambda stem: os.path.join(cfg.build, 'DATA' if stem in DATA else stem, stem + '.OBJ')
    missing = [obj(s) for s in CODE + FAR if s not in DATA and not os.path.exists(obj(s))]
    if missing: sys.exit('missing (build the sources first): ' + ' '.join(missing))
    subprocess.run([sys.executable, os.path.join(here, 'dataobjs.py')], check=True)
    out = os.path.join(cfg.build, 'LINK'); os.makedirs(out, exist_ok=True)
    for f in ('HAUNTED.EXE', 'HAUNTED.MAP', 'LINK.LOG'):
        if os.path.exists(os.path.join(out, f)): os.remove(os.path.join(out, f))
    rsp = os.path.join(out, 'LINK.RSP')
    with open(rsp, 'w', newline='\r\n') as f:
        f.write('/c /s /i C0S ' + ' '.join(CODE + FAR) + '\nHAUNTED\nHAUNTED\n' + ' '.join(LIBS) + '\n')
    cmd = ['node', os.path.join(EXHUME, 'tools', 'dosrun.mjs'), out, '-t', '120',
           '-f', os.environ.get('TLINK', os.path.join(bc, 'BIN', 'TLINK.EXE')) + '=TLINK.EXE',
           '-f', os.path.join(bc, 'LIB', 'C0S.OBJ'), '-f', rsp]
    for lib in LIBS: cmd += ['-f', os.path.join(bc, 'LIB', lib + '.LIB')]
    for stem in CODE + FAR: cmd += ['-f', obj(stem) + '=' + stem + '.OBJ']
    cmd += ['-c', 'TLINK @LINK.RSP > LINK.LOG', '-O', 'HAUNTED.EXE', '-O', 'HAUNTED.MAP', '-o', 'LINK.LOG']
    r = subprocess.run(cmd, capture_output=True, text=True)
    logp = os.path.join(out, 'LINK.LOG')
    log = open(logp, encoding='latin1').read() if os.path.exists(logp) else ''
    exe = os.path.join(out, 'HAUNTED.EXE')
    if r.returncode or not os.path.exists(exe):
        sys.exit('link failed:\n' + log + (r.stdout + r.stderr).strip())
    ours, theirs = image(open(exe, 'rb').read()), image(cfg.exe)
    if ours == theirs:
        print(f'{exe}: load image identical to the original ({len(ours)} bytes)')
        return 0
    diff = [i for i in range(min(len(ours), len(theirs))) if ours[i] != theirs[i]]
    print(log.strip())
    print(f'{exe}: load image DIFFERENT: {len(ours)} vs {len(theirs)} bytes, {len(diff)} differ'
          + (f', the first at image offset {diff[0]:#x}' if diff else ''))
    return 1


if __name__ == '__main__':
    sys.exit(main())
