/*
 * Smart Occupancy Monitoring and Access Control System
 * Author: Gilbert Ntivunwa
 *
 * Two push buttons simulate people entering and leaving a facility with a
 * fixed maximum capacity. A 16x2 LCD shows the people inside and the space
 * left, a green LED means space is available and a red LED means full.
 * Events, statistics and errors are reported on the Serial Monitor (9600).
 *
 * Serial commands: 's' = print statistics, 'r' = reset the occupancy record.
 * Everything is non-blocking: timing uses millis(), never delay().
 *
 * The program logic is written in C (typedef structs, pointers, calloc/free,
 * bool). Only the LCD and Serial calls use the Arduino libraries.
 */
// Tinkercad only accepts library headers; Arduino.h already gives bool, uint8_t, snprintf and calloc.
#include <LiquidCrystal.h>

// ----------------------------------------------------------- configuration

#define ENTRY_BUTTON_PIN 7
#define EXIT_BUTTON_PIN 8
#define GREEN_LED_PIN 9
#define RED_LED_PIN 10

#define MAX_CAPACITY 5
#define DEBOUNCE_MS 50UL
#define LCD_REFRESH_MS 250UL
#define STATS_INTERVAL_MS 15000UL

// LCD wiring: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// -------------------------------------------------------------- structures

typedef struct {
  int current;
  int capacity;
  int peak;
  unsigned long totalEntries;
  unsigned long totalExits;
  unsigned long rejectedEntries;
  unsigned long rejectedExits;
} OccupancyRecord;

typedef struct {
  uint8_t pin;
  int stableState;
  int lastReading;
  unsigned long lastChangeMs;
} Button;

typedef enum {
  EVENT_OK,
  EVENT_REJECTED_FULL,
  EVENT_REJECTED_EMPTY
} EventResult;

OccupancyRecord *occupancy = NULL;
Button entryButton = {ENTRY_BUTTON_PIN, HIGH, HIGH, 0};
Button exitButton = {EXIT_BUTTON_PIN, HIGH, HIGH, 0};
unsigned long lastLcdUpdateMs = 0;
unsigned long lastStatsMs = 0;

// ------------------------------------------------- occupancy record logic

/* Allocates a zeroed record on the heap; returns NULL if memory runs out. */
OccupancyRecord *createOccupancyRecord(int capacity) {
  OccupancyRecord *record = (OccupancyRecord *)calloc(1, sizeof(OccupancyRecord));
  if (record != NULL) {
    record->capacity = capacity;
  }
  return record;
}

/* Frees the record and clears the caller's pointer so it cannot dangle. */
void destroyOccupancyRecord(OccupancyRecord **record) {
  free(*record);
  *record = NULL;
}

bool isFull(const OccupancyRecord *record) {
  return record->current >= record->capacity;
}

int remainingCapacity(const OccupancyRecord *record) {
  return record->capacity - record->current;
}

EventResult recordEntry(OccupancyRecord *record) {
  if (isFull(record)) {
    record->rejectedEntries++;
    return EVENT_REJECTED_FULL;
  }
  record->current++;
  record->totalEntries++;
  if (record->current > record->peak) {
    record->peak = record->current;
  }
  return EVENT_OK;
}

EventResult recordExit(OccupancyRecord *record) {
  if (record->current <= 0) {
    record->rejectedExits++;
    return EVENT_REJECTED_EMPTY;
  }
  record->current--;
  record->totalExits++;
  return EVENT_OK;
}

// ------------------------------------------------------- button handling

/* Returns true once per press. A new reading must stay the same for
 * DEBOUNCE_MS before it is accepted. Buttons use INPUT_PULLUP: LOW = pressed. */
bool buttonPressed(Button *button, unsigned long nowMs) {
  int reading = digitalRead(button->pin);

  if (reading != button->lastReading) {
    button->lastReading = reading;
    button->lastChangeMs = nowMs;
  }
  if (nowMs - button->lastChangeMs >= DEBOUNCE_MS && reading != button->stableState) {
    button->stableState = reading;
    return reading == LOW;
  }
  return false;
}

// ---------------------------------------------------------------- outputs

/* Pads text with spaces to 16 characters so old characters are overwritten
 * without calling lcd.clear(), which would make the display flicker. */
const char *padLine(const char *text) {
  static char padded[17];
  snprintf(padded, sizeof padded, "%-16s", text);
  return padded;
}

void updateLeds(const OccupancyRecord *record) {
  bool full = isFull(record);
  digitalWrite(GREEN_LED_PIN, full ? LOW : HIGH);
  digitalWrite(RED_LED_PIN, full ? HIGH : LOW);
}

void updateLcd(const OccupancyRecord *record) {
  char line[17];

  snprintf(line, sizeof line, "Inside: %d/%d", record->current, record->capacity);
  lcd.setCursor(0, 0);
  lcd.print(padLine(line));

  if (isFull(record)) {
    snprintf(line, sizeof line, "FULL - NO ENTRY");
  } else {
    snprintf(line, sizeof line, "Available: %d", remainingCapacity(record));
  }
  lcd.setCursor(0, 1);
  lcd.print(padLine(line));
}

