#include <Arduino.h>
#include <App.h>
#include <pins.h>

Chip8 chip8;
Chip8Display display(TFT_CS, TFT_DC, TFT_RST);
Chip8Keypad keys(KEYPAD_ROWS, KEYPAD_COLS);
Chip8Storage storage(SD_CS, SPI_SCK, SPI_MISO, SPI_MOSI);
Chip8Accel accel(ACCELEROMETER_SDA, ACCELEROMETER_SCL);

Emulator emulator(chip8, display, keys, storage, accel);
App app(emulator, display, storage);
bool appReady = false;

void setup() {
    Serial.begin(115200);

    if (!app.init()) {
        Serial.println("Emulator initialization failed.");
        return;
    }

    appReady = true;
}

void loop() {
    if (appReady) {
        app.loop();
    }
}
