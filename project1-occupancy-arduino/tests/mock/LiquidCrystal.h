/*
 * LiquidCrystal.h (mock) - keeps the two LCD lines in memory for checking.
 */
#ifndef MOCK_LIQUID_CRYSTAL_H
#define MOCK_LIQUID_CRYSTAL_H

#include <string.h>

#include "Arduino.h"

class LiquidCrystal {
 public:
  char lines[2][17];
  int column = 0;
  int row = 0;

  LiquidCrystal(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t) { clear(); }
  void begin(int, int) { clear(); }
  void clear() {
    memset(lines, ' ', sizeof lines);
    lines[0][16] = '\0';
    lines[1][16] = '\0';
  }
  void setCursor(int c, int r) {
    column = c;
    row = r;
  }
  void print(const char *text) {
    while (*text != '\0' && column < 16) {
      lines[row][column++] = *text++;
    }
  }
};

#endif
