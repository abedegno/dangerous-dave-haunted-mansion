# Dangerous Dave in the Haunted Mansion: build and check (docs/BUILD.md).
#
#   make check       the full gate: every source compiled, matched and verified, symbols.tsv
#                    rebuilt from scratch, and HAUNTED.EXE linked, packed and compared with
#                    the shipped file (recompiles only what changed since it last passed)
#   make check-all   the same, recompiling every source
#   make fast        compile, match and verify the changed sources only (not a proof)
#   make exe         link and pack: build/PACK/HAUNTED.EXE

PYTHON ?= $(if $(wildcard .venv/bin/python),.venv/bin/python,python3)
GATE = $(PYTHON) exhume/tools/gate.py

check:
	$(GATE) check --config exhume.toml
check-all:
	$(GATE) check --all --config exhume.toml
fast:
	$(GATE) check --fast --config exhume.toml
exe:
	$(PYTHON) tools/rebuild.py

.PHONY: check check-all fast exe
