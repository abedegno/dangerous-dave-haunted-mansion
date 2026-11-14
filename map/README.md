# The map

Made by Exhume's `tools/mapsmall.py` from the unpacked executable (there is no IDA listing):

- `functions.tsv`: every function of _TEXT (offset, code bytes, bytes to the next function,
  the library module it belongs to if any, near calls, DGROUP offsets referenced).
- `library.tsv`: Borland C++ 2.0 library code found byte for byte (C0S.OBJ, CS.LIB,
  MATHS.LIB modules).

File boundaries inside _TEXT come from DGROUP: each C file's strings and initialised data
follow the previous file's in link order, the same order as its code. The game's string
literals advance through DGROUP in the order of the functions that use them.
