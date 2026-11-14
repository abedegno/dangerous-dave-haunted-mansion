"""Link and pack: the whole of HAUNTED.EXE from the built objects, as the gate's link command.

    python3 tools/rebuild.py         build/PACK/HAUNTED.EXE (the shipped file, if all is well)

Runs tools/link.py (the load image must equal the original's) and then tools/pack.py (LZEXE
0.91 and the shipped trailer; the result must equal original/game/HAUNTED.EXE). The gate
compares build/PACK/HAUNTED.EXE with the shipped file itself ([gate] original)."""
import os, subprocess, sys
here = os.path.dirname(os.path.abspath(__file__))

for step in ('link.py', 'pack.py'):
    r = subprocess.run([sys.executable, os.path.join(here, step)])
    if r.returncode: sys.exit(r.returncode)
