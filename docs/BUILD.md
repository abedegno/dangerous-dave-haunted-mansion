# Building and checking

The build compiles every source with Borland C++ 2.0 and checks it against your copy of the
game, links and packs a new `HAUNTED.EXE`, and compares it with the shipped file. Nothing
from the game or from Borland is in this repository: you supply both.

## What you need

- **Python 3.10 or later** and **Node.js 22**.
- **A DOS emulator** for Borland's tools: **DOSBox-X** (`brew install dosbox-x`, or your
  system's package), or **emu2**, which `sh exhume/tools/setup-emu2.sh` builds. Exhume uses
  whichever it finds.
- **Borland C++ 2.0** (1991), in `toolchain/BC20` with its `BIN`, `INCLUDE` and `LIB`
  directories. borland-reborn's
  [setup guide](https://github.com/abedegno/borland-reborn/blob/main/docs/SETUP.md) shows
  where to get it and installs it with `scripts/setup-toolchains.py --bc20 <archive>`; copy
  or link its `toolchain/BC20` here.
- **LZEXE 0.91** by Fabrice Bellard (MIT licence), from
  [bellard.org/lzexe](https://bellard.org/lzexe/): the plain `lzexe91.zip`, not 0.91e. Unzip
  it into `toolchain/LZEXE91`.
- **Your copy of the game**: all its files (`HAUNTED.EXE`, `EGATILES.DD2`, `LEVEL01.DD2` ...)
  in `original/game/`.

`toolchain/`, `original/` and `build/` are ignored by git.

## Steps

```sh
git clone --recurse-submodules https://github.com/abedegno/dangerous-dave-haunted-mansion
cd dangerous-dave-haunted-mansion
python3 -m venv .venv && .venv/bin/pip install -r exhume/tools/requirements.txt
# a DOS emulator: DOSBox-X from your package manager, or: sh exhume/tools/setup-emu2.sh
# Borland C++ 2.0 into toolchain/BC20, LZEXE 0.91 into toolchain/LZEXE91,
# the game into original/game, then unpack its EXE (matching works on the unpacked image):
.venv/bin/python exhume/tools/unlzexe.py original/game/HAUNTED.EXE original/HAUNTED.UNP.EXE
make check-all
```

`make check-all` should end with:

```
pass  match + verify             9/9 sources
pass  symbols.tsv from scratch   652 names
pass  exact link                 byte-identical to the original
```

The shipped `HAUNTED.EXE` the build compares with has SHA-256
`07635c6d789206fbf1d84bd733893dbb25ad7343f8d116c9169a24cc57a41ff0`, and the unpacked image
`f15d7af4e101ccb735e1d4ae4658d08812bb5f9ac721b56c7ed34113522f2319`. Another release of the
game would differ.

## The commands

| | |
|---|---|
| `make check` | the full check; recompiles only what changed since it last passed |
| `make check-all` | the same, recompiling everything |
| `make fast` | compile, match and verify the changed sources only: the loop while editing, not a proof |
| `make exe` | link and pack only: `build/PACK/HAUNTED.EXE` |
| `python3 tools/sprites.py` | draw the sprite sets into `build/sprites` |

The C files compile in DOS by default. borland-reborn's native port of Borland C++ 2.0
compiles them in-process, identically and faster: build its shared library
(`make shlib` in borland-reborn's `bcc20-port`) and run with
`EXHUME_NATIVE=bcc20 BCC20_LIB=<path to the library>`.

## Changing the source

Every change that keeps the bytes passes `make check`: names, comments, constants, header
moves. A change meant to alter the game will fail the byte comparison by design; the
checked build is the reference to compare a changed game against. Install the pre-push
hook so nothing is pushed that breaks the reference: `sh exhume/tools/install-hooks.sh .`.
