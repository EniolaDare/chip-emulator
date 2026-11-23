#include "Chip8Keypad.h"

#include <Arduino.h>


Chip8Keypad::Chip8Keypad(const uint8_t rows[CHIP8_ROWS], const uint8_t cols[CHIP8_COLS]) {
    for (uint8_t i = 0; i < CHIP8_ROWS; i++) {
        rowPins[i] = rows[i];
        colPins[i] = cols[i];
    }
}
void Chip8Keypad::init() {
    lastScan = millis();

    for (uint8_t column = 0; column < CHIP8_COLS; column++) {
        pinMode(colPins[column], INPUT);
    }

    for (uint8_t row = 0; row < CHIP8_ROWS; row++) {
        pinMode(rowPins[row], INPUT_PULLUP);
    }
}

void Chip8Keypad::update(uint8_t keypad[KEYS]) {
    if (millis() - lastScan < SCAN_INTERVAL) return;
    lastScan = millis();

    for (uint8_t column = 0; column < CHIP8_COLS; column++) {
        pinMode(colPins[column], OUTPUT);
        digitalWrite(colPins[column], LOW);
        delayMicroseconds(10);

        for (uint8_t row = 0; row < CHIP8_ROWS; row++) {
            uint8_t keyReading = digitalRead(rowPins[row]) == LOW ? 1 : 0;

            if (keyReading != rawKeys[row][column]) {
                rawKeys[row][column] = keyReading;
                debounceTimer[row][column] = millis();
            }

            if ((rawKeys[row][column] != cleanKeys[row][column]) &&
                (millis() - debounceTimer[row][column] >= DEBOUNCE_TIME)) {
                cleanKeys[row][column] = rawKeys[row][column];
                keypad[KEY_MAP[row][column]] = cleanKeys[row][column];
            }
        }

        pinMode(colPins[column], INPUT);
    }
}