#ifndef CHIP8_KEYPAD_H
#define CHIP8_KEYPAD_H

#include <stdint.h>

#define CHIP8_ROWS 4
#define CHIP8_COLS 4
#define KEYS 16


class Chip8Keypad {
    public:
        Chip8Keypad(const uint8_t [CHIP8_ROWS], const uint8_t [CHIP8_COLS]);
        void init();
        void update(uint8_t [KEYS]);

    private:
        uint8_t rowPins[CHIP8_ROWS];
        uint8_t colPins[CHIP8_COLS];

        const uint8_t KEY_MAP[CHIP8_ROWS][CHIP8_COLS] = {
            0x1, 0x2, 0x3, 0xC,
            0x4, 0x5, 0x6, 0xD,
            0x7, 0x8, 0x9, 0xE,
            0xA, 0x0, 0xB, 0xF
        };

        const uint8_t SCAN_INTERVAL = 5;
        const uint8_t DEBOUNCE_TIME = 15;
        unsigned long lastScan = 0;

        uint8_t rawKeys[CHIP8_ROWS][CHIP8_COLS] = {0};
        uint8_t cleanKeys[CHIP8_ROWS][CHIP8_COLS] = {0};
        unsigned long debounceTimer[CHIP8_ROWS][CHIP8_COLS] = {0};
};

#endif
