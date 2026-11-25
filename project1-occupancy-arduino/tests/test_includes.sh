#!/bin/sh
# Tinkercad rejects system headers with "invalid header file", so the sketch may include only LiquidCrystal.h.
cd "$(dirname "$0")/.." || exit 1
bad=$(grep -n '^[[:space:]]*#include' occupancy.ino | grep -v '<LiquidCrystal.h>')
if [ -n "$bad" ]; then
  echo "FAIL: occupancy.ino includes headers Tinkercad does not accept:"
  echo "$bad"
  exit 1
fi
echo "PASS: occupancy.ino only includes LiquidCrystal.h"
