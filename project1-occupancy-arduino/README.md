# Project 1: Smart Occupancy Monitoring and Access Control System

Arduino Uno sketch (`occupancy.ino`) for Tinkercad. Two push buttons count
people entering and leaving, a 16x2 LCD shows occupancy and free space, and a
green or red LED shows whether the facility is open or full. Maximum
capacity is 5 (`MAX_CAPACITY`).

## Components

| Component | Qty |
| --- | --- |
| Arduino Uno R3 | 1 |
| Pushbutton | 2 |
| LED (green, red) | 2 |
| LCD 16 x 2 | 1 |
| Resistor 220 ohm (LEDs) | 2 |
| Resistor 220 ohm (LCD backlight) | 1 |
| Breadboard (small or full) | 1 |
| Jumper wires | as required |

## Wiring

Power rails: Arduino 5V to the breadboard + rail, GND to the - rail.

LCD 16x2 (left to right pins on the Tinkercad part):

| LCD pin | Connect to |
| --- | --- |
| GND (VSS) | GND rail |
| VCC (VDD) | 5V rail |
| V0 (contrast) | GND rail |
| RS | Arduino pin 12 |
| RW | GND rail |
| E | Arduino pin 11 |
| DB4 | Arduino pin 5 |
| DB5 | Arduino pin 4 |
| DB6 | Arduino pin 3 |
| DB7 | Arduino pin 2 |
| LED anode (A) | 5V rail through a 220 ohm resistor |
| LED cathode (K) | GND rail |

Buttons (internal pull-up, so no external resistor is needed):

| Part | Connect to |
| --- | --- |
| Entry button, one leg | Arduino pin 7 |
| Entry button, opposite leg | GND rail |
| Exit button, one leg | Arduino pin 8 |
| Exit button, opposite leg | GND rail |

LEDs:

| Part | Connect to |
| --- | --- |
| Green LED anode (long, bent leg) | Arduino pin 9 through a 220 ohm resistor |
| Green LED cathode | GND rail |
| Red LED anode | Arduino pin 10 through a 220 ohm resistor |
| Red LED cathode | GND rail |

## Run in Tinkercad

1. Create a new Circuit, add the parts and wire them as above.
2. Code > Text, delete the template and paste the whole `occupancy.ino`.
3. Start Simulation and open Serial Monitor at the bottom of the code panel.
4. Click the entry and exit buttons. Type `s` in the Serial Monitor for
   statistics or `r` to reset.

## Tests (on a computer, no Arduino needed)

```
cd project1-occupancy-arduino
make test      # runs occupancy.ino against a mock Arduino API (46 checks)
```
