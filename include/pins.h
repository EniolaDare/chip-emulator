#ifndef PINS_H
#define PINS_H

#include <stdint.h>

// Display pins
#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST -1

// SPI pins
#define SPI_SCK 18
#define SPI_MOSI 23
#define SPI_MISO 19

// MicroSD card 
#define SD_CS 4

// Keypad pins
static constexpr uint8_t KEYPAD_ROWS[4] = {13, 14, 16, 17};
static constexpr uint8_t KEYPAD_COLS[4] = {25, 26, 27, 32};

// Buzzer pin
#define BUZZER 33

// Accelerometer pins
#define ACCELEROMETER_SDA 21
#define ACCELEROMETER_SCL 22

// Rotary encoder and button pins
#define ENCODER_A 34
#define ENCODER_B 35
#define ENCODER_BUTTON 5
#define BACK_BUTTON 12

#endif