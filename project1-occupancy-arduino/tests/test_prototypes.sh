#!/bin/sh
# Tinkercad puts a prototype of every sketch function at the top of the file, before the structs,
# so this test does the same and checks that the result still compiles.
cd "$(dirname "$0")/.." || exit 1
mkdir -p build
out=build/sketch_with_prototypes.cpp
{
  echo '#include "Arduino.h"'
  grep -E '^[A-Za-z_][A-Za-z0-9_ *]*[ *][A-Za-z_][A-Za-z0-9_]*\(.*\) \{$' occupancy.ino | sed 's/ {$/;/'
  echo '#include "../occupancy.ino"'
} > "$out"
count=$(grep -c ');$' "$out")
if ${CXX:-c++} -std=c++17 -fsyntax-only -Itests/mock "$out"; then
  echo "PASS: sketch compiles with $count prototypes placed first, as Tinkercad does"
else
  echo "FAIL: sketch does not compile with its prototypes placed first"
  exit 1
fi