void printStatus(const OccupancyRecord *record) {
  char text[48];
  snprintf(text, sizeof text, " | Inside: %d/%d | Available: %d", record->current,
           record->capacity, remainingCapacity(record));
  Serial.println(text);
}

void printTimestamp(unsigned long nowMs) {
  char text[20];
  snprintf(text, sizeof text, "[%8lu ms] ", nowMs);
  Serial.print(text);
}

void reportEvent(const char *event, const OccupancyRecord *record, unsigned long nowMs) {
  printTimestamp(nowMs);
  Serial.print(event);
  printStatus(record);
}

void reportError(const char *message, const OccupancyRecord *record, unsigned long nowMs) {
  printTimestamp(nowMs);
  Serial.print("ERROR: ");
  Serial.print(message);
  printStatus(record);
}

void reportStatistics(const OccupancyRecord *record, unsigned long nowMs) {
  char text[48];

  Serial.println("------ OCCUPANCY STATISTICS ------");
  snprintf(text, sizeof text, "Uptime            : %lu s", nowMs / 1000);
  Serial.println(text);
  snprintf(text, sizeof text, "People inside     : %d", record->current);
  Serial.println(text);
  snprintf(text, sizeof text, "Maximum capacity  : %d", record->capacity);
  Serial.println(text);
  snprintf(text, sizeof text, "Available capacity: %d", remainingCapacity(record));
  Serial.println(text);
  snprintf(text, sizeof text, "Peak occupancy    : %d", record->peak);
  Serial.println(text);
  snprintf(text, sizeof text, "Total entries     : %lu", record->totalEntries);
  Serial.println(text);
  snprintf(text, sizeof text, "Total exits       : %lu", record->totalExits);
  Serial.println(text);
  snprintf(text, sizeof text, "Rejected entries  : %lu", record->rejectedEntries);
  Serial.println(text);
  snprintf(text, sizeof text, "Rejected exits    : %lu", record->rejectedExits);
  Serial.println(text);
  snprintf(text, sizeof text, "Status            : %s", isFull(record) ? "FULL" : "OPEN");
  Serial.println(text);
  Serial.println("----------------------------------");
}

// ---------------------------------------------------------- event handlers

void handleEntry(OccupancyRecord *record, unsigned long nowMs) {
  if (recordEntry(record) == EVENT_OK) {
    reportEvent("ENTRY", record, nowMs);
    if (isFull(record)) {
      reportEvent("Maximum capacity reached", record, nowMs);
    }
  } else {
    reportError("Entry rejected, facility is at maximum capacity", record, nowMs);
  }
  updateLeds(record);
}

void handleExit(OccupancyRecord *record, unsigned long nowMs) {
  if (recordExit(record) == EVENT_OK) {
    reportEvent("EXIT ", record, nowMs);
  } else {
    reportError("Exit rejected, nobody is inside", record, nowMs);
  }
  updateLeds(record);
}

void reportFatal(void) {
  Serial.println("FATAL: memory allocation failed, system halted.");
  lcd.setCursor(0, 0);
  lcd.print(padLine("MEMORY ERROR"));
  lcd.setCursor(0, 1);
  lcd.print(padLine("System halted"));
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
}

/* Replaces the record with a fresh one: free() the old block, then calloc(). */
void resetOccupancy(unsigned long nowMs) {
  destroyOccupancyRecord(&occupancy);
  occupancy = createOccupancyRecord(MAX_CAPACITY);
  if (occupancy == NULL) {
    reportFatal();
    return;
  }
  reportEvent("RESET", occupancy, nowMs);
  updateLeds(occupancy);
}

void handleSerialCommand(unsigned long nowMs) {
  while (Serial.available() > 0) {
    char command = (char)Serial.read();
    if (command == 's' || command == 'S') {
      reportStatistics(occupancy, nowMs);
    } else if (command == 'r' || command == 'R') {
      resetOccupancy(nowMs);
    }
    if (occupancy == NULL) {
      return;
    }
  }
}

// ------------------------------------------------------------ setup / loop

void setup(void) {
  Serial.begin(9600);
  pinMode(ENTRY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(EXIT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  lcd.begin(16, 2);

  occupancy = createOccupancyRecord(MAX_CAPACITY);
  if (occupancy == NULL) {
    reportFatal();
    return;
  }
  Serial.println("Smart Occupancy Monitoring System started.");
  Serial.println("Commands: 's' = statistics, 'r' = reset.");
  reportEvent("START", occupancy, millis());
  updateLeds(occupancy);
  updateLcd(occupancy);
}

void loop(void) {
  unsigned long nowMs = millis();

  if (occupancy == NULL) {
    return;
  }
  if (buttonPressed(&entryButton, nowMs)) {
    handleEntry(occupancy, nowMs);
  }
  if (buttonPressed(&exitButton, nowMs)) {
    handleExit(occupancy, nowMs);
  }
  handleSerialCommand(nowMs);
  if (occupancy == NULL) {
    return;
  }
  if (nowMs - lastLcdUpdateMs >= LCD_REFRESH_MS) {
    updateLcd(occupancy);
    lastLcdUpdateMs = nowMs;
  }
  if (nowMs - lastStatsMs >= STATS_INTERVAL_MS) {
    reportStatistics(occupancy, nowMs);
    lastStatsMs = nowMs;
  }
}
