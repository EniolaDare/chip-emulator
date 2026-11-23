#include "Chip8Display.h"

#include <stddef.h>
#include <string.h>

#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <SPI.h>


Chip8Display::Chip8Display(uint8_t cs, uint8_t dc, int8_t rst) : CS(cs), DC(dc), RST(rst), tft(CS, DC, RST) {}
void Chip8Display::init() {
    tft.begin();
    tft.setRotation(1);
    xOffset = (tft.width() - (CHIP8_WIDTH * CHIP8_SCALE)) / 2;
    yOffset = (tft.height() - (CHIP8_HEIGHT * CHIP8_SCALE)) / 2;
    clear();
}
void Chip8Display::clear() {
    tft.fillScreen(BG_COLOR);
    tft.fillRect(xOffset, yOffset, CHIP8_WIDTH * CHIP8_SCALE, CHIP8_HEIGHT * CHIP8_SCALE, CHIP8_BG_COLOR);

    memset(lastDisplay, 0, sizeof(lastDisplay));
}

void Chip8Display::drawChip8(uint8_t chip8Display[CHIP8_HEIGHT][CHIP8_WIDTH]) {
    for (uint8_t y = 0; y < CHIP8_HEIGHT; y++) {
        for (uint8_t x = 0; x < CHIP8_WIDTH; x++) {
            if (chip8Display[y][x] == lastDisplay[y][x]) continue;

            uint16_t color = chip8Display[y][x] ? CHIP8_FG_COLOR : CHIP8_BG_COLOR;
            tft.fillRect(xOffset + x * CHIP8_SCALE, yOffset + y * CHIP8_SCALE, CHIP8_SCALE, CHIP8_SCALE, color);
        }
    }

    memcpy(lastDisplay, chip8Display, sizeof(lastDisplay));
}

void Chip8Display::drawStartScreen(unsigned long duration) {
    tft.fillScreen(BG_COLOR);
    tft.setTextWrap(false);

    const char *title = "CHIP-8";
    int16_t textX, textY;
    uint16_t textWidth, textHeight;

    tft.drawFastHLine(120, 78, 80, CHIP8_FG_COLOR);

    tft.setTextColor(CHIP8_FG_COLOR);
    tft.setTextSize(4);
    tft.getTextBounds(title, 0, 0, &textX, &textY, &textWidth, &textHeight);
    tft.setCursor((tft.width() - textWidth)/2 - textX, 96 - textY);
    tft.print(title);

    tft.drawFastHLine(140, 143, 40, CHIP8_FG_COLOR);

    const char *subtitle = "EMULATOR";
    tft.setTextSize(2);
    tft.getTextBounds(subtitle, 0, 0, &textX, &textY, &textWidth, &textHeight);
    tft.setCursor((tft.width() - textWidth) / 2 - textX, 160 - textY);
    tft.print(subtitle);

    delay(duration);
    clear();
}

void Chip8Display::drawMenu(Menu &menu) {
    tft.fillScreen(BG_COLOR);
    tft.setTextWrap(false);
    tft.setTextSize(2);
    tft.setTextColor(TEXT_COLOR);

    const char *title = menu.title ? menu.title : "";
    int16_t titleX, titleY;
    uint16_t titleWidth, titleHeight;
    tft.getTextBounds(title, 0, 0, &titleX, &titleY, &titleWidth, &titleHeight);
    tft.setCursor((tft.width() - titleWidth)/2 - titleX, 8 - titleY);
    tft.print(title);

    uint16_t listTop = 36;
    uint16_t rowHeight = 24;
    uint16_t bottomMargin = 8;
    size_t visibleCount = (tft.height() - listTop - bottomMargin) / rowHeight;
    size_t itemCount = menu.menuItems.size();
    if (itemCount == 0 || visibleCount == 0) return;

    size_t selectedIndex = menu.currentIndex < itemCount ? menu.currentIndex : 0;
    size_t firstVisibleIndex = selectedIndex > visibleCount / 2
        ? selectedIndex - visibleCount / 2
        : 0;
    size_t lastPossibleFirstIndex = itemCount > visibleCount ? itemCount - visibleCount : 0;
    if (firstVisibleIndex > lastPossibleFirstIndex) {
        firstVisibleIndex = lastPossibleFirstIndex;
    }
    size_t endVisibleIndex = firstVisibleIndex + visibleCount < itemCount
        ? firstVisibleIndex + visibleCount
        : itemCount;

    for (size_t index = firstVisibleIndex; index < endVisibleIndex; ++index) {
        bool selected = index == selectedIndex;
        const char *label = menu.menuItems[index].label ? menu.menuItems[index].label : "";

        uint16_t rowY = listTop + (index - firstVisibleIndex) * rowHeight;
        uint16_t rowColor = selected ? CHIP8_FG_COLOR : BG_COLOR;
        uint16_t textColor = selected ? BG_COLOR : TEXT_COLOR;

        if (selected) {
            tft.fillRect(8, rowY, tft.width() - 16, rowHeight, rowColor);
        }

        int16_t textX, textY;
        uint16_t textWidth, textHeight;
        tft.getTextBounds(label, 0, 0, &textX, &textY, &textWidth, &textHeight);
        uint16_t textTop = rowY + (rowHeight - textHeight) / 2;
        tft.setTextColor(textColor);
        tft.setCursor(12 - textX, textTop - textY);
        tft.print(label);
    }
}