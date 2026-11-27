#ifndef EMULATOR_H
#define EMULATOR_H

#include <stdint.h>
#include <Arduino.h>
#include <Chip8.h>
#include <Chip8Accel.h>
#include <Chip8Display.h>
#include <Chip8Keypad.h>
#include <Chip8Storage.h>
#include <pins.h>

struct AccelKeyMapping {
    int8_t center = -1;
    int8_t up = 0x8;
    int8_t down = 0x2;
    int8_t left = 0x4;
    int8_t right = 0x6;
    int8_t shake = -1;
    uint16_t shakePressDurationMs = 150;
};

struct EmulatorState{
    bool keys[16];
    Direction direction;
    bool shake;
    bool accelerometerEnabled;
};

class Emulator {
    public:
        Emulator(Chip8 &chip8, Chip8Display &display, Chip8Keypad &keys, Chip8Storage &storage, Chip8Accel &accel);
        bool init();
        bool start(const char *romPath, RomSource source = SD_CARD);
        void loop();
        void stop();
        void buzzerBeep(uint16_t duration = 100);
        void setAccelerometerEnabled(bool enabled);
        bool isAccelerometerEnabled();
        bool setAccelKeyMapping(AccelKeyMapping &mapping);
        bool setTiltSensitivity(Sensitivity sensitivity);
        bool setShakeSensitivity(Sensitivity sensitivity);
        void updateAccelerometer();
        EmulatorState getState();

    private:
        Chip8 &chip8;
        Chip8Display &display;
        Chip8Keypad &keys;
        Chip8Storage &storage;
        Chip8Accel &accel;

        bool accelReady = false;
        bool accelEnabled = false;

        AccelKeyMapping accelKeyMapping;
        uint8_t accelerometerKeys[16] = {};
        uint32_t lastShakeTime = 0;
        bool shakeKeyActive = false;
        uint8_t physicalKeys[16] = {};

        void updateCombinedKeys();
        void updateAccelerometerKeys();
        static bool isValidMappedKey(int8_t key);

        const uint8_t BUZZER_PIN = BUZZER;
        const uint16_t BUZZER_FREQUENCY = 1000;
        bool buzzerState = false;
        void updateBuzzer();

        const uint32_t CPU_FREQUENCY = 600;
        const uint32_t TIMER_FREQUENCY = 60;
        const uint32_t DISPLAY_FREQUENCY = 60;

        const uint32_t CPU_PERIOD_US = 1000000 / CPU_FREQUENCY;
        const uint32_t TIMER_PERIOD_US = 1000000 / TIMER_FREQUENCY;
        const uint32_t DISPLAY_PERIOD_US = 1000000 / DISPLAY_FREQUENCY;

        uint32_t lastCpuTime = 0;
        uint32_t lastTimerTime = 0;
        uint32_t lastDisplayTime = 0;

        const uint8_t MAX_BACKLOG = 8;
};

#endif