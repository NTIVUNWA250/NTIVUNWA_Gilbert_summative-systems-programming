/*
 * Arduino.h (mock) - just enough of the Arduino API to run occupancy.ino on
 * a computer: simulated clock, pins and Serial Monitor.
 */
#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

// The real Arduino.h brings in these C headers, so the sketch does not include them itself.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

namespace mock {
inline unsigned long &now() {
  static unsigned long value = 0;
  return value;
}
inline int *pins() {
  static int values[20] = {0};
  return values;
}
inline int *modes() {
  static int values[20] = {0};
  return values;
}
}  // namespace mock

inline unsigned long millis() { return mock::now(); }
inline int digitalRead(uint8_t pin) { return mock::pins()[pin]; }
inline void digitalWrite(uint8_t pin, int value) { mock::pins()[pin] = value; }
inline void pinMode(uint8_t pin, int mode) {
  mock::modes()[pin] = mode;
  if (mode == INPUT_PULLUP) {
    mock::pins()[pin] = HIGH;
  }
}

class MockSerial {
 public:
  std::string output;
  std::string input;

  void begin(unsigned long) {}
  void print(const char *text) { output += text; }
  void println(const char *text) { output += text; output += "\n"; }
  int available() { return (int)input.size(); }
  int read() {
    if (input.empty()) return -1;
    char c = input[0];
    input.erase(0, 1);
    return c;
  }
};

inline MockSerial Serial;

#endif
