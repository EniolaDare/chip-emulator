#ifndef CHIP8_DISPLAY_H
#define CHIP8_DISPLAY_H

#include <stdint.h>
#include <Adafruit_ILI9341.h>
#include <Menu.h>

#define CHIP8_WIDTH  64
#define CHIP8_HEIGHT 32


class Chip8Display {
    public:
        Chip8Display(uint8_t, uint8_t, int8_t);
        void init();
        void clear();
        void drawChip8(uint8_t [CHIP8_HEIGHT][CHIP8_WIDTH]);
        void drawStartScreen(unsigned long duration = 1000);
        void drawMenu(Menu &);

    private:
        uint8_t CS;
        uint8_t DC;
        int8_t RST;

        Adafruit_ILI9341 tft;

        const uint8_t CHIP8_SCALE = 5;
        uint16_t xOffset = 0;
        uint16_t yOffset = 0;

        const uint16_t CHIP8_BG_COLOR = 0x2104;
        const uint16_t CHIP8_FG_COLOR = ILI9341_GREEN;
        const uint16_t BG_COLOR = ILI9341_BLACK;
        const uint16_t TEXT_COLOR = ILI9341_GREEN;

        uint8_t lastDisplay[CHIP8_HEIGHT][CHIP8_WIDTH] = {0};
};

#endif