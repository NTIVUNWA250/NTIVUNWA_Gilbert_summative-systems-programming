/*
 * Runs occupancy.ino against the mock Arduino API with simulated time.
 */
#include <stdio.h>
#include <string.h>

#include "Arduino.h"
#include "../occupancy.ino"

static int checks = 0;
static int failures = 0;

#define CHECK(condition)                                                \
  do {                                                                  \
    checks++;                                                           \
    if (!(condition)) {                                                 \
      failures++;                                                       \
      printf("  FAIL line %d: %s\n", __LINE__, #condition);             \
    }                                                                   \
  } while (0)

static void runFor(unsigned long ms) {
  for (unsigned long i = 0; i < ms; i++) {
    mock::now()++;
    loop();
  }
}

static void press(uint8_t pin) {
  mock::pins()[pin] = LOW;
  runFor(80);
  mock::pins()[pin] = HIGH;
  runFor(80);
}

static bool greenOn() { return mock::pins()[GREEN_LED_PIN] == HIGH; }
static bool redOn() { return mock::pins()[RED_LED_PIN] == HIGH; }
static bool serialHas(const char *text) { return Serial.output.find(text) != std::string::npos; }
static bool lcdLine(int row, const char *text) { return strncmp(lcd.lines[row], text, strlen(text)) == 0; }

static void testStartup() {
  setup();
  runFor(300);
  CHECK(occupancy != NULL);
  CHECK(occupancy->current == 0 && occupancy->capacity == MAX_CAPACITY);
  CHECK(mock::modes()[ENTRY_BUTTON_PIN] == INPUT_PULLUP);
  CHECK(greenOn() && !redOn());
  CHECK(lcdLine(0, "Inside: 0/5"));
  CHECK(lcdLine(1, "Available: 5"));
  CHECK(serialHas("START"));
}

static void testExitWhenEmptyIsRejected() {
  press(EXIT_BUTTON_PIN);
  CHECK(occupancy->current == 0);
  CHECK(occupancy->rejectedExits == 1);
  CHECK(serialHas("ERROR: Exit rejected, nobody is inside"));
  CHECK(greenOn() && !redOn());
}

static void testEntriesUntilFull() {
  for (int i = 0; i < MAX_CAPACITY - 1; i++) {
    press(ENTRY_BUTTON_PIN);
  }
  runFor(300);
  CHECK(occupancy->current == 4);
  CHECK(greenOn() && !redOn());
  CHECK(lcdLine(0, "Inside: 4/5"));
  CHECK(lcdLine(1, "Available: 1"));

  press(ENTRY_BUTTON_PIN);
  runFor(300);
  CHECK(occupancy->current == 5);
  CHECK(!greenOn() && redOn());
  CHECK(lcdLine(1, "FULL - NO ENTRY"));
  CHECK(serialHas("Maximum capacity reached"));
}

static void testEntryWhenFullIsRejected() {
  press(ENTRY_BUTTON_PIN);
  CHECK(occupancy->current == 5);
  CHECK(occupancy->rejectedEntries == 1);
  CHECK(serialHas("ERROR: Entry rejected, facility is at maximum capacity"));
  CHECK(!greenOn() && redOn());
}

static void testExitFreesSpace() {
  press(EXIT_BUTTON_PIN);
  runFor(300);
  CHECK(occupancy->current == 4);
  CHECK(occupancy->totalExits == 1);
  CHECK(greenOn() && !redOn());
  CHECK(lcdLine(1, "Available: 1"));
}

static void testDebounce() {
  int before = occupancy->current;

  // Contact bounce: the pin flickers every 5 ms before settling LOW.
  for (int i = 0; i < 6; i++) {
    mock::pins()[EXIT_BUTTON_PIN] = (i % 2 == 0) ? LOW : HIGH;
    runFor(5);
  }
  mock::pins()[EXIT_BUTTON_PIN] = LOW;
  runFor(500);
  mock::pins()[EXIT_BUTTON_PIN] = HIGH;
  runFor(80);
  CHECK(occupancy->current == before - 1);

  // A pulse shorter than the debounce time is ignored.
  mock::pins()[EXIT_BUTTON_PIN] = LOW;
  runFor(20);
  mock::pins()[EXIT_BUTTON_PIN] = HIGH;
  runFor(80);
  CHECK(occupancy->current == before - 1);
}

static void testPeriodicStatisticsAndCommands() {
  Serial.output.clear();
  runFor(STATS_INTERVAL_MS);
  CHECK(serialHas("OCCUPANCY STATISTICS"));
  CHECK(serialHas("Peak occupancy    : 5"));
  CHECK(serialHas("Rejected entries  : 1"));

  Serial.output.clear();
  Serial.input = "s";
  runFor(1);
  CHECK(serialHas("Total entries     : 5"));

  Serial.input = "r";
  runFor(300);
  CHECK(occupancy != NULL && occupancy->current == 0 && occupancy->totalEntries == 0);
  CHECK(serialHas("RESET"));
  CHECK(lcdLine(0, "Inside: 0/5"));
}

static void testRecordFunctionsThroughPointers() {
  OccupancyRecord *record = createOccupancyRecord(2);

  CHECK(record != NULL);
  CHECK(recordEntry(record) == EVENT_OK);
  CHECK(recordEntry(record) == EVENT_OK);
  CHECK(recordEntry(record) == EVENT_REJECTED_FULL);
  CHECK(remainingCapacity(record) == 0 && isFull(record));
  CHECK(recordExit(record) == EVENT_OK);
  CHECK(recordExit(record) == EVENT_OK);
  CHECK(recordExit(record) == EVENT_REJECTED_EMPTY);
  CHECK(record->peak == 2 && record->rejectedEntries == 1 && record->rejectedExits == 1);
  destroyOccupancyRecord(&record);
  CHECK(record == NULL);
}

int main() {
  testStartup();
  testExitWhenEmptyIsRejected();
  testEntriesUntilFull();
  testEntryWhenFullIsRejected();
  testExitFreesSpace();
  testDebounce();
  testPeriodicStatisticsAndCommands();
  testRecordFunctionsThroughPointers();
  destroyOccupancyRecord(&occupancy);

  printf("test_occupancy     %d checks, %d failed\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
